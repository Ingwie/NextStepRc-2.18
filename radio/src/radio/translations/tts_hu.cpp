/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
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

enum HungarianPrompts {
  HU_PROMPT_NUMBERS_BASE = 0,
  HU_PROMPT_ZERO = HU_PROMPT_NUMBERS_BASE+0,       //02-99
  HU_PROMPT_HUNDRED = HU_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900
  HU_PROMPT_THOUSAND = HU_PROMPT_NUMBERS_BASE+109, //1000
  HU_PROMPT_AND = HU_PROMPT_NUMBERS_BASE+110,
  HU_PROMPT_MINUS = HU_PROMPT_NUMBERS_BASE+111,
  HU_PROMPT_POINT = HU_PROMPT_NUMBERS_BASE+112,

  HU_PROMPT_UNITS_BASE = 115,
  HU_PROMPT_VOLTS = HU_PROMPT_UNITS_BASE+UNIT_VOLTS, //(one)volt,(two)volts
  HU_PROMPT_AMPS = HU_PROMPT_UNITS_BASE+(UNIT_AMPS*2),
  HU_PROMPT_METERS_PER_SECOND = HU_PROMPT_UNITS_BASE+(UNIT_METERS_PER_SECOND*2),
  HU_PROMPT_SPARE1 = HU_PROMPT_UNITS_BASE+(UNIT_RAW*2),
  HU_PROMPT_KMH = HU_PROMPT_UNITS_BASE+(UNIT_SPEED*2),
  HU_PROMPT_METERS = HU_PROMPT_UNITS_BASE+(UNIT_DIST*2),
  HU_PROMPT_DEGREES = HU_PROMPT_UNITS_BASE+(UNIT_TEMPERATURE*2),
  HU_PROMPT_PERCENT = HU_PROMPT_UNITS_BASE+(UNIT_PERCENT*2),
  HU_PROMPT_MILLIAMPS = HU_PROMPT_UNITS_BASE+(UNIT_MILLIAMPS*2),
  HU_PROMPT_MAH = HU_PROMPT_UNITS_BASE+(UNIT_MAH*2),
  HU_PROMPT_WATTS = HU_PROMPT_UNITS_BASE+(UNIT_WATTS*2),
  HU_PROMPT_DB = HU_PROMPT_UNITS_BASE+(UNIT_DB*2),
  HU_PROMPT_FEET = HU_PROMPT_UNITS_BASE+(UNIT_FEET*2),
  HU_PROMPT_MPH = HU_PROMPT_UNITS_BASE+(UNIT_KTS*2),
  HU_PROMPT_HOURS = HU_PROMPT_UNITS_BASE+(UNIT_HOURS*2),
  HU_PROMPT_MINUTES = HU_PROMPT_UNITS_BASE+(UNIT_MINUTES*2),
  HU_PROMPT_SECONDS = HU_PROMPT_UNITS_BASE+(UNIT_SECONDS*2),
  HU_PROMPT_RPMS = HU_PROMPT_UNITS_BASE+(UNIT_RPMS*2),
  HU_PROMPT_G = HU_PROMPT_UNITS_BASE+(UNIT_G*2),
#if defined(CPUARM)
  HU_PROMPT_MILLILITERS = HU_PROMPT_UNITS_BASE+(UNIT_MILLILITERS*2),
  HU_PROMPT_FLOZ = HU_PROMPT_UNITS_BASE+(UNIT_FLOZ*2),
  HU_PROMPT_FEET_PER_SECOND = HU_PROMPT_UNITS_BASE+(UNIT_FEET_PER_SECOND*2),
#endif

  HU_PROMPT_POINT_BASE = 160, //.0 - .9

};

#if defined(VOICE)

#if defined(CPUARM)
  #define HU_PUSH_UNIT_PROMPT(p, u) hu_pushUnitPrompt((p), (u), id)
#else
  #define HU_PUSH_UNIT_PROMPT(p, u) pushUnitPrompt((p), (u))
#endif

I18N_PLAY_FUNCTION(hu, pushUnitPrompt, int16_t number, uint8_t unitprompt)
{
  if (number == 1)
    PUSH_NUMBER_PROMPT(unitprompt);
  else
    PUSH_NUMBER_PROMPT(unitprompt+1);
}

I18N_PLAY_FUNCTION(hu, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_MINUS);
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
      PUSH_NUMBER_PROMPT(HU_PROMPT_POINT_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;

  if (number >= 1000) {
    PLAY_NUMBER(number / 1000, 0, 0);
    PUSH_NUMBER_PROMPT(HU_PROMPT_THOUSAND);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_HUNDRED + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_ZERO + number);
  }
  
  if (unit) {
    HU_PUSH_UNIT_PROMPT(tmp, (HU_PROMPT_UNITS_BASE+((unit-1)*2)));
  }
}

I18N_PLAY_FUNCTION(hu, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(HU_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (tmp > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(tmp, UNIT_HOURS+1 , 0);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0) {
    PLAY_NUMBER(tmp, UNIT_MINUTES+1 , 0);
    if (seconds > 0)
      PUSH_NUMBER_PROMPT(HU_PROMPT_AND);
  }

  if (seconds > 0) {
    PLAY_NUMBER(seconds, UNIT_SECONDS+1 , 0);
  }
}

LANGUAGE_PACK_DECLARE(hu, "Hungarian");

#endif
