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

#include "lauscaninspector.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUScanInspector::LAUScanInspector(LAUScan scn, bool enableCancelButton, bool enableDoNotShowAgainCheckBox, QWidget *parent) : QDialog(parent), scan(scn), tool(NULL)
{
    this->setWindowTitle(QString("Scan Inspector"));
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6,6,6,6);

    // CREATE A GLWIDGET TO DISPLAY THE SCAN
    scanWidget = new LAU3DFiducialGLWidget(scan.width(), scan.height(), scan.color(), scan.constPointer());
    //scanWidget->setRangeLimits(scan.minZ(), scan.maxZ(), scan.fieldOfView().x(), scan.fieldOfView().y());
    scanWidget->setLimits(scan.minX(), scan.maxX(), scan.minY(), scan.maxY(), scan.minZ(), scan.maxZ());
    scanWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scanWidget->setFocusPolicy(Qt::StrongFocus);
    scanWidget->setMinimumSize(320, 240);
    scanWidget->onEnableFiducials(false);
    if (scanWidget->menu()){
        QAction *action = scanWidget->menu()->addAction(QString("Enable Fiducials"), this, SLOT(onEnableFiducials(bool)));
        action->setCheckable(true);
        action->setChecked(false);
    }
    this->layout()->addWidget(scanWidget);

    // ADD OKAY AND CANCEL BUTTONS
    QWidget *buttonBox=new QWidget();
    buttonBox->setLayout(new QHBoxLayout());
    buttonBox->layout()->setContentsMargins(0,0,0,0);

    // CREATE DO NOT SHOW AGAIN CHECK BOX IF REQUESTED BY USER
    if (enableDoNotShowAgainCheckBox){
        checkBox=new QCheckBox(QString("Do not show again"));
        checkBox->setChecked(false);
        buttonBox->layout()->addWidget(checkBox);
    }

    // INSERT SPACER
    ((QHBoxLayout*)(buttonBox->layout()))->addStretch();

    // INSERT CANCEL BUTTON IF REQUESTED BY USER
    if (enableCancelButton){
        QPushButton *button=new QPushButton(QString("Cancel"));
        button->setFixedWidth(80);
        connect(button, SIGNAL(clicked()), this, SLOT(reject()));
        buttonBox->layout()->addWidget(button);
    }

    // INSERT OK BUTTON TO ACCEPT THE DIALOG
    QPushButton *button=new QPushButton(QString("Ok"));
    button->setFixedWidth(80);
    connect(button, SIGNAL(clicked()), this, SLOT(accept()));
    buttonBox->layout()->addWidget(button);
    this->layout()->addWidget(buttonBox);

    tool = new LAUFiducialTool(this);
    connect(scanWidget, SIGNAL(emitFiducialsChanged(QVector3D,int,QVector3D)), tool, SLOT(onFiducialsChanged(QVector3D,int,QVector3D)));
    connect(scanWidget, SIGNAL(emitFiducialsChanged(QList<QVector3D>,QList<QVector3D>)), tool, SLOT(onFiducialsChanged(QList<QVector3D>,QList<QVector3D>)));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUScanInspector::doNotShowAgainChecked()
{
    if (checkBox){
        return(checkBox->isChecked());
    }
    return(false);
}

