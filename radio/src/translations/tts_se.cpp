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

#include "../opentx.h"

enum SwedishPrompts {
  SE_PROMPT_NUMBERS_BASE = 0,
  SE_PROMPT_ZERO = SE_PROMPT_NUMBERS_BASE+0,       //02-99
  SE_PROMPT_HUNDRED = SE_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900
  SE_PROMPT_THOUSAND = SE_PROMPT_NUMBERS_BASE+109, //1000
  SE_PROMPT_AND = SE_PROMPT_NUMBERS_BASE+110,
  SE_PROMPT_MINUS = SE_PROMPT_NUMBERS_BASE+111,
  SE_PROMPT_POINT = SE_PROMPT_NUMBERS_BASE+112,
  SE_PROMPT_UNITS_BASE = 113,
  SE_PROMPT_POINT_BASE = 165, //.0 - .9
};

#if defined(VOICE)

I18N_PLAY_FUNCTION(se, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_MINUS);
    number = -number;
  }

  if (unit) {
    unit--;
    convertUnit(number, unit);
    if (IS_IMPERIAL_ENABLE()) {
      if (unit == UNIT_DIST) {
        unit = UNIT_FEET;
      }
      if (unit == UNIT_SPEED) {
    	unit = UNIT_KTS;
      }
    }
    unit++;
  }

  int8_t mode = MODE(att);
  if (mode > 0) {
    // we assume that we are PREC1
    div_t qr = div(number, 10);
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(SE_PROMPT_POINT_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }
  
  int16_t tmpNumber = number;
  
  if (number >= 1000) {
    PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(SE_PROMPT_THOUSAND);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_HUNDRED + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_ZERO + number);
  }
  
  if (unit) {
    if  (tmpNumber == 1 ) 
      PUSH_NUMBER_PROMPT(SE_PROMPT_UNITS_BASE+unit*2);   // Singular prompts
    else
      PUSH_NUMBER_PROMPT(SE_PROMPT_UNITS_BASE+unit*2+1); // Plural prompts
  }
}

I18N_PLAY_FUNCTION(se, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(SE_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (tmp > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(tmp, UNIT_HOURS, 0);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0) {
    PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
    if (seconds > 0)
      PUSH_NUMBER_PROMPT(SE_PROMPT_AND);
  }

  if (seconds > 0) {
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
  }
}

LANGUAGE_PACK_DECLARE(se, "Swedish");

#endif
