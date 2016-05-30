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

#if defined(SIMU)
#define dumpPrintf(...) debugPrintf(__VA_ARGS__)
#else
#define dumpPrintf(...) serialPrintf(__VA_ARGS__)
#endif

uint8_t dumpPosition;

void dumpStart(unsigned int size)
{
  dumpPrintf("DUMP %d bytes ...\n\r", size);
  dumpPosition = 0;
}

void dumpBody(const uint8_t *data, unsigned int size)
{
  for (unsigned int i=0; i<size; i++) {
    dumpPrintf("%.2X ", data[i]);
    dumpPosition++;
    if ((dumpPosition & (32-1)) == 0) {
      dumpPrintf("\r\n");
    }
  }
}

void dumpEnd()
{
  dumpPrintf("\r\n");
}

void dump(const uint8_t *data, unsigned int size)
{
  dumpStart(size);
  dumpBody(data, size);
  dumpEnd();
}
