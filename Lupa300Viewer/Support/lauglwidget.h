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

#ifndef LAUGLWIDGET_H
#define LAUGLWIDGET_H

#include <QMenu>
#include <QScreen>
#include <QWidget>
#include <QObject>
#include <QMatrix4x4>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QCoreApplication>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit LAUAbstractGLWidget(QWidget *parent = 0);
    ~LAUAbstractGLWidget();

    virtual bool isValid() const { return(wasInitialized()); }
    bool wasInitialized() const { return(vertexArrayObject.isCreated()); }
    QMenu* menu() const { return(contextMenu); }

    void setLimits(float xmn, float xmx, float ymn, float ymx, float zmn, float zmx);

    QVector2D xLimits() const { return(QVector2D(xMin, xMax)); }
    QVector2D yLimits() const { return(QVector2D(yMin, yMax)); }
    QVector2D zLimits() const { return(QVector2D(zMin, zMax)); }

    void setColorTransform(QMatrix4x4 mat) { clrTransform = mat; update(); }
    QMatrix4x4 colorTransform() const { return(clrTransform); }

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent *event) {
        lastPos = event->pos();
        if (event->button() == Qt::RightButton){
            if (contextMenu){
                contextMenu->popup(event->globalPos());
            }
            emit emitClicked(event->globalPos());
        }
    }
    void mouseReleaseEvent(QMouseEvent*) { ; }
    void mouseDoubleClickEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void updateProjectionMatrix();

    int localWidth, localHeight;
    float scaleFactor, offset;
    qreal devicePixelRatio;

    float xMin, xMax;
    float yMin, yMax;
    float zMin, zMax;
    float horizontalFieldOfView, verticalFieldOfView;
    float zoomFactor;

    QMenu *contextMenu;

    int xRot, yRot, zRot;
    QMatrix4x4 projection;
    QMatrix4x4 clrTransform;
    QPoint lastPos;

private:
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer noVideoVertexBuffer, noVideoIndexBuffer;
    QOpenGLShaderProgram noVideoProgram;
    QOpenGLTexture *noVideoTexture;
    bool initializedFlag;

signals:
    void emitActivated();
    void emitClicked(QPoint pos);
};

#endif // LAUGLWIDGET_H
