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

#ifndef LAU3DVIDEOTCPCLIENT_H
#define LAU3DVIDEOTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

#include "lau3dcamera.h"
#include "lau3dvideotcpglfilter.h"

#define LUPA300_FRAMES 8

class LAU3DVideoTCPClient : public QObject
{
    Q_OBJECT

public:
    explicit LAU3DVideoTCPClient(QString address, LAUVideoPlaybackColor color = ColorXYZRGB, LAUVideoPlaybackDevice device = DevicePrimeSense, QObject *parent = 0);
    ~LAU3DVideoTCPClient();

    bool isConnected() const
    {
        return (connected);
    }

    unsigned int height() const
    {
        return (numRows);
    }

    unsigned int width() const
    {
        return (numCols);
    }

    unsigned int colors() const
    {
        switch (playbackColor) {
            case ColorUndefined:
                return (0);
            case ColorGray:
                return (1);
            case ColorRGB:
                return (3);
            case ColorRGBA:
                return (4);
            case ColorXYZ:
                return (3);
            case ColorXYZW:
                return (4);
            case ColorXYZG:
                return (4);
            case ColorXYZRGB:
                return (6);
            case ColorXYZWRGBA:
                return (8);
        }
        return (0);
    }

    float horizontalFieldOfViewInRadians() const
    {
        return (horizontalFieldOfView);
    }

    float verticalFieldOfViewInRadians() const
    {
        return (verticalFieldOfView);
    }

    float horizontalFieldOfViewInDegrees() const
    {
        return (horizontalFieldOfView * 180.0 / 3.14159265359);
    }

    float verticalFieldOfViewInDegrees() const
    {
        return (verticalFieldOfView * 180.0 / 3.14159265359);
    }

    float minDistance() const
    {
        return (zMinDistance);
    }

    float maxDistance() const
    {
        return (zMaxDistance);
    }

    void setIPAddress(QString string)
    {
        hostString = string;
    }

    void setPortNumber(int val)
    {
        portNumber = val;
    }

    QString make() const
    {
        return (makeString);
    }

    QString model() const
    {
        return (modelString);
    }

    QString serial() const
    {
        return (serialString);
    }

public slots:
    void onConnect();
    void onDisconnect();
    void onSetExposure(int val);
    void onSetMTNThreshold(int val);
    void onSetSNRThreshold(int val);
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());

private:
    LAUVideoPlaybackColor playbackColor;
    LAUVideoPlaybackDevice playbackDevice;

    bool connected;
    QTcpSocket *socket;
    QString hostString;

    QString message;
    int subState;
    long long messageLength;

    QString makeString;
    QString modelString;
    QString serialString;

    unsigned int numRows, numCols;
    QList<LAUModalityObject> framesList;
    int portNumber;
    QTime time;

    float zMinDistance;
    float zMaxDistance;
    float horizontalFieldOfView;
    float verticalFieldOfView;

    bool allocateBuffers();

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);

signals:
    void emitError(QString error);
    void emitConnected(bool state);
    void emitBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping);
};

#endif // LAU3DVIDEOTCPCLIENT_H
