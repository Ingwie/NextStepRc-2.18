/*
 *************************************************************
 *                      NEXTSTEPRC                           *
 *                                                           *
 *             -> Build your DIY MEGA 2560 TX                *
 *                                                           *
 *      Based on code named                                  *
 *      OpenTx - https://github.com/opentx/opentx            *
 *                                                           *
 *         Only avr code here for lisibility ;-)             *
 *                                                           *
 *  License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html  *
 *                                                           *
 *************************************************************
 */

void lcdSendCtl(uint8_t val)
{
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  PORTA_LCD_DAT = val;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
}

const static pm_uchar lcdInitSequence[] PROGMEM =
{
  0x30, // Set 8-bit interface
  0x36, // Repeat with graphics bit set to ON
  0x0C, // Display ON, cursor and blink OFF
  0x01, // Clear display, reset address
  0x06  // Display ON, no cursor
};

void lcdInit()
{
  LCD_LOCK();
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD reset
  _delay_us(2);
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES);  //LCD normal operation
  _delay_ms(40);
  for (uint8_t i=0; i<DIM(lcdInitSequence); i++) {
    lcdSendCtl(pgm_read_byte(&lcdInitSequence[i])) ;
    _delay_us(80);
  }
  g_eeGeneral.contrast = 0x22;
  LCD_UNLOCK();
}

void lcdSetRefVolt(uint8_t val)
{
}

uint8_t lcdRefresh_ST7920(uint8_t full)
{
  LCD_LOCK();
  static uint8_t state;
  uint8_t yst,yend;
  uint8_t x_addr = 0;
  uint8_t y_addr = 0;
  uint16_t line_offset = 0;
  uint8_t col_offset = 0;
  uint8_t bit_count = 0;
  uint8_t result;
  uint8_t * p;
  if (full != 0) {
    yst=0;
    yend=64;
    state=0;
  } 
  else {
    switch (state) { // Since writing to ST7920 is too slow we need to split it to five bands.
      default:
      case 0:
        yst=0;
        yend=13;
        state=1;
      break;
      case 1:
        yst=13;
        yend=26;
        state=2;
      break;
      case 2:
        yst=26;
        yend=39;
        state=3;
      break;
      case 3:
        yst=39;
        yend=52;
        state=4;
      break;
      case 4:
        yst=52;
        yend=64;
        state=0;
      break;
    }
  }
  
  for (uint8_t y=yst; y<yend; y++) {
    x_addr = 0;
    //Convert coordinates to weirdly-arranged 128x64 screen (the ST7920 is mapped for 256x32 displays)
    if (y > 31) {
      y_addr = y - 32;    //Because there are only 31 addressable lines in the ST7920
      x_addr += 8;        //so we overflow x (7 visible bytes per line) to reach the bottom half
    }
    else {
      y_addr = y;
    }
    lcdSendCtl( 0x80 | y_addr );  //Set Vertical Address
    _delay_us(49);
    lcdSendCtl( 0x80 | x_addr );  //Set Horizontal Address
    _delay_us(49);
    PORTC_LCD_CTRL |= (1<<OUT_C_LCD_A0);    //HIGH RS and LOW RW will put the LCD to
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);  //Write data register mode
    bit_count = y & 0x07; //Count from 0 bis 7 -> 0=0, 1=1..7=7, 8=0, 9=1...
    col_offset = 1 << bit_count; //Build a value for a AND operation with the vorrect bitposition
    line_offset = ( y / 8 ) * 128; //On the ST7565 there are 8 lines with each 128 bytes width
    for (coord_t x=0; x<16; x++) { //Walk through 16 bytes form left to right (128 Pixel)
      p = displayBuf + line_offset + ( x * 8 ); //Calculate the position of the first byte im array
      // adressing the bytes sequential and set the bits at the correct position merging them with an OR operation to get all bits in one byte
      // the position of the LSB is the right-most position of the byte to the ST7920
      result = ((*p++ & col_offset)!=0?0x80:0);
      result |= ((*p++ & col_offset)!=0?0x40:0);
      result |= ((*p++  & col_offset)!=0?0x20:0);
      result |= ((*p++  & col_offset)!=0?0x10:0);
      result |= ((*p++  & col_offset)!=0?0x08:0);
      result |= ((*p++  & col_offset)!=0?0x04:0);
      result |= ((*p++  & col_offset) !=0?0x02:0);
      result |= ((*p++  & col_offset)!=0?0x01:0);
      PORTA_LCD_DAT = result;
      PORTC_LCD_CTRL |= (1<<OUT_C_LCD_E);
      _delay_us(8);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
      _delay_us(49);
    }
  }
  LCD_UNLOCK();
  return state;
}

void lcdRefresh()
{
  lcdRefresh_ST7920(1);
}
