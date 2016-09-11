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

enum menuGeneralHwItems {
  ITEM_SETUP_HW_OPTREX_DISPLAY,
  ITEM_SETUP_HW_STICKS_GAINS_LABELS,
  ITEM_SETUP_HW_STICK_LV_GAIN,
  ITEM_SETUP_HW_STICK_LH_GAIN,
  ITEM_SETUP_HW_STICK_RV_GAIN,
  ITEM_SETUP_HW_STICK_RH_GAIN,
  IF_ROTARY_ENCODERS(ITEM_SETUP_HW_ROTARY_ENCODER)
  ITEM_SETUP_HW_MAX
};

#define GENERAL_HW_PARAM_OFS (2+(15*FW))
void menuGeneralHardware(uint8_t event)
{
  MENU(STR_HARDWARE, menuTabGeneral, e_Hardware, ITEM_SETUP_HW_MAX+1, {0, 0, (uint8_t)-1, 0, 0, 0, IF_ROTARY_ENCODERS(0)});

  uint8_t sub = menuVerticalPosition - 1;

  for (uint8_t i=0; i<LCD_LINES-1; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;
    uint8_t blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    uint8_t attr = (sub == k ? blink : 0);

    switch(k) {
      case ITEM_SETUP_HW_OPTREX_DISPLAY:
        g_eeGeneral.optrexDisplay = selectMenuItem(GENERAL_HW_PARAM_OFS, y, STR_LCD, STR_VLCD, g_eeGeneral.optrexDisplay, 0, 1, attr, event);
        break;

      case ITEM_SETUP_HW_STICKS_GAINS_LABELS:
        lcdDrawTextLeft(y, PSTR("Sticks"));
        break;

      case ITEM_SETUP_HW_STICK_LV_GAIN:
      case ITEM_SETUP_HW_STICK_LH_GAIN:
      case ITEM_SETUP_HW_STICK_RV_GAIN:
      case ITEM_SETUP_HW_STICK_RH_GAIN:
      {
        lcdDrawTextAtIndex(INDENT_WIDTH, y, PSTR("\002LVLHRVRH"), k-ITEM_SETUP_HW_STICK_LV_GAIN, 0);
        lcdDrawText(INDENT_WIDTH+3*FW, y, PSTR("Gain"));
        uint8_t mask = (1<<(k-ITEM_SETUP_HW_STICK_LV_GAIN));
        uint8_t val = (g_eeGeneral.sticksGain & mask ? 1 : 0);
        lcdDrawCharAtt(GENERAL_HW_PARAM_OFS, y, val ? '2' : '1', attr);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, val, 0, 1);
          if (checkIncDec_Ret) {
            g_eeGeneral.sticksGain ^= mask;
            setSticksGain(g_eeGeneral.sticksGain);
          }
        }
        break;
      }

#if defined(ROTARY_ENCODERS)
      case ITEM_SETUP_HW_ROTARY_ENCODER:
        g_eeGeneral.rotarySteps = selectMenuItem(GENERAL_HW_PARAM_OFS, y, PSTR("Rotary Encoder"), PSTR("\0062steps4steps"), g_eeGeneral.rotarySteps, 0, 1, attr, event);
        break;
#endif

#if defined(BLUETOOTH)
      case ITEM_SETUP_HW_BT_BAUDRATE:
        g_eeGeneral.btBaudrate = selectMenuItem(GENERAL_HW_PARAM_OFS, y, STR_BAUDRATE, PSTR("\005115k 9600 19200"), g_eeGeneral.btBaudrate, 0, 2, attr, event);
        if (attr && checkIncDec_Ret) {
          btInit();
        }
        break;
#endif

    }
  }
}
