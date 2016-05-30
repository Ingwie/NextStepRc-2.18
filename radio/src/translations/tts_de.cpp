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

enum GermanPrompts {
  DE_PROMPT_NUMBERS_BASE = 0,
  DE_PROMPT_NULL = DE_PROMPT_NUMBERS_BASE+0,
  DE_PROMPT_HUNDERT = DE_PROMPT_NUMBERS_BASE+100,
  DE_PROMPT_TAUSEND = DE_PROMPT_NUMBERS_BASE+101,
  DE_PROMPT_COMMA = 102,
  DE_PROMPT_UND,
  DE_PROMPT_MINUS,
  DE_PROMPT_UHR,
  DE_PROMPT_MINUTE,
  DE_PROMPT_MINUTEN,
  DE_PROMPT_SECUNDE,
  DE_PROMPT_SECUNDEN,

  DE_PROMPT_UNITS_BASE = 110,
  DE_PROMPT_VOLTS = DE_PROMPT_UNITS_BASE+UNIT_VOLTS,
  DE_PROMPT_AMPS = DE_PROMPT_UNITS_BASE+UNIT_AMPS,
  DE_PROMPT_METERS_PER_SECOND = DE_PROMPT_UNITS_BASE+UNIT_METERS_PER_SECOND,
  DE_PROMPT_SPARE1 = DE_PROMPT_UNITS_BASE+UNIT_RAW,
  DE_PROMPT_KMH = DE_PROMPT_UNITS_BASE+UNIT_SPEED,
  DE_PROMPT_METERS = DE_PROMPT_UNITS_BASE+UNIT_DIST,
  DE_PROMPT_DEGREES = DE_PROMPT_UNITS_BASE+UNIT_TEMPERATURE,
  DE_PROMPT_PERCENT = DE_PROMPT_UNITS_BASE+UNIT_PERCENT,
  DE_PROMPT_MILLIAMPS = DE_PROMPT_UNITS_BASE+UNIT_MILLIAMPS,
  DE_PROMPT_MAH = DE_PROMPT_UNITS_BASE+UNIT_MAH,
  DE_PROMPT_WATTS = DE_PROMPT_UNITS_BASE+UNIT_WATTS,
  DE_PROMPT_FEET = DE_PROMPT_UNITS_BASE+UNIT_FEET,
  DE_PROMPT_KTS = DE_PROMPT_UNITS_BASE+UNIT_KTS,
  DE_PROMPT_HOURS = DE_PROMPT_UNITS_BASE+UNIT_HOURS,
  DE_PROMPT_MINUTES = DE_PROMPT_UNITS_BASE+UNIT_MINUTES,
  DE_PROMPT_SECONDS = DE_PROMPT_UNITS_BASE+UNIT_SECONDS,
  DE_PROMPT_RPMS = DE_PROMPT_UNITS_BASE+UNIT_RPMS,
  DE_PROMPT_G = DE_PROMPT_UNITS_BASE+UNIT_G,

};

#if defined(VOICE)

I18N_PLAY_FUNCTION(de, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
/*  if digit >= 1000000000:
      temp_digit, digit = divmod(digit, 1000000000)
      prompts.extend(self.getNumberPrompt(temp_digit))
      prompts.append(Prompt(GUIDE_00_BILLION, dir=2))
  if digit >= 1000000:
      temp_digit, digit = divmod(digit, 1000000)
      prompts.extend(self.getNumberPrompt(temp_digit))
      prompts.append(Prompt(GUIDE_00_MILLION, dir=2))
*/

  if (number < 0) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_MINUS);
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
    if (qr.rem > 0) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(DE_PROMPT_COMMA);
      if (mode==2 && qr.rem < 10)
        PUSH_NUMBER_PROMPT(DE_PROMPT_NULL);
      PLAY_NUMBER(qr.rem, unit, 0);
    }
    else {
      PLAY_NUMBER(qr.quot, unit, 0);
    }
    return;
  }

  if (number >= 1000) {
    if (number >= 1100) {
      PLAY_NUMBER(number / 1000, 0, 0);
      PUSH_NUMBER_PROMPT(DE_PROMPT_TAUSEND);
    } else {
      PUSH_NUMBER_PROMPT(DE_PROMPT_TAUSEND);
    }
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    if (number >= 200)
      PUSH_NUMBER_PROMPT(DE_PROMPT_NULL + number/100);
    PUSH_NUMBER_PROMPT(DE_PROMPT_HUNDERT);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  PUSH_NUMBER_PROMPT(DE_PROMPT_NULL+number);

  if (unit) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_UNITS_BASE+unit);
  }
}

I18N_PLAY_FUNCTION(de, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t ore = 0;
  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (tmp > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(tmp, 0, 0);
    PUSH_NUMBER_PROMPT(DE_PROMPT_UHR);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0 || ore >0) {
    PLAY_NUMBER(tmp, 0, 0);
    if (tmp != 1) {
      PUSH_NUMBER_PROMPT(DE_PROMPT_MINUTEN);
    } else {
      PUSH_NUMBER_PROMPT(DE_PROMPT_MINUTE);
    }
    PUSH_NUMBER_PROMPT(DE_PROMPT_UND);
  }
  PLAY_NUMBER(seconds, 0, 0);
  if (seconds != 1) {
    PUSH_NUMBER_PROMPT(DE_PROMPT_SECUNDEN);
  } else {
    PUSH_NUMBER_PROMPT(DE_PROMPT_SECUNDE);
  }
}

LANGUAGE_PACK_DECLARE(de, "Deutsch");

#endif
