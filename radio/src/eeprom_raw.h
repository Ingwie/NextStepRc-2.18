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

#ifndef eeprom_arm_h
#define eeprom_arm_h

#include <inttypes.h>
#include <stdint.h>

bool eeLoadGeneral( void ) ;
void eeDeleteModel( uint8_t id ) ;
bool eeCopyModel(uint8_t dst, uint8_t src);
void eeSwapModels(uint8_t id1, uint8_t id2);

#define DISPLAY_PROGRESS_BAR(x)

#if defined(SDCARD)
const pm_char * eeBackupModel(uint8_t i_fileSrc);
const pm_char * eeRestoreModel(uint8_t i_fileDst, char *model_name);
#endif

uint32_t loadGeneralSettings();
uint32_t loadModel(uint32_t index);

enum EepromWriteState {
  EEPROM_IDLE = 0,
  EEPROM_START_WRITE,
  EEPROM_ERASING_FILE_BLOCK1,
  EEPROM_ERASING_FILE_BLOCK1_WAIT,
  EEPROM_ERASE_FILE_BLOCK2,
  EEPROM_ERASING_FILE_BLOCK2,
  EEPROM_ERASING_FILE_BLOCK2_WAIT,
  EEPROM_WRITE_BUFFER,
  EEPROM_WRITING_BUFFER,
  EEPROM_WRITING_BUFFER_WAIT,
  EEPROM_WRITE_NEXT_BUFFER,
  EEPROM_ERASING_FAT_BLOCK,
  EEPROM_ERASING_FAT_BLOCK_WAIT,
  EEPROM_WRITE_NEW_FAT,
  EEPROM_WRITING_NEW_FAT,
  EEPROM_WRITING_NEW_FAT_WAIT,
  EEPROM_END_WRITE
};

extern EepromWriteState eepromWriteState;
inline bool eepromIsWriting()
{
  return (eepromWriteState != EEPROM_IDLE);
}
void eepromWriteProcess();
void eepromWriteWait(EepromWriteState state = EEPROM_IDLE);
bool eepromOpen();

#endif
