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

#include "lau3dvideotcpserver.h"
#include "laulupa300camera.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAU3DVideoTCPServer::LAU3DVideoTCPServer(LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QObject *parent) : QTcpServer(parent), playbackDevice(device), playbackColor(color), socket(NULL), camera(NULL), cameraController(NULL), connected(false), counter(0), zeroConf(NULL)
{
    camera = new LAULUPA300Camera(playbackColor);
    if (camera && camera->isValid()) {
        // ALLOCATE MEMORY OBJECTS TO HOLD INCOMING VIDEO FRAMES
        for (int n = 0; n < NUMFRAMESINBUFFER; n++) {
            LAUModalityObject frame;
            frame.depth = camera->depthMemoryObject();
            frame.color = camera->colorMemoryObject();
            frame.mappi = camera->mappiMemoryObject();
            framesList << frame;
        }

        // MAKE THE CAMERA TO OBJECT CONNECTIONS
        connect(this, SIGNAL(emitExposure(int)), camera, SLOT(onUpdateExposure(int)), Qt::QueuedConnection);
        connect(this, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), camera, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), Qt::QueuedConnection);
        connect(camera, SIGNAL(emitBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), this, SLOT(onUpdateBuffer(LAUMemoryObject, LAUMemoryObject, LAUMemoryObject)), Qt::QueuedConnection);

        // CREATE A THREAD TO HOST THE CAMERA CONTROLLER
        cameraController = new LAU3DCameraController(camera);
        connect(camera, SIGNAL(emitError(QString)), cameraController, SLOT(onError(QString)));

        // MAKE CONNECTIONS BETWEEN THIS OBJECT AND THE BONJOUR SERVICE OBJECT
        zeroConf = new QZeroConf();
        connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));
        connect(zeroConf, SIGNAL(servicePublished()), this, SLOT(onServicePublished()));

        // START THE BONJOUR SERVICE
        zeroConf->startServicePublish(QString("LAU3DVideoTCPServer").toUtf8(), "_qtzeroconf_test._tcp", "local", LAU3DVIDEOTCPSERVERPORTNUMER);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAU3DVideoTCPServer::~LAU3DVideoTCPServer()
{
    if (this->isListening()) {
        this->close();
    }

    if (socket) {
        socket->deleteLater();
        socket = NULL;
    }

    // DELETE THE CAMERA CONTROLLER OR THE CAMERA IF EITHER EXIST
    if (cameraController) {
        delete cameraController;
    } else if (camera) {
        delete camera;
    }

    if (zeroConf) {
        delete zeroConf;
    }
    qDebug() << "LAU3DVideoTCPServer :: ~LAU3DVideoTCPServer()";
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPServer::onServicePublished()
{
    if (!this->listen(QHostAddress::Any, LAU3DVIDEOTCPSERVERPORTNUMER)) {
        qDebug() << "LAU3DVideoTCPServer :: Error trying to listen for incoming connections!";
    } else {
        qDebug() << "LAU3DVideoTCPServer :: Listening for new connections!";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPServer::onServiceError(QZeroConf::error_t error)
{
    switch (error) {
        case QZeroConf::noError:
            qDebug() << "LAU3DVideoTCPServer::ZeroConfServer::no error";
            break;
        case QZeroConf::serviceRegistrationFailed:
            qDebug() << QString("Zero Conf Server Error: Registration failed!");
            break;
        case QZeroConf::serviceNameCollision:
            qDebug() << QString("Zero Conf Server Error: Name collision!");
            break;
        case QZeroConf::browserFailed:
            qDebug() << QString("Zero Conf Server Error: Browser failed!");
            break;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
void LAU3DVideoTCPServer::incomingConnection(qintptr handle)
#else
void LAU3DVideoTCPServer::incomingConnection(int handle)
#endif
{
    // SET THE CONNECTED FLAG HIGH
    connected = true;

    // CALL THE BASE CLASS TO USE ITS DEFAULT METHOD
    QTcpServer::incomingConnection(handle);

    // NOW LET'S CREATE THE SOCKET FOR MESSAGE HANDLING TO THE TRACKER
    socket = this->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()), Qt::QueuedConnection);

    QHostAddress hostAddress = socket->peerAddress();
    clientIPAddress = hostAddress.toString();
    qDebug() << "LAU3DVideoTCPServer :: Accepting incoming connection from " << clientIPAddress;

    // STOP LISTENING FOR NEW CONNECTIONS
    this->close();
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPServer::onReadyRead()
{
    // MAKE SURE WE HAVE ENOUGH BYTES OF DATA TO FORM A COMPLETE MESSAGE
    while (socket && socket->bytesAvailable() >= LAU3DVIDEOTCPMESSAGELENGTH) {
        // READ THE INCOMING MESSAGE FROM THE CLIENT
        QString message = QString(socket->readLine(LAU3DVIDEOTCPMESSAGELENGTH + 1));

        if (message == QString("GRAB")) {
            // MESSAGE SAYING TO GRAB ANOTHER FRAME AND SEND IT
            if (isConnected()) {
                if (framesList.count() > 0) {
                    LAUModalityObject frame = framesList.takeFirst();
                    emit emitBuffer(frame.depth, frame.color, frame.mappi);
                }
            }
        } else if (message == QString("ROWS")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN ROWS
            int rows = camera->height();
            socket->write(message.toLatin1());
            socket->write((const char *)&rows, sizeof(int));
        } else if (message == QString("COLS")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            int cols = camera->width();
            socket->write(message.toLatin1());
            socket->write((const char *)&cols, sizeof(int));
        } else if (message == QString("HFOV")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            float hfov = camera->horizontalFieldOfViewInRadians();
            socket->write(message.toLatin1());
            socket->write((const char *)&hfov, sizeof(float));
        } else if (message == QString("VFOV")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            float vfov = camera->verticalFieldOfViewInRadians();
            socket->write(message.toLatin1());
            socket->write((const char *)&vfov, sizeof(float));
        } else if (message == QString("ZMIN")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            float zmin = camera->minDistance();
            socket->write(message.toLatin1());
            socket->write((const char *)&zmin, sizeof(float));
        } else if (message == QString("ZMAX")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            float zmax = camera->maxDistance();
            socket->write(message.toLatin1());
            socket->write((const char *)&zmax, sizeof(float));
        } else if (message == QString("MAKE")) {
            // MESSAGE ASKING FOR THE CAMERA MAKE STRING
            QString string = camera->make();
            int bytes = string.toLatin1().length();
            socket->write(message.toLatin1());
            socket->write((const char *)&bytes, sizeof(int));
            socket->write(string.toLatin1());
        } else if (message == QString("MODL")) {
            // MESSAGE ASKING FOR THE CAMERA MODEL STRING
            QString string = camera->model();
            int bytes = string.toLatin1().length();
            socket->write(message.toLatin1());
            socket->write((const char *)&bytes, sizeof(int));
            socket->write(string.toLatin1());
        } else if (message == QString("SERL")) {
            // MESSAGE ASKING FOR THE CAMERA SERIAL STRING
            QString string = camera->serial();
            int bytes = string.toLatin1().length();
            socket->write(message.toLatin1());
            socket->write((const char *)&bytes, sizeof(int));
            socket->write(string.toLatin1());
        } else if (message == QString("EXPO")) {
            // MESSAGE GIVING US THE NEW CAMERA EXPOSURE
            int exposure;
            socket->read((char *)&exposure, sizeof(int));
            emit emitExposure(exposure);
        } else if (message == QString("SNRT")) {
            // MESSAGE GIVING US THE NEW CAMERA EXPOSURE
            int threshold;
            socket->read((char *)&threshold, sizeof(int));
            emit emitSNRThreshold(threshold);
        } else if (message == QString("MOTN")) {
            // MESSAGE GIVING US THE NEW CAMERA EXPOSURE
            int threshold;
            socket->read((char *)&threshold, sizeof(int));
            emit emitMTNThreshold(threshold);
        }
    }

    return;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPServer::onDisconnected()
{
    // DELETE SOCKET TO CLOSE CONNECTION
    if (socket) {
        socket->readAll();
        socket->deleteLater();
        socket = NULL;
    }
    connected = false;

    if (!this->isListening()) {
        if (!this->listen(QHostAddress::Any, LAU3DVIDEOTCPSERVERPORTNUMER)) {
            qDebug() << "LAU3DVideoTCPServer :: Error trying to listen for incoming connections!";
        }
    }

    qDebug() << "LAU3DVideoTCPServer :: Closing connection from " << clientIPAddress;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPServer::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "LAU3DVideoTCPServer :: Remote host closed error!";
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "LAU3DVideoTCPServer :: Host not found error!";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "LAU3DVideoTCPServer :: Connection refused error!";
            break;
        default:
            qDebug() << "LAU3DVideoTCPClient :: Default error!";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPServer::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    // TRANSMIT THE INCOMING VIDEO FRAME TO THE CLIENT
    if (socket && socket->isOpen()) {
        // IF THE SOCKET IS READY TO SEND MORE DATA THEN TRANSMIT THE INCOMING VIDEO
        if (socket->bytesToWrite() < 1024) {
            if (playbackDevice == DeviceProsilicaIOS || playbackDevice == DeviceProsilicaLCG) {
                if (depth.isValid()) {
                    // FOR PROSILICA, WE ONLY NEED TO SEND THE FOUR QUADRATURE COMPONENTS
                    // EACH OF WHICH IS OF TYPE SIGNED SHORT
                    unsigned int numBytesToWrite = depth.width() * depth.height() * 4 * sizeof(short);
                    if (depth.length() < numBytesToWrite) {
                        socket->write(QString("GRAB").toLatin1());
                        socket->write((const char *)&numBytesToWrite, sizeof(int));
                        socket->write((const char *)depth.constPointer(), numBytesToWrite);
                    }
                }
            } else {
                // FOR RGB+D CAMERAS, SEND WHAT'S EVER IN EACH BUFFER
                unsigned int numBytesToWrite = depth.length() + color.length() + mapping.length();
                socket->write(QString("GRAB").toLatin1());
                socket->write((const char *)&numBytesToWrite, sizeof(int));
                if (depth.isValid()) {
                    socket->write((const char *)depth.constPointer(), depth.length());
                }
                if (color.isValid()) {
                    socket->write((const char *)color.constPointer(), color.length());
                }
                if (mapping.isValid()) {
                    socket->write((const char *)mapping.constPointer(), mapping.length());
                }
            }
        } else {
            // EMIT A SET OF BUFFERS FROM OUR LIST TO GRAB THE NEXT
            // FRAME WHILE WE WAIT ON THE SOCKET TO BECOME FREE AGAIN
            if (isConnected()) {
                if (framesList.count() > 0) {
                    LAUModalityObject frame = framesList.takeFirst();
                    emit emitBuffer(frame.depth, frame.color, frame.mappi);
                }
            }
        }
    }

    // CONSTRUCT A VIDEO MEMORY OBJECT FROM THE INCOMING MEMORY OBJECTS
    LAUModalityObject frame(depth, color, mapping);

    // SEE IF WE SHOULD KEEP THIS PARTICULAR FRAME
    if (frame.isAnyValid()) {
        framesList << frame;
    }

    // UPDATE THE TEXTURE BUFFERS IF WE HAVE AT LEAST ONE VALID POINTER
    if (depth.isValid() || color.isValid()) {
        // REPORT FRAME RATE TO THE CONSOLE
        counter++;
        if (counter >= 30) {
            qDebug() << QString("LAU3DVideoTCPServer :: %1 fps").arg(1000.0 * (float)counter / (float)time.elapsed());
            time.restart();
            counter = 0;
        }
    }
}
