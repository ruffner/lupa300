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


/* TODO: implement python functionality

import serial
import random

from time import sleep

conn = serial.Serial('COM4',
                     baudrate=500000,
                     timeout=2)


conn.setDTR(False)
sleep(.025)
conn.setDTR(True)
sleep(.025)
conn.setDTR(False)
sleep(.025)
conn.setDTR(True)
sleep(.025)
conn.setDTR(False)
sleep(.025)
conn.setDTR(True)
sleep(.025)
conn.setDTR(False)
sleep(.025)
conn.setDTR(True)
sleep(.025)
conn.setDTR(False)
sleep(.025)
conn.setDTR(True)

conn.flushInput()
conn.flushOutput()

sleep(1)

conn.write('T'.encode('utf-8')) # enter lut entry mode
sleep(.1)
rec = conn.read(1)

print('received ')
print(rec)

if rec == b'R':
    print('mojo is ready for upload')
else:
    print('mojo didnt respond!! :((')
    sleep(5)


print('sending LUT')

lut=open('ml750.txt','r')
hex_string = lut.read().replace('\n',' ').replace(' 0',' 00')
lutBytes=bytes.fromhex(hex_string)
print(len(lutBytes))
print('bytes read from lut file')
lut.close();

print('writing bytes to EEPROM')
bcnt = 0
for b in lutBytes:
    conn.write(bytes([b]))
    #conn.write(bytes([random.randint(0,255)]))
    sleep(.004)
    bcnt=bcnt+1
print("done")

a=conn.read(1)
if a == b'D':
    print('AVR accepted LUT to EEPROM')
else:
    print('got')
    print(a)

conn.write('P'.encode('utf-8')) # enter verify eeprom mode
print('reading EEPROM verfication')
ver=0
for b in lutBytes:
    c=conn.read(1)
    if c != bytes([b]):
        print('eeprom verification error')
        print(bytes([b]))
        print(c)
    else:
        ver=ver+1
if ver==1024:
    print('all EEPROM bytes verified')
else:
    print('EEPROM verification failed')


print('requesting AVR to flash FPGA')
conn.write('L'.encode('utf-8')) # enter load fpga from flash mode
print('waiting for AVR to flash FPGA')
sleep(3)

a=conn.read(1)
if a== b'D':
    print('AVR successfully loaded fpga from flash')
else:
    print('got')
    print(a)



print('closing serial connection')
conn.close()

*/



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
    void onSetEEPROMFileName(QString fileName);
    void onUploadBitFile(bool flash, bool verify);
    void onUploadEEPROM(bool verify);

private slots:
    void onSerialResponseTimeout();

private:
    bool waitForReplyOf(char reply);
    void resetMojoBoard();

    QSerialPort *serialPort;
    QByteArray eeprom;
    bool serialIsValid, bitFileIsValid, romFileIsValid, serialTimeout;
};

#endif // MOJOLOADEROBJECT_H
