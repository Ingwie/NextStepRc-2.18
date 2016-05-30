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

const pm_char * warningText = NULL;
const pm_char * warningInfoText;
uint8_t         warningInfoLength;
uint8_t         warningType;
uint8_t         warningResult = 0;


void displayBox()
{
  drawFilledRect(10, 16, LCD_W-20, 40, SOLID, ERASE);
  lcd_rect(10, 16, LCD_W-20, 40);
  lcd_puts(WARNING_LINE_X, WARNING_LINE_Y, warningText);
  // could be a place for a warningInfoText
}

void displayPopup(const pm_char * pstr)
{
  warningText = pstr;
  displayBox();
  warningText = NULL;
  lcdRefresh();
}

void message(const pm_char *title, const pm_char *t, const char *last MESSAGE_SOUND_ARG)
{
  lcd_clear();
  lcd_img(2, 0, asterisk_lbm, 0, 0);

#define MESSAGE_LCD_OFFSET   6*FW

#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || defined(TRANSLATIONS_CZ)
  lcd_putsAtt(MESSAGE_LCD_OFFSET, 0, STR_WARNING, DBLSIZE);
  lcd_putsAtt(MESSAGE_LCD_OFFSET, 2*FH, title, DBLSIZE);
#else
  lcd_putsAtt(MESSAGE_LCD_OFFSET, 0, title, DBLSIZE);
  lcd_putsAtt(MESSAGE_LCD_OFFSET, 2*FH, STR_WARNING, DBLSIZE);
#endif

  drawFilledRect(0, 0, LCD_W, 32);
  if (t) lcd_putsLeft(5*FH, t);
  if (last) {
    lcd_putsLeft(7*FH, last);
    AUDIO_ERROR_MESSAGE(sound);
  }

#undef MESSAGE_LCD_OFFSET

  lcdRefresh();
  lcdSetContrast();
  clearKeyEvents();
}

void displayWarning(uint8_t event)
{
  warningResult = false;
  displayBox();
  if (warningInfoText) {
    lcd_putsnAtt(WARNING_LINE_X, WARNING_LINE_Y+FH, warningInfoText, warningInfoLength, WARNING_INFO_FLAGS);
  }
  lcd_puts(WARNING_LINE_X, WARNING_LINE_Y+2*FH, warningType == WARNING_TYPE_ASTERISK ? STR_EXIT : STR_POPUPS);
  switch (event) {
#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_BREAK:
#endif
    case EVT_KEY_BREAK(KEY_ENTER):
      if (warningType == WARNING_TYPE_ASTERISK)
        break;
      warningResult = true;
      // no break
#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LONG:
      killEvents(event);
#endif
    case EVT_KEY_BREAK(KEY_EXIT):
      warningText = NULL;
      warningType = WARNING_TYPE_ASTERISK;
      break;
  }
}


#if defined(NAVIGATION_MENUS)
const char *popupMenuItems[POPUP_MENU_MAX_LINES];
uint8_t s_menu_item = 0;
uint16_t popupMenuNoItems = 0;
uint8_t popupMenuFlags = 0;
uint16_t popupMenuOffset = 0;
void (*popupMenuHandler)(const char *result);
const char * displayPopupMenu(uint8_t event)
{
  const char * result = NULL;

  uint8_t display_count = min<uint8_t>(popupMenuNoItems, POPUP_MENU_MAX_LINES);
  uint8_t y = (display_count >= 5 ? MENU_Y - FH - 1 : MENU_Y);
  drawFilledRect(MENU_X, y, MENU_W, display_count * (FH+1) + 2, SOLID, ERASE);
  lcd_rect(MENU_X, y, MENU_W, display_count * (FH+1) + 2);

  for (uint8_t i=0; i<display_count; i++) {
    lcd_putsAtt(MENU_X+6, i*(FH+1) + y + 2, popupMenuItems[i], popupMenuFlags);
    if (i == s_menu_item) drawFilledRect(MENU_X+1, i*(FH+1) + y + 1, MENU_W-2, 9);
  }

  if (popupMenuNoItems > display_count) {
    displayScrollbar(MENU_X+MENU_W-1, y+1, POPUP_MENU_MAX_LINES * (FH+1), popupMenuOffset, popupMenuNoItems, POPUP_MENU_MAX_LINES);
  }

  switch(event) {
#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_LEFT
#endif
    case EVT_KEY_FIRST(KEY_MOVE_UP):
    case EVT_KEY_REPT(KEY_MOVE_UP):
      if (s_menu_item > 0) {
        s_menu_item--;
      }
#if defined(SDCARD)
      else if (popupMenuOffset > 0) {
        popupMenuOffset--;
        result = STR_UPDATE_LIST;
      }
#endif
      else {
        s_menu_item = display_count - 1;
#if defined(SDCARD)
        if (popupMenuNoItems > POPUP_MENU_MAX_LINES) {
          popupMenuOffset = popupMenuNoItems - display_count;
          result = STR_UPDATE_LIST;
        }
#endif
      }
      break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_RIGHT
#endif
    case EVT_KEY_FIRST(KEY_MOVE_DOWN):
    case EVT_KEY_REPT(KEY_MOVE_DOWN):
      if (s_menu_item < display_count - 1 && popupMenuOffset + s_menu_item + 1 < popupMenuNoItems) {
        s_menu_item++;
      }
#if defined(SDCARD)
      else if (popupMenuNoItems > popupMenuOffset + display_count) {
        popupMenuOffset++;
        result = STR_UPDATE_LIST;
      }
#endif
      else {
        s_menu_item = 0;
#if defined(SDCARD)
        if (popupMenuOffset) {
          popupMenuOffset = 0;
          result = STR_UPDATE_LIST;
        }
#endif
      }
      break;
    CASE_EVT_ROTARY_BREAK
    case EVT_KEY_BREAK(KEY_ENTER):
      result = popupMenuItems[s_menu_item];
      // no break
#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_LONG
      killEvents(event);
#endif
    case EVT_KEY_BREAK(KEY_EXIT):
      popupMenuNoItems = 0;
      s_menu_item = 0;
      popupMenuFlags = 0;
      popupMenuOffset = 0;
      break;
  }

  return result;
}
#endif
