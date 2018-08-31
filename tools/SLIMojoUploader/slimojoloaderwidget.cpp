#include "slimojoloaderwidget.h"

SLIMojoLoaderWidget::SLIMojoLoaderWidget(QWidget *parent)
    : QWidget(parent)
{
    // INITIAL LAYOUT SETUP
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6,6,6,6);
    //this->layout()->setSizeConstraint(QLayout::SetFixedSize);
    this->setMinimumSize(480,200);

    // BUTTON CREATION
    buttonChooseBitFile = new QPushButton("Choose Bit File");
    buttonLoadBitFile = new QPushButton("Load Bit File");
    buttonChooseEEPROMFile = new QPushButton("Choose LUT");
    buttonLoadEEPROMFile = new QPushButton("Load LUT");
    buttonSerialChoose = new QPushButton("Select");

    // PROGRESS BAR CREATION
    progressUpload = new QProgressBar();

    // CHECKBOX CREATION
    ckbStoreFlash = new QCheckBox("Save to Flash");
    ckbVerifyFlash = new QCheckBox("Verify Bit");
    ckbVerifyEEPROM = new QCheckBox("Verify EEPROM");

    // LABEL CREATION
    leBitFilePath = new QLineEdit(QString("/path/to/bit_file"));
    leEEPROMFilePath = new QLineEdit(QString("/path/to/LUT"));
    labelStatus = new QLabel();
    // END LABEL CREATION


    // SERIAL PORT SELECTION BOX
    QGroupBox *connectionGroupBox = new QGroupBox(QString("Connection"));
    connectionGroupBox->setLayout(new QHBoxLayout());
    connectionGroupBox->layout()->setContentsMargins(6,6,6,6);
    cmbSerial = new QComboBox(this);
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        cmbSerial->addItem(port.portName());
    }
    connectionGroupBox->layout()->addWidget(cmbSerial);
    connectionGroupBox->layout()->addWidget(buttonSerialChoose);
    this->layout()->addWidget(connectionGroupBox);
    // END SERIAL PORT SELECTION BOX


    // BEGIN UPLOAD PROGRESS BAR
    QGroupBox *gbProgress = new QGroupBox(QString("Upload Progress"));
    gbProgress->setLayout(new QHBoxLayout());
    gbProgress->layout()->addWidget(progressUpload);
    this->layout()->addWidget(gbProgress);
    // END UPLOAD PROGRESS BAR


    // BIT FILE UPLOAD CONTROLS AND PROGRESS
    QGroupBox *gbBitFile = new QGroupBox(QString("Bit File Upload"));
    gbBitFile->setLayout(new QHBoxLayout());
    gbBitFile->setContentsMargins(6,6,6,6);
    QGroupBox *gbBitFileCkb = new QGroupBox();
    gbBitFileCkb->setLayout(new QVBoxLayout());
    gbBitFileCkb->layout()->addWidget(ckbStoreFlash);
    gbBitFileCkb->layout()->addWidget(ckbVerifyFlash);
    gbBitFile->layout()->addWidget(gbBitFileCkb);
    gbBitFile->layout()->addWidget(leBitFilePath);
    QGroupBox *gbBitFileButtons = new QGroupBox();
    gbBitFileButtons->setLayout(new QVBoxLayout());
    gbBitFileButtons->layout()->addWidget(buttonChooseBitFile);
    gbBitFileButtons->layout()->addWidget(buttonLoadBitFile);
    gbBitFile->layout()->addWidget(gbBitFileButtons);
    this->layout()->addWidget(gbBitFile);
    // END BIT FILE UPLOAD CONTROLS AND PROGRESS


    // EEPROM UPLOAD CONTROLS AND PROGRESS
    QGroupBox *gbEEPROM = new QGroupBox(QString("EEPROM File Upload"));
    gbEEPROM->setLayout(new QHBoxLayout());
    gbEEPROM->setContentsMargins(6,6,6,6);
    gbEEPROM->layout()->addWidget(ckbVerifyEEPROM);
    gbEEPROM->layout()->addWidget(leEEPROMFilePath);
    QGroupBox *gbEEPROMButtons = new QGroupBox();
    gbEEPROMButtons->setLayout(new QVBoxLayout());
    gbEEPROMButtons->layout()->addWidget(buttonChooseEEPROMFile);
    gbEEPROMButtons->layout()->addWidget(buttonLoadEEPROMFile);
    gbEEPROM->layout()->addWidget(gbEEPROMButtons);
    this->layout()->addWidget(gbEEPROM);
    // END EEPROM UPLOAD CONTROLS AND PROGRESS


    this->layout()->addWidget(labelStatus);
    // END LAYOUT CREATION
    
    mojoLoader = new MojoLoaderObject();

    connect(buttonLoadBitFile, SIGNAL(clicked()), this, SLOT(onLoadBitFile()));
    connect(buttonSerialChoose, SIGNAL(clicked()), this, SLOT(onChooseSerial()));
    connect(buttonChooseBitFile, SIGNAL(clicked()), this, SLOT(onChooseBitFile()));
    connect(buttonLoadEEPROMFile, SIGNAL(clicked()), this, SLOT(onLoadEEPROMFile()));
    connect(buttonChooseEEPROMFile, SIGNAL(clicked()), this, SLOT(onChooseEEPROMFile()));
    connect(mojoLoader, SIGNAL(emitStatus(QString)), this, SLOT(onLoaderStatusChanged(QString)));



    // SET INITIAL STATES
    onLoaderStatusChanged(BITFILE_INVALID);
    onLoaderStatusChanged(ROMFILE_INVALID);
    onLoaderStatusChanged("PLEASE SELECT A COM PORT");

}

SLIMojoLoaderWidget::~SLIMojoLoaderWidget()
{

}

void SLIMojoLoaderWidget::onChooseSerial()
{
   mojoLoader->onSerialPortInfoChanged(cmbSerial->currentText());
}

void SLIMojoLoaderWidget::onChooseBitFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Bit File"), "/", tr("Bit Files (*.bit)"));

    if( !fileName.isEmpty() ){
        leBitFilePath->setText(fileName);
        mojoLoader->onSetBitFileName(fileName);
    } else {
        qDebug() << "no file selected";
    }
}

void SLIMojoLoaderWidget::onChooseEEPROMFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open LUT File"), "/", tr("Text Files (*.txt)"));

    if( !fileName.isEmpty() ){
        leEEPROMFilePath->setText(fileName);
        mojoLoader->onSetEEPROMFileName(fileName);
    } else {
        qDebug() << "no file selected";
    }
}

void SLIMojoLoaderWidget::onLoadBitFile()
{
    mojoLoader->onUploadBitFile(ckbStoreFlash->isChecked(), ckbVerifyFlash->isChecked());
}

void SLIMojoLoaderWidget::onLoadEEPROMFile()
{
    mojoLoader->onUploadEEPROM(ckbVerifyEEPROM->isChecked());
}

void SLIMojoLoaderWidget::onLoaderStatusChanged(QString status)
{
    labelStatus->setText(status);

    if( status==BITFILE_VALID ){
        buttonLoadBitFile->setEnabled(true);
        ckbStoreFlash->setEnabled(true);
        ckbVerifyFlash->setEnabled(true);
    } else if( status==BITFILE_INVALID ){
        buttonLoadBitFile->setEnabled(false);
        ckbStoreFlash->setEnabled(false);
        ckbVerifyFlash->setEnabled(false);
    } else if( status==ROMFILE_INVALID ){
        buttonLoadEEPROMFile->setEnabled(false);
        ckbVerifyEEPROM->setEnabled(false);
    } else if( status==ROMFILE_VALID ){
        buttonLoadEEPROMFile->setEnabled(true);
        ckbVerifyEEPROM->setEnabled(true);
    }
}
