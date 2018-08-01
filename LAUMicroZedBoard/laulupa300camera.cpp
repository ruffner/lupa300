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
#ifdef Q_OS_WIN
    // ASSUME SOMETHING IS GOING TO GO WRONG UNTIL EVERYTHING WORKS FINE
    numRows = 0;
    numCols = 0;
    bitDpth = 10;
    idsCamera = 0;
    isConnected = false;
    startingFrameIndex = 0;
    numAvailableCameras = 0;
    for (int n = 0; n < IDSNUMFRAMESINBUFFER; n++) {
        energy[n] = 0;
        defaultID[n] = 0;
        defaultBuffer[n] = NULL;
    }

    // DETERMINE IF WE NEED COLOR AND/OR DEPTH VIDEO FOR THE TARGET COLOR SPACE
    switch (playbackColor) {
        case ColorGray:
            numFrms = 6;
            hasDepthVideo = false;
            hasColorVideo = true;
            playbackDevice = Device2DCamera;
            break;
        case ColorRGB:
            numFrms = 6;
            hasDepthVideo = false;
            hasColorVideo = true;
            playbackDevice = Device2DCamera;
            break;
        default:
            errorString = QString("Invalid requested color space!");
            return;
    }

    // ASSUME WE ARE NOT GOING TO FIND A CAMERA
    errorString = QString("No cameras found.");

    // GET A LIST OF AVAILABLE CAMERAS
    QStringList strings = cameraList();

    // ALWAYS CONNECT TO THE FIRST DEVICE
    if (strings.count() > 0) {
        errorString = QString();
        isConnected = connectToHost(strings.first());
    }
#else
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

    //fp = fopen("/dev/xillybus_read_8", "r");
    fd = open("/dev/xillybus_read_32", O_RDONLY);
    if (fd < 0){
		qDebug() << "Failed to open file.";
 		isConnected = false;
		hasDepthVideo = false;
		hasColorVideo = false;
    }

    fdm = open("/dev/xillybus_mem_8", O_WRONLY);
    if(fdm < 0){
		qDebug() << "Failed to open xillybus_mem_8";
    }

/*************************** Upload some register configuration *********************/  
    cfgn = 0x00;	//0x73E5
    if(lseek(fdm, 4, SEEK_SET) < 0){
				qDebug() << "Failed to seek";
				exit(1);
    }
    int rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    cfgn = 0x28;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    //********************* No.2 ***********************************/
    cfgn = 0x00;	//0x427D
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    cfgn = 0x28;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    //******************** No.3 ***********************************/
    cfgn = 0x52;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    cfgn = 0xF5;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    //******************* No.4  *********************************/
    cfgn = 0xCF;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    cfgn = 0xE1;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    //******************* No.5 *************************************/
    cfgn = 0x08;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }
    cfgn = 0xE9;
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
			qDebug() << "Write mem error.";
    }

    /******************* Total No. ********************************/
    cfgn = 0x01;	//0x04;
    if(lseek(fdm, 30, SEEK_SET) < 0){
				qDebug() << "Failed to seek";
				exit(1);
    }
    rcwf = write(fdm, &cfgn, 1);
    if(rcwf < 0){
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

	int rb = close(fd);
	//fclose(fp);
	//file.close();
	if(rb < 0){
		qDebug() << "Failed to close file.";
	}
	else{
		qDebug() << "File closed.";
	}
	
	rb = close(fdm);
	if(rb < 0){
		qDebug() << "Failed to close file.";
	}
	else{
		qDebug() << "File closed.";
	}
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
                return (LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned char), 1));
            } else {
                return (LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short), 1));
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
    //SET UP XIAPI VARIABLES
    QStringList stringList;

    numAvailableCameras = 0;
#ifdef Q_OS_WIN
    if (is_GetNumberOfCameras((int *)&numAvailableCameras) == IS_SUCCESS && numAvailableCameras > 0) {
        // CREATE A VECTOR OF CAMERA STRUCTURES
        UEYE_CAMERA_LIST *cameraList = (UEYE_CAMERA_LIST *)malloc(sizeof(DWORD) + numAvailableCameras * sizeof(UEYE_CAMERA_INFO));

        // SET THE NUMBER OF CAMERAS
        cameraList->dwCount = numAvailableCameras;

        // NOW RETRIEVE THE CAMERA PARAMETERS FOR ALL CONNECTED CAMERAS
        if (is_GetCameraList(cameraList) == IS_SUCCESS) {
            for (unsigned int n = 0; n < numAvailableCameras; n++) {
                stringList << QString("%1").arg(cameraList->uci[n].dwDeviceID);
            }
        }
        free(cameraList);
    }
#endif
    return (stringList);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool LAULUPA300Camera::connectToHost(QString hostString)
{
#ifdef Q_OS_WIN
    // CONVERT THE CAMERA STRING BACK TO A USHORT VALUE
    idsCamera = hostString.toUShort();

    // USE THE USHORT ID TO INITIALIZE AND GRAB THE CAMERA
    if (is_InitCamera(&idsCamera, NULL) == IS_SUCCESS) {
        // FOR SOME REASON, WE NEED TO RESET THE HANDLE
        // TO THE DEVICE ID OF THE CAMERA
        idsCamera = hostString.toUShort();

        CAMINFO camInfo;
        if (is_GetCameraInfo(idsCamera, &camInfo) == IS_SUCCESS) {
            serialString = QString(camInfo.SerNo);
            makeString = QString(camInfo.ID);
        }

        // NOW LET'S GRAB COME CAMERA PARAMETERS FROM THE CAMERA
        SENSORINFO sensorInfo;
        if (is_GetSensorInfo(idsCamera, &sensorInfo) == IS_SUCCESS) {
            modelString = QString(sensorInfo.strSensorName);
            numRows = sensorInfo.nMaxHeight;
            numCols = sensorInfo.nMaxWidth;
            bitDpth = 16;

            // ENABLE BINNING FOR HIGH SPEED FRAME GRABBING
            if (is_SetSubSampling(idsCamera, IS_SUBSAMPLING_4X_HORIZONTAL | IS_SUBSAMPLING_4X_VERTICAL) == IS_SUCCESS) {
                numRows = numRows / 4;
                numCols = numCols / 4;
            }

            // SET THE COLOR MODE
            if (is_SetDisplayMode(idsCamera, IS_SET_DM_DIB) == IS_SUCCESS) {
                if (is_SetColorMode(idsCamera, IS_CM_SENSOR_RAW10) == IS_SUCCESS) {
                    if (is_SetExternalTrigger(idsCamera, IS_SET_TRIGGER_SOFTWARE) == IS_SUCCESS) {
                        if (is_SetFrameRate(idsCamera, 10.0, NULL) == IS_SUCCESS) {
                            for (int n = 0; n < IDSNUMFRAMESINBUFFER; n++) {
                                // ALLOCATE A LOCAL BUFFER FOR HOLDING THE INCOMING VIDEO FRAME
                                if (is_AllocImageMem(idsCamera, (int)numCols, (int)numRows, (int)bitDpth, &(defaultBuffer[n]), &(defaultID[n])) != IS_SUCCESS) {
                                    return (false);
                                }
#ifdef RECORDSEQUENCE
                                // ADD THE NEW BUFFER TO THE RING BUFFER SEQUENCE FOR GRABBING VIDEO
                                if (is_AddToSequence(idsCamera, defaultBuffer[n], defaultID[n]) != IS_SUCCESS) {
                                    return (false);
                                }
#endif
                            }
#ifdef RECORDSEQUENCE
                            // ADD THE NEW BUFFER TO THE RING BUFFER SEQUENCE FOR GRABBING VIDEO
                            if (is_CaptureVideo(idsCamera, IS_WAIT) != IS_SUCCESS) {
                                return (false);
                            }
#endif
                            return (true);
                        }
                    }
                }
            }
        }
    }
#else
	counter = 0;
    Q_UNUSED(hostString);
#endif
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
#ifdef Q_OS_WIN
#ifdef RECORDSEQUENCE
    // CLEAR THE IMAGE FRAME SEQUENCE
    is_ClearSequence(idsCamera);
#endif
    // FREE ANY AND ALL MEMORY
    for (int n = 0; n < IDSNUMFRAMESINBUFFER; n++) {
        if (is_FreeImageMem(idsCamera, defaultBuffer[n], defaultID[n]) != IS_SUCCESS) {
            return (false);
        }
    }

    // CLOSE THE CONNECTION WITH THE CAMERA
    if (is_ExitCamera(idsCamera) == IS_SUCCESS) {
        // DEALLOCATE SPACE FOR OUR VIDEO FRAME STRUCTURES
        frameList.clear();
        isConnected = false;
        idsCamera = 0;

        return (true);
    }
    return (false);
#else
    isConnected = false;
    return (true);
#endif
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULUPA300Camera::onUpdateExposure(int microseconds)
{
    // SET THE CAMERA'S EXPOSURE
    if (isConnected) {
#ifdef Q_OS_WIN
        double exposure = (double)microseconds / 1000.0;
        if (is_Exposure(idsCamera, IS_EXPOSURE_CMD_SET_EXPOSURE, (void *)&exposure, sizeof(double)) == IS_SUCCESS) {
            qDebug() << exposure;
        }
#else
	// TELL FPGA TO CHANGE EXPOSURE SETTING HERE
	// TELL FPGA TO CHANGE EXPOSURE SETTING HERE
	// TELL FPGA TO CHANGE EXPOSURE SETTING HERE

	if(microseconds > 996)
        {
		qDebug() << " Exposure overflow!!!";
        }
        else
        {

		//expo = 64 + (997 - microseconds)/256;		//exposure time
		//expo = 0xcf;					//gain
		expo = 0x80;
        	if(lseek(fdm, 6, SEEK_SET) < 0){
				qDebug() << "Failed to seek";
				exit(1);
        	}
        	int rcwo = write(fdm, &expo, 1);
        	if(rcwo < 0){
			qDebug() << "Write mem error.";
        	}
        	//expo = (unsigned char)((997 - microseconds) & 0x00ff);	//exposure time
		//expo = 224 + (microseconds >> 8);				//gain
		expo = 75 + (microseconds >> 7);
        	rcwo = write(fdm, &expo, 1);
        	if(rcwo < 0){
			qDebug() << "Write mem error.";
        	}
	}

#endif
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULUPA300Camera::onUpdateBuffer(LAUMemoryObject depth, LAUMemoryObject color, LAUMemoryObject mapping)
{
     if (color.isValid()){
	    // SET THE DATA VALUES TO SHOW A CHANGING GRAY LEVEL
    		memset(color.constPointer(), 0, color.length());

		counter += color.frames();
		if(lseek(fdm, 3, SEEK_SET) < 0){
				qDebug() << "Failed to seek";
				exit(1);
    		}

		int rcw = write(fdm, &counter, 1);
		if(rcw < 0){
			qDebug() << "Write mem error.";
		}
	      // ITERATE THROUGH EACH OF 12 FRAMES OF VIDEO
    		for (unsigned int frm = 0; frm < color.frames(); frm++) {
        // ITERATE THROUGH EACH ROW OF THE IMAGE SENSOR
       			 for (unsigned int row = 0; row < color.height(); row++) {
            // ITERATE THROUGH EACH COLUMN OF THE CURRENT SCAN LINE
				char *buffer = (char*)color.constScanLine(row, frm);
				int rc = read(fd, buffer, 1280);
								

	    			if (rc < 0){
               				 qDebug() << "Error occurs, failed to read";
                			 break;
            			} else if (rc == 1280){
					;
            			} else{
					//nobr += rc;
					do{
						int rf = read(fd, buffer+rc, 1280-rc);
						rc += rf;
					}
					while(rc<1280);
        		  	}
    			}
		}
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
