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

const pm_uchar font_5x7[] PROGMEM = {
#include "font_05x07_avr.lbm"
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

#if defined(BOLD_SPECIFIC_FONT)
const pm_uchar font_5x7_B[] PROGMEM = {
#include "font_05x07_B_compressed.lbm"
};
#endif

#if !defined(BOOT)
const pm_uchar font_10x14[] PROGMEM = {
#include "font_10x14_compressed.lbm"
};
#endif

