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

#include "lau3dvideotcpclient.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAU3DVideoTCPClient::LAU3DVideoTCPClient(QString address, LAUVideoPlaybackColor color, LAUVideoPlaybackDevice device, QObject *parent) : QObject(parent), playbackColor(color), playbackDevice(device), connected(false), socket(NULL), hostString(address), numRows(0), numCols(0), portNumber(1234)
{
    socket = new QTcpSocket();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::QueuedConnection);
    connect(socket, SIGNAL(connected()), this, SLOT(onConnected()), Qt::QueuedConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()), Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAU3DVideoTCPClient::~LAU3DVideoTCPClient()
{
    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    qDebug() << "LAU3DVideoTCPClient :: ~LAU3DVideoTCPClient()";
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool LAU3DVideoTCPClient::allocateBuffers()
{
    if ((numRows * numCols) == 0) {
        emit emitError(QString("No camera!"));
        return (false);
    }

    // ALLOCATE MEMORY OBJECTS TO HOLD INCOMING VIDEO FRAMES
    for (int n = 0; n < NUMFRAMESINBUFFER; n++) {
        LAUModalityObject frame;
        if (playbackDevice == DeviceProsilicaIOS || playbackDevice == DeviceProsilicaLCG) {
            frame.depth = LAUMemoryObject(numCols, numRows, 2, sizeof(short));
            frame.color = LAUMemoryObject();
            frame.mappi = LAUMemoryObject();
        } else {
            switch (playbackColor) {
                case ColorUndefined:
                    break;
                case ColorGray:
                    frame.depth = LAUMemoryObject();
                    frame.color = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorRGB:
                    frame.depth = LAUMemoryObject();
                    frame.color = LAUMemoryObject(numCols, numRows, 3, sizeof(unsigned char));
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorRGBA:
                    frame.depth = LAUMemoryObject();
                    frame.color = LAUMemoryObject(numCols, numRows, 3, sizeof(unsigned char));
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorXYZ:
                    frame.depth = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
                    frame.color = LAUMemoryObject();
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorXYZW:
                    frame.depth = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
                    frame.color = LAUMemoryObject();
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorXYZG:
                    frame.depth = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
                    frame.color = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned char));
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorXYZRGB:
                    frame.depth = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
                    frame.color = LAUMemoryObject(numCols, numRows, 3, sizeof(unsigned char));
                    frame.mappi = LAUMemoryObject();
                    break;
                case ColorXYZWRGBA:
                    frame.depth = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
                    frame.color = LAUMemoryObject(numCols, numRows, 3, sizeof(unsigned char));
                    frame.mappi = LAUMemoryObject();
                    break;
            }
        }
        framesList << frame;
    }
    return (true);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onDisconnect()
{
    socket->disconnectFromHost();
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onConnect()
{
    if (hostString.isEmpty() == false) {
        // CLOSE THE OLD CONNECTION IF ITS STILL OPEN
        if (socket->isOpen()) {
            socket->close();
        }

        // CREATE A NEW CONNECTION
        socket->connectToHost(hostString, portNumber, QIODevice::ReadWrite);
        if (socket->waitForConnected(3000) == false) {
            emit emitConnected(false);
        }
    } else {
        emit emitConnected(false);
    }

    qDebug() << "LAU3DVideoTCPClient :: connecting to host ::" << hostString;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onConnected()
{
    qDebug() << "LAU3DVideoTCPClient :: onConnected()";

    // SET THE CONNECTED FLAG TO REFLECT WE ARE NO LONGER CONNECTED
    connected = true;

    subState = 0;
    message = QString();
    messageLength = LAU3DVIDEOTCPMESSAGELENGTH;

    // SEND MESSAGES TO THE SERVER TO GET THE CAMERA SIZE, MAKE, MODEL, AND SERIAL NUMBER
    socket->write(QString("ROWS").toLatin1());
    socket->write(QString("COLS").toLatin1());
    socket->write(QString("HFOV").toLatin1());
    socket->write(QString("VFOV").toLatin1());
    socket->write(QString("ZMIN").toLatin1());
    socket->write(QString("ZMAX").toLatin1());
    socket->write(QString("MAKE").toLatin1());
    socket->write(QString("MODL").toLatin1());
    socket->write(QString("SERL").toLatin1());
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onDisconnected()
{
    qDebug() << "LAU3DVideoTCPClient :: onDisconnected()";

    // SET THE CONNECTED FLAG TO REFLECT WE ARE NO LONGER CONNECTED
    connected = false;

    // CLEAR THE FRAME BUFFER LIST
    framesList.clear();

    // TELL THE WIDGET WE ARE DISCONNECTED
    emit emitConnected(false);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onReadyRead()
{
    while (socket->bytesAvailable() >= messageLength) {
        // MAKE SURE WE HAVE ENOUGH BYTES OF DATA TO FORM A COMPLETE MESSAGE
        if (message == QString()) {
            // READ THE INCOMING MESSAGE FROM THE CLIENT
            message = QString(socket->readLine(LAU3DVIDEOTCPMESSAGELENGTH + 1));
            messageLength = (long long)sizeof(int);
            subState = 0;
        } else if (message == QString("ROWS") && socket->bytesAvailable() >= messageLength) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN ROWS
            socket->read((char *)&numRows, sizeof(int));
            messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
            message = QString();
            qDebug() << "ROWS:" << numRows;
        } else if (message == QString("COLS") && socket->bytesAvailable() >= messageLength) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            socket->read((char *)&numCols, sizeof(int));
            messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
            message = QString();
            qDebug() << "COLS:" << numCols;
        } else if (message == QString("HFOV")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            socket->read((char *)&horizontalFieldOfView, sizeof(float));
            messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
            message = QString();
        } else if (message == QString("VFOV")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            socket->read((char *)&verticalFieldOfView, sizeof(float));
            messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
            message = QString();
        } else if (message == QString("ZMIN")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            socket->read((char *)&zMinDistance, sizeof(float));
            messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
            message = QString();
        } else if (message == QString("ZMAX")) {
            // MESSAGE ASKING FOR THE CAMERA SIZE IN COLUMNS
            socket->read((char *)&zMaxDistance, sizeof(float));
            messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
            message = QString();
        } else if (message == QString("MAKE") && socket->bytesAvailable() >= messageLength) {
            // SEE IF WE NEED TO READ THE LENGTH OF THE MAKE STRING
            // OR DO WE HAVE THE ENTIRE MAKE STRING IN THE BUFFER
            if (subState == 0) {
                socket->read((char *)&messageLength, sizeof(int));
                subState = 1;
            } else {
                if (messageLength > 0) {
                    makeString = QString(socket->readLine(messageLength + 1));
                }
                messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
                message = QString();
            }
        } else if (message == QString("MODL") && socket->bytesAvailable() >= messageLength) {
            // SEE IF WE NEED TO READ THE LENGTH OF THE MODEL STRING
            // OR DO WE HAVE THE ENTIRE MAKE STRING IN THE BUFFER
            if (subState == 0) {
                socket->read((char *)&messageLength, sizeof(int));
                subState = 1;
            } else {
                if (messageLength > 0) {
                    modelString = QString(socket->readLine(messageLength + 1));
                }
                messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
                message = QString();
            }
        } else if (message == QString("SERL") && socket->bytesAvailable() >= messageLength) {
            // SEE IF WE NEED TO READ THE LENGTH OF THE SERIAL STRING
            // OR DO WE HAVE THE ENTIRE MAKE STRING IN THE BUFFER
            if (subState == 0) {
                socket->read((char *)&messageLength, sizeof(int));
                subState = 1;
            } else {
                if (messageLength > 0) {
                    serialString = QString(socket->readLine(messageLength + 1));
                }
                messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
                message = QString();

                // NOW WE CAN ALLOCATE BUFFERS AND SET UP THE GLFILTER AND GLDISPLAY
                if (allocateBuffers()) {
                    socket->write(QString("GRAB").toLatin1());

                    // TELL THE WIDGET WE ARE CONNECTED
                    emit emitConnected(true);
                }
            }
        } else if (message == QString("GRAB") && socket->bytesAvailable() >= messageLength) {
            static long long bytesLeftToRead = 0, bytesSoFarRead = 0;
            static LAUMemoryObject depth, color, mappi;

            // SEE IF WE NEED TO READ THE LENGTH OF THE DATA BUFFER
            // OR DO WE HAVE THE ENTIRE MAKE BUFFER IN THE SOCKET
            if (subState == 0) {
                socket->read((char *)&bytesLeftToRead, sizeof(int));
                bytesSoFarRead = 0;
                messageLength = 1;
                subState = 1;

                // GET THE NEXT AVAILABLE FRAME BUFFERS FROM OUR FRAME LIST
                if (framesList.count() > 0) {
                    LAUModalityObject frame = framesList.takeFirst();
                    depth = frame.depth;
                    color = frame.color;
                    mappi = frame.mappi;
                }
            } else {
                if (bytesSoFarRead < (long long)depth.length()) {
                    long long spaceAvailable = depth.length() - bytesSoFarRead;
                    long long bytesAvailable = qMin(spaceAvailable, socket->bytesAvailable());
                    socket->read((char *)(depth.constPointer() + bytesSoFarRead), bytesAvailable);
                    bytesSoFarRead += bytesAvailable;
                } else if (bytesSoFarRead < (long long)(depth.length() + color.length())) {
                    long long spaceAvailable = depth.length() + color.length() - bytesSoFarRead;
                    long long bytesAvailable = qMin(spaceAvailable, socket->bytesAvailable());
                    socket->read((char *)(color.constPointer() + (bytesSoFarRead - depth.length())), bytesAvailable);
                    bytesSoFarRead += bytesAvailable;
                } else {
                    long long spaceAvailable = depth.length() + color.length() + mappi.length() - bytesSoFarRead;
                    long long bytesAvailable = qMin(spaceAvailable, socket->bytesAvailable());
                    socket->read((char *)(color.constPointer() + (bytesSoFarRead - depth.length() - color.length())), bytesAvailable);
                    bytesSoFarRead += bytesAvailable;
                }
            }

            // SEE IF WE HAVE OUR COMPLETE SET OF FRAME BUFFERS FROM THE SERVER
            if (bytesSoFarRead == bytesLeftToRead) {
                // SEND A MESSAGE TO THE SERVER TO SEND THE NEXT FRAME OF VIDEO TO US
                socket->write(QString("GRAB").toLatin1());
                messageLength = LAU3DVIDEOTCPMESSAGELENGTH;
                message = QString();

                // EMIT THE CURRENT BUFFERS TO THE DISPLAY
                emit emitBuffer(depth, color, mappi);
            }
        }
    }
    return;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            emit emitError(QString("LAU3DVideoTCPClient :: Remote host closed error!"));
            break;
        case QAbstractSocket::HostNotFoundError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            emit emitError(QString("LAU3DVideoTCPClient :: Host not found error!"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            emit emitError(QString("LAU3DVideoTCPClient :: Connection refused error!"));
            break;
        default:
            emit emitError(QString("LAU3DVideoTCPClient :: Default error!"));
            break;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onSetExposure(int val)
{
    // SEND A MESSAGE WITH THE EXPOSURE SETTING
    if (socket->isOpen()) {
        QString message = QString("EXPO");
        socket->write(message.toLatin1());
        socket->write((const char *)&val, sizeof(int));

        qDebug() << message << val;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onSetMTNThreshold(int val)
{
    // SEND A MESSAGE WITH THE MOTION FILTER THRESHOLD
    if (socket->isOpen()) {
        QString message = QString("MOTN");
        socket->write(message.toLatin1());
        socket->write((const char *)&val, sizeof(int));
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onSetSNRThreshold(int val)
{
    // SEND A MESSAGE WITH THE SNR FILTER THRESHOLD
    if (socket->isOpen()) {
        QString message = QString("SNRT");
        socket->write(message.toLatin1());
        socket->write((const char *)&val, sizeof(int));
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAU3DVideoTCPClient::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    // CONSTRUCT A VIDEO MEMORY OBJECT FROM THE INCOMING MEMORY OBJECTS
    LAUModalityObject frame(depth, color, mapping);

    // SEE IF WE SHOULD KEEP THIS PARTICULAR FRAME
    if (frame.isAnyValid()) {
        framesList << frame;
    }
}
