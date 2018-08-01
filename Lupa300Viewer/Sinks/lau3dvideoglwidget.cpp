/*********************************************************************************
 *                                                                               *
 * Copyright (c) 2017, Dr. Daniel L. Lau                                         *
 * All rights reserved.                                                          *
 *                                                                               *
 * Redistribution and use in source and binary forms, with or without            *
 * modification, are permitted provided that the following conditions are met:   *
 * 1. Redistributions of source code must retain the above copyright             *
 *    notice, this list of conditions and the following disclaimer.              *
 * 2. Redistributions in binary form must reproduce the above copyright          *
 *    notice, this list of conditions and the following disclaimer in the        *
 *    documentation and/or other materials provided with the distribution.       *
 * 3. All advertising materials mentioning features or use of this software      *
 *    must display the following acknowledgement:                                *
 *    This product includes software developed by the <organization>.            *
 * 4. Neither the name of the <organization> nor the                             *
 *    names of its contributors may be used to endorse or promote products       *
 *    derived from this software without specific prior written permission.      *
 *                                                                               *
 * THIS SOFTWARE IS PROVIDED BY Dr. Daniel L. Lau ''AS IS'' AND ANY              *
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        *
 * DISCLAIMED. IN NO EVENT SHALL Dr. Daniel L. Lau BE LIABLE FOR ANY             *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND   *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                  *
 *                                                                               *
 *********************************************************************************/

#include "lau3dvideoglwidget.h"
#include <locale.h>
#include <math.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DVideoGLWidget::LAU3DVideoGLWidget(unsigned int depthCols, unsigned int depthRows, unsigned int colorCols, unsigned int colorRows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QWidget *parent) : LAUAbstractGLWidget(parent)
{
    playbackColor = color;
    playbackDevice = device;

    numDepthCols = (depthCols == 0) ? colorCols : depthCols;
    numDepthRows = (depthRows == 0) ? colorRows : depthRows;
    numColorCols = (colorCols == 0) ? depthCols : colorCols;
    numColorRows = (colorRows == 0) ? depthRows : colorRows;

    // INITIALIZE THE MAXIMUM INTENSITY VALUE SO THAT IT DOES NOTHING
    maxIntensityValue = 65535;

    // TURN OFF TEXTURE FOR TEXTURELESS VIDEO
    if (playbackColor == ColorXYZ || playbackColor == ColorXYZW) {
        textureEnableFlag = false;
    }

    initializeClass();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DVideoGLWidget::LAU3DVideoGLWidget(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QWidget *parent) : LAUAbstractGLWidget(parent)
{
    playbackColor = color;
    playbackDevice = device;

    numDepthCols = cols;
    numDepthRows = rows;
    numColorCols = cols;
    numColorRows = rows;

    initializeClass();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::initializeClass()
{
    snrThreshold = 0;
    mtnThreshold = 0;
    numDepthInds = 0;
    textureDepth = NULL;
    textureColor = NULL;
    textureAngles = NULL;
    textureMapping = NULL;
    frameBufferObject = NULL;
    smoothBufferObjectA = NULL;
    smoothBufferObjectB = NULL;
    textureEnableFlag = true;

    switch (playbackColor) {
        case ColorGray:
        case ColorRGB:
        case ColorRGBA:
            valid = (numColorRows * numColorCols) > 0;
            if (valid) {
                this->setMinimumWidth(qMin((int)numColorCols, 640));
                this->setMinimumHeight(qMin((int)numColorRows, 480));
            } else {
                this->setMinimumWidth(320);
                this->setMinimumHeight(240);
            }
            break;
        case ColorXYZ:
        case ColorXYZG:
        case ColorXYZRGB:
        case ColorXYZWRGBA:
            valid = (numDepthRows * numDepthCols) > 0;
            if (valid) {
                this->setMinimumWidth(qMin((int)numDepthCols, 640));
                this->setMinimumHeight(qMin((int)numDepthRows, 480));
            } else {
                this->setMinimumWidth(320);
                this->setMinimumHeight(240);
            }
            break;
        default:
            valid = false;
            this->setMinimumWidth(320);
            this->setMinimumHeight(240);
            break;
    }

    // CREATE A CONTEXT MENU FOR TOGGLING TEXTURE
    contextMenu = new QMenu();
    if (playbackColor == ColorGray || playbackColor == ColorRGB) {
        ;
    } else {
        QAction *action = contextMenu->addAction(QString("Show Texture"));
        action->setCheckable(true);
        action->setChecked(textureEnableFlag);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onEnableTexture(bool)));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DVideoGLWidget::~LAU3DVideoGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();

        if (textureDepth) {
            delete textureDepth;
        }
        if (textureColor) {
            delete textureColor;
        }
        if (textureMapping) {
            delete textureMapping;
        }
        if (frameBufferObject) {
            delete frameBufferObject;
        }
        if (smoothBufferObjectA) {
            delete smoothBufferObjectA;
        }
        if (smoothBufferObjectB) {
            delete smoothBufferObjectB;
        }

        // DELETE THE CONTEXT MENU IF IT EXISTS
        if (contextMenu) {
            delete contextMenu;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::setLookUpTable(LAULookUpTable lut)
{
    // MAKE SURE WE HAVE A VALID LOOK UP TABLE
    if (lut.isValid()) {
        lookUpTable = lut;
    } else {
        lookUpTable = LAULookUpTable(numDepthCols, numDepthRows, playbackDevice);
    }

    if (lookUpTable.isValid()) {
        // CHECK TO SEE IF WE CAN COPY THIS LOOK UP TABLE TO THE GPU
        if (this->wasInitialized()) {
            // UPLOAD THE ABCD AND EFGH COEFFICIENTS AS A FLOATING POINT RGBA TEXTURE
            if (textureAngles) {
                // SET THE GRAPHICS CARD CONTEXT TO THIS ONE
                this->makeCurrent();
                textureAngles->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)lookUpTable.constScanLine(0));
            }
        }
        setLimits(lookUpTable.xLimits().x(), lookUpTable.xLimits().y(), lookUpTable.yLimits().x(), lookUpTable.yLimits().y(), lookUpTable.zLimits().x(), lookUpTable.zLimits().y());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::setRangeLimits(float zmn, float zmx, float hFov, float vFov)
{
    // SAVE THE FIELDS OF VIEW
    horizontalFieldOfView = hFov;
    verticalFieldOfView = vFov;

    // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
    zMax = -qMin(fabs(zmn), fabs(zmx));
    zMin = -qMax(fabs(zmn), fabs(zmx));;
    yMax = tan(verticalFieldOfView / 2.0f) * zMin;
    yMin = -yMax;
    xMax = tan(horizontalFieldOfView / 2.0f) * zMin;
    xMin = -xMax;

    // CALL THE UNDERLYING CLASS'S SET LIMITS TO PROPERLY GENERATE THE PROJECTION MATRIX
    LAUAbstractGLWidget::setLimits(xMin, xMax, yMin, yMax, zMin, zMax);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::copyScan(float *buffer)
{
    if (buffer) {
        // SET THE GRAPHICS CARD CONTEXT TO THIS ONE
        makeCurrent();

        // COPY FRAME BUFFER TEXTURE FROM GPU TO LOCAL CPU BUFFER
        switch (playbackColor) {
            case ColorGray:
                glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, (unsigned char *)buffer);
                break;
            case ColorRGB:
            case ColorXYZ:
            case ColorXYZRGB:
                glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, (unsigned char *)buffer);
                break;
            case ColorRGBA:
            case ColorXYZW:
            case ColorXYZG:
            case ColorXYZWRGBA:
                glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (unsigned char *)buffer);
                break;
            default:
                break;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::smoothBuffer()
{
    makeCurrent();

    // BIND THE FRAME BUFFER OBJECT FOR SMOOTHING ALONG THE X-AXIS
    if (smoothBufferObjectA && smoothBufferObjectA->bind()) {
        if (programC.bind()) {
            // CLEAR THE FRAME BUFFER OBJECT
            glViewport(0, 0, smoothBufferObjectA->width(), smoothBufferObjectA->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (vertexBufferA.bind()) {
                if (indiceBufferA.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                    programC.setUniformValue("qt_texture",   0);

                    // SET THE PIXEL STEP VALUES FOR X-AXIS STEPPING
                    programC.setUniformValue("qt_dx", 2);
                    programC.setUniformValue("qt_dy", 0);

                    // Tell OpenGL programmable pipeline how to locate vertex position data
                    glVertexAttribPointer(programC.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                    programC.enableAttributeArray("qt_vertex");
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    indiceBufferA.release();
                }
                vertexBufferA.release();
            }
            programC.release();
        }
        smoothBufferObjectA->release();
    }

    // BIND THE FRAME BUFFER OBJECT FOR SMOOTHING ALONG THE Y-AXIS
    if (smoothBufferObjectB && smoothBufferObjectB->bind()) {
        if (programC.bind()) {
            // CLEAR THE FRAME BUFFER OBJECT
            glViewport(0, 0, smoothBufferObjectB->width(), smoothBufferObjectB->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (vertexBufferA.bind()) {
                if (indiceBufferA.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, smoothBufferObjectA->texture());
                    programC.setUniformValue("qt_texture",   0);

                    // SET THE PIXEL STEP VALUES FOR X-AXIS STEPPING
                    programC.setUniformValue("qt_dx", 0);
                    programC.setUniformValue("qt_dy", 1);

                    // Tell OpenGL programmable pipeline how to locate vertex position data
                    glVertexAttribPointer(programC.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                    programC.enableAttributeArray("qt_vertex");
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    indiceBufferA.release();
                }
                vertexBufferA.release();
            }
            programC.release();
        }
        smoothBufferObjectB->release();
    }

    // BIND THE FRAME BUFFER OBJECT FOR MASKING THE ALPHA CHANNEL
    if (smoothBufferObjectA && smoothBufferObjectA->bind()) {
        if (programD.bind()) {
            // CLEAR THE FRAME BUFFER OBJECT
            glViewport(0, 0, smoothBufferObjectA->width(), smoothBufferObjectA->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (vertexBufferA.bind()) {
                if (indiceBufferA.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, smoothBufferObjectB->texture());
                    programD.setUniformValue("qt_texture", 0);

                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                    programD.setUniformValue("qt_mask", 1);

                    // Tell OpenGL programmable pipeline how to locate vertex position data
                    glVertexAttribPointer(programD.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                    programD.enableAttributeArray("qt_vertex");
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    indiceBufferA.release();
                }
                vertexBufferA.release();
            }
            programD.release();
        }
        smoothBufferObjectA->release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::initializeGL()
{
    // CALL THE UNDERLYING CLASS TO INITIALIZE THE WIDGET
    LAUAbstractGLWidget::initializeGL();

    if (isValid()) {
        switch (playbackColor) {
            case ColorGray:
                shadersGray();
                texturesGray();
                verticesGray();
                break;
            case ColorRGB:
            case ColorRGBA:
                shadersRGB();
                texturesRGB();
                verticesRGB();
                break;
            case ColorXYZ:
            case ColorXYZG:
                shadersXYZG();
                texturesXYZG();
                verticesXYZG();
                break;
            case ColorXYZRGB:
                shadersXYZRGB();
                texturesXYZRGB();
                verticesXYZRGB();
                break;
            case ColorXYZWRGBA:
                shadersXYZWRGBA();
                texturesXYZWRGBA();
                verticesXYZWRGBA();
                break;
            default:
                break;
        }
    }

    // LOAD A LOOK UP TABLE FROM DISK IF WE ARE CONNECTED TO A PROSILICA PRODUCT
    if (playbackColor == ColorXYZG) {
        if (playbackDevice == DeviceProsilicaLCG || playbackDevice == DeviceProsilicaIOS) {
            if (lookUpTable.isNull()) {
                setLimits(-300.0f, 300.0f, -300.0f, 300.0f, -1.0f, -600.0f);
            } else {
                setLimits(lookUpTable.xLimits().x(), lookUpTable.xLimits().y(), lookUpTable.yLimits().x(), lookUpTable.yLimits().y(), lookUpTable.zLimits().x(), lookUpTable.zLimits().y());
            }
        }
    }

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::shadersGray()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/GRAY/processGrayVideo.vert")) {
        close();
    } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GRAY/processGrayVideo.frag")) {
        close();
    } else if (!programA.link()) {
        close();
    }

    // CREATE GLSL PROGRAM FOR DISPLAYING THE INCOMING VIDEO
    if (!programB.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/GRAY/displayGrayVideo.vert")) {
        close();
    } else if (!programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GRAY/displayGrayVideo.frag")) {
        close();
    } else if (!programB.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::texturesGray()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE INCOMING VIDEO
    textureColor = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureColor->setSize(numColorCols, numColorRows);
    textureColor->setFormat(QOpenGLTexture::RGBA32F);
    textureColor->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureColor->setMinificationFilter(QOpenGLTexture::Nearest);
    textureColor->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureColor->allocateStorage();

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE INCOMING VIDEO
    textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureDepth->setSize(numDepthCols, numDepthRows);
    textureDepth->setFormat(QOpenGLTexture::RGBA32F);
    textureDepth->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureDepth->setMinificationFilter(QOpenGLTexture::Nearest);
    textureDepth->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureDepth->allocateStorage();

    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    frameBufferObject = new QOpenGLFramebufferObject(QSize(numColorCols, numColorRows), frameBufferObjectFormat);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::verticesGray()
{
    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBufferA = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBufferA.create();
    vertexBufferA.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBufferA.bind()) {
        vertexBufferA.allocate(16 * sizeof(float));
        float *vertices = (float *)vertexBufferA.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            vertices[0]  = -1.0;
            vertices[1]  = -1.0;
            vertices[2]  = 0.0;
            vertices[3]  = 1.0;
            vertices[4]  = +1.0;
            vertices[5]  = -1.0;
            vertices[6]  = 0.0;
            vertices[7]  = 1.0;
            vertices[8]  = +1.0;
            vertices[9]  = +1.0;
            vertices[10] = 0.0;
            vertices[11] = 1.0;
            vertices[12] = -1.0;
            vertices[13] = +1.0;
            vertices[14] = 0.0;
            vertices[15] = 1.0;

            vertexBufferA.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBufferA from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE INCOMING DEPTH VIDEO DRAWN AS POINTS
    indiceBufferA = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBufferA.create();
    indiceBufferA.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBufferA.bind()) {
        indiceBufferA.allocate(6 * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBufferA.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;
            indiceBufferA.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::shadersRGB()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    if (playbackDevice == DeviceIDS) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/RGB/processIDSRGBVideo.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/RGB/processIDSRGBVideo.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/RGB/processRGBVideo.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/RGB/processRGBVideo.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    }
    // CREATE GLSL PROGRAM FOR DISPLAYING THE INCOMING VIDEO
    if (!programB.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/RGB/displayRGBVideo.vert")) {
        close();
    } else if (!programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/RGB/displayRGBVideo.frag")) {
        close();
    } else if (!programB.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::texturesRGB()
{
    texturesGray();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::verticesRGB()
{
    verticesGray();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::shadersXYZG()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING DEPTH VIDEO PLUS COLOR TEXTURE VIDEO
    setlocale(LC_NUMERIC, "C");
    if (playbackDevice == DeviceKinect) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawKinectVideoToXYZG.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawKinectVideoToXYZG.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DevicePrimeSense) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawPrimeSenseVideoToXYZG.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawPrimeSenseVideoToXYZG.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DeviceRealSense) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawRealSenseVideoToXYZG.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawRealSenseVideoToXYZG.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DeviceProsilicaLCG) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawProsilicaLCGVideoToXYZG.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawProsilicaLCGVideoToXYZG.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DeviceProsilicaIOS) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawProsilicaIOSVideoToXYZG.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawProsilicaIOSVideoToXYZG.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    }

    // CREATE GLSL PROGRAM FOR DISPLAYING THE INCOMING XYZ + GRAY VIDEO
    if (!programB.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/displayXYZGTextureAsPointCloud.vert")) {
        close();
    } else if (!programB.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/XYZG/XYZG/displayXYZGTextureAsPointCloud.geom")) {
        close();
    } else if (!programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/displayXYZGTextureAsPointCloud.frag")) {
        close();
    } else if (!programB.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::texturesXYZG()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH VIDEO
    if (playbackDevice == DevicePrimeSense || playbackDevice == DeviceKinect || playbackDevice == DeviceRealSense) {
        textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureDepth->setSize(numDepthCols / 4, numDepthRows);
        textureDepth->setFormat(QOpenGLTexture::RGBA32F);
    } else if (playbackDevice == DeviceProsilicaLCG) {
        textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureDepth->setSize(3 * numDepthCols, numDepthRows);
        textureDepth->setFormat(QOpenGLTexture::RGBA32F);
    } else if (playbackDevice == DeviceProsilicaIOS) {
        textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureDepth->setSize(2 * numDepthCols, numDepthRows);
        textureDepth->setFormat(QOpenGLTexture::RGBA32F);
    }
    textureDepth->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureDepth->setMinificationFilter(QOpenGLTexture::Nearest);
    textureDepth->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureDepth->allocateStorage();

    // CREATE TEXTURE FOR HOLDING SPHERICAL TO CARTESIAN COORDINATE TRANSFORMATION
    textureAngles = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureAngles->setSize(3 * numDepthCols, numDepthRows);
    textureAngles->setFormat(QOpenGLTexture::RGBA32F);
    textureAngles->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureAngles->setMinificationFilter(QOpenGLTexture::Nearest);
    textureAngles->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureAngles->allocateStorage();

    if (lookUpTable.isValid()) {
        textureAngles->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)lookUpTable.constScanLine(0));
    }

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE COLOR VIDEO
    if (playbackDevice == DevicePrimeSense || playbackDevice == DeviceKinect || playbackDevice == DeviceRealSense) {
        textureColor = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureColor->setSize(numColorCols, numColorRows);
        textureColor->setFormat(QOpenGLTexture::R32F);
        textureColor->setWrapMode(QOpenGLTexture::ClampToBorder);
        textureColor->setMinificationFilter(QOpenGLTexture::Nearest);
        textureColor->setMagnificationFilter(QOpenGLTexture::Nearest);
        textureColor->allocateStorage();
    }

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    if (playbackDevice == DeviceKinect) {
        textureMapping = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureMapping->setSize(numDepthCols, numDepthRows);
        textureMapping->setFormat(QOpenGLTexture::RG32F);
        textureMapping->setWrapMode(QOpenGLTexture::ClampToBorder);
        textureMapping->setMinificationFilter(QOpenGLTexture::Nearest);
        textureMapping->setMagnificationFilter(QOpenGLTexture::Nearest);
        textureMapping->allocateStorage();
    }

    // CREATE THE FRAME BUFFER OBJECT TO HOLD THE SCAN RESULTS AS A TEXTURE
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    frameBufferObject = new QOpenGLFramebufferObject(QSize(numDepthCols, numDepthRows), frameBufferObjectFormat);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::verticesXYZG()
{
    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBufferA = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBufferA.create();
    vertexBufferA.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBufferA.bind()) {
        vertexBufferA.allocate(16 * sizeof(float));
        float *vertices = (float *)vertexBufferA.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            vertices[0]  = -1.0;
            vertices[1]  = -1.0;
            vertices[2]  = 0.0;
            vertices[3]  = 1.0;
            vertices[4]  =  1.0;
            vertices[5]  = -1.0;
            vertices[6]  = 0.0;
            vertices[7]  = 1.0;
            vertices[8]  =  1.0;
            vertices[9]  =  1.0;
            vertices[10] = 0.0;
            vertices[11] = 1.0;
            vertices[12] = -1.0;
            vertices[13] =  1.0;
            vertices[14] = 0.0;
            vertices[15] = 1.0;

            vertexBufferA.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBufferA from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE INCOMING DEPTH VIDEO DRAWN AS POINTS
    indiceBufferA = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBufferA.create();
    indiceBufferA.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBufferA.bind()) {
        indiceBufferA.allocate(6 * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBufferA.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;
            indiceBufferA.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
    }

    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBufferB = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBufferB.create();
    vertexBufferB.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBufferB.bind()) {
        vertexBufferB.allocate(numDepthRows * numDepthCols * 2 * sizeof(float));
        float *vertices = (float *)vertexBufferB.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            for (unsigned int row = 0; row < numDepthRows; row++) {
                for (unsigned int col = 0; col < numDepthCols; col++) {
                    vertices[2 * (col + row * numDepthCols) + 0] = col;
                    vertices[2 * (col + row * numDepthCols) + 1] = row;
                }
            }
            vertexBufferB.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBufferB from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE RESULTING POINT CLOUD DRAWN AS TRIANGLES
    numDepthInds = 0;
    indiceBufferB = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBufferB.create();
    indiceBufferB.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBufferB.bind()) {
        indiceBufferB.allocate((numDepthRows * numDepthCols * 6)*sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBufferB.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            for (unsigned int row = 0; row < numDepthRows - 1; row++) {
                for (unsigned int col = 0; col < numDepthCols - 1; col++) {
                    indices[numDepthInds++] = (row + 0) * numDepthCols + (col + 0);
                    indices[numDepthInds++] = (row + 0) * numDepthCols + (col + 1);
                    indices[numDepthInds++] = (row + 1) * numDepthCols + (col + 1);

                    indices[numDepthInds++] = (row + 0) * numDepthCols + (col + 0);
                    indices[numDepthInds++] = (row + 1) * numDepthCols + (col + 1);
                    indices[numDepthInds++] = (row + 1) * numDepthCols + (col + 0);
                }
            }
            indiceBufferB.unmap();
        } else {
            qDebug() << QString("Unable to map indiceBuffer from GPU.");
        }
    }

}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::shadersXYZRGB()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING DEPTH VIDEO PLUS COLOR TEXTURE VIDEO
    setlocale(LC_NUMERIC, "C");
    if (playbackDevice == DeviceKinect) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawKinectVideoToXYZRGB.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawKinectVideoToXYZRGB.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DevicePrimeSense) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawPrimeSenseVideoToXYZRGB.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawPrimeSenseVideoToXYZRGB.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DeviceRealSense) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawRealSenseVideoToXYZRGB.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawRealSenseVideoToXYZRGB.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DeviceProsilicaLCG) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawProsilicaLCGVideoToXYZRGB.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawProsilicaLCGVideoToXYZRGB.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    } else if (playbackDevice == DeviceProsilicaIOS) {
        if (!programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawProsilicaIOSVideoToXYZRGB.vert")) {
            close();
        } else if (!programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawProsilicaIOSVideoToXYZRGB.frag")) {
            close();
        } else if (!programA.link()) {
            close();
        }
    }

    // CREATE GLSL PROGRAM FOR DISPLAYING THE INCOMING XYZ + RGB VIDEO
    if (!programB.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/displayXYZRGBTextureAsPointCloud.vert")) {
        close();
    } else if (!programB.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/XYZRGB/XYZRGB/displayXYZRGBTextureAsPointCloud.geom")) {
        close();
    } else if (!programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/displayXYZRGBTextureAsPointCloud.frag")) {
        close();
    } else if (!programB.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::texturesXYZRGB()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH VIDEO
    if (playbackDevice == DevicePrimeSense || playbackDevice == DeviceKinect || playbackDevice == DeviceRealSense) {
        textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureDepth->setSize(numDepthCols / 4, numDepthRows);
    } else if (playbackDevice == DeviceProsilicaLCG) {
        textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureDepth->setSize(3 * numDepthCols, numDepthRows);
    } else if (playbackDevice == DeviceProsilicaIOS) {
        textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureDepth->setSize(2 * numDepthCols, numDepthRows);
    }
    textureDepth->setFormat(QOpenGLTexture::RGBA32F);
    textureDepth->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureDepth->setMinificationFilter(QOpenGLTexture::Nearest);
    textureDepth->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureDepth->allocateStorage();

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE COLOR VIDEO
    textureColor = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureColor->setSize(numColorCols, numColorRows);
    textureColor->setFormat(QOpenGLTexture::RGBA32F);
    textureColor->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureColor->setMinificationFilter(QOpenGLTexture::Nearest);
    textureColor->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureColor->allocateStorage();

    // CREATE TEXTURE FOR HOLDING SPHERICAL TO CARTESIAN COORDINATE TRANSFORMATION
    textureAngles = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureAngles->setSize(3 * numDepthCols, numDepthRows);
    textureAngles->setFormat(QOpenGLTexture::RGBA32F);
    textureAngles->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureAngles->setMinificationFilter(QOpenGLTexture::Nearest);
    textureAngles->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureAngles->allocateStorage();

    // COPY OVER THE LOOKUP TABLE IF WE HAVE A VALID TABLE
    if (lookUpTable.isValid()) {
        textureAngles->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)lookUpTable.constScanLine(0));
    }

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    if (playbackDevice == DeviceKinect) {
        textureMapping = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textureMapping->setSize(numDepthCols, numDepthRows);
        textureMapping->setFormat(QOpenGLTexture::RG32F);
        textureMapping->setWrapMode(QOpenGLTexture::ClampToBorder);
        textureMapping->setMinificationFilter(QOpenGLTexture::Nearest);
        textureMapping->setMagnificationFilter(QOpenGLTexture::Nearest);
        textureMapping->allocateStorage();
    }

    // CREATE THE FRAME BUFFER OBJECT TO HOLD THE SCAN RESULTS AS A TEXTURE
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    frameBufferObject  = new QOpenGLFramebufferObject(QSize(2 * numDepthCols, numDepthRows), frameBufferObjectFormat);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::verticesXYZRGB()
{
    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBufferA = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBufferA.create();
    vertexBufferA.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBufferA.bind()) {
        vertexBufferA.allocate(16 * sizeof(float));
        float *vertices = (float *)vertexBufferA.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            vertices[0]  = -1.0;
            vertices[1]  = -1.0;
            vertices[2]  = 0.0;
            vertices[3]  = 1.0;
            vertices[4]  =  1.0;
            vertices[5]  = -1.0;
            vertices[6]  = 0.0;
            vertices[7]  = 1.0;
            vertices[8]  =  1.0;
            vertices[9]  =  1.0;
            vertices[10] = 0.0;
            vertices[11] = 1.0;
            vertices[12] = -1.0;
            vertices[13] =  1.0;
            vertices[14] = 0.0;
            vertices[15] = 1.0;

            vertexBufferA.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBufferA from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE INCOMING DEPTH VIDEO DRAWN AS POINTS
    indiceBufferA = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBufferA.create();
    indiceBufferA.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBufferA.bind()) {
        indiceBufferA.allocate(6 * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBufferA.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;
            indiceBufferA.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
    }

    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBufferB = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBufferB.create();
    vertexBufferB.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBufferB.bind()) {
        vertexBufferB.allocate(numDepthRows * numDepthCols * 2 * sizeof(int));
        float *vertices = (float *)vertexBufferB.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            for (unsigned int row = 0; row < numDepthRows; row++) {
                for (unsigned int col = 0; col < numDepthCols; col++) {
                    vertices[2 * (col + row * numDepthCols) + 0] = (float)(2 * col);
                    vertices[2 * (col + row * numDepthCols) + 1] = (float)(row);
                }
            }
            vertexBufferB.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBufferB from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE RESULTING POINT CLOUD DRAWN AS TRIANGLES
    numDepthInds = 0;
    indiceBufferB = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBufferB.create();
    indiceBufferB.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBufferB.bind()) {
        indiceBufferB.allocate((numDepthRows * numDepthCols * 6)*sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBufferB.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            for (unsigned int row = 0; row < numDepthRows - 1; row++) {
                for (unsigned int col = 0; col < numDepthCols - 1; col++) {
                    indices[numDepthInds++] = (row + 0) * numDepthCols + (col + 0);
                    indices[numDepthInds++] = (row + 0) * numDepthCols + (col + 1);
                    indices[numDepthInds++] = (row + 1) * numDepthCols + (col + 1);

                    indices[numDepthInds++] = (row + 0) * numDepthCols + (col + 0);
                    indices[numDepthInds++] = (row + 1) * numDepthCols + (col + 1);
                    indices[numDepthInds++] = (row + 1) * numDepthCols + (col + 0);
                }
            }
            indiceBufferB.unmap();
        } else {
            qDebug() << QString("Unable to map indiceBuffer from GPU.");
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::shadersXYZWRGBA()
{
    shadersXYZRGB();

    // CREATE GLSL PROGRAM FOR SMOOTHING THE W AND A CHANNELS OF THE XYZWRGBA SCAN
    setlocale(LC_NUMERIC, "C");
    if (!programC.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/gaussianFilterXYZRGB.vert")) {
        close();
    } else if (!programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/gaussianFilterXYZRGB.frag")) {
        close();
    } else if (!programC.link()) {
        close();
    }

    if (!programD.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/maskFilterXYZRGB.vert")) {
        close();
    } else if (!programD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/maskFilterXYZRGB.frag")) {
        close();
    } else if (!programD.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::texturesXYZWRGBA()
{
    texturesXYZRGB();

    // CREATE THE FRAME BUFFER OBJECT TO HOLD THE SCAN RESULTS AS A TEXTURE
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    smoothBufferObjectA = new QOpenGLFramebufferObject(QSize(2 * numDepthCols, numDepthRows), frameBufferObjectFormat);
    smoothBufferObjectB = new QOpenGLFramebufferObject(QSize(2 * numDepthCols, numDepthRows), frameBufferObjectFormat);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::verticesXYZWRGBA()
{
    verticesXYZRGB();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    makeCurrent();

    // UPDATE THE COLOR TEXTURE
    if (textureColor) {
        if (color.isValid()) {
            if (color.colors() == 1) {
                if (color.depth() == sizeof(unsigned char)) {
                    textureColor->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (const void *)color.constPointer());
                } else if (color.depth() == sizeof(unsigned short)) {
                    textureColor->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, (const void *)color.constPointer());
                } else if (color.depth() == sizeof(float)) {
                    textureColor->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void *)color.constPointer());
                }
            }
            if (color.colors() == 3) {
                if (color.depth() == sizeof(unsigned char)) {
                    textureColor->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)color.constPointer());
                } else if (color.depth() == sizeof(unsigned short)) {
                    textureColor->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, (const void *)color.constPointer());
                } else if (color.depth() == sizeof(float)) {
                    textureColor->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void *)color.constPointer());
                }
            } else if (color.colors() == 4) {
                if (color.depth() == sizeof(unsigned char)) {
                    textureColor->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void *)color.constPointer());
                } else if (color.depth() == sizeof(unsigned short)) {
                    textureColor->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, (const void *)color.constPointer());
                } else if (color.depth() == sizeof(float)) {
                    textureColor->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)color.constPointer());
                }
            }
        } else if (depth.isValid()) {
            // WE WANT A COLOR BUT ITS NOT AVAILABLE, SO LET'S HIJACK THE DEPTH BUFFER
            if (depth.depth() == sizeof(unsigned char)) {
                textureColor->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (const void *)depth.constPointer());
            } else if (depth.depth() == sizeof(unsigned short)) {
                textureColor->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, (const void *)depth.constPointer());
            } else if (depth.depth() == sizeof(float)) {
                textureColor->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void *)depth.constPointer());
            }
        }
    }

    // UPDATE THE DEPTH AND MAPPING TEXTURES
    if (textureDepth) {
        if (depth.isValid()) {
            // BIND TEXTURE FOR CONVERTING FROM SPHERICAL TO CARTESIAN
            if (playbackDevice == DevicePrimeSense || playbackDevice == DeviceKinect || playbackDevice == DeviceRealSense) {
                if (depth.depth() == sizeof(unsigned char)) {
                    textureDepth->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void *)depth.constPointer());
                } else if (depth.depth() == sizeof(unsigned short)) {
                    textureDepth->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, (const void *)depth.constPointer());
                } else if (depth.depth() == sizeof(float)) {
                    textureDepth->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)depth.constPointer());
                }
            } else if (playbackDevice == DeviceProsilicaLCG || playbackDevice == DeviceProsilicaIOS) {
                if (depth.depth() == sizeof(unsigned char)) {
                    textureDepth->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Int8, (const void *)depth.constPointer());
                } else if (depth.depth() == sizeof(unsigned short)) {
                    textureDepth->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Int16, (const void *)depth.constPointer());
                } else if (depth.depth() == sizeof(float)) {
                    textureDepth->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)depth.constPointer());
                }
            }
        }
    }

    // UPDATE THE MAPPING BUFFER
    if (textureMapping) {
        if (mapping.isValid()) {
            textureMapping->setData(QOpenGLTexture::RG, QOpenGLTexture::Float32, (const void *)mapping.constPointer());
        }
    }

    // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
    // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
    if (frameBufferObject && frameBufferObject->bind()) {
        if (programA.bind()) {
            // CLEAR THE FRAME BUFFER OBJECT
            glViewport(0, 0, frameBufferObject->width(), frameBufferObject->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (vertexBufferA.bind()) {
                if (indiceBufferA.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    if (textureColor) {
                        glActiveTexture(GL_TEXTURE0);
                        textureColor->bind();
                        programA.setUniformValue("qt_colorTexture", 0);
                    }

                    // BIND THE DEPTH TEXTURE
                    if (textureDepth) {
                        glActiveTexture(GL_TEXTURE1);
                        textureDepth->bind();
                        programA.setUniformValue("qt_depthTexture", 1);
                    }

                    // BIND TEXTURE FOR CONVERTING FROM SPHERICAL TO CARTESIAN
                    if (textureAngles) {
                        glActiveTexture(GL_TEXTURE3);
                        textureAngles->bind();
                        programA.setUniformValue("qt_spherTexture", 3);
                    }

                    if (playbackDevice == DeviceKinect) {
                        // BIND THE MAPPING TEXTURE
                        if ((playbackColor == ColorXYZRGB) || (playbackColor == ColorXYZWRGBA)) {
                            glActiveTexture(GL_TEXTURE2);
                            textureMapping->bind();
                            programA.setUniformValue("qt_mappingTexture", 2);
                        }
                    } else if (playbackDevice == DeviceProsilicaLCG || playbackDevice == DeviceProsilicaIOS) {
                        // SET THE SNR THRESHOLD FOR DELETING BAD POINTS
                        programA.setUniformValue("qt_snrThreshold", (float)snrThreshold / 1000.0f);
                        programA.setUniformValue("qt_mtnThreshold", (float)qPow((float)mtnThreshold / 1000.0f, 4.0f));
                    }

                    // SET THE RANGE LIMITS FOR THE Z AXIS
                    if (lookUpTable.isValid()) {
                        programA.setUniformValue("qt_depthLimits", lookUpTable.zLimits());
                    } else {
                        programA.setUniformValue("qt_depthLimits", QPointF(-1e6, 1e6));
                    }

                    // SET THE MAXIMUM INTENSITY VALUE FOR THE INCOMING VIDEO
                    if (playbackColor == ColorGray) {
                        programA.setUniformValue("qt_maximum", (float)maxIntensityValue / 65535.0f);
                    }

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    glVertexAttribPointer(programA.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                    programA.enableAttributeArray("qt_vertex");
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    indiceBufferA.release();
                }
                vertexBufferA.release();
            }
            programA.release();
        }
        frameBufferObject->release();
    }
    // REDRAW THE WIDGET ON SCREEN
    update();

    // EMIT THE BUFFERS TO THE NEXT OBJECT
    emit emitBuffer(depth, color, mapping);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoGLWidget::paintGL()
{
    if (isValid()) {
        // ENABLE THE DEPTH FILTER
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // BIND THE GLSL PROGRAMS RESPONSIBLE FOR CONVERTING OUR FRAME BUFFER
        // OBJECT TO AN XYZ+TEXTURE POINT CLOUD FOR DISPLAY ON SCREEN
        if (programB.bind()) {
            glViewport(0, 0, localWidth, localHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if ((playbackColor == ColorGray) || (playbackColor == ColorRGB) || (playbackColor == ColorRGBA)) {
                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (vertexBufferA.bind()) {
                    if (indiceBufferA.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE4);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                        programB.setUniformValue("qt_texture", 4);

                        glVertexAttribPointer(programB.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                        programB.enableAttributeArray("qt_vertex");
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        indiceBufferA.release();
                    }
                    vertexBufferA.release();
                }
                programB.release();
            } else if ((playbackColor == ColorXYZ) || (playbackColor == ColorXYZG) || (playbackColor == ColorXYZRGB) || (playbackColor == ColorXYZWRGBA)) {
                // SET THE PROJECTION MATRIX IN THE SHADER PROGRAM
                programB.setUniformValue("qt_projection", projection);

                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (vertexBufferB.bind()) {
                    if (indiceBufferB.bind()) {
                        // Tell OpenGL programmable pipeline how to locate vertex position data
                        glVertexAttribPointer(programB.attributeLocation("qt_vertex"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
                        programB.enableAttributeArray("qt_vertex");

                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE4);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
                        programB.setUniformValue("qt_texture", 4);

                        // SET THE DELTA VALUE FOR TRIANGLE CULLING IN THE GEOMETRY SHADER
                        if (playbackDevice == DeviceProsilicaLCG) {
                            programB.setUniformValue("qt_delta", (float)1.0f);
                        } else if (playbackDevice == DeviceProsilicaIOS) {
                            programB.setUniformValue("qt_delta", (float)1.0f);
                        } else {
                            programB.setUniformValue("qt_delta", (float)100.0f);
                        }
                        programB.setUniformValue("qt_color", clrTransform);
                        programB.setUniformValue("qt_mode", (int)(!textureEnableFlag));

                        // NOW DRAW ON SCREEN USING OUR POINT CLOUD AS TRIANGLES
                        glDrawElements(GL_TRIANGLES, numDepthInds, GL_UNSIGNED_INT, 0);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        indiceBufferB.release();
                    }
                    vertexBufferB.release();
                }
                programB.release();
            }
        }
    } else {
        LAUAbstractGLWidget::paintGL();
    }
    return;
}
