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

#ifndef board_stock_h
#define board_stock_h

#include "../common_avr/board_avr.h"

//
//                  elev                        thr
//                   LV                         RV
//                 2 ^                        4 ^
//                   1                          2
//                   |     rudd                 |     aile
//              <----X--3-> LH             <----X--0-> RH
//              6    |    7                1    |    0
//                   |                          |
//                 3 v                        5 v
//
//PORTA  7      6       5       4       3       2       1       0
//       O      O       O       O       O       O       O       O
//       ------------------------ LCD_DAT -----------------------
//
//PORTB  7      6       5       4       3       2       1       0
//       O      i       i       i       i       i       i       O
//       light  KEY_LFT KEY_RGT KEY_UP  KEY_DWN KEY_EXT KEY_MEN  PPM
//
//PORTC  7      6       5       4       3       2       1       0
//       -      -       O       O       O       O       O       -
//       NC     NC    LCD_E   LCD_RNW  LCD_A0  LCD_RES LCD_CS1  NC
//
//PORTD  7      6       5       4       3       2       1       0
//       i      i       i       i       i       i       i       i
//     TRM_D_DWN _UP  TRM_C_DWN _UP   TRM_B_DWN _UP   TRM_A_DWN _UP
//
//PORTE  7      6       5       4       3       2       1       0
//       i      i       i       i       O       i       i       i
//     PPM_IN  ID2    Trainer  Gear   Buzzer   ElevDR  AileDR  THRCT
//
//PORTF  7      6       5       4       3       2       1       0
//       ai     ai      ai      ai      ai      ai      ai      ai
// ANA_ BAT   PITT_TRM HOV_THR HOV_PIT  STCK_LH STCK_RV STCK_LV STCK_RH
//                                      rud    thro   elev   aile
//PORTG  7      6       5       4       3       2       1       0
//       -      -       -       O       i               i       i
//                            SIM_CTL  ID1      Haptic      RF_POW RuddDR

// Keys
#define KEYS_GPIO_REG_MENU         pinb
#define KEYS_GPIO_PIN_MENU         (1<<1)
#define KEYS_GPIO_REG_EXIT         pinb
#define KEYS_GPIO_PIN_EXIT         (1<<2)
#define KEYS_GPIO_REG_RIGHT        pinb
#define KEYS_GPIO_PIN_RIGHT        (1<<5)
#define KEYS_GPIO_REG_LEFT         pinb
#define KEYS_GPIO_PIN_LEFT         (1<<6)
#define KEYS_GPIO_REG_UP           pinb
#define KEYS_GPIO_PIN_UP           (1<<4)
#define KEYS_GPIO_REG_DOWN         pinb
#define KEYS_GPIO_PIN_DOWN         (1<<3)

// Trims
#define TRIMS_GPIO_REG_LHL         pind
#define TRIMS_GPIO_PIN_LHL         (1<<6)
#define TRIMS_GPIO_REG_LVD         pind
#define TRIMS_GPIO_PIN_LVD         (1<<3)
#define TRIMS_GPIO_REG_RVU         pind
#define TRIMS_GPIO_PIN_RVU         (1<<4)
#define TRIMS_GPIO_REG_RHL         pind
#define TRIMS_GPIO_PIN_RHL         (1<<1)
#define TRIMS_GPIO_REG_LHR         pind
#define TRIMS_GPIO_PIN_LHR         (1<<7)
#define TRIMS_GPIO_REG_LVU         pind
#define TRIMS_GPIO_PIN_LVU         (1<<2)
#define TRIMS_GPIO_REG_RVD         pind
#define TRIMS_GPIO_PIN_RVD         (1<<5)
#define TRIMS_GPIO_REG_RHR         pind
#define TRIMS_GPIO_PIN_RHR         (1<<0)

#if defined(CPUM2561)
  #define TIMER_16KHZ_VECT         TIMER2_OVF_vect
  #define COUNTER_16KHZ            TCNT2
  #define TIMER_10MS_VECT          TIMER2_COMPA_vect
  #define TIMER_10MS_COMPVAL       OCR2A
  #define PAUSE_10MS_INTERRUPT()   TIMSK2 &= ~(1<<OCIE2A)
  #define RESUME_10MS_INTERRUPT()  TIMSK2 |= (1<<OCIE2A)
  #define PAUSE_PPMIN_INTERRUPT()  TIMSK3 &= ~(1<<ICIE3)
  #define RESUME_PPMIN_INTERRUPT() TIMSK3 |= (1<<ICIE3)
  #define TIMER_AUDIO_VECT         TIMER4_COMPA_vect
  #define SET_TIMER_AUDIO_CTRL()   TCCR4B = (1 << WGM42) | (0b010 << CS40)
  #define PAUSE_AUDIO_INTERRUPT()  TIMSK4 &= ~(1<<OCIE4A)
  #define RESUME_AUDIO_INTERRUPT() TIMSK4 |= (1<<OCIE4A)
#else
  #define TIMER_16KHZ_VECT         TIMER0_OVF_vect
  #define COUNTER_16KHZ            TCNT0
  #define TIMER_10MS_VECT          TIMER0_COMP_vect
  #define TIMER_10MS_COMPVAL       OCR0
  #define PAUSE_10MS_INTERRUPT()   TIMSK &= ~(1<<OCIE0)
  #define RESUME_10MS_INTERRUPT()  TIMSK |= (1<<OCIE0)
  #define PAUSE_PPMIN_INTERRUPT()  ETIMSK &= ~(1<<TICIE3)
  #define RESUME_PPMIN_INTERRUPT() ETIMSK |= (1<<TICIE3)
  #define TIMER_AUDIO_VECT         TIMER2_OVF_vect
  #define SET_TIMER_AUDIO_CTRL()   TCCR2 = (0b010 << CS00) // Norm mode, clk/8
  #define PAUSE_AUDIO_INTERRUPT()  TIMSK &= ~(1<<TOIE2)
  #define RESUME_AUDIO_INTERRUPT() TIMSK |= (1<<TOIE2)
#endif

// Power driver (none)
#define pwrCheck()                 (e_power_on)
#define pwrOff()
#define UNEXPECTED_SHUTDOWN()      (mcusr & (1 << WDRF))

// Trainer driver
bool checkSlaveMode();
#define OUT_G_SIM_CTL              4 // 1 : phone-jack=ppm_in
#define SLAVE_MODE()               checkSlaveMode()
#define JACK_PPM_OUT()             PORTG &= ~(1<<OUT_G_SIM_CTL)
#define JACK_PPM_IN()              PORTG |=  (1<<OUT_G_SIM_CTL)

// PWM Backlight driver
#define OUT_B_LIGHT   7
#if defined(PWM_BACKLIGHT)
  void backlightEnable();
  void backlightDisable();
  bool isBacklightEnable();
  void backlightFade();
#elif defined(SP22)
  #define backlightEnable()        PORTB &= ~(1<<OUT_B_LIGHT)
  #define backlightDisable()       PORTB |=  (1<<OUT_B_LIGHT)
  #define isBacklightEnable()      (~PORTB & (1<<OUT_B_LIGHT))
#else
  #define backlightEnable()        PORTB |=  (1<<OUT_B_LIGHT)
  #define backlightDisable()       PORTB &= ~(1<<OUT_B_LIGHT)
  #define isBacklightEnable()      (PORTB &  (1<<OUT_B_LIGHT))
#endif

#define OUT_B_PPM     0
#define INP_E_PPM_IN  7
#define INP_E_ID2     6
#define INP_E_Trainer 5
#define INP_E_Gear    4
#define OUT_E_BUZZER  3
#define INP_E_ElevDR  2

#if defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA) || defined(MAVLINK)
#define INP_C_ThrCt   6
#define INP_C_AileDR  7
#else
#define INP_E_ThrCt   0
#define INP_E_AileDR  1
#endif

#define INP_G_ID1      3
#define OUT_G_HAPTIC   2
#define INP_G_RF_POW   1
#define INP_G_RuddDR   0

// Keys driver
#if defined(TELEMETRY_MOD_14051) || defined(TELEMETRY_MOD_14051_SWAPPED)
enum MuxInput {
  MUX_BATT,
  MUX_AIL,
  MUX_PF7_DIGITAL_MIN = MUX_AIL,
  MUX_THR,
  MUX_TRM_LV_UP,
  MUX_TRM_LV_DWN,
  MUX_PF7_DIGITAL_MAX = MUX_TRM_LV_DWN,
  MUX_MAX = MUX_PF7_DIGITAL_MAX
};

enum Pf7Digital {
  PF7_AIL = MUX_AIL - MUX_PF7_DIGITAL_MIN,
  PF7_THR = MUX_THR - MUX_PF7_DIGITAL_MIN,
  PF7_TRM_LV_UP = MUX_TRM_LV_UP - MUX_PF7_DIGITAL_MIN,
  PF7_TRM_LV_DWN = MUX_TRM_LV_DWN - MUX_PF7_DIGITAL_MIN,
};

extern uint8_t pf7_digital[MUX_PF7_DIGITAL_MAX - MUX_PF7_DIGITAL_MIN + 1];
void processMultiplexAna();
#endif

#if defined(TELEMETRY_MOD_14051_SWAPPED)
  #define TRIMS_PRESSED()          (~PIND & ~0x0c || pf7_digital[PF7_TRM_LV_UP] || pf7_digital[PF7_TRM_LV_DWN])
#else
  #define TRIMS_PRESSED()          (~PIND)
#endif
#define KEYS_PRESSED()             (~PINB)

// LCD driver
#define PORTA_LCD_DAT              PORTA
#define PORTC_LCD_CTRL             PORTC
#define OUT_C_LCD_E                5
#define OUT_C_LCD_RnW              4
#define OUT_C_LCD_A0               3
#define OUT_C_LCD_RES              2
#define OUT_C_LCD_CS1              1

// Rotary encoder driver
#if defined(ROTARY_ENCODER_NAVIGATION)
extern uint8_t RotEncoder ;
void rotencPoll();
#endif

// Haptic
#define hapticOn()                 PORTG |=  (1 << OUT_G_HAPTIC)
#define hapticOff()                PORTG &= ~(1 << OUT_G_HAPTIC)

// USB fake driver
#define usbPlugged()               false

// Buzzer driver
#define buzzerOn()                 PORTE |=  (1 << OUT_E_BUZZER)
#define buzzerOff()                PORTE &= ~(1 << OUT_E_BUZZER)

// EEPROM driver
#if !defined(SIMU)
#define eepromReadBlock(a, b, c)   eeprom_read_block(a, (const void *)b, c)
#endif

#endif
