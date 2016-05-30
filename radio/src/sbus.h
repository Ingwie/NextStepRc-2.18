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

#ifndef _SBUS_H_
#define _SBUS_H_

#define SBUS_BAUDRATE         100000
#define SBUS_MIN_FRAME_SIZE   23
#define SBUS_MAX_FRAME_SIZE   28

void processSbusInput();
void processSbusFrame(uint8_t * sbus, int16_t * pulses, uint32_t size);

#endif // _SBUS_H_
