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

const int8_t volumeScale[VOLUME_LEVEL_MAX+1] =
{
    0,  2,  4,   6,   8,  10,  13,  17,  22,  27,  33,  40,
    64, 82, 96, 105, 112, 117, 120, 122, 124, 125, 126, 127
} ;

#if !defined(SIMU)

void setSampleRate(uint32_t frequency)
{
  register Tc *ptc ;
  register uint32_t timer ;

  timer = Master_frequency / (8 * frequency) ;		// MCK/8 and 100 000 Hz
  if (timer > 65535)
    timer = 65535 ;
  if (timer < 2)
    timer = 2 ;

  ptc = TC0 ;		// Tc block 0 (TC0-2)
  ptc->TC_CHANNEL[1].TC_CCR = TC_CCR0_CLKDIS ;		// Stop clock
  ptc->TC_CHANNEL[1].TC_RC = timer ;			// 100 000 Hz
  ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
  ptc->TC_CHANNEL[1].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
}

// Start TIMER1 at 100000Hz, used for DACC trigger
void dacTimerStart()
{
  register Tc *ptc ;
  register uint32_t timer ;

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x01000000L ;		// Enable peripheral clock to TC1
  
  timer = Master_frequency / 800000 ;		// MCK/8 and 100 000 Hz
  ptc = TC0 ;		// Tc block 0 (TC0-2)
  ptc->TC_BCR = 0 ;			// No sync
  ptc->TC_BMR = 0 ;
  ptc->TC_CHANNEL[1].TC_CMR = 0x00008000 ;	// Waveform mode
  ptc->TC_CHANNEL[1].TC_RC = timer ;			// 100 000 Hz
  ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
  ptc->TC_CHANNEL[1].TC_CMR = 0x0009C001 ;	// 0000 0000 0000 1001 1100 0000 0000 0001
																						// MCK/8, set @ RA, Clear @ RC waveform
  ptc->TC_CHANNEL[1].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
  // Sound_g.Frequency = 1000 ;
}

// Configure DAC0 (or DAC1 for REVA)
// Not sure why PB14 has not be allocated to the DAC, although it is an EXTRA function
// So maybe it is automatically done
void dacInit()
{
  dacTimerStart() ;

  PMC->PMC_PCER0 |= 0x40000000L ;		// Enable peripheral clock to DAC

  register Dacc *dacptr = DACC;

#if defined(REVA)
  dacptr->DACC_MR = 0x0B010215L ;                       // 0000 1011 0000 0001 0000 0010 0001 0101
  dacptr->DACC_CHER     = 2 ;                                                   // Enable channel 1
#else
  dacptr->DACC_MR = 0x0B000215L ;			// 0000 1011 0000 0001 0000 0010 0001 0101
  dacptr->DACC_CHER     = 1 ;                                                   // Enable channel 0
#endif

  dacptr->DACC_CDR = 2048 ;						// Half amplitude

  dacptr->DACC_PTCR = DACC_PTCR_TXTEN ;
  NVIC_EnableIRQ(DACC_IRQn) ;
}

bool dacQueue(AudioBuffer *buffer)
{
  dacStart();
  return false;
}

extern "C" void DAC_IRQHandler()
{
  uint32_t sr = DACC->DACC_ISR;
  if (sr & DACC_ISR_ENDTX) {
    AudioBuffer *nextBuffer = audioQueue.getNextFilledBuffer();
    if (nextBuffer) {
      // Try the first PDC buffer
      if ((DACC->DACC_TCR == 0) && (DACC->DACC_TNCR == 0)) {
        DACC->DACC_TPR = CONVERT_PTR_UINT(nextBuffer->data);
        DACC->DACC_TCR = nextBuffer->size/2;
        DACC->DACC_PTCR = DACC_PTCR_TXTEN;
        return;
      }
      // Try the second PDC buffer
      if (DACC->DACC_TNCR == 0) {
        DACC->DACC_TNPR = CONVERT_PTR_UINT(nextBuffer->data);
        DACC->DACC_TNCR = nextBuffer->size/2;
        DACC->DACC_PTCR = DACC_PTCR_TXTEN;
        return;
      }
    }
    else {
      dacStop();
    }
  }
}

// Sound routines
void audioInit()
{
  register Pio *pioptr ;

  dacInit() ;

  pioptr = PIOA ;
#if defined(REVA)
  pioptr->PIO_CODR = 0x00010000L ;      // Set bit A16 OFF
  pioptr->PIO_PER = 0x00010000L ;               // Enable bit A16 (Stock buzzer)
  pioptr->PIO_OER = 0x00010000L ;               // Set bit A16 as output
#else
  pioptr->PIO_CODR = 0x02000000L ;      // Set bit A25 OFF
  pioptr->PIO_PER = 0x02000000L ;               // Enable bit A25 (Stock buzzer)
  pioptr->PIO_OER = 0x02000000L ;               // Set bit A25 as output
#endif

#if defined(REVX)
  configure_pins( PIO_PC26, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTC | PIN_NO_PULLUP ) ;
  PIOC->PIO_CODR = PIO_PC26 ;
#endif
}

void audioEnd()
{
  DACC->DACC_IDR = DACC_IDR_ENDTX ;
  DACC->DACC_IDR = DACC_IDR_TXBUFE ;
  NVIC_DisableIRQ(DACC_IRQn) ;
  TWI0->TWI_IDR = TWI_IDR_TXCOMP ;
  NVIC_DisableIRQ(TWI0_IRQn) ;
  PMC->PMC_PCER0 &= ~0x00080000L ;		// Disable peripheral clock to TWI0
  PMC->PMC_PCER0 &= ~0x40000000L ;		// Disable peripheral clock to DAC
}

void setVolume(uint8_t volume)
{
#if !defined(NO_HARDWARE_VOLUME)
  volumeRequired = volumeScale[min<uint8_t>(volume, VOLUME_LEVEL_MAX)];
  __disable_irq() ;
  i2cCheck() ;
  __enable_irq() ;
#endif
}


#endif // #if !defined(SIMU)

