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
  delay(5000);
  Serial.println("Si514 Frequency Change Test");

  // show current reg status
  readSi514Regs();

  // setup Si514 to output 73.250 MHz signal
  Serial.println("setting to 73.25 Mhz");
  Si514SetFreq73_25MHz();

  // max 10ms change time according to si datasheet
  delay(1000); 
  readSi514Regs();

  delay(10000);

  Serial.println("changing to 74.25");
  Si514SetFreq74_25MHz();


  delay(1000);
  readSi514Regs();

  delay(10000);
  Serial.println("changing back to 73.25 with new methos");
  //         m_int, m_frac, hs_div, ls_div, lp1, lp2
  Si514SetFreq(68, 0x16DFD1E4, 30, 0, 3, 3);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void Si514SetFreq(unsigned char m_int, 
                  unsigned long m_frac, 
                  unsigned short hs_div, 
                  unsigned char ls_div, 
                  unsigned char lp1,
                  unsigned char lp2)
{
  //
  // program a 'large' frequnency change
  //
  // 1: Clear OE (bit 2) in register 132 register 
  writeReg(0x84, 0x00);

  // 2: Write LP1 and LP2, reg 0, [7:4],[3:0]
  writeReg(0x00, (lp1&0x0F<<4) | (lp2&0x0F) );

  // 3: Write MFRAC[7:0], reg 5
  writeReg(0x05, (m_frac&0xFF));

  // 4: Write MFRAC[15:8], reg 6
  writeReg(0x06, (m_frac>>8)&0xFF);

  // 5: Write MFRAC[23:16], reg 7
  writeReg(0x07, (mfrac>>16)&0xFF);

  // 6: Write  (M_Int[2:0] M_Frac[28:24]) reg 8
  writeReg(0x08, (m_int&0x05)<<5 | (mfrac>>24)&0x1F);

  // 7: Write  (M_Int[8:3]) reg 9
  writeReg(0x09, (m_int>>3)&0x1F);

  // 8: Write  HS_DIV[7:0] reg 10
  writeReg(0x0A, hs_div&0xFF);

  // 9: Write  LS_DIV[2:0] HS_DIV[9:8] reg 11
  writeReg(0x0B, ((ls_div&0x05)<<2) | ((hs_div>>8)&0x03));

  // 10: start freq cal process, set reg 132 bit 0
  writeReg(0x84, 0x01);

  // 11: output enable
  writeReg(0x84, 0x04);
                  
}

// program a large freq change in the Si514
// values calculated with the following formula
// F_out = (F_xo * M) / (HS_DIV * 2^LS_DIV)
// M = 68.7148217636
// F_out = 73.25 MHz
// F_xo = 31.98 MHz
// M_INT = floor(M) = 68
// M_FRAC = ( M - M_INT ) * 2^29 = 0x16DFD1E4
// LS_DIV = 0
// HS_DIV = 30
// LP1 = 3
// LP2 = 3
void Si514SetFreq73_25MHz()
{
  //
  // program a 'large' frequnency change
  //
  // 1: Clear OE (bit 2) in register 132 register 
  writeReg(0x84, 0x00);

  // 2: Write LP1 and LP2, reg 0, [7:4],[3:0]
  writeReg(0x00, 0x33);

  // 3: Write MFRAC[7:0], reg 5
  writeReg(0x05, 0xE4);

  // 4: Write MFRAC[15:8], reg 6
  writeReg(0x06, 0xD1);

  // 5: Write MFRAC[23:16], reg 7
  writeReg(0x07, 0xDF);

  // 6: Write  (M_Int[2:0] M_Frac[28:24]) reg 8
  writeReg(0x08, 0x96);

  // 7: Write  (M_Int[8:3]) reg 9
  writeReg(0x09, 0x08);

  // 8: Write  HS_DIV[7:0] reg 10
  writeReg(0x0A, 0x1E);

  // 9: Write  LS_DIV[2:0] HS_DIV[9:8] reg 11
  writeReg(0x0B, 0x00);

  // 10: start freq cal process, set reg 132 bit 0
  writeReg(0x84, 0x01);

  // 11: output enable
  writeReg(0x84, 0x04);
  
}

// values taken from page 15 in the Si514 datasheet
void Si514SetFreq74_25MHz()
{
  //
  // program a 'large' frequnency change
  //
  // 1: Clear OE (bit 2) in register 132 register 
  writeReg(0x84, 0x00);

  // 2: Write LP1 and LP2, reg 0, [7:4],[3:0]
  writeReg(0x00, 0x33);

  // 3: Write MFRAC[7:0], reg 5
  writeReg(0x05, 0x76);

  // 4: Write MFRAC[15:8], reg 6
  writeReg(0x06, 0x9F);

  // 5: Write MFRAC[23:16], reg 7
  writeReg(0x07, 0xE4);

  // 6: Write  (M_Int[2:0] M_Frac[28:24]) reg 8
  writeReg(0x08, 0xB4);

  // 7: Write  (M_Int[8:3]) reg 9
  writeReg(0x09, 0x08);

  // 8: Write  HS_DIV[7:0] reg 10
  writeReg(0x0A, 0x1E);

  // 9: Write  LS_DIV[2:0] HS_DIV[9:8] reg 11
  writeReg(0x0B, 0x00);

  // 10: start freq cal process, set reg 132 bit 0
  writeReg(0x84, 0x01);

  // 11: output enable
  writeReg(0x84, 0x04); 
}

void readSi514Regs() {
  readRegs(0x00, 1);
  readRegs(0x05, 7);
  readRegs(0x0E, 1);
  readRegs(0x80, 1);
  readRegs(0x84, 1);
}

void readRegs(unsigned char reg, unsigned char numBytes)
{
  int i=reg;
  Wire.beginTransmission(SI514);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(SI514, numBytes);
  while( Wire.available() ){
    byte b = Wire.read();
    Serial.print("Reg ");
    Serial.print(i);
    Serial.print(":");
    Serial.println(b, HEX);
    i++;
  }
}

void writeReg(unsigned char reg, unsigned char d)
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

