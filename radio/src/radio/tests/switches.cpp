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

#include "gtests.h"

#if !defined(PCBTARANIS)
TEST(getSwitch, undefCSW)
{
  MODEL_RESET();
  EXPECT_EQ(getSwitch(NUM_PSWITCH), false);
  EXPECT_EQ(getSwitch(-NUM_PSWITCH), true); // no good answer there!
}
#endif

#if !defined(CPUARM)
TEST(getSwitch, circularCSW)
{
  MODEL_RESET();
  MIXER_RESET();
  g_model.logicalSw[0] = { SWSRC_SW1, SWSRC_SW1, LS_FUNC_OR };
  g_model.logicalSw[1] = { SWSRC_SW1, SWSRC_SW1, LS_FUNC_AND };

  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(-SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
  EXPECT_EQ(getSwitch(-SWSRC_SW2), true);
}
#endif

#if defined(PCBTARANIS)

void setLogicalSwitch(int index, uint16_t _func, int16_t _v1, int16_t _v2, int16_t _v3 = 0, uint8_t _delay = 0, uint8_t _duration = 0, int8_t _andsw = 0) 
{
  g_model.logicalSw[index].func = _func;
  g_model.logicalSw[index].v1 = _v1;
  g_model.logicalSw[index].v2 = _v2;
  g_model.logicalSw[index].v3 = _v3;
  g_model.logicalSw[index].delay = _delay;
  g_model.logicalSw[index].duration = _duration;
  g_model.logicalSw[index].andsw = _andsw;
}

TEST(getSwitch, OldTypeStickyCSW)
{
  RADIO_RESET();
  MODEL_RESET();
  MIXER_RESET();

  setLogicalSwitch(0, LS_FUNC_AND, SWSRC_SA0, SWSRC_NONE);
  setLogicalSwitch(1, LS_FUNC_OR, SWSRC_SW1, SWSRC_SW2);

  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SA0, both switches should become true
  simuSetSwitch(0, -1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(0, 0);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now reset logical switches
  logicalSwitchesReset();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
}
#endif // #if defined(PCBTARANIS)

TEST(getSwitch, nullSW)
{
  MODEL_RESET();
  EXPECT_EQ(getSwitch(0), true);
}

#if 0
TEST(getSwitch, DISABLED_VfasWithDelay)
{
  MODEL_RESET();
  MIXER_RESET();
  memclear(&frskyData, sizeof(frskyData));
  /*
  Test for logical switch:
      L1  Vfas < 9.6 Delay (0.5s)

  (gdb) print Open9xX9D::g_model.logicalSw[0] 
  $3 = {v1 = -39 '\331', v2 = 96, v3 = 0, func = 4 '\004', delay = 5 '\005', duration = 0 '\000', andsw = 0 '\000'}
  */
  g_model.logicalSw[0] = {int8_t(MIXSRC_FIRST_TELEM+TELEM_VFAS-1), 96, 0, 4, 5, 0, 0};
  frskyData.hub.vfas = 150;   //unit is 100mV

  //telemetry streaming is FALSE, so L1 should be FALSE no matter what value Vfas has
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //every logicalSwitchesTimerTick() represents 100ms
  //so now after 5 ticks we should still have a FALSE value
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //now turn on telemetry
  EXPECT_EQ(TELEMETRY_STREAMING(), false);
  TELEMETRY_RSSI() = 50;
  EXPECT_EQ(TELEMETRY_STREAMING(), true);

  //vfas is 15.0V so L1 should still be FALSE
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //now reduce vfas to 9.5V and L1 should become TRUE after 0.5s
  frskyData.hub.vfas = 95;
  evalLogicalSwitches();

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);

  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);


  //now stop telemetry, L1 should become FALSE immediatelly
  TELEMETRY_RSSI() = 0;
  EXPECT_EQ(TELEMETRY_STREAMING(), false);
  evalLogicalSwitches();

  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
}

TEST(getSwitch, DISABLED_RssiWithDuration)
{
  MODEL_RESET();
  MIXER_RESET();
  memclear(&frskyData, sizeof(frskyData));
  /*
  Test for logical switch:
      L1  RSSI > 10 Duration (0.5s)

  (gdb) print Open9xX9D::g_model.logicalSw[0] 
  $1 = {v1 = -55 '\311', v2 = 10, v3 = 0, func = 3 '\003', delay = 0 '\000', duration = 5 '\005', andsw = 0 '\000'}
  */

  g_model.logicalSw[0] = {int8_t(MIXSRC_FIRST_TELEM+TELEM_RSSI_RX-1), 10, 0, 3, 0, 5, 0};

  EXPECT_EQ(TELEMETRY_STREAMING(), false);

  evalLogicalSwitches();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //now set RSSI to 5, L1 should still be FALSE
  TELEMETRY_RSSI() = 5;
  evalLogicalSwitches();
  EXPECT_EQ(TELEMETRY_STREAMING(), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //now set RSSI to 100, L1 should become TRUE for 0.5s
  TELEMETRY_RSSI() = 100;
  evalLogicalSwitches();
  EXPECT_EQ(TELEMETRY_STREAMING(), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //repeat  telemetry streaming OFF and ON to test for duration processing
  TELEMETRY_RSSI() = 0;
  evalLogicalSwitches();
  EXPECT_EQ(TELEMETRY_STREAMING(), false);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  //now set RSSI to 100, L1 should become TRUE for 0.5s
  TELEMETRY_RSSI() = 100;
  evalLogicalSwitches();
  EXPECT_EQ(TELEMETRY_STREAMING(), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);

  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
}
#endif // #if defined(PCBTARANIS) && defined(FRSKY)


#if !defined(CPUARM)
TEST(getSwitch, recursiveSW)
{
  MODEL_RESET();
  MIXER_RESET();

  g_model.logicalSw[0] = { SWSRC_RUD, -SWSRC_SW2, LS_FUNC_OR };
  g_model.logicalSw[1] = { SWSRC_ELE, -SWSRC_SW1, LS_FUNC_OR };

  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  LS_RECURSIVE_EVALUATION_RESET();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  simuSetSwitch(1, 1);
  LS_RECURSIVE_EVALUATION_RESET();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  LS_RECURSIVE_EVALUATION_RESET();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
}
#endif // #if !defined(CPUARM)

#if defined(PCBTARANIS)
TEST(getSwitch, inputWithTrim)
{
  MODEL_RESET();
  modelDefault(0);
  MIXER_RESET();

  g_model.logicalSw[0] = { LS_FUNC_VPOS, MIXSRC_FIRST_INPUT, 0, 0 };

  doMixerCalculations();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  setTrimValue(0, 0, 32);
  doMixerCalculations();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
}
#endif

#if defined(PCBTARANIS)
TEST(evalLogicalSwitches, playFile)
{
  SYSTEM_RESET();
  MODEL_RESET();
  modelDefault(0);
  MIXER_RESET();

  extern uint64_t sdAvailableLogicalSwitchAudioFiles;
  sdAvailableLogicalSwitchAudioFiles = 0xffffffffffffffff;
  char filename[AUDIO_FILENAME_MAXLEN+1];
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (0 << 16) + AUDIO_EVENT_OFF, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/MODEL01/L1-off.wav"), 0);
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (0 << 16) + AUDIO_EVENT_ON, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/MODEL01/L1-on.wav"), 0);
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (31 << 16) + AUDIO_EVENT_OFF, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/MODEL01/L32-off.wav"), 0);
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (31 << 16) + AUDIO_EVENT_ON, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/MODEL01/L32-on.wav"), 0);
}

TEST(getSwitch, edgeInstant)
{
  MODEL_RESET();
  MIXER_RESET();
  // LS1 setup: EDGE SFup  (0:instant)
  // LS2 setup: (EDGE SFup  (0:instant)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, SWSRC_SF2, -129, -1);
  setLogicalSwitch(1, LS_FUNC_EDGE, SWSRC_SF2, -129, -1, 0, 0, SWSRC_SA2 );

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 0);   //SF down
  // EXPECT_EQ(getSwitch(SWSRC_SF2), false);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SFup, LS1 should become true
  simuSetSwitch(5, 1);    //SF up
  // EXPECT_EQ(getSwitch(SWSRC_SF2), true);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);


  // second part with SAup

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 0);   //SF down
  // EXPECT_EQ(getSwitch(SWSRC_SF2), false);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SFup, LS1 should become true
  simuSetSwitch(5, 1);    //SF up
  // EXPECT_EQ(getSwitch(SWSRC_SF2), true);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now bug #2939  
  // SF is kept up and SA is toggled
  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  //test what happens when EDGE condition is true and
  //logical switches are reset - the switch should fire again

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);  //switch will not trigger, because SF was already up
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
}

TEST(getSwitch, edgeRelease)
{
  MODEL_RESET();
  MIXER_RESET();
  // test for issue #2728
  // LS1 setup: EDGE SFup  (0:release)
  // LS2 setup: (EDGE SFup  (0:release)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, SWSRC_SF2, -129, 0);
  setLogicalSwitch(1, LS_FUNC_EDGE, SWSRC_SF2, -129, 0, 0, 0, SWSRC_SA2 );

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);


  // second part with SAup
  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // with switches reset both should remain false
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

}

#endif  // #if defined(CPUARM)
