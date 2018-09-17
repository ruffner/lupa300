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

#include "lau3dcamera.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DCameraController::LAU3DCameraController(LAU3DCamera *cam, QObject *parent) : QObject(parent), camera(cam), cameraThread(NULL)
{
    // CREATE A THREAD AND MOVE THE CAMERA TO IT
    cameraThread = new QThread();

    // CONNECT THE ERROR STRING SIGNAL
    connect(camera, SIGNAL(emitError(QString)), this, SLOT(onError(QString)), Qt::QueuedConnection);

    // START THE CAMERA THREAD
    camera->moveToThread(cameraThread);
    cameraThread->start();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DCameraController::~LAU3DCameraController()
{
    // STOP AND DELETE THREAD
    if (cameraThread) {
        cameraThread->quit();
        while (cameraThread->isRunning()) {
            qApp->processEvents();
        }
        delete cameraThread;
    }

    if (camera) {
        delete camera;
    }

    // REPORT TO THE APPLICATION OUTPUT THAT WE ARE HERE
    qDebug() << QString("LAU3DCameraController::~LAU3DCameraController()");
}
