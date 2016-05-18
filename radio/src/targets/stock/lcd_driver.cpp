/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

#if defined(LCD_KS108) // KS108 diver
#define DISPLAY_SET_COLUMN       0x40
#define DISPLAY_SET_PAGE         0xB8
#define DISPLAY_SET_START        0XC0 
#define DISPLAY_ON_CMD	         0x3F
#define CS1_on   PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1)
#define CS1_off   PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1)
#define CS2_on   PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS2)
#define CS2_off   PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS2)
#define A0_on   PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0)
#define A0_off   PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0)
#define E_on   PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E)
#define E_off   PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E)
#endif

#if defined(LCD_KS108)
void lcdPulseEnable(void)
{
  E_on;
  _delay_us(4);
  E_off;
}
#endif

void lcdSendCtl(uint8_t val)
{
#if defined(LCD_KS108)
  PORTA_LCD_DAT = val;
  A0_off;
  lcdPulseEnable();
  A0_on;
# else
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
#if defined(LCD_MULTIPLEX)
  DDRA = 0xFF; //Set LCD_DAT pins to output
#endif
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  PORTA_LCD_DAT = val;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
#if defined(LCD_MULTIPLEX)
  DDRA = 0x00; //Set LCD_DAT pins to input
#endif
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
#endif
}

#if defined(PCBSTD) && defined(VOICE)
  volatile uint8_t LcdLock ;
  #define LCD_LOCK() LcdLock = 1
  #define LCD_UNLOCK() LcdLock = 0
#else
  #define LCD_LOCK()
  #define LCD_UNLOCK()
#endif

#if !defined(LCD_KS108)
const static pm_uchar lcdInitSequence[] PROGMEM =
{
//ST7565 eq. : KS0713, SED1565, S6B1713, SPLC501C, NT7532 /34 /38, TL03245
#if defined(LCD_ST7565R)
   0xE2, //Initialize the internal functions
   0xAE, //DON = 0: display OFF
   0xA0, //ADC = 0: normal direction (SEG132->SEG1)
   0xA6, //REV = 0: non-reverse display
   0xA4, //EON = 0: normal display. non-entire
   0xA2, //Select LCD bias
   0xC8, //SHL = 1: reverse direction (COM64->COM1)
   0x2F, //Control power circuit operation VC=VR=VF=1
   0x25, //Select int resistance ratio R2 R1 R0 =5
   0x81, //Set reference voltage Mode
   0x22, //24 SV5 SV4 SV3 SV2 SV1 SV0 = 0x18
   0xAF, //DON = 1: display ON
   0x60  //Set the display start line to zero
#elif defined(LCD_ERC12864FSF)
   0xE2, //Initialize the internal functions
   0xAE, //DON = 0: display OFF
   0xA1, //ADC = 1: reverse direction (SEG132->SEG1)
   0xA6, //REV = 0: non-reverse display
   0xA4, //EON = 0: normal display. non-entire
   0xA3, //Select LCD bias
   0xC0, //SHL = 0: normal direction (COM1->COM64)
   0x2F, //Control power circuit operation VC=VR=VF=1
   0x27, //Select int resistance ratio R2 R1 R0
   0x81, //Set reference voltage Mode
   0x2D, //24 SV5 SV4 SV3 SV2 SV1 SV0
   0xAF  //DON = 1: display ON
#elif defined(LCD_ST7920)
   0x30, //Set 8-bit interface
   0x36, //Repeat with graphics bit set to ON
   0x0C, //Display ON, cursor and blink OFF
   0x01, //Clear display, reset address
   0x06  //Display ON, no cursor
#else    //ST7565P (default 9x LCD)
   0xE2, //Initialize the internal functions
   0xAE, //DON = 0: display OFF
   0xA1, //ADC = 1: reverse direction(SEG132->SEG1)
   0xA6, //REV = 0: non-reverse display
   0xA4, //EON = 0: normal display. non-entire
   0xA2, //Select LCD bias=0
   0xC0, //SHL = 0: normal direction (COM1->COM64)
   0x2F, //Control power circuit operation VC=VR=VF=1
   0x25, //Select int resistance ratio R2 R1 R0 =5
   0x81, //Set reference voltage Mode
   0x22, //24 SV5 SV4 SV3 SV2 SV1 SV0 = 0x18
   0xAF  //DON = 1: display ON
#endif
};
#endif

void lcdInit()
{
#if defined(LCD_KS108)
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD reset
  _delay_us(20);
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES);  //LCD normal operation
  CS1_on;
  lcdSendCtl(DISPLAY_ON_CMD);
  lcdSendCtl(DISPLAY_SET_START);
  CS1_off;
  CS2_on;
  lcdSendCtl(DISPLAY_ON_CMD);
  lcdSendCtl(DISPLAY_SET_START);
  CS2_off;
#else
  LCD_LOCK();
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD reset
  _delay_us(2);
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES);  //LCD normal operation
#if defined(LCD_ST7920)
  _delay_ms(40);
#else
  _delay_us(1500);
#endif
  for (uint8_t i=0; i<DIM(lcdInitSequence); i++) {
    lcdSendCtl(pgm_read_byte(&lcdInitSequence[i])) ;
#if defined(LCD_ST7920)
    _delay_us(80);
#endif
  }
#if defined(LCD_ERC12864FSF)
  g_eeGeneral.contrast = 0x2D;
#else
  g_eeGeneral.contrast = 0x22;
#endif
  LCD_UNLOCK();
#endif  
}

void lcdSetRefVolt(uint8_t val)
{
#if !defined(LCD_ST7920) && !defined(LCD_KS108)// No contrast setting for ST7920 and KS108
  LCD_LOCK();
  lcdSendCtl(0x81);
  lcdSendCtl(val);
  LCD_UNLOCK();
#endif
}

#if defined(LCD_ST7920)
void lcdRefresh(){
  lcdRefresh_ST7920(1);
}

uint8_t lcdRefresh_ST7920(uint8_t full)
{
#else
void lcdRefresh()
{
#endif
#if defined(LCD_KS108)
static uint8_t change = 0; // toggle left or right lcd writing

 uint8_t *p;
 
if (change == 0) // Right
{
 p = displayBuf;
 CS1_on;
 change = 1;
}
else // Left
{
 CS2_on;
 p = displayBuf + 64;
 change = 0;	
}
 
  for (uint8_t page=0; page < 8; page++)
 {
    lcdSendCtl(DISPLAY_SET_COLUMN); // Column addr 0
    lcdSendCtl( page | DISPLAY_SET_PAGE); //Page addr
    A0_on;
    for (coord_t x=64; x>0; --x)
    {
      PORTA_LCD_DAT = *p++;
      lcdPulseEnable();
    }
   p += 64;
  }
  
  A0_off;
  CS2_off;
  CS1_off;  
#else // Not a KS108
  LCD_LOCK();
#if defined(LCD_ST7920)
  static uint8_t state;
  uint8_t yst,yend;
  uint8_t x_addr = 0;
  uint8_t y_addr = 0;
  uint16_t line_offset = 0;
  uint8_t col_offset = 0;
  uint8_t bit_count = 0;
  uint8_t result;
  uint8_t *p;
  if(full!=0){
    yst=0;
    yend=64;
    state=0;
  } 
  else{
    switch (state){//Since writing to ST7920 is too slow we need to split it to five bands.
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
      p=displayBuf + line_offset + ( x * 8 ); //Calculate the position of the first byte im array
      // adressing the bytes sequential and set the bits at the correct position merging them with an OR operation to get all bits in one byte
      // the position of the LSB is the right-most position of the byte to the ST7920
    result = ((*p++ & col_offset)!=0?0x80:0); 
    result|= ((*p++ & col_offset)!=0?0x40:0);
    result|= ((*p++  & col_offset)!=0?0x20:0);
    result|= ((*p++  & col_offset)!=0?0x10:0);
    result|= ((*p++  & col_offset)!=0?0x08:0);
    result|= ((*p++  & col_offset)!=0?0x04:0);
    result|= ((*p++  & col_offset) !=0?0x02:0);
    result|= ((*p++  & col_offset)!=0?0x01:0);
    PORTA_LCD_DAT = result;
      PORTC_LCD_CTRL |= (1<<OUT_C_LCD_E);
      _delay_us(8);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
      _delay_us(49);
    }
  }
#else  //All other LCD
  uint8_t * p = displayBuf;
  for (uint8_t y=0; y < 8; y++) {
#if defined(LCD_ST7565R)
    lcdSendCtl(0x01);
#else
    lcdSendCtl(0x04);
#endif
    lcdSendCtl(0x10); // Column addr 0
    lcdSendCtl( y | 0xB0); //Page addr y
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
#if defined(LCD_MULTIPLEX)
    DDRA = 0xFF; // Set LCD_DAT pins to output
#endif
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
    for (coord_t x=LCD_W; x>0; --x) {
      PORTA_LCD_DAT = *p++;
      PORTC_LCD_CTRL |= (1<<OUT_C_LCD_E);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
    }
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
  }
#endif  
  LCD_UNLOCK();
#if defined(LCD_ST7920)
  return state;
#endif  
#endif
}
