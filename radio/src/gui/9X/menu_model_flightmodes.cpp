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


void displayFlightModes(coord_t x, coord_t y, FlightModesType value)
{
  uint8_t p = MAX_FLIGHT_MODES;
  do {
    --p;
    if (!(value & (1<<p)))
      lcdDrawChar(x, y, '0'+p);
    x -= FWNUM;
  } while (p!=0);
}

enum menuModelPhaseItems {
  ITEM_MODEL_PHASE_NAME,
  ITEM_MODEL_PHASE_SWITCH,
  ITEM_MODEL_PHASE_TRIMS,
  IF_ROTARY_ENCODERS(ITEM_MODEL_PHASE_ROTARY_ENCODERS)
  ITEM_MODEL_PHASE_FADE_IN,
  ITEM_MODEL_PHASE_FADE_OUT,
#if defined(GVARS) && !defined(PCBSTD)
  ITEM_MODEL_PHASE_GVARS_LABEL,
  ITEM_MODEL_PHASE_GV1,
  ITEM_MODEL_PHASE_GV2,
  ITEM_MODEL_PHASE_GV3,
  ITEM_MODEL_PHASE_GV4,
  ITEM_MODEL_PHASE_GV5,
#endif
  ITEM_MODEL_PHASE_MAX
};

void menuModelPhaseOne(uint8_t event)
{
  FlightModeData *fm = flightModeAddress(s_currIdx);
  putsFlightMode(13*FW, 0, s_currIdx+1, (getFlightMode()==s_currIdx ? BOLD : 0));

#if defined(GVARS) && !defined(PCBSTD)
  static const pm_uint8_t mstate_tab_fm1[] PROGMEM = {0, 0, 0, (uint8_t)-1, 1, 1, 1, 1, 1};
  static const pm_uint8_t mstate_tab_others[] PROGMEM = {0, 0, 3, IF_ROTARY_ENCODERS(NUM_ROTARY_ENCODERS-1) 0, 0, (uint8_t)-1, 2, 2, 2, 2, 2};

  check(event, 0, NULL, 0, (s_currIdx == 0) ? mstate_tab_fm1 : mstate_tab_others, DIM(mstate_tab_others)-1, ITEM_MODEL_PHASE_MAX - 1 - (s_currIdx==0 ? (ITEM_MODEL_PHASE_FADE_IN-ITEM_MODEL_PHASE_SWITCH) : 0));

  TITLE(STR_MENUFLIGHTPHASE);

  #define PHASE_ONE_FIRST_LINE (1+1*FH)
#else
  SUBMENU(STR_MENUFLIGHTPHASE, 3 + (s_currIdx==0 ? 0 : 2 + (bool)NUM_ROTARY_ENCODERS), {0, 0, 3, IF_ROTARY_ENCODERS(NUM_ROTARY_ENCODERS-1) 0/*, 0*/});
  #define PHASE_ONE_FIRST_LINE (1+1*FH)
#endif

  int8_t sub = menuVerticalPosition;
  int8_t editMode = s_editMode;

#if defined(GVARS) && !defined(PCBSTD)
  if (s_currIdx == 0 && sub>=ITEM_MODEL_PHASE_SWITCH) sub += ITEM_MODEL_PHASE_FADE_IN-ITEM_MODEL_PHASE_SWITCH;

  for (uint8_t k=0; k<LCD_LINES-1; k++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
    int8_t i = k + menuVerticalOffset;
    if (s_currIdx == 0 && i>=ITEM_MODEL_PHASE_SWITCH) i += ITEM_MODEL_PHASE_FADE_IN-ITEM_MODEL_PHASE_SWITCH;
    uint8_t attr = (sub==i ? (editMode>0 ? BLINK|INVERS : INVERS) : 0);
#else
  for (uint8_t i=0, k=0, y=PHASE_ONE_FIRST_LINE; i<ITEM_MODEL_PHASE_MAX; i++, k++, y+=FH) {
    if (s_currIdx == 0 && i==ITEM_MODEL_PHASE_SWITCH) i = ITEM_MODEL_PHASE_FADE_IN;
    uint8_t attr = (sub==k ? (editMode>0 ? BLINK|INVERS : INVERS) : 0);
#endif
    switch(i) {
      case ITEM_MODEL_PHASE_NAME:
        editSingleName(MIXES_2ND_COLUMN, y, STR_PHASENAME, fm->name, sizeof(fm->name), event, attr);
        break;
      case ITEM_MODEL_PHASE_SWITCH:
        fm->swtch = switchMenuItem(MIXES_2ND_COLUMN, y, fm->swtch, attr, event);
        break;
      case ITEM_MODEL_PHASE_TRIMS:
        lcdDrawTextLeft(y, STR_TRIMS);
        for (uint8_t t=0; t<NUM_STICKS; t++) {
          lcdPutsTrimMode(MIXES_2ND_COLUMN+(t*FW), y, s_currIdx, t, menuHorizontalPosition==t ? attr : 0);
          if (attr && menuHorizontalPosition==t && ((editMode>0) || p1valdiff)) {
            int16_t v = getRawTrimValue(s_currIdx, t);
            if (v < TRIM_EXTENDED_MAX) v = TRIM_EXTENDED_MAX;
            v = checkIncDec(event, v, TRIM_EXTENDED_MAX, TRIM_EXTENDED_MAX+MAX_FLIGHT_MODES-1, EE_MODEL);
            if (checkIncDec_Ret) {
              if (v == TRIM_EXTENDED_MAX) v = 0;
              setTrimValue(s_currIdx, t, v);
            }
          }
        }
        break;

#if ROTARY_ENCODERS > 0
      case ITEM_MODEL_PHASE_ROTARY_ENCODERS:
        lcdDrawTextLeft(y, STR_ROTARY_ENCODER);
        for (uint8_t t=0; t<NUM_ROTARY_ENCODERS; t++) {
          lcdPutsRotaryEncoderMode(MIXES_2ND_COLUMN+(t*FW), y, s_currIdx, t, menuHorizontalPosition==t ? attr : 0);
          if (attr && menuHorizontalPosition==t && ((editMode>0) || p1valdiff)) {
            int16_t v = flightModeAddress(s_currIdx)->rotaryEncoders[t];
            if (v < ROTARY_ENCODER_MAX) v = ROTARY_ENCODER_MAX;
            v = checkIncDec(event, v, ROTARY_ENCODER_MAX, ROTARY_ENCODER_MAX+MAX_FLIGHT_MODES-1, EE_MODEL);
            if (checkIncDec_Ret) {
              if (v == ROTARY_ENCODER_MAX) v = 0;
              flightModeAddress(s_currIdx)->rotaryEncoders[t] = v;
            }
          }
        }
        break;
#endif

      case ITEM_MODEL_PHASE_FADE_IN:
        fm->fadeIn = EDIT_DELAY(0, y, event, attr, STR_FADEIN, fm->fadeIn);
        break;

      case ITEM_MODEL_PHASE_FADE_OUT:
        fm->fadeOut = EDIT_DELAY(0, y, event, attr, STR_FADEOUT, fm->fadeOut);
        break;

#if defined(GVARS) && !defined(PCBSTD)
      case ITEM_MODEL_PHASE_GVARS_LABEL:
        lcdDrawTextLeft(y, STR_GLOBAL_VARS);
        break;

      default:
      {
        uint8_t idx = i-ITEM_MODEL_PHASE_GV1;
        uint8_t posHorz = menuHorizontalPosition;
        if (attr && posHorz > 0 && s_currIdx==0) posHorz++;

        lcdDrawStringWithIndex(INDENT_WIDTH, y, STR_GV, idx+1);

        editName(4*FW, y, g_model.gvars[idx].name, LEN_GVAR_NAME, event, posHorz==0 ? attr : 0);

        int16_t v = fm->gvars[idx];
        if (v > GVAR_MAX) {
          uint8_t p = v - GVAR_MAX - 1;
          if (p >= s_currIdx) p++;
          putsFlightMode(11*FW, y, p+1, posHorz==1 ? attr : 0);
        }
        else {
          lcdDrawTextAtt(11*FW, y, STR_OWN, posHorz==1 ? attr : 0);
        }
        if (attr && s_currIdx>0 && posHorz==1 && (editMode>0 || p1valdiff)) {
          if (v < GVAR_MAX) v = GVAR_MAX;
          v = checkIncDec(event, v, GVAR_MAX, GVAR_MAX+MAX_FLIGHT_MODES-1, EE_MODEL);
          if (checkIncDec_Ret) {
            if (v == GVAR_MAX) v = 0;
            fm->gvars[idx] = v;
          }
        }

        uint8_t p = getGVarFlightPhase(s_currIdx, idx);
        lcdDrawNumberAttUnit(21*FW, y, GVAR_VALUE(idx, p), posHorz==2 ? attr : 0);
        if (attr && posHorz==2 && ((editMode>0) || p1valdiff)) {
          GVAR_VALUE(idx, p) = checkIncDec(event, GVAR_VALUE(idx, p), -GVAR_LIMIT, GVAR_LIMIT, EE_MODEL);
        }

        break;
      }
#endif
    }
  }
}

#if defined(ROTARY_ENCODERS)
  #if ROTARY_ENCODERS > 2
    #define NAME_OFS (-4-12)
    #define SWITCH_OFS (-FW/2-2-13)
    #define TRIMS_OFS  (-FW/2-4-15)
    #define ROTARY_ENC_OFS (0)
  #else
    #define NAME_OFS (-4)
    #define SWITCH_OFS (-FW/2-2)
    #define TRIMS_OFS  (-FW/2-4)
    #define ROTARY_ENC_OFS (2)
  #endif
#else
  #define NAME_OFS 0
  #define SWITCH_OFS (FW/2)
  #define TRIMS_OFS  (FW/2)
#endif

void menuModelFlightModesAll(uint8_t event)
{
  SIMPLE_MENU(STR_MENUFLIGHTPHASES, menuTabModel, e_FlightModesAll, 1+MAX_FLIGHT_MODES+1);

  int8_t sub = menuVerticalPosition - 1;

  switch (event) {
    CASE_EVT_ROTARY_BREAK
    case EVT_KEY_FIRST(KEY_ENTER):
      if (sub == MAX_FLIGHT_MODES) {
        s_editMode = 0;
        trimsCheckTimer = 200; // 2 seconds
      }
      // no break
    case EVT_KEY_FIRST(KEY_RIGHT):
      if (sub >= 0 && sub < MAX_FLIGHT_MODES) {
        s_currIdx = sub;
        pushMenu(menuModelPhaseOne);
      }
      break;
  }

  uint8_t att;
  for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
    uint8_t y = 1 + (i+1)*FH;
    att = (i==sub ? INVERS : 0);
    FlightModeData *p = flightModeAddress(i);
    putsFlightMode(0, y, i+1, att|(getFlightMode()==i ? BOLD : 0));

    lcdDrawSizedTextAtt(4*FW+NAME_OFS, y, p->name, sizeof(p->name), ZCHAR);
    if (i == 0) {
      lcdDrawText((5+LEN_FLIGHT_MODE_NAME)*FW+SWITCH_OFS, y, STR_DEFAULT);
    }
    else {
      lcdPutsSwitches((5+LEN_FLIGHT_MODE_NAME)*FW+SWITCH_OFS, y, p->swtch, 0);
      for (uint8_t t=0; t<NUM_STICKS; t++) {
        lcdPutsTrimMode((9+LEN_FLIGHT_MODE_NAME+t)*FW+TRIMS_OFS, y, i, t, 0);
      }
#if defined(CPUM2560)
      for (uint8_t t=0; t<NUM_ROTARY_ENCODERS; t++) {
        lcdPutsRotaryEncoderMode((13+LEN_FLIGHT_MODE_NAME+t)*FW+TRIMS_OFS+ROTARY_ENC_OFS, y, i, t, 0);
      }
#endif
    }

    if (p->fadeIn || p->fadeOut) {
      lcdDrawChar(LCD_W-FW-MENUS_SCROLLBAR_WIDTH, y, (p->fadeIn && p->fadeOut) ? '*' : (p->fadeIn ? 'I' : 'O'));
    }
  }


  lcdDrawTextLeft((LCD_LINES-1)*FH+1, STR_CHECKTRIMS);
  putsFlightMode(OFS_CHECKTRIMS, (LCD_LINES-1)*FH+1, mixerCurrentFlightMode+1);
  if (sub==MAX_FLIGHT_MODES && !trimsCheckTimer) {
    lcd_status_line();
  }
}
