// Si514 Programmable Oscillator test

#include <Wire.h>
#include <FreqCount.h>

// i2c address
#define SI514 0x55

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  FreqCount.begin(1000);
  Serial.begin(9600);
  delay(1000);
  Serial.println("Si514 Frequency Change Test");

  // show current crystal frequency
  freqMeasureThreeTimes();

  // setup Si514 to output 73.250 MHz signal
  setupSi514();

  delay(10); // max 10ms change time according to si datasheet

  // check to see if the change worked
  freqMeasureThreeTimes();
}

void loop() {
  // put your main code here, to run repeatedly:

}


void setupSi514()
{
  //
  // program a 'large' frequnency change
  //
  // 1: Clear OE (bit 2) in register 132 register 
  sendByte(0x84, 0x00);

  // 2: Write LP1 and LP2, reg 0, [7:4],[3:0]
  sendByte(0x00, 0x33);

  // 3: Write MFRAC[7:0], reg 5
  sendByte(0x05, 0xE4);

  // 4: Write MFRAC[15:8], reg 6
  sendByte(0x06, 0xD1);

  // 5: Write MFRAC[23:16], reg 7
  sendByte(0x07, 0xDF);

  // 6: Write  (M_Int[2:0] M_Frac[28:24]) reg 8
  sendByte(0x08, 0x56);

  // 7: Write  (M_Int[8:3]) reg 9
  sendByte(0x09, 0x08);

  // 8: Write  HS_DIV[7:0] reg 10
  sendByte(0x0A, 0x1E);

  // 9: Write  LS_DIV[2:0] HS_DIV[9:8] reg 11
  sendByte(0x0B, 0x00);

  // 10: start freq cal process, set reg 132 bit 0
  sendByte(0x84, 0x01);

  // 11: output enable
  sendByte(0x84, 0x04);
  
}

void sendByte(unsigned char reg, unsigned char d)
{
  Wire.beginTransmission(SI514);
  Wire.write(reg);
  Wire.write(d);
  Wire.endTransmission();
}

void freqMeasureThreeTimes()
{
  Serial.println("Output Frequency:");
  for(int i=0; i<3; i++) {
    if (FreqCount.available()) {
      unsigned long count = FreqCount.read();
      Serial.println(count);
    }
    delay(100);
  }
}

