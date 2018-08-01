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

#include "lauabstractfilter.h"
#include <locale.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractFilterController::LAUAbstractFilterController(LAUAbstractGLFilter *contxt, QSurface *srfc, QObject *parent) : QObject(parent), context(contxt), filter(NULL), localSurface(false), surface(srfc), thread(NULL)
{
    // SEE IF THE USER GAVE US A TARGET SURFACE, IF NOT, THEN CREATE AN OFFSCREEN SURFACE BY DEFAULT
    if (surface == NULL) {
        surface = new QOffscreenSurface();
        ((QOffscreenSurface *)surface)->create();
        localSurface = true;
    }

    // NOW SEE IF WE HAVE A VALID PROCESSING CONTEXT FROM THE USER, AND THEN SPIN IT INTO ITS OWN THREAD
    if (context) {
        context->setFormat(surface->format());
        context->setSurface(surface);
        context->create();
        context->initialize();
        if (context->isValid()) {
            thread = new QThread();
            context->moveToThread(thread);
            thread->start();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractFilterController::LAUAbstractFilterController(LAUAbstractFilter *fltr, QObject *parent) : QObject(parent), context(NULL), filter(fltr), localSurface(false), surface(NULL), thread(NULL)
{
    if (filter) {
        thread = new QThread();
        filter->moveToThread(thread);
        thread->start();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractFilterController::~LAUAbstractFilterController()
{
    if (thread) {
        thread->quit();
        while (thread->isRunning()) {
            qApp->processEvents();
        }
        delete thread;
    }
    if (context) {
        context->moveToThread(qApp->thread());
        delete context;
        if (localSurface) {
            delete surface;
        }
    }
    if (filter) {
        delete filter;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::initialize()
{
    if (makeCurrent(surface)) {
        initializeOpenGLFunctions();
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

        // get context opengl-version
        qDebug() << "Really used OpenGl: " << format().majorVersion() << "." << format().minorVersion();
        qDebug() << "OpenGl information: VENDOR:       " << (const char *)glGetString(GL_VENDOR);
        qDebug() << "                    RENDERDER:    " << (const char *)glGetString(GL_RENDERER);
        qDebug() << "                    VERSION:      " << (const char *)glGetString(GL_VERSION);
        qDebug() << "                    GLSL VERSION: " << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

        // CREATE THE VERTEX ARRAY OBJECT FOR FEEDING VERTICES TO OUR SHADER PROGRAMS
        vertexArrayObject.create();
        vertexArrayObject.bind();

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

                vertexBuffer.unmap();
            } else {
                qDebug() << QString("Unable to map vertexBuffer from GPU.");
            }
            vertexBuffer.release();
        }

        // CREATE AN INDEX BUFFER FOR THE INCOMING DEPTH VIDEO DRAWN AS POINTS
        indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        indexBuffer.create();
        indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (indexBuffer.bind()) {
            indexBuffer.allocate(6 * sizeof(unsigned int));
            unsigned int *indices = (unsigned int *)indexBuffer.map(QOpenGLBuffer::WriteOnly);
            if (indices) {
                indices[0] = 0;
                indices[1] = 1;
                indices[2] = 2;
                indices[3] = 0;
                indices[4] = 2;
                indices[5] = 3;
                indexBuffer.unmap();
            } else {
                qDebug() << QString("indexBuffer buffer mapped from GPU.");
            }
            indexBuffer.release();
        }

        switch (playbackColor) {
            case ColorGray:
                shadersGray();
                texturesGray();
                break;
            case ColorRGB:
            case ColorRGBA:
                shadersRGB();
                texturesRGB();
                break;
            case ColorXYZ:
            case ColorXYZG:
                shadersXYZG();
                texturesXYZG();
                break;
            case ColorXYZRGB:
            case ColorXYZWRGBA:
                shadersXYZRGB();
                texturesXYZRGB();
                break;
            default:
                break;
        }

        // CALL THE VIRTUAL METHOD TO BE OVERRIDEN BY SUBCLASSES
        initializeGL();

        // RELEASE THIS CONTEXT AS THE CURRENT GL CONTEXT
        doneCurrent();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::shadersGray()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/GRAY/processGrayVideo.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GRAY/processGrayVideo.frag");
    program.link();
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::texturesGray()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE INCOMING VIDEO
    textureColor = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureColor->setSize(numColorCols, numColorRows);
    textureColor->setFormat(QOpenGLTexture::RGBA32F);
    textureColor->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureColor->setMinificationFilter(QOpenGLTexture::Nearest);
    textureColor->setMagnificationFilter(QOpenGLTexture::Nearest);
    textureColor->allocateStorage();

    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    frameBufferObject = new QOpenGLFramebufferObject(QSize(numColorCols, numColorRows), frameBufferObjectFormat);
    frameBufferObject->release();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::shadersRGB()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/RGB/processRGBVideo.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/RGB/processRGBVideo.frag");
    program.link();
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::texturesRGB()
{
    texturesGray();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::shadersXYZG()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING DEPTH VIDEO PLUS COLOR TEXTURE VIDEO
    setlocale(LC_NUMERIC, "C");
    if (playbackDevice == DeviceKinect) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawKinectVideoToXYZG.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawKinectVideoToXYZG.frag");
        program.link();
    } else if (playbackDevice == DevicePrimeSense) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawPrimeSenseVideoToXYZG.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawPrimeSenseVideoToXYZG.frag");
        program.link();
    } else if (playbackDevice == DeviceRealSense) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawPrimeSenseVideoToXYZG.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawPrimeSenseVideoToXYZG.frag");
        program.link();
    } else if (playbackDevice == DeviceProsilicaLCG) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZG/XYZG/rawProsilicaLCGVideoToXYZG.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZG/XYZG/rawProsilicaLCGVideoToXYZG.frag");
        program.link();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::texturesXYZG()
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

    //    if (playbackDevice == DeviceProsilicaLCG || playbackDevice == DeviceProsilicaIOS) {
    //        // CREATE TEXTURE FOR HOLDING SPHERICAL TO CARTESIAN COORDINATE TRANSFORMATION
    //        textureAngles = new QOpenGLTexture(QOpenGLTexture::Target2D);
    //        textureAngles->setSize(2 * numDepthCols, numDepthRows);
    //        textureAngles->setFormat(QOpenGLTexture::RGBA32F);
    //        textureAngles->setWrapMode(QOpenGLTexture::ClampToBorder);
    //        textureAngles->setMinificationFilter(QOpenGLTexture::Nearest);
    //        textureAngles->setMagnificationFilter(QOpenGLTexture::Nearest);
    //        textureAngles->allocateStorage();

    //        if (lookUpTable.isNull()) {
    //            // CREATE BUFFER TO HOLD ABCD AND EFGH TEXTURES
    //            float *buffer = (float *)malloc(8 * numDepthCols * numDepthRows * sizeof(float));
    //            if (buffer) {
    //                for (unsigned int row = 0; row < numDepthRows; row++) {
    //                    for (unsigned int col = 0; col < numDepthCols; col++) {
    //                        buffer[8 * (col + row * numDepthCols) + 0] =   0.0f;                       // A
    //                        buffer[8 * (col + row * numDepthCols) + 1] = (float)(numDepthCols / 2) - (float)col; // B
    //                        buffer[8 * (col + row * numDepthCols) + 2] =    0.0f;                      // C
    //                        buffer[8 * (col + row * numDepthCols) + 3] = (float)(numDepthRows / 2) - (float)row; // D
    //                        buffer[8 * (col + row * numDepthCols) + 4] =    0.0f; // E
    //                        buffer[8 * (col + row * numDepthCols) + 5] = -300.0f; // F
    //                        buffer[8 * (col + row * numDepthCols) + 6] =    0.0f; // G
    //                        buffer[8 * (col + row * numDepthCols) + 7] =    1.0f; // H
    //                    }
    //                }
    //                textureAngles->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, buffer);
    //            }
    //            free(buffer);
    //        } else {
    //            textureAngles->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void*)lookUpTable.constScanLine(0));
    //        }
    //    } else if (playbackDevice == DevicePrimeSense || playbackDevice == DeviceKinect) {
    //        // CREATE TEXTURE FOR HOLDING SPHERICAL TO CARTESIAN COORDINATE TRANSFORMATION
    //        textureAngles = new QOpenGLTexture(QOpenGLTexture::Target2D);
    //        textureAngles->setSize(numDepthCols, numDepthRows);
    //        textureAngles->setFormat(QOpenGLTexture::RG32F);
    //        textureAngles->setWrapMode(QOpenGLTexture::ClampToBorder);
    //        textureAngles->setMinificationFilter(QOpenGLTexture::Nearest);
    //        textureAngles->setMagnificationFilter(QOpenGLTexture::Nearest);
    //        textureAngles->allocateStorage();

    //        float *buffer = (float *)malloc(2 * numDepthCols * numDepthRows * sizeof(float));
    //        if (buffer) {
    //            for (unsigned int row = 0; row < numDepthRows; row++) {
    //                for (unsigned int col = 0; col < numDepthCols; col++) {
    //                    buffer[2 * (col + row * numDepthCols) + 0] = tan(((float)col / (float)numDepthCols - 0.5f) * horizontalFieldOfView);
    //                    buffer[2 * (col + row * numDepthCols) + 1] = tan(((float)row / (float)numDepthRows - 0.5f) * verticalFieldOfView);
    //                }
    //            }
    //            textureAngles->setData(QOpenGLTexture::RG, QOpenGLTexture::Float32, buffer);
    //        } else {
    //            qDebug() << QString("Unable to create spherical texture buffer.");
    //        }
    //        free(buffer);
    //    }

    // CREATE THE FRAME BUFFER OBJECT TO HOLD THE SCAN RESULTS AS A TEXTURE
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    frameBufferObject = new QOpenGLFramebufferObject(QSize(numDepthCols, numDepthRows), frameBufferObjectFormat);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::shadersXYZRGB()
{
    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING DEPTH VIDEO PLUS COLOR TEXTURE VIDEO
    setlocale(LC_NUMERIC, "C");
    if (playbackDevice == DeviceKinect) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawKinectVideoToXYZRGB.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawKinectVideoToXYZRGB.frag");
        program.link();
    } else if (playbackDevice == DevicePrimeSense || playbackDevice == DeviceRealSense) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/XYZRGB/XYZRGB/rawPrimeSenseVideoToXYZRGB.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/XYZRGB/XYZRGB/rawPrimeSenseVideoToXYZRGB.frag");
        program.link();
    }
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::texturesXYZRGB()
{
    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH VIDEO
    textureDepth = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureDepth->setSize(numDepthCols / 4, numDepthRows);
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

    //    // CREATE TEXTURE FOR HOLDING SPHERICAL TO CARTESIAN COORDINATE TRANSFORMATION
    //    textureAngles = new QOpenGLTexture(QOpenGLTexture::Target2D);
    //    textureAngles->setSize(numDepthCols, numDepthRows);
    //    textureAngles->setFormat(QOpenGLTexture::RG32F);
    //    textureAngles->setWrapMode(QOpenGLTexture::ClampToBorder);
    //    textureAngles->setMinificationFilter(QOpenGLTexture::Nearest);
    //    textureAngles->setMagnificationFilter(QOpenGLTexture::Nearest);
    //    textureAngles->allocateStorage();

    //    float *vertices = (float *)malloc(2 * numDepthCols * numDepthRows * sizeof(float));
    //    if (vertices) {
    //        for (unsigned int row = 0; row < numDepthRows; row++) {
    //            for (unsigned int col = 0; col < numDepthCols; col++) {
    //                vertices[2 * (col + row * numDepthCols) + 0] = tan(((float)col / (float)numDepthCols - 0.5f) * horizontalFieldOfView);
    //                vertices[2 * (col + row * numDepthCols) + 1] = tan(((float)row / (float)numDepthRows - 0.5f) * verticalFieldOfView);
    //            }
    //        }
    //        textureAngles->setData(QOpenGLTexture::RG, QOpenGLTexture::Float32, vertices);
    //    } else {
    //        qDebug() << QString("Unable to create spherical texture buffer.");
    //    }
    //    free(vertices);

    // CREATE THE FRAME BUFFER OBJECT TO HOLD THE SCAN RESULTS AS A TEXTURE
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);
    frameBufferObject = new QOpenGLFramebufferObject(QSize(2 * numDepthCols, numDepthRows), frameBufferObjectFormat);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::onUpdateBuffer(LAUScan scan)
{
    updateBuffer(scan);
    emit emitBuffer(scan);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::setLookUpTable(LAULookUpTable lut)
{
    // MAKE SURE WE HAVE A VALID LOOK UP TABLE
    if (lut.isValid()) {
        // SAVE A LOCAL COPY OF THE USER SUPPLIED LOOK UP TABLE
        lookUpTable = lut;

        // CHECK TO SEE IF WE CAN COPY THIS LOOK UP TABLE TO THE GPU
        if (wasInitialized()) {
            // UPLOAD THE ABCD AND EFGH COEFFICIENTS AS A FLOATING POINT RGBA TEXTURE
            if (textureAngles) {
                // SET THE GRAPHICS CARD CONTEXT TO THIS ONE
                makeCurrent(surface);
                textureAngles->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)lookUpTable.constScanLine(0));
                doneCurrent();
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::updateBuffer(LAUScan scan)
{
    if (makeCurrent(surface)) {
        // COPY FRAME BUFFER TEXTURE FROM GPU TO LOCAL CPU BUFFER
        glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
        switch (playbackColor) {
            case ColorGray:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, scan.constPointer());
                break;
            case ColorRGB:
            case ColorXYZ:
            case ColorXYZRGB:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, scan.constPointer());
                break;
            case ColorRGBA:
            case ColorXYZW:
            case ColorXYZG:
            case ColorXYZWRGBA:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, scan.constPointer());
                break;
            default:
                break;
        }
    }
    if (scan.isValid()) {
        scan.save(QString("/tmp/scan.tif"));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    // CALL THE FILTER'S UPDATE BUFFER
    updateBuffer(depth, color, mapping);
    emit emitBuffer(depth, color, mapping);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::updateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    if (makeCurrent(surface)) {
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
            if (program.bind()) {
                // CLEAR THE FRAME BUFFER OBJECT
                glViewport(0, 0, frameBufferObject->width(), frameBufferObject->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (vertexBuffer.bind()) {
                    if (indexBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        if (textureColor) {
                            glActiveTexture(GL_TEXTURE0);
                            textureColor->bind();
                            program.setUniformValue("qt_colorTexture", 0);
                        }

                        // BIND THE DEPTH TEXTURE
                        if (textureDepth) {
                            glActiveTexture(GL_TEXTURE1);
                            textureDepth->bind();
                            program.setUniformValue("qt_depthTexture", 1);
                        }

                        // PASS THE HANDLE TO OUR FRAME BUFFER OBJECT'S TEXTURE
                        if (playbackDevice == DeviceKinect) {
                            if ((playbackColor == ColorXYZRGB) || (playbackColor == ColorXYZWRGBA)) {
                                glActiveTexture(GL_TEXTURE2);
                                textureMapping->bind();
                                program.setUniformValue("qt_mappingTexture", 2);
                            }
                        }

                        // BIND TEXTURE FOR CONVERTING FROM SPHERICAL TO CARTESIAN
                        if (textureAngles) {
                            glActiveTexture(GL_TEXTURE3);
                            textureAngles->bind();
                            program.setUniformValue("qt_spherTexture", 3);
                        }

                        // SET THE RANGE LIMITS FOR THE Z AXIS
                        if (lookUpTable.isValid()) {
                            program.setUniformValue("qt_depthLimits", lookUpTable.zLimits());
                        } else {
                            program.setUniformValue("qt_depthLimits", QPointF(-1e6, 1e6));
                        }

                        // SET THE SNR THRESHOLD FOR DELETING BAD POINTS
                        if (playbackDevice == DeviceProsilicaLCG || playbackDevice == DeviceProsilicaIOS) {
                            program.setUniformValue("qt_snrThreshold", (float)snrThreshold / 1000.0f);
                            program.setUniformValue("qt_mtnThreshold", (float)qPow((float)mtnThreshold / 1000.0f, 4.0f));
                        }

                        // Tell OpenGL programmable pipeline how to locate vertex position data
                        glVertexAttribPointer(program.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                        program.enableAttributeArray("qt_vertex");
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        indexBuffer.release();
                    }
                    vertexBuffer.release();
                }
                program.release();
            }
            frameBufferObject->release();
        }
        swapBuffers(surface);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::saveTextureToDisk(QOpenGLFramebufferObject *fbo, QString filename)
{
    // CREATE A BUFFER TO HOLD THE TEXTURE
    LAUScan scan(fbo->width(), fbo->height(), ColorXYZW);

    // NOW COPY THE TEXTURE BACK FROM THE FBO TO THE LAUIMAGE
    glBindTexture(GL_TEXTURE_2D, fbo->texture());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, scan.constPointer());
    scan.save(filename);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::saveTextureToDisk(QOpenGLTexture *texture, QString filename)
{
    // CREATE A BUFFER TO HOLD THE TEXTURE
    LAUScan scan(texture->width(), texture->height(), ColorXYZW);

    // NOW COPY THE TEXTURE BACK FROM THE FBO TO THE LAUIMAGE
    texture->bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, scan.constPointer());
    scan.save(filename);
}
