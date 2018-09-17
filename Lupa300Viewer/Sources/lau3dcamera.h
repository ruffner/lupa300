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

#ifndef LAUTHREEDIMENSIONALCAMERACONTROLLER_H
#define LAUTHREEDIMENSIONALCAMERACONTROLLER_H

#include <QDebug>
#include <QLabel>
#include <QSlider>
#include <QObject>
#include <QThread>
#include <QWidget>
#include <QLayout>
#include <QSpinBox>
#include <QGridLayout>
#include <QMessageBox>

#include "laumemoryobject.h"

#define NUMFRAMESINBUFFER 2

using namespace LAU3DVideoParameters;

class LAUCameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUCameraWidget(QWidget *parent = 0) : QWidget(parent)
    {
        this->setWindowFlags(Qt::Tool);
        this->setWindowTitle(QString("Camera Settings"));
        this->setLayout(new QGridLayout());
        this->layout()->setContentsMargins(6,6,6,6);
        this->layout()->setSpacing(3);
        this->setFixedSize(435, 45);

        QSettings settings;
        exposure = settings.value(QString("LAUCameraWidget::exposure"), 1000).toInt();

        expSlider = new QSlider(Qt::Horizontal);
        expSlider->setMinimum(1);
        expSlider->setMaximum(50000);
        expSlider->setValue(exposure);

        expSpinBox = new QSpinBox();
        expSpinBox->setFixedWidth(80);
        expSpinBox->setAlignment(Qt::AlignRight);
        expSpinBox->setMinimum(1);
        expSpinBox->setMaximum(50000);
        expSpinBox->setValue(exposure);

        QLabel *label = new QLabel(QString("Exposure"));
        label->setToolTip(QString("exposure time of the camera in microseconds"));
        ((QGridLayout*)(this->layout()))->addWidget(label, 0, 0, 1, 1, Qt::AlignRight);
        ((QGridLayout*)(this->layout()))->addWidget(expSlider, 0, 1, 1, 1);
        ((QGridLayout*)(this->layout()))->addWidget(expSpinBox, 0, 2, 1, 1);

        connect(expSlider, SIGNAL(valueChanged(int)), expSpinBox, SLOT(setValue(int)));
        connect(expSpinBox, SIGNAL(valueChanged(int)), expSlider, SLOT(setValue(int)));
        connect(expSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onUpdateExposurePrivate(int)));
    }

    ~LAUCameraWidget()
    {
        QSettings settings;
        settings.setValue(QString("LAUCameraWidget::exposure"), exposure);
        qDebug() << "LAUCameraWidget::~LAUCameraWidget()";
    }

    void setExp(int val) { expSpinBox->setValue(val); }

    int exp() { return(exposure); }

public slots:
    void onUpdateExposure(int val) { if (val != exposure) { exposure = val; expSpinBox->setValue(exposure); }}

private slots:
    void onUpdateExposurePrivate(int val)     { if (val != exposure) { exposure = val; emit emitUpdateExposure(exposure); }}

private:
    int exposure;
    QSlider *expSlider;
    QSpinBox *expSpinBox;

signals:
    void emitUpdateExposure(int val);
};

class LAU3DCamera : public QObject
{
    Q_OBJECT

public:
    LAU3DCamera(LAUVideoPlaybackColor color = ColorXYZRGB, QObject *parent = 0) : QObject(parent), playbackColor(color) { ; }
    LAU3DCamera(QObject *parent) : QObject(parent), playbackColor(ColorXYZRGB) { ; }

    virtual bool hasDepth() const = 0;
    virtual bool hasColor() const = 0;
    virtual bool hasMapping() const = 0;

    virtual unsigned short maxIntensityValue() const = 0;
    virtual LAUVideoPlaybackDevice device() const = 0;
    virtual bool isValid() const = 0;
    virtual bool isNull() const { return(!isValid()); }
    virtual QString error() const = 0;
    virtual float horizontalFieldOfViewInRadians() const = 0;
    virtual float verticalFieldOfViewInRadians() const = 0;
    virtual float horizontalFieldOfViewInDegrees() const = 0;
    virtual float verticalFieldOfViewInDegrees() const = 0;
    virtual float minDistance() const = 0;
    virtual float maxDistance() const = 0;
    virtual unsigned int depthWidth() const = 0;
    virtual unsigned int depthHeight() const = 0;
    virtual unsigned int colorWidth() const = 0;
    virtual unsigned int colorHeight() const = 0;
    virtual QList<LAUVideoPlaybackColor> playbackColors() = 0;
    LAUVideoPlaybackColor color() const { return(playbackColor); }
    virtual unsigned int colors() const {
        switch (playbackColor){
            case ColorUndefined: return(0);
            case ColorGray:      return(1);
            case ColorRGB:       return(3);
            case ColorRGBA:      return(4);
            case ColorXYZ:       return(3);
            case ColorXYZW:      return(4);
            case ColorXYZG:      return(4);
            case ColorXYZRGB:    return(6);
            case ColorXYZWRGBA:  return(8);
        }
        return(0);
    }

    virtual LAUMemoryObject colorMemoryObject() const = 0;
    virtual LAUMemoryObject depthMemoryObject() const = 0;
    virtual LAUMemoryObject mappiMemoryObject() const = 0;

    virtual QString make() const { return(makeString); }
    virtual QString model() const { return(modelString); }
    virtual QString serial() const { return(serialString); }

    unsigned int height() { if (playbackColor == ColorGray || playbackColor == ColorRGB) return(colorHeight()); else return(depthHeight()); }
    unsigned int width() { if (playbackColor == ColorGray || playbackColor == ColorRGB) return(colorWidth()); else return(depthWidth()); }
    QSize size() { return(QSize(width(), height())); }

public slots:
    virtual void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject()) = 0;
    virtual void onUpdateBuffer(LAUMemoryObject buffer, int index, void *userData) = 0;

protected:
    LAUVideoPlaybackColor playbackColor;
    QString makeString;
    QString modelString;
    QString serialString;

    QString errorString;

signals:
    void emitError(QString);
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
    void emitBuffer(LAUMemoryObject buffer, int index, void *userData);
};

class LAU3DCameraController : public QObject
{
    Q_OBJECT

public:
    explicit LAU3DCameraController(LAU3DCamera *cam, QObject *parent = 0);
    ~LAU3DCameraController();

public slots:
    void onError(QString string) { qDebug() << string; }

signals:
    void emitStopCameraTimer();

private:
    LAU3DCamera *camera;
    QThread *cameraThread;
};

#endif // LAUTHREEDIMENSIONALCAMERACONTROLLER_H
