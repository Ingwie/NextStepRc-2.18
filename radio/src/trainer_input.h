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

#ifndef trainer_input_h
#define trainer_input_h

#include "myeeprom.h"

// Trainer input channels
extern int16_t ppmInput[NUM_TRAINER];

// Timer gets decremented in per10ms()
#define PPM_IN_VALID_TIMEOUT 100 // 1s
extern uint8_t ppmInputValidityTimer;

#define IS_TRAINER_INPUT_VALID() (ppmInputValidityTimer != 0)

#define checkTrainerSignalWarning()

// Needs to be inlined to avoid slow function calls in ISR routines
inline void captureTrainerPulses(uint16_t capture)
{
  static uint16_t lastCapt=0;
  static uint8_t channelNumber=0;

  uint16_t val = (uint16_t)(capture - lastCapt) / 2;
  lastCapt = capture;

  // We process ppmInput right here to make servo movement as smooth as possible
  //    while under trainee control
  //
  // G: Prioritize reset pulse. (Needed when less than 16 incoming pulses)
  //
  if (val>4000 && val<19000) {
    channelNumber = 1; // triggered
  }
  else {
    if ((channelNumber > 0) && (channelNumber <= NUM_TRAINER)) {
      if (val>800 && val<2200) {
        ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;
        ppmInput[channelNumber++ - 1] =
          //+-500 != 512, but close enough.
          (int16_t)(val - 1500)*(g_eeGeneral.PPM_Multiplier+10)/10; 
      }
      else {
        channelNumber = 0; // not triggered
      }
    }
  }
}

#endif
