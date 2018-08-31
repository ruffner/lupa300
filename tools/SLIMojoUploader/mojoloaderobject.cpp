#include "mojoloaderobject.h"

MojoLoaderObject::MojoLoaderObject(QObject *parent) : QObject(parent), serialPort(NULL), serialIsValid(false), bitFileIsValid(false), romFileIsValid(false), serialTimeout(false)
{
    
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
        delete serialPort;
    }

    serialIsValid = false;
    serialPort = new QSerialPort(portName);
    serialPort->setBaudRate(500000);

    if( serialPort->open(QSerialPort::ReadWrite) ){
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
        serialPort->write("T");
    } else {
        serialPort->write("U");
    }
    serialPort->flush();

    // WAIT FOR REPLY OF 'R'
    if( waitForReplyOf('R') ){
        emit emitStatus(EEPROM_HANDSHAKE_OK);
    } else {
        emit emitStatus(EEPROM_HANDSHAKE_ERROR);
        return;
    }

    // SEND 1024 LUT BYTES
    serialPort->write(eeprom);


    // WAIT FOR SEND TO COMPLETE
    while( serialPort->bytesToWrite() );


    // WAIT FOR REPLY OF 'D'
    if( waitForReplyOf('D') ){
        emit emitStatus(AVR_ACK_EEPROM);
    } else {
        emit emitStatus(EEPROM_UPLOAD_ERROR);
        return;
    }

    if( verify ){
        // REQUEST EEPROM BE READ BACK FOR VERIFICATION
        serialPort->write("P");
        serialPort->flush();

        // WAIT FOR ENTIRE RESPONSE
        while( serialPort->bytesAvailable() < 1024 );

        QByteArray ver = serialPort->readAll();
        if( ver!=eeprom ){
            emit emitStatus(EEPROM_VER_FAILED);
        } else {
            emit emitStatus(EEPROM_VER_SUCCESS);
        }
    }

    // TELL AVR TO CONFIGURE FPGA FROM SPI FLASH MEMORY
    serialPort->write("L");
    serialPort->flush();

    emit emitStatus(FPGA_FLASH_REQUEST);


    if( waitForReplyOf('D') ){
        emit emitStatus(FPGA_CONFIG_SUCCESS);
    } else {
        emit emitStatus(FPGA_CONFIG_ERROR);
        return;
    }
}

void MojoLoaderObject::resetMojoBoard()
{
    // 5 PULSES ON DTR TO BRING MOJO INTO LOADER MODE
    for( int i=0; i<5; i++ ){
        serialPort->setDataTerminalReady(true);
        QThread::msleep(25);
        serialPort->setDataTerminalReady(false);
        QThread::msleep(25);
    }
}

bool MojoLoaderObject::waitForReplyOf(char reply)
{
    serialTimeout = false;
    QTimer::singleShot(3000, this, SLOT(onSerialResponseTimeout()));
    while( serialPort->bytesAvailable()==0 && !serialTimeout );
    if( serialPort->bytesAvailable()==0 ){
        emit emitStatus(SERIAL_TIMEOUT);
        return false;
    } else {
        auto resp = serialPort->read(1);
        if( resp.at(0) != reply ){
            return false;
        } else {
            return true;
        }
    }
}

void MojoLoaderObject::onSerialResponseTimeout()
{
    serialTimeout = true;
}
