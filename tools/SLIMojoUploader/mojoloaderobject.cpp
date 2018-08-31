#include "mojoloaderobject.h"

MojoLoaderObject::MojoLoaderObject(QObject *parent) : QObject(parent), serialPort(NULL), serialIsValid(false), bitFileIsValid(false), romFileIsValid(false)
{
    serialPort = new QSerialPort();
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

MojoLoaderObject::~MojoLoaderObject()
{
    if( serialPort ){
        if( serialPort->isOpen() ){
            serialPort->close();
        }
        delete serialPort;
    }
}

void MojoLoaderObject::onSerialPortInfoChanged(QString portName)
{
    if( serialPort ){
        if( serialPort->isOpen() ){
            serialPort->close();
        }
    }

    serialIsValid = false;
    serialPort->setPortName(portName);
    serialPort->setBaudRate(500000);

    if( serialPort->open(QIODevice::ReadWrite) ){
        serialPort->setDataTerminalReady(false);
        serialIsValid = true;
        emit emitStatus(SERIAL_OK);
    } else {
        serialIsValid = false;
        emit emitStatus(SERIAL_ERROR);
    }
}

void MojoLoaderObject::onSetBitFileName(QString fileName)
{
    qDebug() << "in onSetBitFIleName " << fileName;


    QFile bitFile(fileName);
    unsigned long fileSize = 0;

    if( bitFile.open(QFile::ReadOnly) ){
        fileSize = (unsigned long) bitFile.size();
        bitFileIsValid = true;
        emit emitStatus(BITFILE_VALID);
    } else {
        bitFileIsValid = false;
        emit emitStatus(BITFILE_INVALID);
    }

    bitFile.close();

}

void MojoLoaderObject::onSetEEPROMFileName(QString fileName)
{
    QFile romFile(fileName);
    unsigned long fileSize = 0;

    if( romFile.open(QFile::ReadOnly) ){
        eeprom.clear();

        while( romFile.bytesAvailable() ){
            QByteArray s = romFile.readLine();
            if( s.length()==1 && s.at(0)=='0' ){
                eeprom.append((char)0x00);
            } else {
                eeprom.append(QByteArray::fromHex(s));
            }
        }

        fileSize = (unsigned long) eeprom.size();

        if( fileSize!=1024 ){
            romFileIsValid = false;
            emit emitStatus(ROMFILE_INVALID);
        } else {
            romFileIsValid = true;
            emit emitStatus(ROMFILE_VALID);
        }

        romFile.close();
    } else {
        romFileIsValid = false;
        emit emitStatus(ROMFILE_INVALID);
    }

}

void MojoLoaderObject::onUploadBitFile(bool flash, bool verify)
{
    if( !isValid() ){
        emit emitStatus("NO SERIAL PORT SELECTED");
        return;
    }
}

void MojoLoaderObject::onUploadEEPROM(bool verify)
{
    if( !isValid() ){
        emit emitStatus("NO SERIAL PORT SELECTED");
        return;
    }

    // ENTER LOADER MODE
    resetMojoBoard();

    // START EEPROM WRITE AND OR VERIFY
    if( verify ){
        qDebug() << "wrote T";
        serialPort->write("T");
    } else {
        qDebug() << "wrote U";
        serialPort->write("U");
    }

    responseLetter = 'R';
    if( waitForResponse() ){
        response.remove(0,1);
        emit emitStatus(EEPROM_HANDSHAKE_OK);
    } else {
        emit emitStatus(EEPROM_HANDSHAKE_ERROR);
    }

    // SEND 1024 LUT BYTES
    serialPort->write(eeprom);

    if( serialPort->waitForBytesWritten(1000) ){
        qDebug() << "wrote eeprom";
    }


    responseLetter = 'D';
    if( waitForResponse() ){
        response.remove(0,1);
        emit emitStatus(AVR_ACK_EEPROM);
    } else {
        emit emitStatus(EEPROM_UPLOAD_ERROR);
        return;
    }

    if( verify ){
        // REQUEST EEPROM BE READ BACK FOR VERIFICATION
        serialPort->write("P");

        // WAIT FOR ENTIRE RESPONSE
        while( response.size()<1024 ){
            serialPort->waitForReadyRead(100);
        }

        qDebug() << "response size: " << response.size();

        if( response!=eeprom ){
            emit emitStatus(EEPROM_VER_FAILED);
        } else {
            emit emitStatus(EEPROM_VER_SUCCESS);
        }

        response.clear();
    }

    // TELL AVR TO CONFIGURE FPGA FROM SPI FLASH MEMORY
    serialPort->write("L");
    emit emitStatus(FPGA_FLASH_REQUEST);

    responseLetter = 'D';
    if( waitForResponse() ){
        response.remove(0,1);
        emit emitStatus(FPGA_CONFIG_SUCCESS);
    } else {
        emit emitStatus(FPGA_CONFIG_ERROR);
        return;
    }
}

void MojoLoaderObject::resetMojoBoard()
{
    qDebug() << "putting mojo into loader mode...";

    // 5 PULSES ON DTR TO BRING MOJO INTO LOADER MODE
    for( int i=0; i<5; i++ ){
        serialPort->setDataTerminalReady(true);
        QThread::msleep(25);
        serialPort->setDataTerminalReady(false);
        QThread::msleep(25);
    }
    serialPort->setDataTerminalReady(true);
    qDebug() << "done\n";
}

bool MojoLoaderObject::waitForResponse()
{
    while( response.size()==0 ){
        serialPort->waitForReadyRead(10);
    }
    if( (char)response.at(0)==responseLetter ){
        return true;
    } else {
        return false;
    }
}

void MojoLoaderObject::onReadyRead()
{
    response.append(serialPort->readAll());
}
