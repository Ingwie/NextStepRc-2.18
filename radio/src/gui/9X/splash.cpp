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
}
#endif
