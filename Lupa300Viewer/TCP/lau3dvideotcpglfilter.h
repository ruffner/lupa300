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

#ifndef LAU3DVIDEOTCPGLFILTER_H
#define LAU3DVIDEOTCPGLFILTER_H

#include "lauglwidget.h"
#include "lauabstractfilter.h"

#define LAU3DVIDEOTCPMESSAGELENGTH  4

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DVideoTCPGLFilter : public LAUAbstractGLFilter
{
    Q_OBJECT

public:
    explicit LAU3DVideoTCPGLFilter(unsigned int depthCols, unsigned int depthRows, unsigned int colorCols, unsigned int colorRows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QWidget *parent = NULL);
    ~LAU3DVideoTCPGLFilter();

protected:
    void initializeGL();
    void updateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());

public slots:
    void onSetMTNThreshold(int val)
    {
        mtnThreshold = val;
    }
    void onSetSNRThreshold(int val)
    {
        snrThreshold = val;
    }

private:
    QOpenGLShaderProgram programA; // THIS SHADER PROGRAM APPLIES A GREEN SCREEN TO THE INCOMING TEXTURES
    int snrThreshold;
    int mtnThreshold;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DVideoTCPGLWidget : public LAUAbstractGLWidget
{
    Q_OBJECT

public:
    LAU3DVideoTCPGLWidget(unsigned int depthCols, unsigned int depthRows, unsigned int colorCols, unsigned int colorRows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device = DevicePrimeSense, QWidget *parent = 0);
    LAU3DVideoTCPGLWidget(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device = DevicePrimeSense, QWidget *parent = 0);
    ~LAU3DVideoTCPGLWidget();

    bool isValid() const
    {
        return (valid);
    }

    LAUVideoPlaybackColor color() const
    {
        return (playbackColor);
    }
    LAUVideoPlaybackDevice device() const
    {
        return (playbackDevice);
    }

    void copyScan(float *buffer);                                                        // COPY THE CURRENT SCAN INTO THE PROVIDED MEMORY BUFFER
    void setLookUpTable(LAULookUpTable lut = LAULookUpTable(QString()));                 // SET THE SCANNER LOOK UP TABLE THAT INCLUDES XYZ LIMITS
    void setRangeLimits(float zmn, float zmx, float hFov, float vFov);                   // SET THE CAMERA BOUNDING BOX FOR THE 3D DISPLAY
    void setMaximumIntensityValue(unsigned short value)
    {
        maxIntensityValue = value;    // SET THE MAXIMUM INTENSITY VALUE FOR RAW DATA
    }

public slots:
    void onEnableTexture(bool state)
    {
        textureEnableFlag = state;
        update();
    }
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());

protected:
    void paintGL();
    void initializeGL();

private:
    LAUVideoPlaybackColor playbackColor;
    LAUVideoPlaybackDevice playbackDevice;
    QOpenGLBuffer vertexBufferA, vertexBufferB;
    QOpenGLBuffer indiceBufferA, indiceBufferB;
    QOpenGLFramebufferObject *frameBufferObject;
    QOpenGLFramebufferObject *smoothBufferObjectA, *smoothBufferObjectB;
    QOpenGLShaderProgram programA, programB, programC, programD;

    QOpenGLTexture *textureDepth, *textureColor, *textureMapping, *textureAngles;

    LAULookUpTable lookUpTable;

    bool valid, textureEnableFlag;
    unsigned int numDepthRows, numDepthCols, numDepthInds;
    unsigned int numColorRows, numColorCols;
    unsigned short maxIntensityValue;

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

    void smoothBuffer();

signals:
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
};

#endif // LAU3DVIDEOTCPGLFILTER_H
