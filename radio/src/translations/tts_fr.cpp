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

enum FrenchPrompts {
  FR_PROMPT_NUMBERS_BASE = 0,
  FR_PROMPT_ZERO = FR_PROMPT_NUMBERS_BASE+0,
  FR_PROMPT_CENT = FR_PROMPT_NUMBERS_BASE+100,
  FR_PROMPT_DEUX_CENT = FR_PROMPT_CENT+1,
  FR_PROMPT_MILLE = FR_PROMPT_CENT+9,

  FR_PROMPT_UNE = 110,
  FR_PROMPT_ONZE,
  FR_PROMPT_VINGT_ET_UNE,
  FR_PROMPT_TRENTE_ET_UNE,
  FR_PROMPT_QUARANTE_ET_UNE,
  FR_PROMPT_CINQUANTE_ET_UNE,
  FR_PROMPT_SOIXANTE_ET_UNE,
  FR_PROMPT_SOIXANTE_ET_ONZE,
  FR_PROMPT_QUATRE_VINGT_UNE,

  FR_PROMPT_VIRGULE = 119,
  FR_PROMPT_ET = 120,
  FR_PROMPT_MOINS = 121,
  FR_PROMPT_MINUIT = 122,
  FR_PROMPT_MIDI = 123,

  FR_PROMPT_UNITS_BASE = 124,
  
#if defined (PCBMEGA2560)
  FR_PROMPT_HEURE = FR_PROMPT_UNITS_BASE+UNIT_HOURS+1, //test Bracame
  FR_PROMPT_MINUTE = FR_PROMPT_UNITS_BASE+UNIT_MINUTES+1,
  FR_PROMPT_SECONDE = FR_PROMPT_UNITS_BASE+UNIT_SECONDS+1,
#else
  FR_PROMPT_HEURE = FR_PROMPT_UNITS_BASE+UNIT_HOURS,
  FR_PROMPT_MINUTE = FR_PROMPT_UNITS_BASE+UNIT_MINUTES,
  FR_PROMPT_SECONDE = FR_PROMPT_UNITS_BASE+UNIT_SECONDS,
#endif
  FR_PROMPT_VIRGULE_BASE = 180, //,0 - ,9
};

#if defined(VOICE)

#define FEMININ 0x80

I18N_PLAY_FUNCTION(fr, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
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
    PUSH_NUMBER_PROMPT(FR_PROMPT_MOINS);
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
      PUSH_NUMBER_PROMPT(FR_PROMPT_VIRGULE_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  if (number >= 1000) {
    if (number >= 2000)
      PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(FR_PROMPT_MILLE);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    if (number >= 200)
      PUSH_NUMBER_PROMPT(FR_PROMPT_ZERO + number/100);
    PUSH_NUMBER_PROMPT(FR_PROMPT_CENT);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (((number % 10) == 1) && number < 90 && (att & FEMININ)) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_UNE+(number/10));
  }
  else if (number >= 0) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_ZERO+number);
  }

  if (unit) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_UNITS_BASE+unit);
  }
}

I18N_PLAY_FUNCTION(fr, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(0, 0, FEMININ);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_MOINS);
    seconds = -seconds;
  }

  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (IS_PLAY_TIME() && tmp==0) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_MINUIT);
  }
  else if (IS_PLAY_TIME() && tmp==12) {
    PUSH_NUMBER_PROMPT(FR_PROMPT_MIDI);
  }
  else if (tmp > 0) {
    PLAY_NUMBER(tmp, 0, FEMININ);
    PUSH_NUMBER_PROMPT(FR_PROMPT_HEURE);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0) {
    if (IS_PLAY_TIME()) {
      PLAY_NUMBER(tmp, 0, tmp==1 ? FEMININ : 0);
    }
    else {
      PLAY_NUMBER(tmp, 0, FEMININ);
      PUSH_NUMBER_PROMPT(FR_PROMPT_MINUTE);
      if (seconds > 0)
        PUSH_NUMBER_PROMPT(FR_PROMPT_ET);
    }
  }

  if (!IS_PLAY_TIME() && seconds > 0) {
    PLAY_NUMBER(seconds, 0, FEMININ);
    PUSH_NUMBER_PROMPT(FR_PROMPT_SECONDE);
  }
}

LANGUAGE_PACK_DECLARE(fr, "Francais");

#endif
