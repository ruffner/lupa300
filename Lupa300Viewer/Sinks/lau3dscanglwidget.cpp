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

#include "lau3dscanglwidget.h"
#include "lauscan.h"
#include <locale.h>
#include <math.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DScanGLWidget::LAU3DScanGLWidget(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, unsigned char *buffer, QWidget *parent) : LAUAbstractGLWidget(parent)
{
    numInds = 0;
    numCols = cols;
    numRows = rows;
    scanBuffer = (float *)buffer;
    playbackColor = color;
    texturePlayback = NULL;
    textureEnableFlag = true;

    valid = (numRows * numCols) > 0;
    if (valid) {
        this->setMinimumWidth(qMin((int)numCols, 640));
        this->setMinimumHeight(qMin((int)numRows, 480));
    } else {
        this->setMinimumWidth(320);
        this->setMinimumHeight(240);
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
LAU3DScanGLWidget::~LAU3DScanGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();
        if (texturePlayback) {
            delete texturePlayback;
        }
        if (contextMenu) {
            delete contextMenu;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::setRangeLimits(float zmn, float zmx, float hFov, float vFov)
{
    // SAVE THE FIELDS OF VIEW
    horizontalFieldOfView = hFov;
    verticalFieldOfView = vFov;

    // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
    zMax = -qMin(fabs(zmn), fabs(zmx));
    zMin = -qMax(fabs(zmn), fabs(zmx));;
    yMax = tan(horizontalFieldOfView / 2.0f) * zMin;
    yMin = -yMax;
    xMax = tan(verticalFieldOfView / 2.0f) * zMin;
    xMin = -xMax;

    // CALL THE UNDERLYING CLASS'S SET LIMITS TO PROPERLY GENERATE THE PROJECTION MATRIX
    LAUAbstractGLWidget::setLimits(xMin, xMax, yMin, yMax, zMin, zMax);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::onUpdatePlaybackBuffer(LAUMemoryObject buffer)
{
    // UPDATE THE PLAYBACK BUFFER
    if (buffer.isValid()) {
        if (wasInitialized()) {
            this->makeCurrent();
            switch (playbackColor) {
                case ColorGray:
                    texturePlayback->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void*)buffer.constPointer());
                    break;
                case ColorRGB:
                case ColorXYZRGB:
                    texturePlayback->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void*)buffer.constPointer());
                    break;
                case ColorRGBA:
                case ColorXYZG:
                case ColorXYZW:
                case ColorXYZWRGBA:
                    texturePlayback->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void*)buffer.constPointer());
                    break;
                default:
                    break;
            }
            update();
        } else {
            scanBuffer = (float *)buffer.constPointer();
        }
    }
    emit emitPlaybackBuffer(buffer);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::onUpdatePlaybackBuffer(float *buffer)
{
    // UPDATE THE PLAYBACK BUFFER
    if (buffer && wasInitialized()) {
        this->makeCurrent();
        switch (playbackColor) {
            case ColorGray:
                texturePlayback->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void*)buffer);
                break;
            case ColorRGB:
            case ColorXYZRGB:
                texturePlayback->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void*)buffer);
                break;
            case ColorRGBA:
            case ColorXYZG:
            case ColorXYZW:
            case ColorXYZWRGBA:
                texturePlayback->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void*)buffer);
                break;
            default:
                break;
        }
        update();
    }
    emit emitPlaybackBuffer(buffer);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::onUpdatePlaybackBuffer(LAUScan scan)
{
    // UPDATE THE PLAYBACK BUFFER
    if (scan.isValid() && wasInitialized()) {
        // SAVE THE SCAN'S TRANSFORM
        symmetry = scan.transform();

        this->makeCurrent();
        switch (playbackColor) {
            case ColorGray:
                texturePlayback->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void*)scan.constPointer());
                break;
            case ColorRGB:
            case ColorXYZRGB:
                texturePlayback->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void*)scan.constPointer());
                break;
            case ColorRGBA:
            case ColorXYZG:
            case ColorXYZW:
            case ColorXYZWRGBA:
                texturePlayback->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void*)scan.constPointer());
                break;
            default:
                break;
        }
        update();
    }
    emit emitPlaybackBuffer(scan);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAU3DScanGLWidget::grabMouseBuffer(GrabMouseBufferMode mode)
{
    // GET A LOCAL COPY OF THE TEXTURE FOR MOUSE CLICKS
    QOpenGLTexture *mouseTexture = grabMouseTexture();

    if (mouseTexture && isValid()) {
        // MAKE THE CONTEXT CURRENT
        makeCurrent();

        // CREATE A NEW FRAME BUFFER OBJECT TO HOLD THE BUFFER
        // MAKING SURE TO INCLUDE A DEPTH BUFFER FOR OCCLUDING SURFACES
        QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
        frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
        frameBufferObjectFormat.setAttachment(QOpenGLFramebufferObject::Depth);
        QOpenGLFramebufferObject *frameBufferObject = new QOpenGLFramebufferObject(QSize(localWidth, localHeight), frameBufferObjectFormat);

        // ENABLE THE DEPTH FILTER
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // BIND THE GLSL PROGRAMS RESPONSIBLE FOR CONVERTING OUR FRAME BUFFER
        // OBJECT TO AN XYZ+TEXTURE POINT CLOUD FOR DISPLAY ON SCREEN
        if (frameBufferObject->bind()) {
            // SET BACKGROUND COLOR TO NANS
            glClearColor(NAN, NAN, NAN, 0.0f);
            glViewport(0, 0, localWidth, localHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (program.bind()) {
                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (vertexBuffer.bind()) {
                    if (indiceBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE0);
                        mouseTexture->bind();
                        program.setUniformValue("qt_texture", 0);

                        if ((playbackColor == ColorGray) || (playbackColor == ColorRGB) || (playbackColor == ColorRGBA)) {
                            glVertexAttribPointer(program.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                            program.enableAttributeArray("qt_vertex");
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                        } else if ((playbackColor == ColorXYZ) || (playbackColor == ColorXYZG) || (playbackColor == ColorXYZRGB) || (playbackColor == ColorXYZWRGBA)) {
                            // SET THE PROJECTION MATRIX IN THE SHADER PROGRAM
                            program.setUniformValue("qt_projection", projection);

                            // Tell OpenGL programmable pipeline how to locate vertex position data
                            glVertexAttribPointer(program.attributeLocation("qt_vertex"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
                            program.enableAttributeArray("qt_vertex");

                            // SET THE DELTA VALUE FOR TRIANGLE CULLING IN THE GEOMETRY SHADER
                            program.setUniformValue("qt_delta", (float)100.0f);

                            switch (mode) {
                                case MouseModeRGB:
                                    program.setUniformValue("qt_color", QMatrix4x4());
                                    program.setUniformValue("qt_mode", (int)0);
                                    break;
                                case MouseModeXYZ:
                                    program.setUniformValue("qt_color", QMatrix4x4());
                                    program.setUniformValue("qt_mode", (int)2);
                                    break;
                            }

                            // NOW DRAW ON SCREEN USING OUR POINT CLOUD AS TRIANGLES
                            glDrawElements(GL_TRIANGLES, numInds, GL_UNSIGNED_INT, 0);
                        }
                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        indiceBuffer.release();
                    }
                    vertexBuffer.release();
                }
                program.release();
            }
            frameBufferObject->release();
        }
        // FORCE ALL DRAWING COMMANDS TO EXECUTE
        glFlush();

        // NOW DOWNLOAD THE BUFFER
        LAUMemoryObject object(localWidth, localHeight, 4, sizeof(float));
        glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, object.constPointer());

        // SET BACKGROUND COLOR TO DEFAULT COLOR FOR ON SCREEN UPDATES
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

        // DELETE THE FRAME BUFFER OBJECT SINCE WE ARE DONE WITH IT
        delete frameBufferObject;

        // RETURN THE BUFFER OBJECT TO THE USER
        return (object);
    }
    return (LAUMemoryObject());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::initializeGL()
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
            case ColorXYZW:
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

    // CHECK TO SEE IF WE HAVE A BUFFER TO DISPLAY
    if (scanBuffer) {
        onUpdatePlaybackBuffer(scanBuffer);
    }

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::shadersGray()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/GRAY/displayGrayVideo.vert")) {
        close();
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GRAY/displayGrayVideo.frag")) {
        close();
    }
    if (!program.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::texturesGray()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    texturePlayback = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texturePlayback->setSize(numCols, numRows);
    texturePlayback->setFormat(QOpenGLTexture::RGBA32F);
    texturePlayback->setWrapMode(QOpenGLTexture::ClampToBorder);
    texturePlayback->setMinificationFilter(QOpenGLTexture::Nearest);
    texturePlayback->setMagnificationFilter(QOpenGLTexture::Nearest);
    texturePlayback->allocateStorage();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::verticesGray()
{
    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBuffer.bind()) {
        vertexBuffer.allocate(16 * sizeof(float));
        float *vertices = (float *)vertexBuffer.map(QOpenGLBuffer::WriteOnly);
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

            vertexBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBuffer from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE INCOMING DEPTH VIDEO DRAWN AS POINTS
    indiceBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBuffer.create();
    indiceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBuffer.bind()) {
        indiceBuffer.allocate(6 * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;
            indiceBuffer.unmap();
        } else {
            qDebug() << QString("indiceBuffer buffer mapped from GPU.");
        }
    }

    // SET NUMBER OF INDICES FOR RAW VIDEO
    numInds = 6;

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::shadersRGB()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/RGB/displayRGBVideo.vert")) {
        close();
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/RGB/displayRGBVideo.frag")) {
        close();
    }
    if (!program.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::texturesRGB()
{
    texturesGray();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::verticesRGB()
{
    verticesGray();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::shadersXYZG()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING DEPTH VIDEO PLUS COLOR TEXTURE VIDEO
    setlocale(LC_NUMERIC, "C");
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/displayXYZGTextureAsPointCloud.vert")) {
        close();
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/XYZG/XYZG/displayXYZGTextureAsPointCloud.geom")) {
        close();
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/displayXYZGTextureAsPointCloud.frag")) {
        close();
    }
    if (!program.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::texturesXYZG()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    texturePlayback = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texturePlayback->setSize(numCols, numRows);
    texturePlayback->setFormat(QOpenGLTexture::RGBA32F);
    texturePlayback->setWrapMode(QOpenGLTexture::ClampToBorder);
    texturePlayback->setMinificationFilter(QOpenGLTexture::Nearest);
    texturePlayback->setMagnificationFilter(QOpenGLTexture::Nearest);
    texturePlayback->allocateStorage();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::verticesXYZG()
{
    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBuffer.bind()) {
        vertexBuffer.allocate(numRows * numCols * 2 * sizeof(float));
        float *vertices = (float *)vertexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            for (unsigned int row = 0; row < numRows; row++) {
                for (unsigned int col = 0; col < numCols; col++) {
                    vertices[2 * (col + row * numCols) + 0] = col;
                    vertices[2 * (col + row * numCols) + 1] = row;
                }
            }
            vertexBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBuffer from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE RESULTING POINT CLOUD DRAWN AS TRIANGLES
    numInds = 0;
    indiceBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBuffer.create();
    indiceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBuffer.bind()) {
        indiceBuffer.allocate((numRows * numCols * 6)*sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            for (unsigned int row = 0; row < numRows - 1; row++) {
                for (unsigned int col = 0; col < numCols - 1; col++) {
                    indices[numInds++] = (row + 0) * numCols + (col + 0);
                    indices[numInds++] = (row + 0) * numCols + (col + 1);
                    indices[numInds++] = (row + 1) * numCols + (col + 1);

                    indices[numInds++] = (row + 0) * numCols + (col + 0);
                    indices[numInds++] = (row + 1) * numCols + (col + 1);
                    indices[numInds++] = (row + 1) * numCols + (col + 0);
                }
            }
            indiceBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map indiceBuffer from GPU.");
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::shadersXYZRGB()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING DEPTH VIDEO PLUS COLOR TEXTURE VIDEO
    setlocale(LC_NUMERIC, "C");
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/displayXYZRGBTextureAsPointCloud.vert")) {
        close();
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/XYZRGB/XYZRGB/displayXYZRGBTextureAsPointCloud.geom")) {
        close();
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/displayXYZRGBTextureAsPointCloud.frag")) {
        close();
    }
    if (!program.link()) {
        close();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::texturesXYZRGB()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    texturePlayback = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texturePlayback->setSize(2 * numCols, numRows);
    texturePlayback->setFormat(QOpenGLTexture::RGBA32F);
    texturePlayback->setWrapMode(QOpenGLTexture::ClampToBorder);
    texturePlayback->setMinificationFilter(QOpenGLTexture::Nearest);
    texturePlayback->setMagnificationFilter(QOpenGLTexture::Nearest);
    texturePlayback->allocateStorage();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::verticesXYZRGB()
{
    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (vertexBuffer.bind()) {
        vertexBuffer.allocate(numRows * numCols * 2 * sizeof(int));
        float *vertices = (float *)vertexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            for (unsigned int row = 0; row < numRows; row++) {
                for (unsigned int col = 0; col < numCols; col++) {
                    vertices[2 * (col + row * numCols) + 0] = (float)(2 * col);
                    vertices[2 * (col + row * numCols) + 1] = (float)(row);
                }
            }
            vertexBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBuffer from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE RESULTING POINT CLOUD DRAWN AS TRIANGLES
    numInds = 0;
    indiceBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indiceBuffer.create();
    indiceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (indiceBuffer.bind()) {
        indiceBuffer.allocate((numRows * numCols * 6)*sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)indiceBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            for (unsigned int row = 0; row < numRows - 1; row++) {
                for (unsigned int col = 0; col < numCols - 1; col++) {
                    indices[numInds++] = (row + 0) * numCols + (col + 0);
                    indices[numInds++] = (row + 0) * numCols + (col + 1);
                    indices[numInds++] = (row + 1) * numCols + (col + 1);

                    indices[numInds++] = (row + 0) * numCols + (col + 0);
                    indices[numInds++] = (row + 1) * numCols + (col + 1);
                    indices[numInds++] = (row + 1) * numCols + (col + 0);
                }
            }
            indiceBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map indiceBuffer from GPU.");
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::shadersXYZWRGBA()
{
    shadersXYZRGB();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::texturesXYZWRGBA()
{
    texturesXYZRGB();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::verticesXYZWRGBA()
{
    verticesXYZRGB();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DScanGLWidget::paintGL()
{
    if (isValid()) {
        // ENABLE THE DEPTH FILTER
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // BIND THE GLSL PROGRAMS RESPONSIBLE FOR CONVERTING OUR FRAME BUFFER
        // OBJECT TO AN XYZ+TEXTURE POINT CLOUD FOR DISPLAY ON SCREEN
        if (program.bind()) {
            glViewport(0, 0, localWidth, localHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (vertexBuffer.bind()) {
                if (indiceBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    texturePlayback->bind();
                    program.setUniformValue("qt_texture", 0);

                    if ((playbackColor == ColorGray) || (playbackColor == ColorRGB) || (playbackColor == ColorRGBA)) {
                        glVertexAttribPointer(program.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                        program.enableAttributeArray("qt_vertex");
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    } else if ((playbackColor == ColorXYZ) || (playbackColor == ColorXYZW) || (playbackColor == ColorXYZG) || (playbackColor == ColorXYZRGB) || (playbackColor == ColorXYZWRGBA)) {
                        // SET THE PROJECTION MATRIX IN THE SHADER PROGRAM
                        program.setUniformValue("qt_projection", projection);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        glVertexAttribPointer(program.attributeLocation("qt_vertex"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
                        program.enableAttributeArray("qt_vertex");

                        // SET THE DELTA VALUE FOR TRIANGLE CULLING IN THE GEOMETRY SHADER
                        program.setUniformValue("qt_color", clrTransform);
                        program.setUniformValue("qt_delta", (float)100.0f);
                        program.setUniformValue("qt_mode", (int)(!textureEnableFlag));

                        // NOW DRAW ON SCREEN USING OUR POINT CLOUD AS TRIANGLES
                        glDrawElements(GL_TRIANGLES, numInds, GL_UNSIGNED_INT, 0);
                    }
                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    indiceBuffer.release();
                }
                vertexBuffer.release();
            }
            program.release();
        }
    } else {
        LAUAbstractGLWidget::paintGL();
    }
    return;
}
