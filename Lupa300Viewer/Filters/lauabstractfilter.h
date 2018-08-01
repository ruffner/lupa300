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

#ifndef LAUABSTRACTFILTERGLWIDGET_H
#define LAUABSTRACTFILTERGLWIDGET_H

#include <QObject>
#include <QThread>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>

#include "emmintrin.h"
#include "xmmintrin.h"
#include "tmmintrin.h"
#include "smmintrin.h"

#include "lauscan.h"
#include "laulookuptable.h"

class LAUAbstractFilter;
class LAUAbstractGLFilter;

using namespace LAU3DVideoParameters;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractGLFilter : public QOpenGLContext, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit LAUAbstractGLFilter(unsigned int depthCols, unsigned int depthRows, unsigned int colorCols, unsigned int colorRows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QWidget *parent = NULL) : QOpenGLContext(parent), numDepthCols((depthCols == 0) ? colorCols : depthCols), numDepthRows((depthRows == 0) ? colorRows : depthRows), numColorCols((colorCols == 0) ? depthCols : colorCols), numColorRows((colorRows == 0) ? depthRows : colorRows), snrThreshold(0), mtnThreshold(1000), playbackColor(color), playbackDevice(device), surface(NULL), frameBufferObject(NULL), textureDepth(NULL), textureColor(NULL), textureMapping(NULL), textureAngles(NULL) { ; }
    explicit LAUAbstractGLFilter(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QWidget *parent) : QOpenGLContext(parent), numDepthCols(cols), numDepthRows(rows), numColorCols(cols), numColorRows(rows), snrThreshold(0), mtnThreshold(1000), playbackColor(color), playbackDevice(device), surface(NULL), frameBufferObject(NULL), textureDepth(NULL), textureColor(NULL), textureMapping(NULL), textureAngles(NULL) { ; }
    ~LAUAbstractGLFilter()
    {
        if (surface && makeCurrent(surface)) {
            if (textureDepth) {
                delete textureDepth;
            }
            if (textureColor) {
                delete textureColor;
            }
            if (textureAngles) {
                delete textureAngles;
            }
            if (textureMapping) {
                delete textureMapping;
            }
            if (frameBufferObject) {
                delete frameBufferObject;
            }
            doneCurrent();
        }
    }

    bool isValid() const
    {
        return (wasInitialized());
    }
    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    int width() const
    {
        return (numDepthCols);
    }
    int height() const
    {
        return (numDepthRows);
    }

    void initialize();
    void setLookUpTable(LAULookUpTable lut = LAULookUpTable(QString()));
    void setFieldsOfView(float hFov, float vFov)
    {
        horizontalFieldOfView = hFov;
        verticalFieldOfView = vFov;
    }
    void setSurface(QSurface *srfc)
    {
        surface = srfc;
    }

    void saveTextureToDisk(QOpenGLFramebufferObject *fbo, QString filename);
    void saveTextureToDisk(QOpenGLTexture *texture, QString filename);

public slots:
    void onUpdateBuffer(LAUScan scan);
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());
    void onSetMTNThreshold(int val)
    {
        if (val != mtnThreshold) {
            mtnThreshold = val;
        }
    }
    void onSetSNRThreshold(int val)
    {
        if (val != snrThreshold) {
            snrThreshold = val;
        }
    }

protected:
    unsigned int numDepthCols, numDepthRows;
    unsigned int numColorCols, numColorRows;
    float horizontalFieldOfView, verticalFieldOfView;
    int snrThreshold;
    int mtnThreshold;
    LAULookUpTable lookUpTable;

    LAUVideoPlaybackColor playbackColor;
    LAUVideoPlaybackDevice playbackDevice;

    QSurface *surface;
    QOpenGLShaderProgram program;
    QOpenGLBuffer vertexBuffer, indexBuffer;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLFramebufferObject *frameBufferObject;
    QOpenGLTexture *textureDepth, *textureColor, *textureMapping, *textureAngles;

    virtual void initializeGL() { ; }
    virtual void updateBuffer(LAUScan scan);
    virtual void updateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());

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

signals:
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
    void emitBuffer(LAUScan scan);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractFilter : public QObject
{
    Q_OBJECT

public:
    explicit LAUAbstractFilter(int cols, int rows, QObject *parent = NULL) : QObject(parent), numCols(cols), numRows(rows) { ; }
    ~LAUAbstractFilter()
    {
        qDebug() << QString("LAUAbstractFilter::~LAUAbstractFilter()");
    }

    int width() const
    {
        return (numCols);
    }
    int height() const
    {
        return (numRows);
    }

public slots:
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject())
    {
        updateBuffer(depth, color, mapping);
        emit emitBuffer(depth, color, mapping);
    }

    void onUpdateBuffer(LAUScan scan)
    {
        updateBuffer(scan);
        emit emitBuffer(scan);
    }

protected:
    int numCols, numRows;
    virtual void updateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject())
    {
        Q_UNUSED(depth);
        Q_UNUSED(color);
        Q_UNUSED(mapping);
    }
    virtual void updateBuffer(LAUScan scan = LAUScan())
    {
        Q_UNUSED(scan);
    }

signals:
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
    void emitBuffer(LAUScan scan);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractFilterController : public QObject
{
    Q_OBJECT

public:
    explicit LAUAbstractFilterController(LAUAbstractGLFilter *contxt, QSurface *srfc = NULL, QObject *parent = NULL);
    explicit LAUAbstractFilterController(LAUAbstractFilter *fltr, QObject *parent = NULL);
    ~LAUAbstractFilterController();

protected:
    LAUAbstractGLFilter *context;
    LAUAbstractFilter *filter;
    bool localSurface;
    QSurface *surface;
    QThread *thread;
};

#endif // LAUABSTRACTFILTERGLWIDGET_H
