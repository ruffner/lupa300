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

#include "laulupa300camera.h"

bool LAULUPA300Camera::libraryInitializedFlag = false;

QFile file("/dev/xillybus_read_8");
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULUPA300Camera::LAULUPA300Camera(LAUVideoPlaybackColor color, QObject *parent) : LAU3DCamera(color, parent)
{
    initialize();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULUPA300Camera::initialize()
{
    // ASSUME SOMETHING IS GOING TO GO WRONG UNTIL EVERYTHING WORKS FINE
    numRows = LUPA300_HEIGHT;
    numCols = LUPA300_WIDTH;
    numFrms = 6;
    bitDpth = 10;

    idsCamera = 0;
    isConnected = true;
    startingFrameIndex = 0;
    hasDepthVideo = false;
    hasColorVideo = true;
    playbackDevice = Device2DCamera;

#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
    //fp = fopen("/dev/xillybus_read_8", "r");
    fd = open("/dev/xillybus_read_32", O_RDONLY);
    if (fd < 0) {
        qDebug() << "Failed to open file.";
        isConnected = false;
        hasDepthVideo = false;
        hasColorVideo = false;
    }

    fdm = open("/dev/xillybus_mem_8", O_WRONLY);
    if (fdm < 0) {
        qDebug() << "Failed to open xillybus_mem_8";
    }

    /*************************** Upload some register configuration *********************/
    cfgn = 0x00;	//0x73E5
    if (lseek(fdm, 4, SEEK_SET) < 0) {
        qDebug() << "Failed to seek";
        exit(1);
    }
    int rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    cfgn = 0x28;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    //********************* No.2 ***********************************/
    cfgn = 0x00;	//0x427D
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    cfgn = 0x28;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    //******************** No.3 ***********************************/
    cfgn = 0x52;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    cfgn = 0xF5;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    //******************* No.4  *********************************/
    cfgn = 0xCF;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    cfgn = 0xE1;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    //******************* No.5 *************************************/
    cfgn = 0x08;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
    cfgn = 0xE9;
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }

    /******************* Total No. ********************************/
    cfgn = 0x01;	//0x04;
    if (lseek(fdm, 30, SEEK_SET) < 0) {
        qDebug() << "Failed to seek";
        exit(1);
    }
    rcwf = write(fdm, &cfgn, 1);
    if (rcwf < 0) {
        qDebug() << "Write mem error.";
    }
#endif
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULUPA300Camera::~LAULUPA300Camera()
{
    // DISCONNECT FROM CAMERA
    if (isConnected) {
        disconnectFromHost();

#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
        int rb = close(fd);
        //fclose(fp);
        //file.close();
        if (rb < 0) {
            qDebug() << "Failed to close file.";
        } else {
            qDebug() << "File closed.";
        }

        rb = close(fdm);
        if (rb < 0) {
            qDebug() << "Failed to close file.";
        } else {
            qDebug() << "File closed.";
        }
#endif
    }
    qDebug() << QString("LAULUPA300Camera::~LAULUPA300Camera()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<LAUVideoPlaybackColor> LAULUPA300Camera::playbackColors()
{
    QList<LAUVideoPlaybackColor> list;
    list << ColorGray << ColorRGB << ColorRGBA;
    return (list);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAULUPA300Camera::colorMemoryObject() const
{
    switch (playbackColor) {
        case ColorGray:
        case ColorXYZG:
        case ColorRGB:
        case ColorXYZRGB:
        case ColorRGBA:
        case ColorXYZWRGBA:
            if (bitDpth == 8) {
                return (LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned char), LUPA300_FRAMES));
            } else {
                return (LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short), LUPA300_FRAMES));
            }
        case ColorUndefined:
        case ColorXYZ:
        case ColorXYZW:
            return (LAUMemoryObject());
    }
    return (LAUMemoryObject());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAULUPA300Camera::depthMemoryObject() const
{
    switch (playbackColor) {
        case ColorXYZ:
        case ColorXYZW:
        case ColorXYZG:
        case ColorXYZRGB:
        case ColorXYZWRGBA:
            return (LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short), 8));
        case ColorRGBA:
        case ColorRGB:
        case ColorGray:
        case ColorUndefined:
            return (LAUMemoryObject());
    }
    return (LAUMemoryObject());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAULUPA300Camera::mappiMemoryObject() const
{
    return (LAUMemoryObject());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QStringList LAULUPA300Camera::cameraList()
{
    QStringList stringList;
    numAvailableCameras = 0;
    return (stringList);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool LAULUPA300Camera::connectToHost(QString hostString)
{
    counter = 0;
    Q_UNUSED(hostString);
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAULUPA300Camera::setSynchronization()
{
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAULUPA300Camera::disconnectFromHost()
{
    isConnected = false;
    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULUPA300Camera::onUpdateExposure(int microseconds)
{
    // SET THE CAMERA'S EXPOSURE
    if (isConnected) {
        if (microseconds > 996) {
            qDebug() << " Exposure overflow!!!";
        } else {
#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
            //expo = 64 + (997 - microseconds)/256;		//exposure time
            //expo = 0xcf;					//gain
            expo = 0x80;
            if (lseek(fdm, 6, SEEK_SET) < 0) {
                qDebug() << "Failed to seek";
                exit(1);
            }
            int rcwo = write(fdm, &expo, 1);
            if (rcwo < 0) {
                qDebug() << "Write mem error.";
            }
            //expo = (unsigned char)((997 - microseconds) & 0x00ff);	//exposure time
            //expo = 224 + (microseconds >> 8);				//gain
            expo = 75 + (microseconds >> 7);
            rcwo = write(fdm, &expo, 1);
            if (rcwo < 0) {
                qDebug() << "Write mem error.";
            }
#endif
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULUPA300Camera::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
    if (color.isValid()) {
        // SET THE DATA VALUES TO SHOW A CHANGING GRAY LEVEL
        memset(color.constPointer(), 0, color.length());

        // UPDATE THE FRAME COUNTER TO REFLECT THE NEW FRAMES
        counter += color.frames();

#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
        if (lseek(fdm, 3, SEEK_SET) < 0) {
            qDebug() << "Failed to seek";
            exit(1);
        }

        int rcw = write(fdm, &counter, 1);
        if (rcw < 0) {
            qDebug() << "Write mem error.";
        }

        // ITERATE THROUGH EACH OF 12 FRAMES OF VIDEO
        for (unsigned int frm = 0; frm < color.frames(); frm++) {
            // ITERATE THROUGH EACH ROW OF THE IMAGE SENSOR
            for (unsigned int row = 0; row < color.height(); row++) {
                // ITERATE THROUGH EACH COLUMN OF THE CURRENT SCAN LINE
                char *buffer = (char *)color.constScanLine(row, frm);
                int rc = read(fd, buffer, 1280);

                if (rc < 0) {
                    qDebug() << "Error occurs, failed to read";
                    break;
                } else if (rc == 1280) {
                    ;
                } else {
                    //nobr += rc;
                    do {
                        int rf = read(fd, buffer + rc, 1280 - rc);
                        rc += rf;
                    } while (rc < 1280);
                }
            }
        }
#else
        for (unsigned int frm = 0; frm < color.frames(); frm++) {
            if (frm % 2 == 0) {
                for (unsigned int row = 0; row < color.height(); row++) {
                    unsigned short *buffer = (unsigned short *)color.constScanLine(row, frm);
                    for (unsigned int col = 0; col < color.width(); col++) {
                        buffer[col] = 16 * qMin(col % 64, row % 48);
                    }
                }
            } else {
                for (unsigned int row = 0; row < color.height(); row++) {
                    unsigned short *buffer = (unsigned short *)color.constScanLine(row, frm);
                    for (unsigned int col = 0; col < color.width(); col++) {
                        buffer[col] = 1023 - 16 * qMin(col % 64, row % 48);
                    }
                }
            }
        }
#endif
    }

    // EMIT THE BUFFERS NOW THAT THEY HAVE BEEN UPDATED WITH NEW DATA
    emit emitBuffer(depth, color, mapping);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QString LAULUPA300Camera::errorMessages(int err)
{
    switch (err) {
        case 0:
        default:
            return QString("Error Unknown");
    }
}
