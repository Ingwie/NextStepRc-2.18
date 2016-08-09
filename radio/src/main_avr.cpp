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

#include "opentx.h"

extern void checkBattery();
extern uint8_t checkTrim(uint8_t event);

void perMain()
{
#if defined(SIMU)
  doMixerCalculations();
#endif
#if defined(LCD_ST7920)
  uint8_t lcdstate=0;
#endif
  uint16_t t0 = getTmr16KHz();
  int16_t delta = (nextMixerEndTime - lastMixerDuration) - t0;
  if (delta > 0 && delta < MAX_MIXER_DELTA) {
#if defined(PCBSTD) && defined(ROTARY_ENCODER_NAVIGATION)
    rotencPoll();
#endif

    // @@@ open.20.fsguruh
    // SLEEP();   // wouldn't that make sense? should save a lot of battery power!!!
    /*  for future use; currently very very beta...  */
#if defined(POWER_SAVE)
    ADCSRA&=0x7F;   // disable ADC for power saving
    ACSR&=0xF7;   // disable ACIE Interrupts
    ACSR|=0x80;   // disable Analog Comparator
    // maybe we disable here a lot more hardware components in future to save even more power



    MCUCR|=0x20;  // enable Sleep (bit5)
    // MCUCR|=0x28;  // enable Sleep (bit5) enable ADC Noise Reduction (bit3)
    // first tests showed: simple sleep would reduce cpu current from 40.5mA to 32.0mA
    //                     noise reduction sleep would reduce it down to 28.5mA; However this would break pulses in theory
    // however with standard module, it will need about 95mA. Therefore the drop to 88mA is not much noticable
    do {
      asm volatile(" sleep        \n\t");  // if _SLEEP() is not defined use this
      t0=getTmr16KHz();
      delta= (nextMixerEndTime - lastMixerDuration) - t0;
    } while ((delta>0) && (delta<MAX_MIXER_DELTA));
    
    // reenabling of the hardware components needed here
    MCUCR&=0x00;  // disable sleep
    ADCSRA|=0x80;  // enable ADC
#endif
    return;
  }  

  nextMixerEndTime = t0 + MAX_MIXER_DELTA;
  // this is a very tricky implementation; lastMixerEndTime is just like a default value not to stop mixcalculations totally;
  // the real value for lastMixerEndTime is calculated inside pulses_XXX.cpp which aligns the timestamp to the pulses generated
  // nextMixerEndTime is actually defined inside pulses_XXX.h  

  doMixerCalculations();

  t0 = getTmr16KHz() - t0;
  lastMixerDuration = t0;
  if (t0 > maxMixerDuration) maxMixerDuration = t0;

#if defined(MODULE_ALWAYS_SEND_PULSES)
  if (startupWarningState < STARTUP_WARNING_DONE) {
    // don't do menu's until throttle and switch warnings are handled
    return;
  }
#endif

  if (!eeprom_buffer_size) {
    if (theFile.isWriting())
    theFile.nextWriteStep();
    else if (TIME_TO_WRITE())
    eeCheck(false);
  }

#if defined(SDCARD)
  sdMountPoll();
  writeLogs();
#endif

  uint8_t evt = getEvent();
  evt = checkTrim(evt);

#if defined(FRSKY) || defined(MAVLINK)
  telemetryWakeup();
#endif

#if defined(NAVIGATION_STICKS)
  uint8_t sticks_evt = getSticksNavigationEvent();
  if (sticks_evt) evt = EVT_KEY_FIRST(sticks_evt);
#endif

  if (evt && (g_eeGeneral.backlightMode & e_backlight_mode_keys)) backlightOn(); // on keypress turn the light on
  checkBacklight();

#if defined(GUI)
  const char *warn = warningText;
  bool popupMenuActive = (popupMenuNoItems > 0);

  if(IS_LCD_REFRESH_ALLOWED()){//No need to redraw until lcdRefresh_ST7920(0) below completely refreshes the display.
    lcdClear();
    if (menuEvent) {
      menuVerticalPosition = menuEvent == EVT_ENTRY_UP ? menuVerticalPositions[menuLevel] : 0;
      menuHorizontalPosition = 0;
      evt = menuEvent;
      menuEvent = 0;
      AUDIO_MENUS();
    }
    menuHandlers[menuLevel]((warn || popupMenuActive) ? 0 : evt);


    if (warn) DISPLAY_WARNING(evt);
#if defined(NAVIGATION_MENUS)
    if (popupMenuActive) {
      const char * result = displayPopupMenu(evt);
      if (result) {
        popupMenuHandler(result);
        putEvent(EVT_MENU_UP);
      }
    }
#endif
    drawStatusLine();
  }

#if defined(LCD_KS108)
  lcdRefreshSide();
#elif defined(LCD_ST7920)
  lcdstate = lcdRefresh_ST7920(0);
#else
  lcdRefresh();
#endif
#endif

  if (SLAVE_MODE()) {
    JACK_PPM_OUT();
  }
  else {
    JACK_PPM_IN();
  }

  checkBattery();
}
