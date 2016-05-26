/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "opentx.h"

uint8_t g_beepCnt;
uint8_t beepAgain = 0;
uint8_t beepAgainOrig = 0;
uint8_t beepOn = false;
bool warble = false;
bool warbleC;

// The various "beep" tone lengths
static const pm_uint8_t beepTab[] PROGMEM = {
    // key, trim, warn2, warn1, error
    1,  1,  2, 10,  60, //xShort
    1,  1,  4, 20,  80, //short
    1,  1,  8, 30, 100, //normal
    2,  2, 15, 40, 120, //long
    5,  5, 30, 50, 150, //xLong
};

void beep(uint8_t val)
{
#if defined(HAPTIC) && !defined(AUDIO)
  haptic.event(val==0 ? AU_KEYPAD_UP : (val==4 ? AU_ERROR : AU_TIMER_LT10+beepAgain));
#endif

#if !defined(AUDIO)
  if (g_eeGeneral.alarmsFlash && val>1) flashCounter = FLASH_DURATION;
#endif

  if (g_eeGeneral.beepMode>0 || (g_eeGeneral.beepMode==0 && val!=0) || (g_eeGeneral.beepMode==-1 && val>=3)) {
    _beep(pgm_read_byte(beepTab+5*(2+g_eeGeneral.beepLength)+val));
  }
}

