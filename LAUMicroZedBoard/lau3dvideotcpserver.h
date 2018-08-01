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

#ifndef LAU3DVIDEOTCPSERVER_H
#define LAU3DVIDEOTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "qzeroconf.h"

#include "lau3dcamera.h"

#define LAU3DVIDEOTCPSERVERPORTNUMER  11364
#define LAU3DVIDEOTCPMESSAGELENGTH    4

class LAU3DVideoTCPServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit LAU3DVideoTCPServer(LAUVideoPlaybackColor color = ColorXYZRGB, LAUVideoPlaybackDevice device = DevicePrimeSense, QObject *parent = 0);
    ~LAU3DVideoTCPServer();

    bool isConnected()
    {
        return (connected);
    }
    inline QString ipAddress()
    {
        return (clientIPAddress);
    }

public slots:
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());

protected:
    void incomingConnection(int handle);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onServicePublished();
    void onServiceError(QZeroConf::error_t error);
    void onTcpError(QAbstractSocket::SocketError error);

private:
    LAUVideoPlaybackDevice playbackDevice;          // KEEP TRACK OF CAMERA DEVICE
    LAUVideoPlaybackColor playbackColor;            // KEEP TRACK OF THE CAMERA COLOR SPACE

    QTcpSocket *socket;                             // TCP SOCKET TO HOLD THE INCOMING CONNECTION
    LAU3DCamera *camera;                            // CAMERA OBJECT
    LAU3DCameraController *cameraController;        // CAMERA CONTROLLER THREAD

    QList<LAUModalityObject> framesList;

    QString clientIPAddress;
    bool connected;
    int counter;
    QTime time;

    QZeroConf *zeroConf;

signals:
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
    void emitSNRThreshold(int val);
    void emitMTNThreshold(int val);
    void emitExposure(int val);
};

#endif // LAU3DVIDEOTCPSERVER_H
