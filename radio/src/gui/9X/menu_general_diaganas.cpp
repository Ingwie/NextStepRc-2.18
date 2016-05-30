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

void menuGeneralDiagAna(uint8_t event)
{
#if defined(TX_CAPACITY_MEASUREMENT)
  #define ANAS_ITEMS_COUNT 4
#else
  #define ANAS_ITEMS_COUNT 2
#endif

  SIMPLE_MENU(STR_MENUANA, menuTabGeneral, e_Ana, ANAS_ITEMS_COUNT);

  STICK_SCROLL_DISABLE();

  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS; i++) {
#if (NUM_STICKS+NUM_POTS) > 9
    coord_t y = MENU_HEADER_HEIGHT + 1 + (i/3)*FH;
    const uint8_t x_coord[] = {0, 70, 154};
    uint8_t x = x_coord[i%3];
    lcd_outdezNAtt(x, y, i+1, LEADING0|LEFT, 2);
    lcd_putc(x+2*FW-2, y, ':');
#else
    coord_t y = MENU_HEADER_HEIGHT + 1 + (i/2)*FH;
    uint8_t x = i&1 ? 64+5 : 0;
    putsStrIdx(x, y, PSTR("A"), i+1);
    lcd_putc(lcdNextPos, y, ':');
#endif
    lcd_outhex4(x+3*FW-1, y, anaIn(i));
    lcd_outdez8(x+10*FW-1, y, (int16_t)calibratedStick[CONVERT_MODE(i)]*25/256);
  }

  // Display raw BandGap result (debug)
  lcd_puts(64+5, MENU_HEADER_HEIGHT+1+3*FH, STR_BG);
  lcd_outdezAtt(64+5+6*FW-3, 1+4*FH, BandGap, 0);

#if   defined(PCBGRUVIN9X)
  lcd_putsLeft(6*FH-2, STR_BATT_CALIB);
  // Gruvin wants 2 decimal places and instant update of volts calib field when button pressed
  static uint16_t adcBatt;
  adcBatt = ((adcBatt * 7) + anaIn(TX_VOLTAGE)) / 8; // running average, sourced directly (to avoid unending debate :P)
  uint32_t batCalV = ((uint32_t)adcBatt*1390 + (10*(int32_t)adcBatt*g_eeGeneral.txVoltageCalibration)/8) / BandGap;
  lcd_outdezNAtt(LEN_CALIB_FIELDS*FW+4*FW, 6*FH-2, batCalV, PREC2|(menuVerticalPosition==1 ? INVERS : 0));
#else
  lcd_putsLeft(6*FH-2, STR_BATT_CALIB);
  putsVolts(LEN_CALIB_FIELDS*FW+4*FW, 6*FH-2, g_vbat100mV, (menuVerticalPosition==1 ? INVERS : 0));
#endif
  if (menuVerticalPosition==1) CHECK_INCDEC_GENVAR(event, g_eeGeneral.txVoltageCalibration, -127, 127);

#if defined(TX_CAPACITY_MEASUREMENT)
  lcd_putsLeft(6*FH+1, STR_CURRENT_CALIB);
  putsValueWithUnit(LEN_CALIB_FIELDS*FW+4*FW, 6*FH+1, getCurrent(), UNIT_MILLIAMPS, (menuVerticalPosition==2 ? INVERS : 0)) ;
  if (menuVerticalPosition==2) CHECK_INCDEC_GENVAR(event, g_eeGeneral.txCurrentCalibration, -49, 49);
#endif

}
