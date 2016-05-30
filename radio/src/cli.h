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

#ifndef _CLI_H_
#define _CLI_H_

#include "serial.h"

extern uint8_t cliTracesEnabled;

#ifdef __cplusplus
#include "fifo.h"
extern Fifo<256> cliRxFifo;
#include "tasks_arm.h"
#define CLI_STACK_SIZE                 1000
extern TaskStack<CLI_STACK_SIZE> cliStack;
#endif

void cliStart();

#endif // _CLI_H_
