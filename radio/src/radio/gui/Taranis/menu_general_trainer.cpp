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

#include "../../opentx.h"

#define TRAINER_CALIB_POS 12

void menuGeneralTrainer(uint8_t event)
{
  uint8_t y;
  bool slave = SLAVE_MODE();

  MENU(STR_MENUTRAINER, menuTabGeneral, e_Trainer, (slave ? 0 : 6), { 2, 2, 2, 2, 0/*, 0*/ });

  if (slave) {
    lcd_puts(7*FW, 4*FH, STR_SLAVE);
    return;
  }

  LcdFlags attr;
  LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);

  lcd_puts(3*FW, MENU_HEADER_HEIGHT+1, STR_MODESRC);

  y = MENU_HEADER_HEIGHT + 1 + FH;
  int sub = menuVerticalPosition + 1;

  for (int i=1; i<=NUM_STICKS; i++) {
    uint8_t chan = channel_order(i);
    volatile TrainerMix *td = &g_eeGeneral.trainer.mix[chan-1];

    putsStickName(0, y, chan-1, (sub==i && CURSOR_ON_LINE()) ? INVERS : 0);

    for (int j=0; j<3; j++) {

      attr = ((sub==i && menuHorizontalPosition==j) ? blink : 0);

      switch(j) {
        case 0:
          lcd_putsiAtt(4*FW, y, STR_TRNMODE, td->mode, attr);
          if (attr&BLINK) CHECK_INCDEC_GENVAR(event, td->mode, 0, 2);
          break;

        case 1:
          lcd_outdezAtt(11*FW, y, td->studWeight, attr);
          if (attr&BLINK) CHECK_INCDEC_GENVAR(event, td->studWeight, -125, 125);
          break;

        case 2:
          lcd_putsiAtt(12*FW, y, STR_TRNCHN, td->srcChn, attr);
          if (attr&BLINK) CHECK_INCDEC_GENVAR(event, td->srcChn, 0, 3);
          break;
      }
    }
    y += FH;
  }

  attr = (sub==5) ? blink : 0;
  lcd_putsLeft(MENU_HEADER_HEIGHT+1+5*FH, STR_MULTIPLIER);
  lcd_outdezAtt(LEN_MULTIPLIER*FW+3*FW, MENU_HEADER_HEIGHT+1+5*FH, g_eeGeneral.PPM_Multiplier+10, attr|PREC1);
  if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.PPM_Multiplier, -10, 40);

  attr = (sub==6) ? INVERS : 0;
  if (attr) s_editMode = 0;
  lcd_putsAtt(0*FW, MENU_HEADER_HEIGHT+1+6*FH, STR_CAL, attr);
  for (int i=0; i<4; i++) {
    uint8_t x = (i*TRAINER_CALIB_POS+16)*FW/2;
#if defined (PPM_UNIT_PERCENT_PREC1)
    lcd_outdezAtt(x, MENU_HEADER_HEIGHT+1+6*FH, (ppmInput[i]-g_eeGeneral.trainer.calib[i])*2, PREC1);
#else
    lcd_outdezAtt(x, MENU_HEADER_HEIGHT+1+6*FH, (ppmInput[i]-g_eeGeneral.trainer.calib[i])/5, 0);
#endif
  }

  if (attr) {
    if (event==EVT_KEY_LONG(KEY_ENTER)){
      memcpy(g_eeGeneral.trainer.calib, ppmInput, sizeof(g_eeGeneral.trainer.calib));
      eeDirty(EE_GENERAL);
      AUDIO_WARNING1();
    }
  }
}
