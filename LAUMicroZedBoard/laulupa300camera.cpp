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

#ifdef FIFO_TEST
    unsigned int fifo_size = 16384;

    if (fifo_init(&fifo, fifo_size)) {
      qDebug() << "Failed to init fifo";
      exit(1);
    }
#endif


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

#ifdef FIFO_TEST
        fifo_destroy(&fifo);
#endif
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
qDebug() << "entering onUpdateBuffer";
    if (color.isValid()) {
        // SET THE DATA VALUES TO SHOW A CHANGING GRAY LEVEL
        memset(color.constPointer(), 0, color.length());

        // UPDATE THE FRAME COUNTER TO REFLECT THE NEW FRAMES
        counter=1;//counter += color.frames();

#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
        if (lseek(fdm, 3, SEEK_SET) < 0) {
            qDebug() << "Failed to seek";
            exit(1);
        }

        int rcw = write(fdm, &counter, 1);
        if (rcw < 0) {
            qDebug() << "Write mem error.";
        }
#ifdef FIFO_TEST
        int do_bytes, read_bytes;
        struct xillyinfo info;
        unsigned char *buf;

        while (1) {
          do_bytes = fifo_request_write(&fifo, &info);

          if (do_bytes == 0){
            //return NULL;
            break;
          }

          for (buf = (unsigned char *)info.addr; do_bytes > 0;
           buf += read_bytes, do_bytes -= read_bytes) {

            read_bytes = read(fd, buf, do_bytes);

            if ((read_bytes < 0) && (errno != EINTR)) {
                perror("read() failed");
                break;
                //return NULL;
            }

            if (read_bytes == 0) {
              // Reached EOF. Quit without complaining.
              fifo_done(&fifo);
              break;
              //return NULL;
            }

            if (read_bytes < 0) { // errno is EINTR
                qDebug() << "read bytes less than 0";
              read_bytes = 0;
              continue;
            }

            fifo_wrote(&fifo, read_bytes);
            qDebug() << "read " << read_bytes << " bytes";
          }
        }
#else
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
		qDebug() << "didnt read all the bytes rc= " << rc;
                    //nobr += rc;
                    do {
qDebug() << "  before extra byte read";
qDebug() << "   calling read(fd, buffer + " << rc << ", 1280 - " << rc << ");";
                        int rf = read(fd, buffer + rc, 1280 - rc);
qDebug() << "  after extra byte read";
                        if( rf<0 ){
				qDebug() << "read error in frame " << counter+frm;
			}  else {
qDebug() << "   read a remaining " << rf << " bytes";
			rc += rf;
			}
                    } while (rc < 1280);
                }
            }
        }

#endif

        if (lseek(fdm, 3, SEEK_SET) < 0) {
            qDebug() << "Failed to seek";
            exit(1);
        }
        counter = 0;
        rcw = write(fdm, &counter, 1);
        if (rcw < 0) {
            qDebug() << "Write mem error.";
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

qDebug() << "leaving onUpdateBuffer. emitting buffer";
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

#ifdef FIFO_TEST
// FROM fifo.c XILLYBUS DEMO CODE
/*********************************************************************
 *                                                                   *
 *                 A P I   F U N C T I O N S                         *
 *                                                                   *
 *********************************************************************/

// IMPORTANT:
// =========
//
// NEITHER of the fifo_* functions is reentrant. Only one thread should have
// access to any set of them. This is pretty straightforward when one thread
// writes and one thread reads from the FIFO.
//
// Also make sure that fifo_drained() and fifo_wrote() are NEVER called with
// req_bytes larger than what their request-counterparts RETURNED, or
// things will go crazy pretty soon.


int LAULUPA300Camera::fifo_init(struct xillyfifo *fifo,unsigned int size)
{

  fifo->baseaddr = NULL;
  fifo->size = 0;
  fifo->bytes_in_fifo = 0;
  fifo->read_position = 0;
  fifo->write_position = 0;
  fifo->read_total = 0;
  fifo->write_total = 0;
  fifo->done = 0;

  if (sem_init(&fifo->read_sem, 0, 0) == -1)
    return -1; // Fail!

  if (sem_init(&fifo->write_sem, 0, 1) == -1)
    return -1;

  fifo->baseaddr = (unsigned char *)malloc(size);

  if (!fifo->baseaddr)
    return -1;

  if (mlock(fifo->baseaddr, size)) {
    unsigned int i;
    unsigned char *buf = fifo->baseaddr;

    fprintf(stderr, "Warning: Failed to lock RAM, so FIFO's memory may swap to disk.\n"
        "(You may want to use ulimit -l)\n");

    // Write something every 1024 bytes (4096 should be OK, actually).
    // Hopefully all pages are in real RAM after this. Better than nothing.

    for (i=0; i<size; i+=1024)
      buf[i] = 0;
  }

  fifo->size = size;

  return 0; // Success
}

void LAULUPA300Camera::fifo_done(struct xillyfifo *fifo)
{
  fifo->done = 1;
  sem_post(&fifo->read_sem);
  sem_post(&fifo->write_sem);
}

void LAULUPA300Camera::fifo_destroy(struct xillyfifo *fifo)
{
  if (!fifo->baseaddr)
    return; // Better safe than SEGV

  munlock(fifo->baseaddr, fifo->size);
  free(fifo->baseaddr);

  sem_destroy(&fifo->read_sem);
  sem_destroy(&fifo->write_sem);

  fifo->baseaddr = NULL;
}

int LAULUPA300Camera::fifo_request_drain(struct xillyfifo *fifo, struct xillyinfo *info)
{
  int taken = 0;
  unsigned int now_bytes, max_bytes;

  info->slept = 0;
  info->addr = NULL;

  now_bytes = __sync_add_and_fetch(&fifo->bytes_in_fifo, 0);

  while (now_bytes == 0) {
    if (fifo->done)
      goto fail; // FIFO will not be used by other side, and is empty

    // fifo_wrote() updates bytes_in_fifo and then increments semaphore,
    // so there's no chance for oversleeping. On the other hand, it's
    // possible that the data was drained between the bytes_in_fifo
    // update and the semaphore increment, leading to a false wakeup.
    // That's why we're in a while loop ( + other race conditions).

    info->slept = 1;

    if (sem_wait(&fifo->read_sem) && (errno != EINTR))
      goto fail;

    now_bytes = __sync_add_and_fetch(&fifo->bytes_in_fifo, 0);
  }

  max_bytes = fifo->size - fifo->read_position;
  taken = (now_bytes < max_bytes) ? now_bytes : max_bytes;
  info->addr = fifo->baseaddr + fifo->read_position;

 fail:
  info->bytes = taken;
  info->position = fifo->read_position;

  return taken;
}

void LAULUPA300Camera::fifo_drained(struct xillyfifo *fifo, unsigned int req_bytes)
{

  int semval;

  if (req_bytes == 0)
    return;

  __sync_sub_and_fetch(&fifo->bytes_in_fifo, req_bytes);
  __sync_add_and_fetch(&fifo->read_total, req_bytes);

  fifo->read_position += req_bytes;

  if (fifo->read_position >= fifo->size)
    fifo->read_position -= fifo->size;

  if (sem_getvalue(&fifo->write_sem, &semval))
    semval = 1; // This fallback should never happen

  // Don't increment the semaphore if it's nonzero anyhow. The possible
  // race condition between reading and possibly incrementing has no effect.

  if (semval == 0)
    sem_post(&fifo->write_sem);
}

int LAULUPA300Camera::fifo_request_write(struct xillyfifo *fifo, struct xillyinfo *info)
{
  int taken = 0;
  unsigned int now_bytes, max_bytes;

  info->slept = 0;
  info->addr = NULL;

  now_bytes = __sync_add_and_fetch(&fifo->bytes_in_fifo, 0);

  if (fifo->done)
    goto fail; // No point filling an abandoned FIFO

  while (now_bytes >= (fifo->size - FIFO_BACKOFF)) {
    // fifo_drained() updates bytes_in_fifo and then increments semaphore,
    // so there's no chance for oversleeping. On the other hand, it's
    // possible that the data was drained between the bytes_in_fifo
    // update and the semaphore increment, leading to a false wakeup.
    // That's why we're in a while loop ( + other race conditions).

    info->slept = 1;

    if (sem_wait(&fifo->write_sem) && (errno != EINTR))
      goto fail;

    if (fifo->done)
      goto fail; // No point filling an abandoned FIFO

    now_bytes = __sync_add_and_fetch(&fifo->bytes_in_fifo, 0);
  }

  taken = fifo->size - (now_bytes + FIFO_BACKOFF);

  max_bytes = fifo->size - fifo->write_position;

  if (taken > max_bytes)
    taken = max_bytes;
  info->addr = fifo->baseaddr + fifo->write_position;

 fail:
  info->bytes = taken;
  info->position = fifo->write_position;

  return taken;
}

void LAULUPA300Camera::fifo_wrote(struct xillyfifo *fifo, unsigned int req_bytes)
{

  int semval;

  if (req_bytes == 0)
    return;

  __sync_add_and_fetch(&fifo->bytes_in_fifo, req_bytes);
  __sync_add_and_fetch(&fifo->write_total, req_bytes);

  fifo->write_position += req_bytes;

  if (fifo->write_position >= fifo->size)
    fifo->write_position -= fifo->size;

  if (sem_getvalue(&fifo->read_sem, &semval))
    semval = 1; // This fallback should never happen

  // Don't increment the semaphore if it's nonzero anyhow. The possible
  // race condition between reading and possibly incrementing has no effect.

  if (semval == 0)
    sem_post(&fifo->read_sem);
}
#endif
