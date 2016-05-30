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

#ifndef _DUMP_H_
#define _DUMP_H_

#if defined(DEBUG) || defined(CLI)
void dumpStart(unsigned int size);
void dumpBody(const uint8_t * data, unsigned int size);
void dumpEnd();
void dump(const uint8_t * data, unsigned int size);
#else
#define dump(data, size)
#endif

#endif // _DUMP_H_
