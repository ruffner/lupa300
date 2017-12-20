/* Matt Ruffner 
 * August 2017
 * 
 * Firmware for interfacing the LUPA300 
 * image sensor with an Teensy 3.2
 */

#define LUPA_CLK          2
#define LUPA_RESET        3
#define LUPA_SPI_CLK      13
#define LUPA_SPI_DATA     11
#define LUPA_SPI_EN       10
#define LUPA_FRAME_VALID  7
#define LUPA_LINE_VALID   8
#define LUPA_INT1         4
#define LUPA_INT2         5
#define LUPA_INT3         6
#define LUPA_D9           14
#define LUPA_D8           15
#define LUPA_D7           16
#define LUPA_D6           17
#define LUPA_D5           18
#define LUPA_D4           19
#define LUPA_D3           20
#define LUPA_D2           21
#define LUPA_D1           22
#define LUPA_D0           23

const uint8_t dataPins[10] = { LUPA_D0, LUPA_D1, LUPA_D2, LUPA_D3, LUPA_D4, LUPA_D5, LUPA_D6, LUPA_D7, LUPA_D8, LUPA_D9};

void setup() {
  // initialize data pins as inputs
  for( uint8_t i=0; i<10; i++ ){
    pinMode( dataPins[i], INPUT );  
  }

  // frame and line valid as inputs
  pinMode( LUPA_FRAME_VALID, INPUT );
  pinMode( LUPA_LINE_VALID, INPUT );
}

void loop() {
  // put your main code here, to run repeatedly:

}
