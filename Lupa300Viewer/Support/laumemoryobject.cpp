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

#include "laumemoryobject.h"

#ifdef Q_PROCESSOR_ARM
void *_mm_malloc(int size, int align)
{
    Q_UNUSED(align);
    return (malloc(size));
}

void _mm_free(void *pointer)
{
    free(pointer);
}
#endif

using namespace libtiff;

int LAUMemoryObjectData::instanceCounter = 0;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData()
{
    numRows = 0;
    numCols = 0;
    numChns = 0;
    numByts = 0;
    numFrms = 0;

    stepBytes = 0;
    frameBytes = 0;

    buffer = NULL;
    numBytesTotal = 0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData(unsigned int cols, unsigned int rows, unsigned int chns, unsigned int byts, unsigned int frms)
{
    numRows = rows;
    numCols = cols;
    numChns = chns;
    numByts = byts;
    numFrms = frms;

    buffer = NULL;
    stepBytes = 0;
    frameBytes = 0;
    numBytesTotal = 0;

    allocateBuffer();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData(unsigned long long bytes)
{
    numRows = bytes;
    numCols = 1;
    numChns = 1;
    numByts = 1;
    numFrms = 1;

    buffer = NULL;
    stepBytes = 0;
    frameBytes = 0;
    numBytesTotal = 0;

    allocateBuffer();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData(const LAUMemoryObjectData &other)
{
    // COPY OVER THE SIZE PARAMETERS FROM THE SUPPLIED OBJECT
    numRows = other.numRows;
    numCols = other.numCols;
    numChns = other.numChns;
    numByts = other.numByts;
    numFrms = other.numFrms;

    // SET THESE VARIABLES TO ZERO AND LET THEM BE MODIFIED IN THE ALLOCATION METHOD
    buffer = NULL;
    stepBytes = 0;
    frameBytes = 0;
    numBytesTotal = 0;

    allocateBuffer();
    if (buffer) {
        memcpy(buffer, other.buffer, numBytesTotal);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::~LAUMemoryObjectData()
{
    if (buffer != NULL) {
        instanceCounter = instanceCounter - 1;
        qDebug() << QString("LAUMemoryObjectData::~LAUMemoryObjectData() %1").arg(instanceCounter) << numRows << numCols << numChns << numByts << numBytesTotal;
        _mm_free(buffer);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectData::allocateBuffer()
{
    // ALLOCATE SPACE FOR HOLDING PIXEL DATA BASED ON NUMBER OF CHANNELS AND BYTES PER PIXEL
    numBytesTotal  = (unsigned long long)numRows;
    numBytesTotal *= (unsigned long long)numCols;
    numBytesTotal *= (unsigned long long)numChns;
    numBytesTotal *= (unsigned long long)numByts;
    numBytesTotal *= (unsigned long long)numFrms;

    if (numBytesTotal > 0) {
        qDebug() << QString("LAUMemoryObjectData::allocateBuffer() %1").arg(instanceCounter++) << numRows << numCols << numChns << numByts << numFrms;

        stepBytes  = numCols * numChns * numByts;
        frameBytes = numCols * numChns * numByts * numRows;
        buffer     = _mm_malloc(numBytesTotal + 128, 16);
        if (buffer == NULL) {
            qDebug() << QString("LAUVideoBufferData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAUVideoBufferData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAUVideoBufferData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject::LAUMemoryObject(QString filename) : data(new LAUMemoryObjectData()), transformMatrix(QMatrix4x4()), anchorPt(QPoint(-1, -1)), elapsedTime(0)
{
    // GET A FILE TO OPEN FROM THE USER IF NOT ALREADY PROVIDED ONE
    if (filename.isNull()) {
        filename = QFileDialog::getOpenFileName(0, QString("Load scan from disk (*.tif)"), QString(), QString("*.tif;*.tiff"));
        if (filename.isNull()) {
            return;
        }
    }

    // IF WE HAVE A VALID TIFF FILE, LOAD FROM DISK
    // OTHERWISE TRY TO CONNECT TO SCANNER
    if (QFile::exists(filename)) {
        // OPEN INPUT TIFF FILE FROM DISK
        TIFF *inTiff = TIFFOpen(filename.toLatin1(), "r");
        if (!inTiff) {
            return;
        }
        load(inTiff);
        TIFFClose(inTiff);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject::LAUMemoryObject(TIFF *inTiff) : data(new LAUMemoryObjectData()), transformMatrix(QMatrix4x4()), anchorPt(QPoint(-1, -1)), elapsedTime(0)
{
    load(inTiff);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMemoryObject::save(QString filename)
{
    // LET THE USER SELECT A FILE FROM THE FILE DIALOG
    if (filename.isNull()) {
        filename = QFileDialog::getSaveFileName(0, QString("Save image to disk (*.tif)"), QString(), QString("*.tif;*.tiff"));
        if (filename.isNull()) {
            return (false);
        }
    }

    // OPEN TIFF FILE FOR SAVING THE IMAGE
    TIFF *outputTiff = TIFFOpen(filename.toLatin1(), "w");
    if (!outputTiff) {
        return (false);
    }

    // WRITE IMAGE TO CURRENT DIRECTORY
    if (save(outputTiff)) {
        // CLOSE TIFF FILE
        TIFFClose(outputTiff);
        return (true);
    }

    // CLOSE TIFF FILE
    TIFFClose(outputTiff);
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMemoryObject::save(TIFF *otTiff, int index)
{
    // WRITE FORMAT PARAMETERS TO CURRENT TIFF DIRECTORY
    TIFFSetField(otTiff, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField(otTiff, TIFFTAG_IMAGEWIDTH, (unsigned long)width());
    TIFFSetField(otTiff, TIFFTAG_IMAGELENGTH, (unsigned long)(height()*frames()));
    TIFFSetField(otTiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(otTiff, TIFFTAG_XRESOLUTION, 72.0);
    TIFFSetField(otTiff, TIFFTAG_YRESOLUTION, 72.0);
    TIFFSetField(otTiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(otTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(otTiff, TIFFTAG_SAMPLESPERPIXEL, (unsigned short)colors());
    TIFFSetField(otTiff, TIFFTAG_BITSPERSAMPLE, (unsigned short)(8 * depth()));
    TIFFSetField(otTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(otTiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(otTiff, TIFFTAG_XPOSITION, qMax(0.0f, (float)anchorPt.x()));
    TIFFSetField(otTiff, TIFFTAG_YPOSITION, qMax(0.0f, (float)anchorPt.y()));
    TIFFSetField(otTiff, TIFFTAG_PREDICTOR, 2);
    TIFFSetField(otTiff, TIFFTAG_ROWSPERSTRIP, 1);

    if (depth() == sizeof(float)) {
        // SEE IF WE HAVE TO TELL THE TIFF READER THAT WE ARE STORING
        // PIXELS IN 32-BIT FLOATING POINT FORMAT
        TIFFSetField(otTiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }

    // MAKE TEMPORARY BUFFER TO HOLD CURRENT ROW BECAUSE COMPRESSION DESTROYS
    // WHATS EVER INSIDE THE BUFFER
    unsigned char *tempBuffer = (unsigned char *)malloc(step());
    for (unsigned int row = 0; row < height()*frames(); row++) {
        memcpy(tempBuffer, constScanLine(row), step());
        TIFFWriteScanline(otTiff, tempBuffer, row, 0);
    }
    free(tempBuffer);

    // WRITE THE CURRENT DIRECTORY AND PREPARE FOR THE NEW ONE
    TIFFWriteDirectory(otTiff);

    // WRITE THE ELAPSED TIME STAMP TO AN EXIF TAG
    uint64 dir_offset;
    TIFFCreateEXIFDirectory(otTiff);
    TIFFSetField(otTiff, EXIFTAG_SUBSECTIME, QString("%1").arg(elapsed()).toLatin1().data());
    TIFFWriteCustomDirectory(otTiff, &dir_offset);

    TIFFSetDirectory(otTiff, index);
    TIFFSetField(otTiff, TIFFTAG_EXIFIFD, dir_offset);
    TIFFRewriteDirectory(otTiff);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMemoryObject::load(TIFF *inTiff)
{
    // LOAD INPUT TIFF FILE PARAMETERS IMPORTANT TO RESAMPLING THE IMAGE
    unsigned long uLongVariable;
    unsigned short uShortVariable;

    // NUMBER OF FRAMES IS ALWAYS EQUAL TO ONE
    data->numFrms = 1;

    // GET THE HEIGHT AND WIDTH OF INPUT IMAGE IN PIXELS
    TIFFGetField(inTiff, TIFFTAG_IMAGEWIDTH, &uLongVariable);
    data->numCols = uLongVariable;
    TIFFGetField(inTiff, TIFFTAG_IMAGELENGTH, &uLongVariable);
    data->numRows = uLongVariable;
    TIFFGetField(inTiff, TIFFTAG_SAMPLESPERPIXEL, &uShortVariable);
    data->numChns = uShortVariable;
    TIFFGetField(inTiff, TIFFTAG_BITSPERSAMPLE, &uShortVariable);
    data->numByts = uShortVariable / 8;

    // READ AND CHECK THE PHOTOMETRIC INTERPRETATION FIELD AND MAKE SURE ITS WHAT WE EXPECT
    TIFFGetField(inTiff, TIFFTAG_PHOTOMETRIC, &uShortVariable);
    if (uShortVariable != PHOTOMETRIC_SEPARATED && uShortVariable != PHOTOMETRIC_MINISBLACK) {
        return (false);
    }

    // LOAD THE ANCHOR POINT
    float xPos = -1.0f, yPos = -1.0f;
    TIFFGetField(inTiff, TIFFTAG_XPOSITION, &xPos);
    TIFFGetField(inTiff, TIFFTAG_YPOSITION, &yPos);
    anchorPt = QPoint(qRound(xPos), qRound(yPos));

    // ALLOCATE SPACE TO HOLD IMAGE DATA
    data->allocateBuffer();

    // READ DATA AS EITHER CHUNKY OR PLANAR FORMAT
    if (data->buffer) {
        short shortVariable;
        TIFFGetField(inTiff, TIFFTAG_PLANARCONFIG, &shortVariable);
        if (shortVariable == PLANARCONFIG_SEPARATE) {
            unsigned char *tempBuffer = new unsigned char [step()];
            for (unsigned int chn = 0; chn < colors(); chn++) {
                for (unsigned int row = 0; row < height(); row++) {
                    unsigned char *pBuffer = scanLine(row);
                    TIFFReadScanline(inTiff, tempBuffer, (int)row, (int)chn);
                    for (unsigned int col = 0; col < width(); col++) {
                        ((float *)pBuffer)[col * colors() + chn] = ((float *)tempBuffer)[col];
                    }
                }
            }
            delete [] tempBuffer;
        } else if (shortVariable == PLANARCONFIG_CONTIG) {
            for (unsigned int row = 0; row < height(); row++) {
                TIFFReadScanline(inTiff, (unsigned char *)scanLine(row), (int)row);
            }
        }
    }

    // GET THE ELAPSED TIME VALUE FROM THE EXIF TAG FOR SUBSECOND TIME
    uint64 directoryOffset;
    if (TIFFGetField(inTiff, TIFFTAG_EXIFIFD, &directoryOffset)) {
        char *byteArray;
        TIFFReadEXIFDirectory(inTiff, directoryOffset);
        if (TIFFGetField(inTiff, EXIFTAG_SUBSECTIME, &byteArray)) {
            setElapsed(QString(QByteArray(byteArray)).toInt());
        }
    }

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectManager::~LAUMemoryObjectManager()
{
    framesAvailable.clear();
    qDebug() << QString("LAUMemoryObjectManager::~LAUMemoryObjectManager()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectManager::onGetFrame()
{
    // CREATE NEW BUFFER IF NONE AVAILABLE, OTHERWISE TAKE FIRST AVAILABLE
    if (framesAvailable.isEmpty()) {
        emit emitFrame(LAUMemoryObject(numCols, numRows, numChns, numByts));
    } else {
        emit emitFrame(framesAvailable.takeFirst());
    }

    // NOW CREATE TWO NEW BUFFERS TO EVERY BUFFER RELEASED UP UNTIL
    // WE HAVE A MINIMUM NUMBER OF BUFFERS AVAILABLE FOR SUBSEQUENT CALLS
    if (framesAvailable.count() < MINNUMBEROFFRAMESAVAILABLE) {
        framesAvailable << LAUMemoryObject(numCols, numRows, numChns, numByts);
        framesAvailable << LAUMemoryObject(numCols, numRows, numChns, numByts);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectManager::onReleaseFrame(LAUMemoryObject frame)
{
    // WE CAN EITHER ADD THE BUFFER BACK TO THE AVAILABLE LIST, IF THERE IS SPACE
    // OR OTHERWISE FREE THE MEMORY
    if (framesAvailable.count() < MAXNUMBEROFFRAMESAVAILABLE) {
        framesAvailable << frame;
    }
}
