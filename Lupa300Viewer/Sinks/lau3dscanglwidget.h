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

#ifndef LAU3DSCANGLWIDGET_H
#define LAU3DSCANGLWIDGET_H

#include <QtGlobal>
#include "lauscan.h"
#include "lauglwidget.h"
#include "laumemoryobject.h"

using namespace LAU3DVideoParameters;

class LAU3DScanGLWidget : public LAUAbstractGLWidget
{
    Q_OBJECT

public:
    enum GrabMouseBufferMode { MouseModeXYZ, MouseModeRGB };

    LAU3DScanGLWidget(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, unsigned char *buffer = NULL, QWidget *parent = 0);
    ~LAU3DScanGLWidget();

    bool isValid() const { return(valid); }

    QSize size() const { return(QSize(numCols,numRows)); }
    void setRangeLimits(float zmn, float zmx, float hFov, float vFov); // SET THE CAMERA BOUNDING BOX FOR THE 3D DISPLAY
    LAUMemoryObject grabMouseBuffer(GrabMouseBufferMode mode = MouseModeXYZ);

    virtual QOpenGLTexture* grabMouseTexture() { return(texturePlayback); }

public slots:
    void onEnableTexture(bool state) { textureEnableFlag=state; update(); }
    void onUpdatePlaybackBuffer(LAUMemoryObject buffer);
    void onUpdatePlaybackBuffer(float *buffer);
    void onUpdatePlaybackBuffer(LAUScan scan);

protected:
    void paintGL();
    void initializeGL();

    LAUVideoPlaybackColor playbackColor;
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer indiceBuffer;
    QOpenGLShaderProgram program;
    QMatrix4x4 symmetry;

    QOpenGLTexture *texturePlayback;

    bool valid, textureEnableFlag;
    unsigned int numCols, numRows, numInds;
    float *scanBuffer;

    void initializeClass();

    void shadersGray();
    void shadersRGB();
    void shadersXYZG();
    void shadersXYZRGB();
    void shadersXYZWRGBA();

    void texturesGray();
    void texturesRGB();
    void texturesXYZG();
    void texturesXYZRGB();
    void texturesXYZWRGBA();

    void verticesGray();
    void verticesRGB();
    void verticesXYZG();
    void verticesXYZRGB();
    void verticesXYZWRGBA();

signals:
    void emitPlaybackBuffer(LAUMemoryObject buffer);
    void emitPlaybackBuffer(float *buffer);
    void emitPlaybackBuffer(LAUScan scan);
};

#endif // LAU3DSCANGLWIDGET_H
