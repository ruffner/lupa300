#ifndef MOJOLOADEROBJECT_H
#define MOJOLOADEROBJECT_H

#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "laumemoryobject.h"

#define BITFILE_INVALID "NO BITFILE CHOSEN"
#define BITFILE_VALID   "VALID BITFILE CHOSEN"
#define ROMFILE_INVALID "ERROR: LUT IS NOT 1024 BYTES LONG"
#define ROMFILE_VALID   "LUT IS VALID"
#define READY           "READY"
#define SERIAL_OK       "OPENED SERIAL PORT"
#define SERIAL_ERROR    "COULD NOT OPEN SERIAL PORT"
#define EEPROM_HANDSHAKE_ERROR "MOJO DID NOT RESPOND TO EEPROM DL REQUEST"
#define EEPROM_HANDSHAKE_OK "MOJO ACKD REQUEST TO UPLOAD EEPROM"
#define EEPROM_UPLOAD_ERROR "MOJO DID NOT ACK EEPROM DL"
#define EEPROM_VER_FAILED "EEPROM VERIFICATION FAILED"
#define EEPROM_VER_SUCCESS "SUCCESSFULLY VERIFIED EEPROM DATA"
#define FPGA_FLASH_REQUEST "CONFIGURING FPGA FROM SPI FLASH MEMORY"
#define AVR_ACK_EEPROM "AVR ACCEPTED EEPROM UPLOAD"
#define FPGA_CONFIG_ERROR "AVR DID NOT RESPOND AFTER FLASHING FPGA"
#define FPGA_CONFIG_SUCCESS "AVR SUCCESSFULLY CONFIGURED FPGA"
#define SERIAL_TIMEOUT "TIMED OUT AFTER WAITING FOR REPLY FROM AVR"


class MojoLoaderObject : public QObject
{
    Q_OBJECT
public:
    explicit MojoLoaderObject(QObject *parent = nullptr);
    ~MojoLoaderObject();

    bool isValid() { return serialIsValid; }

signals:
    void emitStatus (QString errorString);

public slots:
    void onSerialPortInfoChanged(QString portName);
    void onSetBitFileName(QString fileName);
    void onSetEEPROMFileName(QString fileName, bool isLauMem=false);
    void onUploadBitFile(bool flash, bool verify);
    void onUploadEEPROM(bool verify);
    void onReadyRead();

private:
    bool waitForResponse();
    void resetMojoBoard();

    QSerialPort *serialPort;
    QByteArray eeprom, response;
    char responseLetter;
    bool serialIsValid, bitFileIsValid, romFileIsValid;
};

#endif // MOJOLOADEROBJECT_H
