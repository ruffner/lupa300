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

#include "lauglwidget.h"
#include <locale.h>
#include <math.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractGLWidget::LAUAbstractGLWidget(QWidget *parent) : QOpenGLWidget(parent), noVideoTexture(NULL)
{
    contextMenu = NULL;

    lastPos = QPoint(0, 0);
    localHeight = 480;
    localWidth = 640;

    zoomFactor = 1.0;

    xRot = 0.0;
    yRot = 0.0;
    zRot = 0.0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractGLWidget::~LAUAbstractGLWidget()
{
    qDebug() << QString("~LAUAbstractGLWidget()");

    if (wasInitialized()){
        makeCurrent();
        if (noVideoTexture) delete noVideoTexture;
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::setLimits(float xmn, float xmx, float ymn, float ymx, float zmn, float zmx)
{
    // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
    xMin = qMin(xmn, xmx); xMax = qMax(xmn, xmx);
    yMin = qMin(ymn, ymx); yMax = qMax(ymn, ymx);
    zMin = qMin(zmn, zmx); zMax = qMax(zmn, zmx);

    // CALCULATE FOV
    float   phiA = atan(yMin / zMin);
    float   phiB = atan(yMax / zMin);
    float thetaA = atan(xMin / zMin);
    float thetaB = atan(xMax / zMin);

    horizontalFieldOfView = fabs(thetaA) + fabs(thetaB);
    verticalFieldOfView   = fabs(phiA) + fabs(phiB);

    // MAKE SURE WE HAVE ALREADY INITIALIZED THIS GLWIDGET BEFORE TELLING IT TO UPDATE ITSELF
    if (wasInitialized()){
        updateProjectionMatrix();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::wheelEvent(QWheelEvent *event)
{
    // CHANGE THE ZOOM FACTOR BASED ON HOW MUCH WHEEL HAS MOVED
    zoomFactor *= (1.0 + (float)event->angleDelta().y()/160.0);
    zoomFactor = qMax(0.10f, qMin(zoomFactor, 10.0f));

    // UPDATE THE PROJECTION MATRIX SINCE WE CHANGED THE ZOOM FACTOR
    updateProjectionMatrix();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = lastPos.x() - event->x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        xRot = qMin(qMax(xRot + 4 * dy, -1200), 1200);
        yRot = qMin(qMax(yRot + 4 * dx, -1200), 1200);
        zRot = qMin(qMax(zRot + 0, -1200), 1200);
    } else if (event->buttons() & Qt::RightButton) {
        xRot = qMin(qMax(xRot + 4 * dy, -1200), 1200);
        yRot = qMin(qMax(yRot + 0, -1200), 1200);
        zRot = qMin(qMax(zRot + 4 * dx, -1200), 1200);
    }
    lastPos = event->pos();

    // UPDATE THE PROJECTION MATRIX SINCE WE CHANGED THE ROTATION ANGLE
    updateProjectionMatrix();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    xRot = 0;
    yRot = 0;
    zRot = 0;
    zoomFactor = 1.0;
    updateProjectionMatrix();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::updateProjectionMatrix()
{
    float aspectRatio = (float)width()/(float)height();
    float xCenter = (xMin + xMax)/2.0f;
    float yCenter = (yMin + yMax)/2.0f;
    float zCenter = (zMin + zMax)/2.0f;

    QMatrix4x4 eyeTransform;
    eyeTransform.setToIdentity();
    eyeTransform.translate(xCenter, yCenter, zCenter);
    eyeTransform.rotate(-xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    eyeTransform.rotate( yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    eyeTransform.translate(-xCenter, -yCenter, -zCenter);

    QVector4D eye = eyeTransform * QVector4D(0.0f, 0.0f, 0.0f, 1.0f);

    float fov   = qMin(120.0f, qMax(zoomFactor * 180.0f/3.1415f * verticalFieldOfView, 0.5f));
    float zNear = qMin(qAbs(zMin), qAbs(zMax));
    float zFar  = qMax(qAbs(zMin), qAbs(zMax));

    // INITIALIZE THE PROJECTION MATRIX TO IDENTITY
    projection.setToIdentity();
    projection.perspective(fov, aspectRatio, zNear/4.0f, 3.0f*zFar);
    projection.lookAt(QVector3D(eye), QVector3D(xCenter, yCenter, zCenter), QVector3D(0.0f, 1.0f, 0.0f));

    // UPDATE THE DISPLAY
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // CREATE THE VERTEX ARRAY OBJECT FOR FEEDING VERTICES TO OUR SHADER PROGRAMS
    vertexArrayObject.create();
    vertexArrayObject.bind();

    // CREATE VERTEX BUFFER TO HOLD CORNERS OF QUADRALATERAL
    noVideoVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    noVideoVertexBuffer.create();
    noVideoVertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if (noVideoVertexBuffer.bind()){
        // ALLOCATE THE VERTEX BUFFER FOR HOLDING THE FOUR CORNERS OF A RECTANGLE
        noVideoVertexBuffer.allocate(16*sizeof(float));
        float *buffer = (float*)noVideoVertexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (buffer){
            buffer[0]  = -1.0; buffer[1]  = -1.0; buffer[2]  = 0.0; buffer[3]  = 1.0;
            buffer[4]  = +1.0; buffer[5]  = -1.0; buffer[6]  = 0.0; buffer[7]  = 1.0;
            buffer[8]  = +1.0; buffer[9]  = +1.0; buffer[10] = 0.0; buffer[11] = 1.0;
            buffer[12] = -1.0; buffer[13] = +1.0; buffer[14] = 0.0; buffer[15] = 1.0;
            noVideoVertexBuffer.unmap();
        } else {
            qDebug() << QString("noVideoVertexBuffer not allocated.") << glGetError();
        }
        noVideoVertexBuffer.release();
    }

    // CREATE INDEX BUFFER TO ORDERINGS OF VERTICES FORMING POLYGON
    noVideoIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    noVideoIndexBuffer.create();
    noVideoIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (noVideoIndexBuffer.bind()){
        noVideoIndexBuffer.allocate(6*sizeof(unsigned int));
        unsigned int *indices = (unsigned int*)noVideoIndexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices){
            indices[0] = 0; indices[1] = 1; indices[2] = 2;
            indices[3] = 0; indices[4] = 2; indices[5] = 3;
            noVideoIndexBuffer.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
        noVideoIndexBuffer.release();
    }

    // CREATE TEXTURE FOR DISPLAYING NO VIDEO SCREEN
    QImage image(":/Images/NoVideoScreen.jpg");
    noVideoTexture = new QOpenGLTexture(image);
    noVideoTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    noVideoTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    noVideoTexture->setMagnificationFilter(QOpenGLTexture::Nearest);

    // CREATE SHADER FOR SHOWING THE VIDEO NOT AVAILABLE IMAGE
    setlocale(LC_NUMERIC, "C");
    if (!noVideoProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/RGB/displayRGBVideo.vert")) close();
    if (!noVideoProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/RGB/displayRGBVideo.frag")) close();
    if (!noVideoProgram.link()) close();
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::resizeGL(int w, int h)
{
    // Get the Desktop Widget so that we can get information about multiple monitors connected to the system.
    QDesktopWidget *dkWidget = QApplication::desktop();
    QList<QScreen*> screenList = QGuiApplication::screens();
    qreal devicePixelRatio = screenList[dkWidget->screenNumber(this)]->devicePixelRatio();
    localHeight = h*devicePixelRatio;
    localWidth = w*devicePixelRatio;

    // WE'LL SET THE VIEW PORT MANUALLY IN THE PAINTGL METHOD
    // BUT LET'S MAKE SURE WE HAVE THE MOST UP TO DATE PROJECTION MATRIX
    updateProjectionMatrix();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLWidget::paintGL()
{
    // SET THE VIEW PORT AND CLEAR THE SCREEN BUFFER
    glViewport(0, 0, localWidth, localHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // MAKE SURE WE HAVE A TEXTURE TO SHOW
    if (noVideoTexture){
        if (noVideoProgram.bind()){
            if (noVideoVertexBuffer.bind()){
                if (noVideoIndexBuffer.bind()){
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    noVideoTexture->bind();
                    noVideoProgram.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    noVideoProgram.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4*sizeof(float));
                    noVideoProgram.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    noVideoIndexBuffer.release();
                }
                noVideoVertexBuffer.release();
            }
            noVideoProgram.release();
        }
    }
}
