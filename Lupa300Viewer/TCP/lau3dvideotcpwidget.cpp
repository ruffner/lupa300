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

#include "lau3dvideotcpwidget.h"

LAUMemoryObject lastVideoFrame;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAU3DVideoTCPWidget::LAU3DVideoTCPWidget(LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QString address, QWidget *parent) : QWidget(parent), playbackColor(color), playbackDevice(device), tcpClient(NULL), videoPlayerLabel(NULL), glWidget(NULL), scannerWidget(NULL), connectedButton(NULL), tcpAddressComboBox(NULL), snapShotModeFlag(true), videoRecordingFlag(false), counter(0), zeroConf(NULL)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->setSpacing(0);
    this->setWindowTitle(QString("TCP 3D Video Recorder"));

    QWidget *widget = new QWidget();
    widget->setLayout(new QVBoxLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);
    this->layout()->addWidget(widget);

    QGroupBox *box = new QGroupBox(QString("Server Address"));
    box->setLayout(new QHBoxLayout());
    box->layout()->setContentsMargins(6, 6, 6, 6);
    widget->layout()->addWidget(box);

    tcpAddressComboBox = new QComboBox();
    QLabel *label = new QLabel(QString("Host:"));
    label->setFixedWidth(60);
    box->layout()->addWidget(label);
    box->layout()->addWidget(tcpAddressComboBox);

    connectedButton = new QPushButton(QString("Connect"));
    connectedButton->setCheckable(true);
    connectedButton->setChecked(false);
    connectedButton->setFixedWidth(140);
    connect(connectedButton, SIGNAL(clicked(bool)), this, SLOT(onConnectButton_clicked(bool)));
    box->layout()->addWidget(connectedButton);

    // CREATE THE TCP CLIENT FOR MANAGING COMMUNICATION WITH THE SERVER
    tcpClient = new LAU3DVideoTCPClient(QString(), playbackColor, playbackDevice);
    connect(tcpClient, SIGNAL(emitConnected(bool)), this, SLOT(onTcpClient_connected(bool)), Qt::QueuedConnection);
    connect(tcpClient, SIGNAL(emitError(QString)), this, SLOT(onTcpClient_error(QString)), Qt::QueuedConnection);

#ifdef PROSILICA
    if (playbackDevice == DeviceProsilicaIOS || playbackDevice == DeviceProsilicaLCG) {
        // CREATE A SCANNER WIDGET FOR ADJUSTING SCAN PARAMETERS
        scannerWidget = new LAU3DProsilicaScannerWidget(this);

        connect((LAU3DProsilicaScannerWidget *)scannerWidget, SIGNAL(emitUpdateExposure(int)), tcpClient, SLOT(onSetExposure(int)), Qt::QueuedConnection);
        connect((LAU3DProsilicaScannerWidget *)scannerWidget, SIGNAL(emitUpdateSnrThreshold(int)), tcpClient, SLOT(onSetSNRThreshold(int)), Qt::QueuedConnection);
        connect((LAU3DProsilicaScannerWidget *)scannerWidget, SIGNAL(emitUpdateMtnThreshold(int)), tcpClient, SLOT(onSetMTNThreshold(int)), Qt::QueuedConnection);
    }
#else
    if (playbackDevice == DeviceIDS) {
        // CREATE A SCANNER WIDGET FOR ADJUSTING SCAN PARAMETERS
        scannerWidget = new LAU3DVideoTCPSettingsWidget(this);
        connect((LAU3DVideoTCPSettingsWidget *)scannerWidget, SIGNAL(emitUpdateExposure(int)), tcpClient, SLOT(onSetExposure(int)), Qt::QueuedConnection);
    }
#endif

    // CREATE A GLWIDGET TO PROCESS THE DFT COEFFICIENTS AND DISPLAY THE POINT CLOUD
    glWidget = new LAU3DVideoTCPGLWidget(0, 0, ColorGray);
    glWidget->setMinimumSize(320, 240);
    glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(glWidget);

    videoPlayerLabel = new LAUVideoPlayerLabel(LAUVideoPlayerLabel::StateVideoRecorder);
    videoPlayerLabel->setEnabled(false);
    this->layout()->addWidget(videoPlayerLabel);
    connect(videoPlayerLabel, SIGNAL(playButtonClicked(bool)), this, SLOT(onRecordButtonClicked(bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(emitVideoFrames(LAUMemoryObject)), this, SLOT(onReceiveVideoFrames(LAUMemoryObject)));

    if (address.isEmpty()) {
        // MAKE CONNECTIONS BETWEEN THIS OBJECT AND THE BONJOUR SERVICE OBJECT
        zeroConf = new QZeroConf();

        connect(zeroConf, SIGNAL(serviceAdded(QZeroConfService)), this, SLOT(onAddService(QZeroConfService)));
        connect(zeroConf, SIGNAL(serviceRemoved(QZeroConfService)), this, SLOT(onRemoveService(QZeroConfService)));
        connect(zeroConf, SIGNAL(serviceUpdated(QZeroConfService)), this, SLOT(onUpdateService(QZeroConfService)));
        connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));

        zeroConf->startBrowser("_qtzeroconf_test._tcp", QAbstractSocket::IPv4Protocol);
    } else {
        // INSERT THE USER SUPPLIED ADDRESS INTO THE ADDRESS COMBO BOX
        tcpAddressComboBox->addItem(address, address);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAU3DVideoTCPWidget::~LAU3DVideoTCPWidget()
{
    if (tcpClient) {
        delete tcpClient;
    }
qDebug() << "here1";
    // CLEAR THE RECORDED VIDEO FRAMES LIST
    recordedVideoFramesBufferList.clear();
qDebug() << "here2";
    if (zeroConf) {
        //delete zeroConf;
        //zeroConf->~QZeroConf();
    }
qDebug() << "here3";
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoTCPWidget::onRecordButtonClicked(bool state)
{
    if (state && lastVideoFrame.isValid()){
        videoPlayerLabel->onPlayButtonClicked(false);
        lastVideoFrame.save(QString());
    }
    return;

    if (tcpClient && tcpClient->isConnected()) {
        if (snapShotModeFlag) {
            if (state) {
                // GRAB A COPY OF THE PACKET WE INTEND TO COPY INTO
                LAUMemoryObject packet(tcpClient->width(), tcpClient->height(), tcpClient->colors(), sizeof(float), 1);

                // COPY SCAN FROM GLWIDGET AND SEND TO VIDEO SINK OBJECT RUNNING IN A SEPARATE THREAD
                if (glWidget) {
                    glWidget->copyScan((float *)packet.pointer());
                }

                // PRESERVE THE TIME ELAPSED
                packet.setElapsed(timeStamp.elapsed());

                // EMIT THE VIDEO FRAME
                emit emitVideoFrames(packet);

                // STOP THE FRAME RECORDING
                videoPlayerLabel->onPlayButtonClicked(false);
            }
        } else {
            videoRecordingFlag = state;
            if (state) {
                // RESET RECORDING FRAME COUNTER AND TIMER AND WE CAN JUST DUMP
                // THE INCOMING VIDEO TO OUR VIDEO FRAME BUFFER LIST
                pressStartButtonTime = QTime::currentTime();
                timeStamp.restart();
            } else {
                // EMIT THE LIST OF RECORDED FRAMES OUT TO A RECEIVER OBJECT
                emit emitVideoFrames(recordedVideoFramesBufferList);

                // NOW THAT THE RECIEVER OBJECT HAS THE LIST, WE CAN DELETE IT
                recordedVideoFramesBufferList.clear();

                // RESET THE PROGRESS BAR TO SHOW NO VIDEO IS CURRENTLY RECORDED
                videoPlayerLabel->onUpdateSliderPosition(0.0f);
                videoPlayerLabel->onUpdateTimeStamp(0);
            }
        }
    } else {
        if (state) {
            videoPlayerLabel->onPlayButtonClicked(false);
            QMessageBox::warning(this, QString("Video Recorded Widget"), QString("No device available."));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoTCPWidget::onReceiveVideoFrames(LAUMemoryObject frame)
{
    // CREATE A LAUSCAN TO HOLD THE INCOMING SNAP SHOT
    LAUScan scan(frame, playbackColor);
    if (scan.isValid()) {
        scan.updateLimits();
        scan.setSoftware(QString("Lau 3D Video Recorder"));
        scan.setMake(tcpClient->make());
        scan.setModel(tcpClient->model());

        // ASK THE USER TO APPROVE THE SCAN BEFORE SAVING TO DISK
        while (scan.approveImage()) {
            // NOW LET THE USER SAVE THE SCAN TO DISK
            if (scan.save() == true) {
                break;
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoTCPWidget::onReceiveVideoFrames(QList<LAUMemoryObject> frameList)
{
    // NOW WE NEED TO PASS OUR VIDEO FRAMES TO A DEPTH VIDEO OBJECT
    // WHICH CAN THEN BRING UP A VIDEO PLAYER WIDGET TO REPLAYING
    // THE VIDEO ON SCREEN, AND GIVE THE USER THE CHANCE TO SAVE TO DISK
    if (frameList.count() > 0) {
        // CREATE THE VIDEO PLAYER WIDGET AND SEND IT THE VIDEO FRAMES
        LAU3DVideoPlayerWidget *replayWidget = new LAU3DVideoPlayerWidget(tcpClient->width(), tcpClient->height(), playbackColor, this);

        // DISPLAY THE REPLAY WIDGET SO THE USER CAN SEE THE FINE MERGE'S PROGRESS
        // SEND THE RECORDED FRAME PACKETS TO OUR REPLAY VIDEO WIDGET FOR PLAYBACK
        while (!frameList.isEmpty()) {
            replayWidget->onInsertPacket(frameList.takeFirst());
        }

        // GET THE VIEW LIMITS OF THE GPU WIDGET AND COPY THEM TO THE REPLAY WIDGET
        QVector2D xLimits = glWidget->xLimits();
        QVector2D yLimits = glWidget->yLimits();
        QVector2D zLimits = glWidget->zLimits();

        replayWidget->setLimits(xLimits.x(), xLimits.y(), yLimits.x(), yLimits.y(), zLimits.x(), zLimits.y());
        replayWidget->setAttribute(Qt::WA_DeleteOnClose);
        replayWidget->show();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoTCPWidget::onReceiveFrameBuffer(LAUMemoryObject buffer)
{
    videoFramesBufferList << buffer;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onServiceError(QZeroConf::error_t error)
{
    switch (error) {
        case QZeroConf::noError:
            qDebug() << "LAU3DVideoTCPWidget::ZeroConfServer::no error";
            break;
        case QZeroConf::serviceRegistrationFailed:
            QMessageBox::warning(NULL, QString("LAU3DVideoTCPWidget"), QString("Zero Conf Server Error: Registration failed!"), QMessageBox::Ok);
            break;
        case QZeroConf::serviceNameCollision:
            QMessageBox::warning(NULL, QString("LAU3DVideoTCPWidget"), QString("Zero Conf Server Error: Name collision!"), QMessageBox::Ok);
            break;
        case QZeroConf::browserFailed:
            QMessageBox::warning(NULL, QString("LAU3DVideoTCPWidget"), QString("Zero Conf Server Error: Browser failed!"), QMessageBox::Ok);
            break;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onAddService(QZeroConfService item)
{
#ifdef Q_OS_WIN
    tcpAddressComboBox->addItem(QString("%1::%2").arg(item.name()).arg(item.port()), item.ip().toString());
#else
    tcpAddressComboBox->addItem(QString("%1::%2").arg(item.name).arg(item.port), item.ip.toString());
#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onRemoveService(QZeroConfService item)
{
#ifdef Q_OS_WIN
    tcpAddressComboBox->removeItem(tcpAddressComboBox->findText(QString("%1::%2").arg(item.name()).arg(item.port())));
#else
    tcpAddressComboBox->removeItem(tcpAddressComboBox->findText(QString("%1::%2").arg(item.name).arg(item.port)));
#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onUpdateService(QZeroConfService item)
{
    Q_UNUSED(item);
    qDebug() << "LAU3DVideoTCPWidget :: Update service!";
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onConnectButton_clicked(bool checked)
{
    if (checked) {
        QString ipString = tcpAddressComboBox->currentData().toString();
        QStringList strings = tcpAddressComboBox->currentText().split(QString("::"));
        if (strings.count() == 2) {
            tcpClient->setIPAddress(ipString);
            tcpClient->setPortNumber(strings.last().toInt());
            tcpClient->onConnect();
            connectedButton->setEnabled(false);
        } else {
            connectedButton->setChecked(false);
        }
    } else {
        tcpClient->onDisconnect();
        connectedButton->setEnabled(false);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    if (depth.isValid()){
        if (lastVideoFrame.isNull()){
            qDebug() << "lastVideoFrame = depth;";
            lastVideoFrame = depth;
            lastVideoFrame.pointer();
        } else {
            memcpy(lastVideoFrame.constPointer(), depth.constPointer(), qMin(lastVideoFrame.length(), depth.length()));
        }
    } else if (color.isValid()){
        if (lastVideoFrame.isNull()){
            qDebug() << "lastVideoFrame = color;";
            lastVideoFrame = color;
            lastVideoFrame.pointer();
        } else {
            memcpy(lastVideoFrame.constPointer(), color.constPointer(), qMin(lastVideoFrame.length(), color.length()));
        }
    }

    // UPDATE THE TEXTURE BUFFERS IF WE HAVE AT LEAST ONE VALID POINTER
    if (depth.isValid() || color.isValid()) {
        // REPORT FRAME RATE TO THE CONSOLE
        counter++;
        if (counter >= 30) {
            qDebug() << QString("LAU3DVideoTCPWidget :: %1 fps").arg(1000.0 * (float)counter / (float)time.elapsed());
            time.restart();
            counter = 0;
        }
    }

    if (videoRecordingFlag) {
        if (recordedVideoFramesBufferList.count() < MAXRECORDEDFRAMECOUNT) {
            // GRAB A COPY OF THE PACKET WE INTEND TO COPY INTO
            LAUMemoryObject packet(tcpClient->width(), tcpClient->height(), tcpClient->colors(), sizeof(float), 1);

            // COPY SCAN FROM GLWIDGET AND SEND TO VIDEO SINK OBJECT RUNNING IN A SEPARATE THREAD
            if (glWidget) {
                glWidget->copyScan((float *)packet.pointer());
            }

            // PRESERVE THE TIME ELAPSED
            packet.setElapsed(timeStamp.elapsed());

            // HAVE THE VIDEO LABEL UPDATE ITS PROGRESS BAR SO THE USER KNOWS HOW MUCH SPACE IS LEFT
            videoPlayerLabel->onUpdateSliderPosition((float)recordedVideoFramesBufferList.count() / (float)MAXRECORDEDFRAMECOUNT);
            videoPlayerLabel->onUpdateTimeStamp(packet.elapsed());

            // ADD THE INCOMING PACKET TO OUR RECORDED FRAME BUFFER LIST
            recordedVideoFramesBufferList << packet;
        } else {
            // NOW THE BUFFER IS FULL, TELL VIDEO LABEL TO STOP RECORDING
            videoPlayerLabel->onPlayButtonClicked(false);
        }
    }

    emit emitBuffer(depth, color, mapping);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DVideoTCPWidget::onContextMenuTriggered()
{
    if (scannerWidget) {
        scannerWidget->hide();
        scannerWidget->show();
    } else {
        qDebug() << "LAU3DVideoTCPWidget::onContextMenuTriggered()";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onTcpClient_connected(bool state)
{
    if (state) {
        // REMOVE OLD GLWIDGET IF IT EXISTS
        if (glWidget) {
            ((QVBoxLayout *)(this->layout()))->removeWidget(glWidget);
            delete glWidget;
            glWidget = NULL;
        }

        // CREATE A NEW GLWIDGET TO DISPLAY THE POINT CLOUD
        glWidget = new LAU3DVideoTCPGLWidget(tcpClient->width(), tcpClient->height(), playbackColor, playbackDevice);
        glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        glWidget->setRangeLimits(tcpClient->minDistance(), tcpClient->maxDistance(), tcpClient->horizontalFieldOfViewInRadians(), tcpClient->verticalFieldOfViewInRadians());
        glWidget->setLookUpTable(LAULookUpTable(tcpClient->width(), tcpClient->height(), playbackDevice, tcpClient->horizontalFieldOfViewInRadians(), tcpClient->verticalFieldOfViewInRadians(), tcpClient->minDistance(), tcpClient->maxDistance()));
        ((QVBoxLayout *)(this->layout()))->insertWidget(1, glWidget);

        // ADD AN ACTION TO THE WIDGET SO THAT WE CAN ACCESS THE SCANNER WIDGET
        if (scannerWidget) {
            QMenu *menu = glWidget->menu();
            if (menu) {
                QAction *action = new QAction(QString("Adjust camera settings..."), NULL);
                action->setCheckable(false);
                connect(action, SIGNAL(triggered()), this, SLOT(onContextMenuTriggered()));
                menu->addAction(action);
            }
        }
        connect(this, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), tcpClient, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), Qt::QueuedConnection);
        connect(tcpClient, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), glWidget, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), Qt::QueuedConnection);
        connect(glWidget, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), this, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), Qt::QueuedConnection);

        connectedButton->setText(QString("Disconnect"));
        connectedButton->setChecked(true);
    } else {
        if (glWidget) {
            disconnect(this, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), tcpClient, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)));
            disconnect(tcpClient, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), glWidget, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)));
            disconnect(glWidget, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), this, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)));
        }
        connectedButton->setText(QString("Connect"));
        connectedButton->setChecked(false);
    }
    videoPlayerLabel->setEnabled(state);
    connectedButton->setEnabled(true);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPWidget::onTcpClient_error(QString error)
{
    // TELL THE USER ABOUT THE TCP CLIENT ERROR
    QMessageBox::warning(this, QString("TCP Client Error"), error, QMessageBox::Ok);

    // CLOSE THE CONNECTION
    tcpClient->onDisconnect();
}
