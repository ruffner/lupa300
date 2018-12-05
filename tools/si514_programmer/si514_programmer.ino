// Si514 Programmable Oscillator Utility
// Matt Ruffner 
// March 2018

#include <Wire.h>

// i2c address of Si514 programmable oscillator
#define SI514 0x55

long freq = 73250000;

void setup() {
  delay(2000);
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Si514 Frequency Change Test");

  // show current reg status
  delay(10);
  readSi514Regs();


  Serial.println("changing to 80.00 MHz");
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
  //         m_int, m_frac, hs_div, ls_div, lp1, lp2
  // Si514SetFreq(68, 0x16DFD1E4, 30, 0, 3, 3);

  // 80.00  MHz
  Si514SetFreq(80000000);

  // show current reg status
  delay(10);
  readSi514Regs();

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

  if( Serial.available() ){

    long newFreq = Serial.parseInt();
    Si514SetFreq((double)newFreq);
    Serial.flush(); 
    Serial.print("setting to ");
    Serial.print(newFreq);
    Serial.println("Hz");
  }
}

// write Si514 registers in order to program a 'large' frequency change
// can also be used for small frequency changes
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
  writeReg(0x00, uint8_t(((lp1 << 4) | (lp2))) );

  // 3: Write MFRAC[7:0], reg 5
  writeReg(0x05, (m_frac & 0xFF));

  // 4: Write MFRAC[15:8], reg 6
  writeReg(0x06, (m_frac >> 8) & 0xFF);

  // 5: Write MFRAC[23:16], reg 7
  writeReg(0x07, (m_frac >> 16) & 0xFF);

  // 6: Write  (M_Int[2:0] M_Frac[28:24]) reg 8
  writeReg(0x08, (m_int & 0x05) << 5 | (m_frac >> 24) & 0x1F);

  // 7: Write  (M_Int[8:3]) reg 9
  writeReg(0x09, (m_int >> 3) & 0x1F);

  // 8: Write  HS_DIV[7:0] reg 10
  writeReg(0x0A, hs_div & 0xFF);

  // 9: Write  LS_DIV[2:0] HS_DIV[9:8] reg 11
  writeReg(0x0B, ((ls_div & 0x05) << 4) | ((hs_div >> 8) & 0x03));

  // 10: start freq cal process, set reg 132 bit 0
  writeReg(0x84, 0x01);

  // 11: output enable
  writeReg(0x84, 0x04);

}

// calculates reg values for frequencies where hs_div=30 is valid
// around 71Mhz to 75Mhz
void Si514SetFreq(double freq)
{
  unsigned char m_int;
  unsigned long m_frac;
  unsigned short hs_div = 30;
  unsigned char ls_div;
  unsigned char lp1;
  unsigned char lp2;

  double M = (freq*hs_div)/31980000.0;
  m_int = floor(M);
  m_frac = (float)(M-m_int)*pow(2,29);
  ls_div = 0;
  lp1 = 3;
  lp2 = 3;

  Serial.println("setting regs");
  Serial.println(M);
  Serial.println(m_int, DEC);
  Serial.println(m_frac, HEX);
  Serial.println(hs_div, DEC);
  
  
  Si514SetFreq(m_int,m_frac,hs_div,ls_div,lp1,lp2);
}

// arduino utility for reading Si514 reg values.
// not necessary for operation, only i2c write operation verification
void readSi514Regs() {
  readRegs(0x00, 1);
  readRegs(0x05, 7);
  readRegs(0x0B, 1);
  readRegs(0x0E, 1);
  readRegs(0x80, 1);
  readRegs(0x84, 1);
}

// do a burst read from the Si514 of a certain number of bytes
void readRegs(unsigned char reg, unsigned char numBytes)
{
  int i = reg;
  Wire.beginTransmission(SI514);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(SI514, numBytes);
  while ( Wire.available() && i - reg < numBytes ) {
    byte b = Wire.read();
    Serial.print("Reg ");
    Serial.print(i);
    Serial.print(":");
    Serial.println(b, HEX);
    i++;
  }
}

// write a register to a certain value
void writeReg(unsigned char reg, unsigned char d)
{
  Wire.beginTransmission(SI514);
  Wire.write(reg);
  Wire.write(d);
  Wire.endTransmission();
}
