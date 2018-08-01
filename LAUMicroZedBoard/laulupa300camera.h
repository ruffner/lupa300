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

#ifndef LAULUPA300CAMERA_H
#define LAULUPA300CAMERA_H

#include <QList>
#include <QDebug>
#include <QString>
#include <QObject>
#include <QTime>
#include <QTimer>
#include <QtCore>
#include <QFile>


#include <sys/types.h>
#include <linux/hdreg.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#ifdef Q_OS_WIN
#include "uEye.h"

#define IDSALLOCATEFRAMEBUFFERS
#define IDSNUMFRAMESINBUFFER        10
#define IDSWAITFORFRAMEDELAY     10000
#else
#define LUPA300_WIDTH  640
#define LUPA300_HEIGHT 480
#endif

#include "lau3dcamera.h"

class LAULUPA300Camera : public LAU3DCamera
{
    Q_OBJECT

public:
    explicit LAULUPA300Camera(LAUVideoPlaybackColor color, QObject *parent = 0);
    ~LAULUPA300Camera();

    bool isValid() const
    {
        return (isConnected);
    }
    bool hasDepth() const
    {
        return (hasDepthVideo);
    }
    bool hasColor() const
    {
        return (hasColorVideo);
    }

    bool hasMapping() const
    {
        return (false);
    }

    LAUVideoPlaybackDevice device() const
    {
        return (DeviceIDS);
    }

    unsigned short maxIntensityValue() const
    {
        return (1 << bitDpth);
    }

    QString error() const
    {
        return (errorString);
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
        return ((double)zMinDistance);
    }
    float maxDistance() const
    {
        return ((double)zMaxDistance);
    }

    unsigned int depthWidth() const
    {
        return (numCols);
    }
    unsigned int depthHeight() const
    {
        return (numRows);
    }
    unsigned int colorWidth() const
    {
        return (numCols);
    }
    unsigned int colorHeight() const
    {
        return (numRows);
    }

    LAUMemoryObject colorMemoryObject() const;
    LAUMemoryObject depthMemoryObject() const;
    LAUMemoryObject mappiMemoryObject() const;

    QList<LAUVideoPlaybackColor> playbackColors();

public slots:
    void onUpdateExposure(int microseconds);
    void onUpdateBuffer(LAUMemoryObject depth = LAUMemoryObject(), LAUMemoryObject color = LAUMemoryObject(), LAUMemoryObject mapping = LAUMemoryObject());
    void onUpdateBuffer(LAUMemoryObject buffer, int index, void *userData)
    {
        emit emitBuffer(buffer, index, userData);
    }

private:
    static bool libraryInitializedFlag;

    LAUVideoPlaybackDevice playbackDevice;

    unsigned short zMinDistance;
    unsigned short zMaxDistance;
    float horizontalFieldOfView;
    float verticalFieldOfView;
    bool hasDepthVideo, hasColorVideo;
    bool isConnected;

    // DECLARE POINTERS TO PRIMESENSE SENSOR OBJECTS
    unsigned int numAvailableCameras;

    unsigned int numRows;
    unsigned int numCols;
    unsigned int numFrms;
    unsigned int bitDpth;

    int fd, fdm;
    FILE *fp;		//used for fopen(). fread(). fclose().
    unsigned char counter;
    unsigned char cfgn;
    unsigned char expo;

    QList<LAUMemoryObject> frameList;

#ifdef Q_OS_WIN
    HIDS idsCamera;
    int defaultID[IDSNUMFRAMESINBUFFER];
    char *defaultBuffer[IDSNUMFRAMESINBUFFER];
    unsigned int energy[IDSNUMFRAMESINBUFFER];
#else
    void *idsCamera;
#endif
    unsigned int startingFrameIndex;

    QString errorMessages(int err);
    QStringList cameraList();

    void initialize();
    bool setSynchronization();
    bool disconnectFromHost();
    bool connectToHost(QString);
};

#endif // LAULUPA300CAMERA_H
