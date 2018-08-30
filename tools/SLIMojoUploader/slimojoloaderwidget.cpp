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
    buttonClose = new QPushButton("Close");

    // PROGRESS BAR CREATION
    progressUpload = new QProgressBar();

    // CHECKBOX CREATION
    ckbStoreFlash = new QCheckBox("Save to Flash");
    ckbVerifyFlash = new QCheckBox("Verify Bit");

    // LABEL CREATION
    leBitFilePath = new QLineEdit(QString("/path/to/bit_file"));
    leEEPROMFilePath = new QLineEdit(QString("/path/to/LUT"));
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
    gbEEPROM->layout()->addWidget(leEEPROMFilePath);
    QGroupBox *gbEEPROMButtons = new QGroupBox();
    gbEEPROMButtons->setLayout(new QVBoxLayout());
    gbEEPROMButtons->layout()->addWidget(buttonChooseEEPROMFile);
    gbEEPROMButtons->layout()->addWidget(buttonLoadEEPROMFile);
    gbEEPROM->layout()->addWidget(gbEEPROMButtons);
    this->layout()->addWidget(gbEEPROM);
    // END EEPROM UPLOAD CONTROLS AND PROGRESS







    this->layout()->addWidget(buttonClose);


    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
}

SLIMojoLoaderWidget::~SLIMojoLoaderWidget()
{

}
