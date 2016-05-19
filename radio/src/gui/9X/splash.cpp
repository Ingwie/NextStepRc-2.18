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

#include "../../opentx.h"

#if defined(SPLASH)

#if !defined GET_FAR_ADDRESS
#define GET_FAR_ADDRESS(var)                          \
({                                                    \
    uint_farptr_t tmp;                                \
                                                      \
    __asm__ __volatile__(                             \
                                                      \
            "ldi    %A0, lo8(%1)"           "\n\t"    \
            "ldi    %B0, hi8(%1)"           "\n\t"    \
            "ldi    %C0, hh8(%1)"           "\n\t"    \
            "clr    %D0"                    "\n\t"    \
        :                                             \
            "=d" (tmp)                                \
        :                                             \
            "p"  (&(var))                             \
    );                                                \
    tmp;                                              \
})
#endif

const pm_uchar splashdata[] PROGMEM = {
  'S','P','S',0,
  #include "bitmaps/9X/splash.lbm"
  'S','P','E',0 };

#if !defined(PCBMEGA2560)
const pm_uchar * const splash_lbm = splashdata+4;
#endif
  
void displaySplash()
{
  lcd_clear();
#if defined(PCBMEGA2560)
  lcd_imgfar(0, 0, (GET_FAR_ADDRESS(splashdata)+4), 0, 0); //use progmem "far" for splash working with all other options enabled
#else
  lcd_img(0, 0, splash_lbm, 0, 0);  
#endif
  
#if MENUS_LOCK == 1
  if (readonly == false) {
    lcdDrawFilledRect((LCD_W-(sizeof(TR_UNLOCKED)-1)*FW)/2 - 9, 50, (sizeof(TR_UNLOCKED)-1)*FW+16, 11, SOLID, ERASE|ROUND);
    lcdDrawText((LCD_W-(sizeof(TR_UNLOCKED)-1)*FW)/2 , 53, STR_UNLOCKED);
  }
#endif

  lcdRefresh();
 #if defined(LCD_KS108)
  lcdRefresh(); // For the other side
 #endif
}
#endif
