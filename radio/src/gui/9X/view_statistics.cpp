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
#include "../../timers.h"

void menuStatisticsView(uint8_t event)
{
  TITLE(STR_MENUSTAT);

  switch(event)
  {
    case EVT_KEY_FIRST(KEY_UP):
      chainMenu(menuStatisticsDebug);
      break;

    case EVT_KEY_FIRST(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }

  lcd_puts(  1*FW, FH*0, STR_TOTTM1TM2THRTHP);
  putsTimer(    5*FW+5*FWNUM+1, FH*1, timersStates[0].val, 0, 0);
  putsTimer(   12*FW+5*FWNUM+1, FH*1, timersStates[1].val, 0, 0);

  putsTimer(    5*FW+5*FWNUM+1, FH*2, s_timeCumThr, 0, 0);
  putsTimer(   12*FW+5*FWNUM+1, FH*2, s_timeCum16ThrP/16, 0, 0);

  putsTimer(   12*FW+5*FWNUM+1, FH*0, sessionTimer, 0, 0);
  
#if defined(THRTRACE)
  coord_t traceRd = (s_traceCnt < 0 ? s_traceWr : 0);
  const coord_t x = 5;
  const coord_t y = 60;
  lcd_hline(x-3, y, MAXTRACE+3+3);
  lcd_vline(x, y-32, 32+3);

  for (coord_t i=0; i<MAXTRACE; i+=6) {
    lcd_vline(x+i+6, y-1, 3);
  }
  for (coord_t i=1; i<=MAXTRACE; i++) {
    lcd_vline(x+i, y-s_traceBuf[traceRd], s_traceBuf[traceRd]);
    traceRd++;
    if (traceRd>=MAXTRACE) traceRd = 0;
    if (traceRd==s_traceWr) break;
  }
#endif
}

  #define MENU_DEBUG_COL1_OFS   (14*FW)

void menuStatisticsDebug(uint8_t event)
{
  TITLE(STR_MENUDEBUG);

  switch(event)
  {
    case EVT_KEY_FIRST(KEY_ENTER):
      g_tmr1Latency_min = 0xff;
      g_tmr1Latency_max = 0;
      maxMixerDuration  = 0;
      AUDIO_KEYPAD_UP();
      break;

#if defined(DEBUG_TRACE_BUFFER)
    case EVT_KEY_FIRST(KEY_UP):
      pushMenu(menuTraceBuffer);
      return;
#endif

    case EVT_KEY_FIRST(KEY_DOWN):
      chainMenu(menuStatisticsView);
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }


#if defined(TX_CAPACITY_MEASUREMENT)
  // current
  lcd_putsLeft(MENU_DEBUG_Y_CURRENT, STR_CPU_CURRENT);
  putsValueWithUnit(MENU_DEBUG_COL1_OFS, MENU_DEBUG_Y_CURRENT, getCurrent(), UNIT_MILLIAMPS, LEFT);
  uint32_t current_scale = 488 + g_eeGeneral.txCurrentCalibration;
  lcd_putc(MENU_DEBUG_COL2_OFS, MENU_DEBUG_Y_CURRENT, '>');
  putsValueWithUnit(MENU_DEBUG_COL2_OFS+FW+1, MENU_DEBUG_Y_CURRENT, Current_max*10*current_scale/8192, UNIT_RAW, LEFT);
  // consumption
  lcd_putsLeft(MENU_DEBUG_Y_MAH, STR_CPU_MAH);
  putsValueWithUnit(MENU_DEBUG_COL1_OFS, MENU_DEBUG_Y_MAH, g_eeGeneral.mAhUsed + Current_used*current_scale/8192/36, UNIT_MAH, LEFT|PREC1);
#endif


#if defined(COPROCESSOR)
  lcd_putsLeft(MENU_DEBUG_Y_COPROC, STR_COPROC_TEMP);

  if (Coproc_read==0) {
    lcd_putsAtt(MENU_DEBUG_COL1_OFS, MENU_DEBUG_Y_COPROC, PSTR("Co Proc NACK"),INVERS);
  }
  else if (Coproc_read==0x81) {
    lcd_putsAtt(MENU_DEBUG_COL1_OFS, MENU_DEBUG_Y_COPROC, PSTR("Inst.TinyApp"),INVERS);
  }
  else if (Coproc_read<3) {
    lcd_putsAtt(MENU_DEBUG_COL1_OFS, MENU_DEBUG_Y_COPROC, PSTR("Upgr.TinyApp"),INVERS);
  }
  else {
    putsValueWithUnit(MENU_DEBUG_COL1_OFS, MENU_DEBUG_Y_COPROC, Coproc_temp, UNIT_TEMPERATURE, LEFT);
    putsValueWithUnit(MENU_DEBUG_COL2_OFS, MENU_DEBUG_Y_COPROC, Coproc_maxtemp, UNIT_TEMPERATURE, LEFT);
  }
#endif



  lcd_putsLeft(1*FH, STR_TMR1LATMAXUS);
  lcd_outdez8(MENU_DEBUG_COL1_OFS , 1*FH, g_tmr1Latency_max/2 );
  lcd_putsLeft(2*FH, STR_TMR1LATMINUS);
  lcd_outdez8(MENU_DEBUG_COL1_OFS , 2*FH, g_tmr1Latency_min/2 );
  lcd_putsLeft(3*FH, STR_TMR1JITTERUS);
  lcd_outdez8(MENU_DEBUG_COL1_OFS , 3*FH, (g_tmr1Latency_max - g_tmr1Latency_min) /2 );
  lcd_putsLeft(4*FH, STR_TMIXMAXMS);
  lcd_outdezAtt(MENU_DEBUG_COL1_OFS, 4*FH, DURATION_MS_PREC2(maxMixerDuration), PREC2);
  lcd_putsLeft(5*FH, STR_FREESTACKMINB);
  lcd_outdezAtt(14*FW, 5*FH, stackAvailable(), UNSIGN) ;

  lcd_puts(4*FW, 7*FH+1, STR_MENUTORESET);
  lcd_status_line();
}
