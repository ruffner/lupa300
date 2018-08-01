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

#ifndef LAU3DVIDEOTCPWIDGET_H
#define LAU3DVIDEOTCPWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QSpinBox>
#include <QSettings>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QHostAddress>
#include <QDialogButtonBox>

#include "qzeroconf.h"
#include "qzeroconfservice.h"

#include "lau3dvideoglwidget.h"
#include "lau3dvideotcpclient.h"
#include "lau3dvideoplayerwidget.h"

#ifdef PROSILICA
#include "LAU3DVideoTCPSettingsWidget.h"
#else
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DVideoTCPSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAU3DVideoTCPSettingsWidget(QWidget *parent = 0) : QWidget(parent)
    {
        this->setWindowFlags(Qt::Tool);
        this->setWindowTitle(QString("Camera Settings"));
        this->setLayout(new QGridLayout());
        this->layout()->setContentsMargins(6,6,6,6);
        this->layout()->setSpacing(3);
        this->setFixedSize(435, 36);

        QSettings settings;
        exposure = settings.value(QString("LAU3DVideoTCPSettingsWidget::exposure"), 5000).toInt();

        expSlider = new QSlider(Qt::Horizontal);
        expSlider->setMinimum(1);
        expSlider->setMaximum(4095);
        expSlider->setValue(exposure);

        expSpinBox = new QSpinBox();
        expSpinBox->setFixedWidth(80);
        expSpinBox->setAlignment(Qt::AlignRight);
        expSpinBox->setMinimum(1);
        expSpinBox->setMaximum(4095);
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

    ~LAU3DVideoTCPSettingsWidget()
    {
        QSettings settings;
        settings.setValue(QString("LAU3DVideoTCPSettingsWidget::exposure"), exposure);

        qDebug() << "LAU3DVideoTCPSettingsWidget::~LAU3DVideoTCPSettingsWidget()";
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
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DVideoTCPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAU3DVideoTCPWidget(LAUVideoPlaybackColor color = ColorXYZRGB, LAUVideoPlaybackDevice device = DevicePrimeSense, QString address = QString(), QWidget *parent = 0);
    ~LAU3DVideoTCPWidget();

public slots:
    void onRecordButtonClicked(bool state);
    void onReceiveFrameBuffer(LAUMemoryObject buffer);
    void onReceiveVideoFrames(LAUMemoryObject frame);
    void onReceiveVideoFrames(QList<LAUMemoryObject> frameList);

    void onConnectButton_clicked(bool checked);
    void onTcpClient_error(QString error);
    void onTcpClient_connected(bool state);
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());
    void onContextMenuTriggered();

private:
    LAUVideoPlaybackColor playbackColor;
    LAUVideoPlaybackDevice playbackDevice;

    LAU3DVideoTCPClient *tcpClient;
    LAUVideoPlayerLabel *videoPlayerLabel;
    LAUAbstractGLWidget *abstractGLWidget;
    LAU3DVideoTCPGLWidget  *glWidget;

    QList<LAUMemoryObject> videoFramesBufferList;
    QList<LAUMemoryObject> recordedVideoFramesBufferList;

    QWidget *scannerWidget;
    QPushButton *connectedButton;
    QComboBox *tcpAddressComboBox;
    bool snapShotModeFlag, videoRecordingFlag;
    QTime time, pressStartButtonTime, timeStamp;
    int counter;

    QZeroConf *zeroConf;

private slots:
    void onServiceError(QZeroConf::error_t error);
    void onAddService(QZeroConfService item);
    void onRemoveService(QZeroConfService item);
    void onUpdateService(QZeroConfService item);

signals:
    void emitVideoFrames(LAUMemoryObject frame);
    void emitVideoFrames(QList<LAUMemoryObject> frameList);
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DVideoTCPDialog : public QDialog
{
    Q_OBJECT

public:
    LAU3DVideoTCPDialog(LAUVideoPlaybackColor color = ColorXYZRGB, LAUVideoPlaybackDevice device = DevicePrimeSense, QString address = QString(), QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(0, 0, 0, 0);
        this->layout()->setSpacing(6);

        widget = new LAU3DVideoTCPWidget(color, device, address, this);
        this->setWindowTitle(widget->windowTitle());
        this->layout()->addWidget(widget);
        return;

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }
    ~LAU3DVideoTCPDialog() { ; }

private:
    LAU3DVideoTCPWidget *widget;
};

#endif // LAU3DVIDEOTCPWIDGET_H
