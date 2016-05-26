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

uint16_t ResetReason;
uint32_t Master_frequency ;
volatile uint32_t Tenms ;
volatile uint8_t lcdLock;
volatile uint32_t lcdInputs;

#if !defined(REVA)
uint16_t Current_analogue;
uint16_t Current_max;
uint32_t Current_accumulator;
uint32_t Current_used;
#endif

extern "C" void sam_boot( void ) ;

// Prototype
// Free pins (PA16 is stock buzzer)
// PA23, PA24, PA25, PB7, PB13
// PC20, PC21(labelled 17), PC22, PC24
// REVB
// PA25, use for stock buzzer
// PB14, PB6
// PC21, PC19, PC15 (PPM2 output)
inline void config_free_pins()
{
#if defined(REVA)
  register Pio * pioptr = PIOA ;
  pioptr->PIO_PER = 0x03800000L ;         // Enable bits A25,24,23
  pioptr->PIO_ODR = 0x03800000L ;         // Set as input
  pioptr->PIO_PUER = 0x03800000L ;        // Enable pullups

  pioptr = PIOB ;
  pioptr->PIO_PER = 0x00002080L ;         // Enable bits B13, 7
  pioptr->PIO_ODR = 0x00002080L ;         // Set as input
  pioptr->PIO_PUER = 0x00002080L ;        // Enable pullups

  pioptr = PIOC ;
  pioptr->PIO_PER = 0x01700000L ;         // Enable bits C24,22,21,20
  pioptr->PIO_ODR = 0x01700000L ;         // Set as input
  pioptr->PIO_PUER = 0x01700000L ;        // Enable pullups
#else
  configure_pins( PIO_PB14, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
#endif
}

// Assumes PMC has already enabled clocks to ports
inline void setup_switches()
{
#if defined(REVA)
  register Pio *pioptr = PIOA ;
  pioptr->PIO_PER = 0xF8008184 ;          // Enable bits
  pioptr->PIO_ODR = 0xF8008184 ;          // Set bits input
  pioptr->PIO_PUER = 0xF8008184 ;         // Set bits with pullups

  pioptr = PIOB ;
  pioptr->PIO_PER = 0x00000010 ;          // Enable bits
  pioptr->PIO_ODR = 0x00000010 ;          // Set bits input
  pioptr->PIO_PUER = 0x00000010 ;         // Set bits with pullups

  pioptr = PIOC ;
  pioptr->PIO_PER = 0x10014900 ;          // Enable bits
  pioptr->PIO_ODR = 0x10014900 ;          // Set bits input
  pioptr->PIO_PUER = 0x10014900 ;         // Set bits with pullups
#else
  configure_pins( 0x01808087, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
  configure_pins( 0x00000030, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
  configure_pins( 0x91114900, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;
#endif
}

#if defined(SIMU)
#define stop_trainer_capture()
#define sam_boot()
#else

inline void UART3_Configure(uint32_t baudrate, uint32_t masterClock)
{
  register Uart *pUart = BT_USART;

  /* Configure PIO */
  configure_pins( (PIO_PB2 | PIO_PB3), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTB | PIN_NO_PULLUP ) ;

  /* Configure PMC */
  PMC->PMC_PCER0 = 1 << BT_ID;

  /* Reset and disable receiver & transmitter */
  pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX
                 | UART_CR_RXDIS | UART_CR_TXDIS;

  /* Configure mode */
  pUart->UART_MR =  0x800 ;  // NORMAL, No Parity

  /* Configure baudrate */
  /* Asynchronous, no oversampling */
  pUart->UART_BRGR = ( (masterClock / baudrate) + 8 ) / 16;

  /* Disable PDC channel */
  pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

  /* Enable receiver and transmitter */
  pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
  
#if 0
  pUart->UART_IER = UART_IER_RXRDY ;
  NVIC_EnableIRQ(UART1_IRQn) ;
#endif
}

void UART3_Stop()
{
  BT_USART->UART_IDR = UART_IDR_RXRDY ;
  NVIC_DisableIRQ(UART1_IRQn) ;
}

// Starts TIMER0 at full speed (MCK/2) for delay timing
// @ 36MHz this is 18MHz
// This was 6 MHz, we may need to slow it to TIMER_CLOCK2 (MCK/8=4.5 MHz)
inline void start_timer0()
{
  register Tc *ptc ;

  // Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x00800000L ;               // Enable peripheral clock to TC0

  ptc = TC0 ;           // Tc block 0 (TC0-2)
  ptc->TC_BCR = 0 ;                       // No sync
  ptc->TC_BMR = 2 ;
  ptc->TC_CHANNEL[0].TC_CMR = 0x00008001 ;      // Waveform mode MCK/8 for 36MHz osc.
  ptc->TC_CHANNEL[0].TC_RC = 0xFFF0 ;
  ptc->TC_CHANNEL[0].TC_RA = 0 ;
  ptc->TC_CHANNEL[0].TC_CMR = 0x00008040 ;        // 0000 0000 0000 0000 1000 0000 0100 0000, stop at regC
  ptc->TC_CHANNEL[0].TC_CCR = 5 ;                 // Enable clock and trigger it (may only need trigger)
}

// TIMER2 at 200Hz, provides 5mS for sound and 10mS tick on interrupt
// Starts TIMER2 at 200Hz,  commentd out drive of TIOA2 (A26, EXT2)
inline void start_timer2()
{
  register Tc *ptc ;
  register uint32_t timer ;

  // Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x02000000L ;               // Enable peripheral clock to TC2

  timer = Master_frequency / 12800 / 2;            // MCK/128 and 200 Hz

  ptc = TC0 ;                                   // Tc block 0 (TC0-2)
  ptc->TC_BCR = 0 ;                             // No sync
  ptc->TC_BMR = 0 ;
  ptc->TC_CHANNEL[2].TC_CMR = 0x00008000 ;      // Waveform mode
  ptc->TC_CHANNEL[2].TC_RC = timer ;            // 10 Hz
  ptc->TC_CHANNEL[2].TC_RA = timer >> 1 ;
  ptc->TC_CHANNEL[2].TC_CMR = 0x0009C003 ;      // 0000 0000 0000 1001 1100 0000 0000 0011
  ptc->TC_CHANNEL[2].TC_CCR = 5 ;               // Enable clock and trigger it (may only need trigger)

  NVIC_EnableIRQ(TC2_IRQn) ;
  TC0->TC_CHANNEL[2].TC_IER = TC_IER0_CPCS ;
}

// Start TIMER3 for input capture
inline void start_timer3()
{
  register Tc *ptc ;

  // Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x04000000L ;               // Enable peripheral clock to TC3

  ptc = TC1 ;           // Tc block 1 (TC3-5)
  ptc->TC_BCR = 0 ;                       // No sync
  ptc->TC_BMR = 2 ;
  ptc->TC_CHANNEL[0].TC_CMR = 0x00000000 ;        // Capture mode
  ptc->TC_CHANNEL[0].TC_CMR = 0x00090005 ;        // 0000 0000 0000 1001 0000 0000 0000 0101, XC0, A rise, b fall
  ptc->TC_CHANNEL[0].TC_CCR = 5 ;         // Enable clock and trigger it (may only need trigger)

  configure_pins( PIO_PC23, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_PULLUP ) ;

  NVIC_SetPriority( TC3_IRQn, 14 ) ; // Low priority interrupt
  NVIC_EnableIRQ(TC3_IRQn) ;
  ptc->TC_CHANNEL[0].TC_IER = TC_IER0_LDRAS ;
}

// Start Timer4 to provide 0.5uS clock for input capture
void start_timer4()
{
  register Tc *ptc ;
  register uint32_t timer ;

  timer = Master_frequency / (2*2000000) ;                // MCK/2 and 2MHz

  // Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x08000000L ;               // Enable peripheral clock to TC4

  ptc = TC1 ;           // Tc block 1 (TC3-5)
  ptc->TC_BCR = 0 ;                       // No sync
  ptc->TC_BMR = 0 ;
  ptc->TC_CHANNEL[1].TC_CMR = 0x00008000 ;        // Waveform mode
  ptc->TC_CHANNEL[1].TC_RC = timer ;
  ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
  ptc->TC_CHANNEL[1].TC_CMR = 0x0009C000 ;        // 0000 0000 0000 1001 1100 0000 0100 0000
                                                                                                                                                                                // MCK/2, set @ RA, Clear @ RC waveform
  ptc->TC_CHANNEL[1].TC_CCR = 5 ;         // Enable clock and trigger it (may only need trigger)
}

// Timer3 used for PPM_IN pulse width capture. Counter running at 16MHz / 8 = 2MHz
// equating to one count every half millisecond. (2 counts = 1ms). Control channel
// count delta values thus can range from about 1600 to 4400 counts (800us to 2200us),
// corresponding to a PPM signal in the range 0.8ms to 2.2ms (1.5ms at center).
// (The timer is free-running and is thus not reset to zero at each capture interval.)
// Timer 4 generates the 2MHz clock to clock Timer 3

extern "C" void TC3_IRQHandler() //capture ppm in at 2MHz
{
  uint16_t capture = TC1->TC_CHANNEL[0].TC_RA ;
  (void) TC1->TC_CHANNEL[0].TC_SR ;               // Acknowledge the interrupt

  captureTrainerPulses(capture);
}

void init_trainer_capture()
{
  start_timer4() ;
  start_timer3() ;
}

void stop_trainer_capture()
{
  TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;
  NVIC_DisableIRQ(TC3_IRQn) ;
}

void interrupt5ms()
{
  static uint32_t pre_scale ;             // Used to get 10 Hz counter

  HAPTIC_HEARTBEAT();
  AUDIO_HEARTBEAT();

  if ( ++pre_scale >= 2 ) {
    BUZZER_HEARTBEAT();
    pre_scale = 0 ;
    per10ms();
  }
}

extern "C" void TC2_IRQHandler()
{
  register uint32_t dummy;

  /* Clear status bit to acknowledge interrupt */
  dummy = TC0->TC_CHANNEL[2].TC_SR;
  (void) dummy ;          // Discard value - prevents compiler warning

  interrupt5ms();
}

// PWM used for PPM generation, and LED Backlight
// Output pin PB5 not used, PA17 used as PWMH3 peripheral C
// PWM peripheral ID = 31 (0x80000000)
// Ensure PB5 is three state/input, used on REVB for MENU KEY

// Configure PWM3 as PPM drive,
// PWM0 is LED backlight PWM on PWMH0
// This is PC18 peripheral B, Also enable PC22 peripheral B, this is PPM-JACK (PWML3)
//
// REVB board:
// PWML2, output as peripheral C on PA16, is for HAPTIC
// For testing, just drive it out with PWM
// PWML1 for PPM2 output as peripheral B on PC15
// For testing, just drive it out with PWM
void init_pwm()
{
  register Pwm *pwmptr ;
  register uint32_t timer ;

  PMC->PMC_PCER0 |= ( 1 << ID_PWM ) ;           // Enable peripheral clock to PWM

  MATRIX->CCFG_SYSIO |= 0x00000020L ;                             // Disable TDO let PB5 work!

  /* Configure PIO */
#if defined(REVA)
  register Pio *pioptr = PIOB ;
  pioptr->PIO_PER = 0x00000020L ;         // Enable bit B5
  pioptr->PIO_ODR = 0x00000020L ;         // set as input
#else
  configure_pins( PIO_PA16, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_C | PIN_PORTA | PIN_NO_PULLUP ) ;
#endif

  configure_pins( PIO_PC18, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP ) ;

#if !defined(REVA)
  configure_pins( PIO_PC22, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP ) ;
#endif

  // Configure clock - depends on MCK frequency
  timer = Master_frequency / 2000000 ;
  timer |= ( Master_frequency / ( 32* 10000 ) ) << 16 ;
  timer &= 0x00FF00FF ;

  pwmptr = PWM ;
  pwmptr->PWM_CLK = 0x05000000 | timer ;  // MCK for DIVA, DIVA = 18 gives 0.5uS clock period @35MHz                                                                                                                                              // MCK/32 / timer = 10000Hz for CLKB

  // PWM0 for LED backlight
  pwmptr->PWM_CH_NUM[0].PWM_CMR = 0x0000000C ;    // CLKB
  pwmptr->PWM_CH_NUM[0].PWM_CPDR = 100 ;                  // Period
  pwmptr->PWM_CH_NUM[0].PWM_CPDRUPD = 100 ;               // Period
  pwmptr->PWM_CH_NUM[0].PWM_CDTY = 40 ;                           // Duty
  pwmptr->PWM_CH_NUM[0].PWM_CDTYUPD = 40 ;                // Duty
  pwmptr->PWM_ENA = PWM_ENA_CHID0 ;                                               // Enable channel 0

#if !defined(REVA)
  // PWM2 for HAPTIC drive 100Hz test
  pwmptr->PWM_CH_NUM[2].PWM_CMR = 0x0000000C ;    // CLKB
  pwmptr->PWM_CH_NUM[2].PWM_CPDR = 100 ;                  // Period
  pwmptr->PWM_CH_NUM[2].PWM_CPDRUPD = 100 ;               // Period
  pwmptr->PWM_CH_NUM[2].PWM_CDTY = 40 ;                           // Duty
  pwmptr->PWM_CH_NUM[2].PWM_CDTYUPD = 40 ;                // Duty
  pwmptr->PWM_OOV &= ~0x00040000 ;      // Force low
  pwmptr->PWM_OSS = 0x00040000 ;  // Force low
  // pwmptr->PWM_ENA = PWM_ENA_CHID2 ;                   // Enable channel 2 // TODO on REVA?
#endif
}

// LCD i/o pins
// LCD_RES     PC27
// LCD_CS1     PC26
// LCD_E       PC12
// LCD_RnW     PC13
// LCD_A0      PC15
// LCD_D0      PC0
// LCD_D1      PC7
// LCD_D2      PC6
// LCD_D3      PC5
// LCD_D4      PC4
// LCD_D5      PC3
// LCD_D6      PC2
// LCD_D7      PC1

#define LCD_DATA        0x000000FFL
#if defined(REVA)
#define LCD_A0    0x00008000L
#else
#define LCD_A0    0x00000080L
#endif
#define LCD_RnW   0x00002000L
#define LCD_E     0x00001000L
#define LCD_CS1   0x04000000L
#define LCD_RES   0x08000000L

void configure_pins( uint32_t pins, uint16_t config )
{
        register Pio *pioptr ;

        pioptr = PIOA + ( ( config & PIN_PORT_MASK ) >> 6) ;
        if ( config & PIN_PULLUP )
        {
                pioptr->PIO_PPDDR = pins ;
                pioptr->PIO_PUER = pins ;
        }
        else
        {
                pioptr->PIO_PUDR = pins ;
        }

        if ( config & PIN_PULLDOWN )
        {
                pioptr->PIO_PUDR = pins ;
                pioptr->PIO_PPDER = pins ;
        }
        else
        {
                pioptr->PIO_PPDDR = pins ;
        }

        if ( config & PIN_HIGH )
        {
                pioptr->PIO_SODR = pins ;
        }
        else
        {
                pioptr->PIO_CODR = pins ;
        }

        if ( config & PIN_INPUT )
        {
                pioptr->PIO_ODR = pins ;
        }
        else
        {
                pioptr->PIO_OER = pins ;
        }

        if ( config & PIN_PERI_MASK_L )
        {
                pioptr->PIO_ABCDSR[0] |= pins ;
        }
        else
        {
                pioptr->PIO_ABCDSR[0] &= ~pins ;
        }
        if ( config & PIN_PERI_MASK_H )
        {
                pioptr->PIO_ABCDSR[1] |= pins ;
        }
        else
        {
                pioptr->PIO_ABCDSR[1] &= ~pins ;
        }

        if ( config & PIN_ENABLE )
        {
                pioptr->PIO_PER = pins ;
        }
        else
        {
                pioptr->PIO_PDR = pins ;
        }
}

void opentxBootloader();

// Set up for volume control (TWI0)
// Need PA3 and PA4 set to peripheral A
void i2cInit()
{
  register Pio *pioptr;
  register uint32_t timing;

  PMC->PMC_PCER0 |= 0x00080000L ;               // Enable peripheral clock to TWI0

  TWI0->TWI_CR = TWI_CR_SWRST ;                           // Reset in case we are restarting

  /* Configure PIO */
  pioptr = PIOA ;
  pioptr->PIO_ABCDSR[0] &= ~0x00000018 ;        // Peripheral A
  pioptr->PIO_ABCDSR[1] &= ~0x00000018 ;        // Peripheral A
  pioptr->PIO_PDR = 0x00000018 ;                                        // Assign to peripheral

  timing = Master_frequency * 5 / 1000000 ;             // 5uS high and low
  timing += 15 - 4 ;
  timing /= 16 ;
  timing |= timing << 8 ;

  TWI0->TWI_CWGR = 0x00040000 | timing ;                        // TWI clock set
  TWI0->TWI_CR = TWI_CR_MSEN | TWI_CR_SVDIS ;           // Master mode enable
  TWI0->TWI_MMR = 0x002F0000 ;          // Device 5E (>>1) and master is writing
  NVIC_EnableIRQ(TWI0_IRQn) ;
}

extern const uint8_t BootCode[];

void boardInit()
{
  // TODO this is not clean, completely unuseful, but prevents the bootloader from being optimized away...
  uint8_t dummy[3];
  memcpy(&dummy, BootCode, sizeof(dummy));
  for (unsigned int i=0; i<sizeof(dummy); i++) {
    if (dummy[i] == 0) {
    __asm("nop");
    }
  }

  register Pio *pioptr ;

  ResetReason = RSTC->RSTC_SR;

  MATRIX->CCFG_SYSIO |= 0x000000F0L ;             // Disable syspins, enable B4,5,6,7

  PMC->PMC_PCER0 = (1<<ID_PIOC)|(1<<ID_PIOB)|(1<<ID_PIOA)|(1<<ID_UART0) ;                               // Enable clocks to PIOB and PIOA and PIOC and UART0

  PIOC->PIO_PER = PIO_PC25 ;            // Enable bit C25 (USB-detect)

#if defined(REVA)
  // On REVB, PA21 is used as AD8, and measures current consumption.
  pioptr = PIOA ;
  pioptr->PIO_PER = PIO_PA21 ;            // Enable bit A21 (EXT3)
  pioptr->PIO_OER = PIO_PA21 ;            // Set bit A21 as output
  pioptr->PIO_SODR = PIO_PA21 ;   // Set bit A21 ON

  // Configure RF_power (PC17) and PPM-jack-in (PC19), neither need pullups
  pioptr->PIO_PER = 0x000A0000L ;         // Enable bit C19, C17
  pioptr->PIO_ODR = 0x000A0000L ;         // Set bits C19 and C17 as input
#endif

#if !defined(REVA)
  pwrInit() ;
#endif

  config_free_pins() ;

  // Next section configures the key inputs on the LCD data
  pioptr = PIOC;
#if defined(REVA)
  pioptr->PIO_PER = 0x0000003DL ;         // Enable bits 2,3,4,5, 0
  pioptr->PIO_OER = PIO_PC0 ;             // Set bit 0 output
  pioptr->PIO_ODR = 0x0000003CL ;         // Set bits 2, 3, 4, 5 input
  pioptr->PIO_PUER = 0x0000003CL ;                // Set bits 2, 3, 4, 5 with pullups
#else
  pioptr->PIO_PER = 0x0000003BL ;         // Enable bits 1,3,4,5, 0
  pioptr->PIO_OER = PIO_PC0 ;             // Set bit 0 output
  pioptr->PIO_ODR = 0x0000003AL ;         // Set bits 1, 3, 4, 5 input
  pioptr->PIO_PUER = 0x0000003AL ;                // Set bits 1, 3, 4, 5 with pullups
#endif

  pioptr = PIOB ;
#if defined(REVA)
  pioptr->PIO_PUER = PIO_PB6 ;                                    // Enable pullup on bit B6 (MENU)
  pioptr->PIO_PER = PIO_PB6 ;                                     // Enable bit B6
#else
  pioptr->PIO_PUER = PIO_PB5 ;                                    // Enable pullup on bit B5 (MENU)
  pioptr->PIO_PER = PIO_PB5 ;                                     // Enable bit B5
#endif

  setup_switches() ;

  // Enable PCK2 on PB3, This is for testing of Timer 2 working
  // It will be used as serial data to the Bluetooth module
  pioptr->PIO_ABCDSR[0] |=  PIO_PB3 ;     // Peripheral B
  pioptr->PIO_ABCDSR[1] &= ~PIO_PB3 ;     // Peripheral B
  pioptr->PIO_PDR = PIO_PB3 ;             // Assign to peripheral
  PMC->PMC_SCER |= 0x0400 ;               // PCK2 enabled
  PMC->PMC_PCK[2] = 2 ;                   // PCK2 is PLLA

  start_timer2() ;
  start_timer0() ;
  adcInit() ;
  init_pwm() ;

  __enable_irq();

  audioInit();

  i2cInit();

  eepromInit();

#if defined(ROTARY_ENCODERS)
  rotencInit();
#endif

  init_SDcard();
}
#endif

uint8_t temperature = 0;          // Raw temp reading
uint8_t maxTemperature = 0 ;       // Raw temp reading

void end_bt_tx_interrupt()
{
  Uart *pUart = BT_USART ;
  pUart->UART_IDR = UART_IDR_TXBUFE ;
  NVIC_DisableIRQ(UART1_IRQn) ;
}

#if !defined(REVA)
uint16_t getCurrent()
{
  static uint16_t Current ;
  static uint32_t Current_sum ;
  static uint8_t  Current_count ;

  Current_sum += anaIn(8); // TODO enum
  if (++Current_count >= 50) {
    Current = Current_sum / 5 ;
    Current_sum = 0 ;
    Current_count = 0 ;
  }

  uint32_t current_scale = 488 + g_eeGeneral.txCurrentCalibration ;
  return (current_scale * Current) / 8192;
}
#endif

uint8_t getTemperature()
{
  return temperature + g_eeGeneral.temperatureCalib;
}

#define STICK_LV_GAIN 0x01
#define STICK_LH_GAIN 0x02
#define STICK_RV_GAIN 0x04
#define STICK_RH_GAIN 0x08

#define GAIN_LV  0x00080000
#define GAIN_LH  0x00000020
#define GAIN_RV  0x20000000
#define GAIN_RH  0x00000008

#define OFF_LV   0x00000200
#define OFF_LH   0x00000004
#define OFF_RV   0x00004000
#define OFF_RH   0x00000002

void setSticksGain(uint8_t gains)
{
  register Adc *padc ;
  uint32_t gain ;
  uint32_t offset ;

  gain = 0 ;
  offset = 0 ;
  padc = ADC ;

  if ( gains & STICK_LV_GAIN ) {
    gain |= GAIN_LV ;
    offset |= OFF_LV ;
  }
  if ( gains & STICK_LH_GAIN ) {
    gain |= GAIN_LH ;
    offset |= OFF_LH ;
  }
  if ( gains & STICK_RV_GAIN ) {
    gain |= GAIN_RV ;
    offset |= OFF_RV ;
  }
  if ( gains & STICK_RH_GAIN ) {
    gain |= GAIN_RH ;
    offset |= OFF_RH ;
  }

  padc->ADC_CGR = gain;
  padc->ADC_COR = offset;
}


#if !defined(REVA)
void calcConsumption()
{
  Current_accumulator += Current_analogue ;
  static uint32_t OneSecTimer;
  if (++OneSecTimer >= 100) {
    OneSecTimer -= 100 ;
    Current_used += Current_accumulator / 100 ;                     // milliAmpSeconds (but scaled)
    Current_accumulator = 0 ;
  }
}
#endif

void checkTrainerSettings()
{
  if (SLAVE_MODE()) {
    PIOC->PIO_PDR = PIO_PC22;
  }
  else {
    PIOC->PIO_PER = PIO_PC22;
  }
}

