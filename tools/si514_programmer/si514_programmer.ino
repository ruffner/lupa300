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

  // measure frequency from startup
  Serial.println("Initial frequency:");
  for(int i=0; i<3; i++) {
    if (FreqCount.available()) {
      unsigned long count = FreqCount.read();
      Serial.println(count);
    }
    delay(100);
  }
  // end initial freq measure

  //
  // program a 'large' frequnency change
  //
  // Write OE register bit to a 0 (Register 132, bit2)
  Wire.requestFrom(SI514, 1);
  char c = Wire.read();
  c |= (1 << 2);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
