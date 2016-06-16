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

#define MODEL_CUSTOM_FUNC_1ST_COLUMN          (3)
#define MODEL_CUSTOM_FUNC_2ND_COLUMN          (5*FW-2)
#define MODEL_CUSTOM_FUNC_3RD_COLUMN          (15*FW+2)
#define MODEL_CUSTOM_FUNC_4TH_COLUMN          (20*FW)
#if defined(GRAPHICS)
  #define MODEL_CUSTOM_FUNC_4TH_COLUMN_ONOFF  (20*FW)
#else
  #define MODEL_CUSTOM_FUNC_4TH_COLUMN_ONOFF  (18*FW+2)
#endif


void menuCustomFunctions(uint8_t event, CustomFunctionData * functions, CustomFunctionsContext * functionsContext)
{
  int8_t sub = menuVerticalPosition - 1;

#if   !defined(CPUM64) || defined(AUTOSWITCH)
  uint8_t eeFlags = EE_MODEL;
#endif

  for (uint8_t i=0; i<LCD_LINES-1; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;

    CustomFunctionData *cfn = &functions[k];
    uint8_t func = CFN_FUNC(cfn);
    for (uint8_t j=0; j<5; j++) {
      uint8_t attr = ((sub==k && menuHorizontalPosition==j) ? ((s_editMode>0) ? BLINK|INVERS : INVERS) : 0);
      uint8_t active = (attr && (s_editMode>0 || p1valdiff));
      switch (j) {
        case 0:
          lcdPutsSwitches(MODEL_CUSTOM_FUNC_1ST_COLUMN, y, CFN_SWITCH(cfn), attr | ((functionsContext->activeSwitches & ((MASK_CFN_TYPE)1 << k)) ? BOLD : 0));
          if (active || AUTOSWITCH_ENTER_LONG()) CHECK_INCDEC_SWITCH(event, CFN_SWITCH(cfn), SWSRC_FIRST, SWSRC_LAST, eeFlags, isSwitchAvailableInCustomFunctions);
          break;

        case 1:
          if (CFN_SWITCH(cfn)) {
            lcdDrawTextAtIndex(MODEL_CUSTOM_FUNC_2ND_COLUMN, y, STR_VFSWFUNC, func, attr);
            if (active) {
              CHECK_INCDEC_MODELVAR_ZERO(event, CFN_FUNC(cfn), FUNC_MAX-1);
              if (checkIncDec_Ret) CFN_RESET(cfn);
            }
          }
          else {
            j = 4; // skip other fields
            if (sub==k && menuHorizontalPosition > 0) {
              REPEAT_LAST_CURSOR_MOVE();
            }
          }
          break;

        case 2:
        {
          int8_t maxParam = NUM_CHNOUT-1;
#if defined(OVERRIDE_CHANNEL_FUNCTION)
          if (func == FUNC_OVERRIDE_CHANNEL) {
            putsChn(lcdNextPos, y, CFN_CH_INDEX(cfn)+1, attr);
          }
          else
#endif
          if (func == FUNC_TRAINER) {
            maxParam = 4;
            putsMixerSource(lcdNextPos, y, MIXSRC_Rud+CFN_CH_INDEX(cfn)-1, attr);
          }
#if defined(GVARS)
          else if (func == FUNC_ADJUST_GVAR) {
            maxParam = MAX_GVARS-1;
            lcdDrawStringWithIndex(lcdNextPos, y, STR_GV, CFN_GVAR_INDEX(cfn)+1, attr);
            if (active) CHECK_INCDEC_MODELVAR_ZERO(event, CFN_GVAR_INDEX(cfn), maxParam);
            break;
          }
#endif
          else if (attr) {
            REPEAT_LAST_CURSOR_MOVE();
          }
          if (active) CHECK_INCDEC_MODELVAR_ZERO(event, CFN_CH_INDEX(cfn), maxParam);
          break;
        }

        case 3:
        {
          INCDEC_DECLARE_VARS(eeFlags);
          int16_t val_displayed = CFN_PARAM(cfn);
          int8_t val_min = 0;
          uint8_t val_max = 255;
          if (func == FUNC_RESET) {
            val_max = FUNC_RESET_PARAM_LAST;
            lcdDrawTextAtIndex(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, STR_VFSWRESET, CFN_PARAM(cfn), attr);
          }
#if defined(OVERRIDE_CHANNEL_FUNCTION)
          else if (func == FUNC_OVERRIDE_CHANNEL) {
            val_displayed = (int8_t)CFN_PARAM(cfn);
            val_min = -LIMIT_EXT_PERCENT; val_max = +LIMIT_EXT_PERCENT;
            lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
          }
#endif
#if defined(AUDIO)
          else if (func == FUNC_PLAY_SOUND) {
            val_max = AU_FRSKY_LAST-AU_FRSKY_FIRST-1;
            lcdDrawTextAtIndex(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, STR_FUNCSOUNDS, val_displayed, attr);
          }
#endif
#if defined(HAPTIC)
          else if (func == FUNC_HAPTIC) {
            val_max = 3;
            lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
          }
#endif
#if   defined(VOICE)
          else if (func == FUNC_PLAY_TRACK) {
#if defined(GVARS)
            if (attr && event==EVT_KEY_LONG(KEY_ENTER)) {
              killEvents(event);
              s_editMode = !s_editMode;
              active = true;
              val_displayed = (val_displayed > 250 ? 0 : 251);
            }
            if (val_displayed > 250) {
              lcdDrawStringWithIndex(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, STR_GV, val_displayed-250, attr);
            }
            else {
              lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed+PROMPT_CUSTOM_BASE, attr|LEFT);
            }
#else
            lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed+PROMPT_CUSTOM_BASE, attr|LEFT);
#endif
          }
          else if (func == FUNC_PLAY_BOTH) {
            lcdDrawCharAtt(MODEL_CUSTOM_FUNC_3RD_COLUMN+3*FWNUM, y, '|', attr);
            lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN+3*FWNUM, y, val_displayed+PROMPT_CUSTOM_BASE, attr);
            lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN+2+3*FWNUM, y, (val_displayed+PROMPT_CUSTOM_BASE+1)%10, attr|LEFT);
          }
          else if (func == FUNC_PLAY_VALUE) {
            val_max = MIXSRC_FIRST_TELEM + TELEM_DISPLAY_MAX - 1;
            putsMixerSource(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed, attr);
            INCDEC_ENABLE_CHECK(functionsContext == &globalFunctionsContext ? isSourceAvailableInGlobalFunctions : isSourceAvailable);
          }
#endif
#if defined(SDCARD)
          else if (func == FUNC_LOGS) {
            if (val_displayed) {
              lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed, attr|PREC1|LEFT);
              lcdDrawChar(lcdLastPos, y, 's');
            }
            else {
              lcdDrawTextAtIndex(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, STR_MMMINV, 0, attr);
            }
          }
#endif
#if defined(GVARS)
          else if (func == FUNC_ADJUST_GVAR) {
            switch (CFN_GVAR_MODE(cfn)) {
              case FUNC_ADJUST_GVAR_CONSTANT:
                val_displayed = (int16_t)CFN_PARAM(cfn);
                val_min = -CFN_GVAR_CST_MAX; val_max = +CFN_GVAR_CST_MAX;
                lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
                break;
              case FUNC_ADJUST_GVAR_SOURCE:
                val_max = MIXSRC_LAST_CH;
                putsMixerSource(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, val_displayed, attr);
                INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE);
                INCDEC_ENABLE_CHECK(isSourceAvailable);
                break;
              case FUNC_ADJUST_GVAR_GVAR:
                val_max = MAX_GVARS-1;
                lcdDrawStringWithIndex(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, STR_GV, val_displayed+1, attr);
                break;
              default: // FUNC_ADJUST_GVAR_INC
                val_max = 1;
                lcdDrawTextAtIndex(MODEL_CUSTOM_FUNC_3RD_COLUMN, y, PSTR("\003-=1+=1"), val_displayed, attr);
                break;
            }

            if (attr && event==EVT_KEY_LONG(KEY_ENTER)) {
              killEvents(event);
              s_editMode = !s_editMode;
              active = true;
              CFN_GVAR_MODE(cfn) += 1;
              val_displayed = 0;
            }
          }
#endif
          else if (attr) {
            REPEAT_LAST_CURSOR_MOVE();
          }

          if (active) {
            CFN_PARAM(cfn) = CHECK_INCDEC_PARAM(event, val_displayed, val_min, val_max);
          }
          break;
        }

        case 4:
          if (HAS_ENABLE_PARAM(func)) {
            menu_lcd_onoff(MODEL_CUSTOM_FUNC_4TH_COLUMN_ONOFF, y, CFN_ACTIVE(cfn), attr);
            if (active) CHECK_INCDEC_MODELVAR_ZERO(event, CFN_ACTIVE(cfn), 1);
          }
          else if (HAS_REPEAT_PARAM(func)) {
            if (CFN_PLAY_REPEAT(cfn) == 0) {
              lcdDrawCharAtt(MODEL_CUSTOM_FUNC_4TH_COLUMN_ONOFF+3, y, '-', attr);
            }
            else {
              lcdDrawNumberAttUnit(MODEL_CUSTOM_FUNC_4TH_COLUMN+2+FW, y, CFN_PLAY_REPEAT(cfn)*CFN_PLAY_REPEAT_MUL, attr);
            }
            if (active) CHECK_INCDEC_MODELVAR_ZERO(event, CFN_PLAY_REPEAT(cfn), 60/CFN_PLAY_REPEAT_MUL);
          }
          else if (attr) {
            REPEAT_LAST_CURSOR_MOVE();
          }
          break;
      }
    }
  }
}

void menuModelCustomFunctions(uint8_t event)
{
  MENU(STR_MENUCUSTOMFUNC, menuTabModel, e_CustomFunctions, NUM_CFN+1, {0, NAVIGATION_LINE_BY_LINE|4/*repeated*/});
  return menuCustomFunctions(event, g_model.customFn, &modelFunctionsContext);
}
