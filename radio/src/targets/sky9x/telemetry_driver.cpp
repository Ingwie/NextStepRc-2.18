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

#define RX_UART_BUFFER_SIZE     128

struct t_rxUartBuffer
{
  uint8_t fifo[RX_UART_BUFFER_SIZE] ;
  uint8_t *outPtr ;
};

struct t_rxUartBuffer TelemetryInBuffer ;
uint16_t DsmRxTimeout;

// USART0 configuration
// Work in Progress, UNTESTED
// Uses PA5 and PA6 (RXD and TXD)
void UART2_Configure(uint32_t baudrate, uint32_t masterClock)
{
  register Usart *pUsart = SECOND_USART;

  // Configure PIO
  configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;

  // Configure PMC
  PMC->PMC_PCER0 = 1 << SECOND_ID;

  // Reset and disable receiver & transmitter */
  pUsart->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS;

  // Configure mode
  pUsart->US_MR =  0x000008C0 ;  // NORMAL, No Parity, 8 bit

  // Configure baudrate
  // Asynchronous, no oversampling
  pUsart->US_BRGR = (masterClock / baudrate) / 16;

  // Disable PDC channel
  pUsart->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;

  // Enable receiver and transmitter
  pUsart->US_CR = US_CR_RXEN | US_CR_TXEN;
}

void UART2_timeout_enable()
{
  register Usart *pUsart = SECOND_USART;
  pUsart->US_CR = US_CR_STTTO ;
  pUsart->US_RTOR = 115 ;               // Bits @ 115200 ~= 1mS
  pUsart->US_IER = US_IER_TIMEOUT ;
  DsmRxTimeout = 0 ;
  NVIC_EnableIRQ(USART0_IRQn);
}

void UART2_timeout_disable()
{
  register Usart *pUsart = SECOND_USART;
  pUsart->US_RTOR = 0 ;
  pUsart->US_IDR = US_IDR_TIMEOUT ;
  NVIC_DisableIRQ(USART0_IRQn);
}

extern "C" void USART0_IRQHandler()
{
  register Usart *pUsart = SECOND_USART;
  pUsart->US_CR = US_CR_STTTO ;         // Clears timeout bit
  DsmRxTimeout = 1;
}

void startPdcUsartReceive()
{
  register Usart *pUsart = SECOND_USART;
  TelemetryInBuffer.outPtr = TelemetryInBuffer.fifo ;
#ifndef SIMU
  pUsart->US_RPR = (uint32_t)TelemetryInBuffer.fifo ;
  pUsart->US_RNPR = (uint32_t)TelemetryInBuffer.fifo ;
#endif
  pUsart->US_RCR = RX_UART_BUFFER_SIZE ;
  pUsart->US_RNCR = RX_UART_BUFFER_SIZE ;
  pUsart->US_PTCR = US_PTCR_RXTEN ;
}

void rxPdcUsart( void (*pChProcess)(uint8_t x) )
{
#if !defined(SIMU)
  register Usart *pUsart = SECOND_USART;
  uint8_t *ptr ;
  uint8_t *endPtr ;

  // Find out where the DMA has got to
  endPtr = (uint8_t *)pUsart->US_RPR ;
  // Check for DMA passed end of buffer
  if ( endPtr > &TelemetryInBuffer.fifo[RX_UART_BUFFER_SIZE-1] )
  {
    endPtr = TelemetryInBuffer.fifo ;
  }
  ptr = TelemetryInBuffer.outPtr ;
  while ( ptr != endPtr )
  {
    (*pChProcess)(*ptr++) ;
    if ( ptr > &TelemetryInBuffer.fifo[RX_UART_BUFFER_SIZE-1] )       // last byte
    {
      ptr = TelemetryInBuffer.fifo ;
    }
  }
  TelemetryInBuffer.outPtr = ptr ;

  if ( pUsart->US_RNCR == 0 )
  {
    pUsart->US_RNPR = (uint32_t)TelemetryInBuffer.fifo ;
    pUsart->US_RNCR = RX_UART_BUFFER_SIZE ;
  }
#endif
}

uint32_t txPdcUsart(uint8_t *buffer, uint32_t size)
{
  register Usart *pUsart = SECOND_USART;

  if ( pUsart->US_TNCR == 0 )
  {
#ifndef SIMU
    pUsart->US_TNPR = (uint32_t)buffer ;
#endif
    pUsart->US_TNCR = size ;
    pUsart->US_PTCR = US_PTCR_TXTEN ;
    return 1 ;
  }
  return 0 ;
}

uint32_t telemetryTransmitPending()
{
  register Usart *pUsart = SECOND_USART;
  uint32_t x ;

  __disable_irq() ;
  pUsart->US_PTCR = US_PTCR_TXTDIS ;              // Freeze DMA
  x = pUsart->US_TNCR ;                           // Total
  x += pUsart->US_TCR ;                           // Still to send
  pUsart->US_PTCR = US_PTCR_TXTEN ;               // DMA active again
  __enable_irq() ;

  return x ;
}

void telemetryPortInit(uint32_t baudrate)
{
#if !defined(SIMU)
  UART2_Configure(baudrate, Master_frequency);
  startPdcUsartReceive();
#endif
}

void telemetryTransmitBuffer(uint8_t * buffer, uint32_t size)
{
  txPdcUsart(buffer, size);
}
