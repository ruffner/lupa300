#ifndef MOJOLOADEROBJECT_H
#define MOJOLOADEROBJECT_H

#include <QObject>

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

signals:

public slots:
};

#endif // MOJOLOADEROBJECT_H
