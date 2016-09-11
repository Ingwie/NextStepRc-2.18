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

const pm_uchar about_bmp[] PROGMEM = {
#include "../../bitmaps/about.lbm"
};

enum AboutScreens {
  ABOUT_OPENTX,
  ABOUT_BERTRAND,
  ABOUT_ANDRE,
  ABOUT_MIKE,
  ABOUT_KJELL,
  ABOUT_MARTIN,
  ABOUT_ROMOLO,
  ABOUT_ROB,
  ABOUT_HARDWARE,
  ABOUT_PARENTS,
  ABOUT_END,
  ABOUT_COUNT,
};

#define ABOUT_X      2
#define ABOUT_INDENT 4

void menuAboutView(uint8_t event)
{
  static uint8_t screenIndex;
  static uint8_t greyIndex;

  switch(event)
  {
    case EVT_ENTRY:
      screenIndex = 0;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_DOWN):
      screenIndex < ABOUT_PARENTS ? screenIndex++ : screenIndex = ABOUT_OPENTX;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_UP):
      screenIndex > ABOUT_OPENTX ? screenIndex-- : screenIndex = ABOUT_PARENTS;
      greyIndex = 0;
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }

  lcdDrawTextAtt(17, 0, STR_ABOUTUS, DBLSIZE|INVERS);
  lcdDrawSolidHorizontalLine(17, 16, LCD_W-17);
  lcd_img(8, 0, about_bmp, 0);
  LcdFlags att = 0;

  uint8_t screenDuration = 150;

  switch (screenIndex) {
    case ABOUT_OPENTX:
    case ABOUT_END:
      lcdDrawTextAtt(ABOUT_X, 22, STR_ABOUT_OPENTX_1, SMLSIZE);
      lcdDrawTextAtt(ABOUT_X, 30, STR_ABOUT_OPENTX_2, SMLSIZE);
      lcdDrawTextAtt(ABOUT_X, 38, STR_ABOUT_OPENTX_3, SMLSIZE);
      lcdDrawTextAtt(ABOUT_X, 46, STR_ABOUT_OPENTX_4, SMLSIZE);
      lcdDrawTextAtt(ABOUT_X, 54, STR_ABOUT_OPENTX_5, SMLSIZE);
      screenDuration = 255;
      break;

    case ABOUT_BERTRAND:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_BERTRAND_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_BERTRAND_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_BERTRAND_3, att|SMLSIZE);
      break;

    case ABOUT_ANDRE:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_ANDRE_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_ANDRE_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_ANDRE_3, att|SMLSIZE);
      break;

    case ABOUT_MIKE:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_MIKE_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_MIKE_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_MIKE_3, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 52, STR_ABOUT_MIKE_4, att|SMLSIZE);
      break;

    case ABOUT_KJELL:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_KJELL_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_KJELL_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_KJELL_3, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 52, STR_ABOUT_KJELL_4, att|SMLSIZE);
      break;

    case ABOUT_MARTIN:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_MARTIN_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_MARTIN_2, att|SMLSIZE);
      break;

    case ABOUT_ROMOLO:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_ROMOLO_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_ROMOLO_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_ROMOLO_3, att|SMLSIZE);
      break;

    case ABOUT_ROB:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_ROB_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_ROB_2, att|SMLSIZE);
      break;

    case ABOUT_HARDWARE:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_HARDWARE_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_HARDWARE_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_HARDWARE_3, att|SMLSIZE);
      break;

    case ABOUT_PARENTS:
      lcdDrawTextAtt(ABOUT_X, 24, STR_ABOUT_PARENTS_1, att);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 36, STR_ABOUT_PARENTS_2, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 44, STR_ABOUT_PARENTS_3, att|SMLSIZE);
      lcdDrawTextAtt(ABOUT_X+ABOUT_INDENT, 52, STR_ABOUT_PARENTS_4, att|SMLSIZE);
      screenDuration = 255;
      break;
  }

  if (++greyIndex == screenDuration) {
    greyIndex = 0;
    if (++screenIndex == ABOUT_COUNT) {
      chainMenu(menuMainView);
    }
  }
}
