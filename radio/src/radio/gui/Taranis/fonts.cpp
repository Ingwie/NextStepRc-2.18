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

const pm_uchar font_5x7[] PROGMEM = {
#include "font_05x07.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_05x07.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_05x07.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_05x07.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_05x07.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_05x07.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_05x07.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_05x07.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_05x07.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_05x07.lbm"
#endif
};

#if defined(BOLD_FONT)
const pm_uchar font_5x7_B[] PROGMEM = {
#include "font_05x07_B_compressed.lbm"
};
#endif

#if !defined(BOOT)
const pm_uchar font_10x14[] PROGMEM = {
#include "font_10x14_compressed.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_10x14.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_10x14.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_10x14.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_10x14.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_10x14.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_10x14.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_10x14.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_10x14.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_10x14.lbm"
#endif
};

const pm_uchar font_3x5[] PROGMEM = {
#include "font_03x05.lbm"
};

const pm_uchar font_4x6[] PROGMEM = {
#include "font_04x06.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_04x06.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_04x06.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_04x06.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_04x06.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_04x06.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_04x06.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_04x06.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_04x06.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_04x06.lbm"
#endif
};

const pm_uchar font_8x10[] PROGMEM = {
#include "font_08x10.lbm"
#if defined(TRANSLATIONS_DE)
#include "font_de_08x10.lbm"
#elif defined(TRANSLATIONS_CZ)
#include "font_cz_08x10.lbm"
#elif defined(TRANSLATIONS_ES)
#include "font_es_08x10.lbm"
#elif defined(TRANSLATIONS_FI)
#include "font_fi_08x10.lbm"
#elif defined(TRANSLATIONS_FR)
#include "font_fr_08x10.lbm"
#elif defined(TRANSLATIONS_IT)
#include "font_it_08x10.lbm"
#elif defined(TRANSLATIONS_PL)
#include "font_pl_08x10.lbm"
#elif defined(TRANSLATIONS_PT)
#include "font_pt_08x10.lbm"
#elif defined(TRANSLATIONS_SE)
#include "font_se_08x10.lbm"
#endif
};

const pm_uchar font_22x38_num[] PROGMEM = {
#include "font_22x38_num.lbm"
};

const pm_uchar font_4x6_extra[] PROGMEM = {
#include "font_04x06_extra.lbm"
};

const pm_uchar font_5x7_extra[] PROGMEM = {
#include "font_05x07_extra.lbm"
};

const pm_uchar font_10x14_extra[] PROGMEM = {
#include "font_10x14_extra.lbm"
};

#endif // !defined(BOOT)
