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

#include "opentx.h"
#include "stamp-opentx.h"

#define STR2(s) #s
#define DEFNUMSTR(s)  STR2(s)

#if defined(PCBSTD)
  #define EEPROM_STR DEFNUMSTR(EEPROM_VER) "-" DEFNUMSTR(EEPROM_VARIANT)
#else
  #define EEPROM_STR DEFNUMSTR(EEPROM_VER);
#endif

#if defined(COLORLCD)
  const pm_char vers_stamp[] PROGMEM =   "VERS\037\050: " "opentx-" FLAVOUR "-" VERS_STR;
  const pm_char date_stamp[] PROGMEM =   "DATE\037\050: " DATE_STR;
  const pm_char time_stamp[] PROGMEM =   "TIME\037\050: " TIME_STR;
  const pm_char eeprom_stamp[] PROGMEM = "EEPR\037\050: " EEPROM_STR;
#else
  const pm_char vers_stamp[] PROGMEM = "FW\037\033: " "opentx-" FLAVOUR "\036VERS\037\033: " VERS_STR "\036DATE\037\033: " DATE_STR"\036TIME\037\033: " TIME_STR "\036EEPR\037\033: " EEPROM_STR;
#endif
