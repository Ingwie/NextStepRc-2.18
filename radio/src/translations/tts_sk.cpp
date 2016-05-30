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

enum SlovakPrompts {
  SK_PROMPT_NUMBERS_BASE = 0,
  SK_PROMPT_NULA = SK_PROMPT_NUMBERS_BASE+0,    //0-99
  SK_PROMPT_STO = SK_PROMPT_NUMBERS_BASE+100,   //100,200 .. 900
  SK_PROMPT_TISIC = SK_PROMPT_NUMBERS_BASE+109, //1000
  SK_PROMPT_DVETISIC = SK_PROMPT_NUMBERS_BASE+110,
  SK_PROMPT_JEDEN = SK_PROMPT_NUMBERS_BASE+111,
  SK_PROMPT_JEDNO = SK_PROMPT_NUMBERS_BASE+112,
  SK_PROMPT_DVE = SK_PROMPT_NUMBERS_BASE+113,
  SK_PROMPT_CELA = SK_PROMPT_NUMBERS_BASE+114,
  SK_PROMPT_CELE = SK_PROMPT_NUMBERS_BASE+115,
  SK_PROMPT_CELYCH = SK_PROMPT_NUMBERS_BASE+116,
  SK_PROMPT_MINUS = SK_PROMPT_NUMBERS_BASE+117,

  SK_PROMPT_UNITS_BASE = 118, //(jeden)volt,(dva)volty,(pet)voltu,(desetina)voltu

};

#if defined(VOICE)

  #define SK_PUSH_UNIT_PROMPT(p, u) pushUnitPrompt((p), (u))

#define MUZSKY 0x80
#define ZENSKY 0x81
#define STREDNI 0x82

I18N_PLAY_FUNCTION(sk, pushUnitPrompt, int16_t number, uint8_t unitprompt)
{
  if (number == 1)
    PUSH_NUMBER_PROMPT(unitprompt);
  else if (number > 1 && number < 5)
    PUSH_NUMBER_PROMPT(unitprompt+1);
  else
    PUSH_NUMBER_PROMPT(unitprompt+2);
}

I18N_PLAY_FUNCTION(sk, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{

  if (number < 0) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_MINUS);
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
      PLAY_NUMBER(qr.quot, 0, ZENSKY);
      if (qr.quot == 0)
        PUSH_NUMBER_PROMPT(SK_PROMPT_CELA);
      else
        SK_PUSH_UNIT_PROMPT(qr.quot, SK_PROMPT_CELA);
      PLAY_NUMBER(qr.rem, 0, ZENSKY);
      PUSH_NUMBER_PROMPT(SK_PROMPT_UNITS_BASE+((unit-1)*4)+3);
      return;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;
  
  switch(unit) {
    case 0:
      break;
    case 4:
    case 10:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
      att = ZENSKY;
      break;
    case 8:
    case 19:
      att = STREDNI;
      break;
    default:
      att = MUZSKY;
      break;
  }

  if ((number == 1) && (att == MUZSKY)) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_JEDEN);
    number = -1;
  }
  
  if ((number == 1) && (att == STREDNI)) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_JEDNO);
    number = -1;
  }
  
  if ((number == 2) && ((att == ZENSKY) || (att == STREDNI))) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_DVE);
    number = -1;
  }
  
  if (number >= 1000) { 
    if (number >= 3000)
      PLAY_NUMBER(number / 1000, 0, 0);     
    if (number >= 2000 && number < 3000)
      PUSH_NUMBER_PROMPT(SK_PROMPT_DVETISIC);
    else
      PUSH_NUMBER_PROMPT(SK_PROMPT_TISIC);

    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_STO + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  
  if (number >= 0) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_NULA+number);
  }

  if (unit) {
    SK_PUSH_UNIT_PROMPT(tmp, (SK_PROMPT_UNITS_BASE+((unit-1)*4)));
  }
}

I18N_PLAY_FUNCTION(sk, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(SK_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (tmp > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(tmp, UNIT_HOURS+1, ZENSKY);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0) {
    PLAY_NUMBER(tmp, UNIT_MINUTES+1, ZENSKY);
  }

  if (seconds > 0) {
    PLAY_NUMBER(seconds, UNIT_SECONDS+1, ZENSKY);
  }
}

LANGUAGE_PACK_DECLARE(sk, "Slovak");

#endif
