/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Bertrand Songis <bsongis@gmail.com>
 * - Jean-Pierre van Melis
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "../opentx.h"

enum DutchPrompts {
  NL_PROMPT_NUMBERS_BASE = 0,
  NL_PROMPT_ZERO = NL_PROMPT_NUMBERS_BASE+0,       //02-99
  NL_PROMPT_HUNDRED = NL_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900
  NL_PROMPT_THOUSAND = NL_PROMPT_NUMBERS_BASE+109, //1000
  NL_PROMPT_AND = NL_PROMPT_NUMBERS_BASE+110,
  NL_PROMPT_MINUS = NL_PROMPT_NUMBERS_BASE+111,
  NL_PROMPT_POINT = NL_PROMPT_NUMBERS_BASE+112,
  NL_PROMPT_UNITS_BASE = 113,
  NL_PROMPT_POINT_BASE = 165, //.0 - .9
};

#if defined(VOICE)

































#if defined(CPUARM)
  #define NL_PUSH_UNIT_PROMPT(p, u) en_pushUnitPrompt((p), (u), id)
#else
  #define NL_PUSH_UNIT_PROMPT(p, u) pushUnitPrompt((p), (u))



#endif

I18N_PLAY_FUNCTION(nl, pushUnitPrompt, int16_t number, uint8_t unitprompt)
{
  if (number == 1)
    PUSH_NUMBER_PROMPT(unitprompt);
  else
    PUSH_NUMBER_PROMPT(unitprompt+1);
}




I18N_PLAY_FUNCTION(nl, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{










  if (number < 0) {
    PUSH_NUMBER_PROMPT(NL_PROMPT_MINUS);
    number = -number;
  }

#if !defined(CPUARM)
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
#endif

  int8_t mode = MODE(att);
  if (mode > 0) {
#if defined(CPUARM)
    if (mode == 2) {
      number /= 10;
    }
#else
    // we assume that we are PREC1
#endif
    div_t qr = div(number, 10);
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(NL_PROMPT_POINT_BASE + qr.rem);
      number = -1;



    }
    else {
      number = qr.quot;

    }

  }

  int16_t tmp = number;

  if (number >= 1000) {

    PLAY_NUMBER(number / 1000, 0, 0);


    PUSH_NUMBER_PROMPT(NL_PROMPT_THOUSAND);

    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {

    PUSH_NUMBER_PROMPT(NL_PROMPT_HUNDRED + (number/100)-1);

    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
    PUSH_NUMBER_PROMPT(NL_PROMPT_ZERO + number);
  }
  

  if (unit) {
    NL_PUSH_UNIT_PROMPT(tmp, NL_PROMPT_UNITS_BASE + unit*2);
  }
}

I18N_PLAY_FUNCTION(nl, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(seconds, 0, 0);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(NL_PROMPT_MINUS);
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

      PUSH_NUMBER_PROMPT(NL_PROMPT_AND);




  }


  if (seconds > 0) {
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);



  }
}

LANGUAGE_PACK_DECLARE(nl, "Nederlands");

#endif
