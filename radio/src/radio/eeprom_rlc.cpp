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

#include <inttypes.h>
#include <string.h>
#include "opentx.h"
#include "timers.h"

uint8_t   s_write_err = 0;    // error reasons
RlcFile   theFile;  //used for any file operation
EeFs      eeFs;

#if defined(CPUARM)
blkid_t   freeBlocks = 0;
#endif

uint8_t  s_sync_write = false;

#if !defined(CPUARM)
uint16_t eeprom_pointer;
uint8_t * eeprom_buffer_data;
volatile int8_t eeprom_buffer_size = 0;

#if !defined(SIMU)
inline void eeprom_write_byte()
{
  EEAR = eeprom_pointer;
  EEDR = *eeprom_buffer_data;
// TODO add some define here
#if defined (CPUM2560) || defined(CPUM2561)
  EECR |= 1<<EEMPE;
  EECR |= 1<<EEPE;
#else
  EECR |= 1<<EEMWE;
  EECR |= 1<<EEWE;
#endif
  eeprom_pointer++;
  eeprom_buffer_data++;
}

ISR(EE_READY_vect)
{
  if (--eeprom_buffer_size > 0) {
    eeprom_write_byte();
  }
  else {
#if defined(CPUM2560) || defined(CPUM2561)
    EECR &= ~(1<<EERIE);
#else
    EECR &= ~(1<<EERIE);
#endif
  }
}
#endif

void eepromWriteBlock(uint8_t * i_pointer_ram, uint16_t i_pointer_eeprom, size_t size)
{
  assert(!eeprom_buffer_size);

  eeprom_pointer = i_pointer_eeprom;
  eeprom_buffer_data = i_pointer_ram;
  eeprom_buffer_size = size+1;

#if defined(SIMU)
  sem_post(eeprom_write_sem);
#elif defined (CPUM2560) || defined(CPUM2561)
  EECR |= (1<<EERIE);
#else
  EECR |= (1<<EERIE);
#endif

  if (s_sync_write) {
    while (eeprom_buffer_size > 0) wdt_reset();
  }
}
#endif

static uint8_t EeFsRead(blkid_t blk, uint8_t ofs)
{
  uint8_t ret;
  eepromReadBlock(&ret, (uint16_t)(blk*BS+ofs+BLOCKS_OFFSET), 1);
  return ret;
}

static blkid_t EeFsGetLink(blkid_t blk)
{
#if defined(CPUARM)
  blkid_t ret;
  eepromReadBlock((uint8_t *)&ret, blk*BS+BLOCKS_OFFSET, sizeof(blkid_t));
  return ret;
#else
  return EeFsRead(blk, 0);
#endif
}

static void EeFsSetLink(blkid_t blk, blkid_t val)
{
  static blkid_t s_link; // we write asynchronously, then nothing on the stack!
  s_link = val;
  eepromWriteBlock((uint8_t *)&s_link, (blk*BS)+BLOCKS_OFFSET, sizeof(blkid_t));
}

static uint8_t EeFsGetDat(blkid_t blk, uint8_t ofs)
{
  return EeFsRead(blk, ofs+sizeof(blkid_t));
}

static void EeFsSetDat(blkid_t blk, uint8_t ofs, uint8_t *buf, uint8_t len)
{
  eepromWriteBlock(buf, (blk*BS)+ofs+sizeof(blkid_t)+BLOCKS_OFFSET, len);
}

static void EeFsFlushFreelist()
{
  eepromWriteBlock((uint8_t *)&eeFs.freeList, offsetof(EeFs, freeList), sizeof(eeFs.freeList));
}

static void EeFsFlushDirEnt(uint8_t i_fileId)
{
  eepromWriteBlock((uint8_t *)&eeFs.files[i_fileId], offsetof(EeFs, files) + sizeof(DirEnt)*i_fileId, sizeof(DirEnt));
}

static void EeFsFlush()
{
  eepromWriteBlock((uint8_t *)&eeFs, 0, sizeof(eeFs));
}

uint16_t EeFsGetFree()
{
#if defined(CPUARM)
  int32_t ret = freeBlocks * (BS-sizeof(blkid_t));
#else
  int16_t ret = 0;
  blkid_t i = eeFs.freeList;
  while (i) {
    ret += BS-sizeof(blkid_t);
    i = EeFsGetLink(i);
  }
#endif
  ret += eeFs.files[FILE_TMP].size;
  ret -= eeFs.files[FILE_MODEL(g_eeGeneral.currModel)].size;
  return (ret > 0 ? ret : 0);
}

/// free one or more blocks
static void EeFsFree(blkid_t blk)
{
  blkid_t i = blk;
  blkid_t tmp;

#if defined(CPUARM)
  freeBlocks++;
#endif

  while ((tmp=EeFsGetLink(i))) {
    i = tmp;
#if defined(CPUARM)
    freeBlocks++;
#endif
  }

  EeFsSetLink(i, eeFs.freeList);
  eeFs.freeList = blk; //chain in front
  EeFsFlushFreelist();
}

void eepromCheck()
{
  ENABLE_SYNC_WRITE(true);

  uint8_t *bufp = (uint8_t *)&g_model;
  memclear(bufp, BLOCKS);
  blkid_t blk ;

#if defined(CPUARM)
  blkid_t blocksCount;
#endif
  for (uint8_t i=0; i<=MAXFILES; i++) {
#if defined(CPUARM)
    blocksCount = 0;
#endif
    blkid_t *startP = (i==MAXFILES ? &eeFs.freeList : &eeFs.files[i].startBlk);
    blkid_t lastBlk = 0;
    blk = *startP;
    while (blk) {
      if (blk < FIRSTBLK || // bad blk index
          blk >= BLOCKS  || // bad blk indexchan
          bufp[blk])        // blk double usage
      {
        if (lastBlk) {
          EeFsSetLink(lastBlk, 0);
        }
        else {
          *startP = 0; // interrupt chain at startpos
          EeFsFlush();
        }
        blk = 0; // abort
      }
      else {
#if defined(CPUARM)
        blocksCount++;
#endif
        bufp[blk] = i+1;
        lastBlk   = blk;
        blk       = EeFsGetLink(blk);
      }
    }
  }

#if defined(CPUARM)
  freeBlocks = blocksCount;
#endif

  for (blk=FIRSTBLK; blk<BLOCKS; blk++) {
    if (!bufp[blk]) { // unused block
#if defined(CPUARM)
      freeBlocks++;
#endif
      EeFsSetLink(blk, eeFs.freeList);
      eeFs.freeList = blk; // chain in front
      EeFsFlushFreelist();
    }
  }

  ENABLE_SYNC_WRITE(false);
}

void eepromFormat()
{
  ENABLE_SYNC_WRITE(true);

#ifdef SIMU
  // write zero to the end of the new EEPROM file to set it's proper size
  uint8_t dummy = 0;
  eepromWriteBlock(&dummy, EESIZE-1, 1);
#endif

  memclear(&eeFs, sizeof(eeFs));
  eeFs.version  = EEFS_VERS;
  eeFs.mySize   = sizeof(eeFs);
  eeFs.freeList = 0;
  eeFs.bs       = BS;
  for (blkid_t i=FIRSTBLK; i<BLOCKS-1; i++) {
    EeFsSetLink(i, i+1);
  }
  EeFsSetLink(BLOCKS-1, 0);
  eeFs.freeList = FIRSTBLK;
#if defined(CPUARM)
  freeBlocks = BLOCKS;
#endif
  EeFsFlush();

  ENABLE_SYNC_WRITE(false);
}

bool eepromOpen()
{
  eepromReadBlock((uint8_t *)&eeFs, 0, sizeof(eeFs));

#ifdef SIMU
  if (eeFs.version != EEFS_VERS) {
    TRACE("bad eeFs.version (%d instead of %d)", eeFs.version, EEFS_VERS);
  }
  if (eeFs.mySize != sizeof(eeFs)) {
    TRACE("bad eeFs.mySize (%d instead of %d)", (int)eeFs.mySize, (int)sizeof(eeFs));
  }
#endif  

  if (eeFs.version != EEFS_VERS || eeFs.mySize != sizeof(eeFs)) {
    return false;
  }

  eepromCheck();
  return true;
}

bool EFile::exists(uint8_t i_fileId)
{
  return eeFs.files[i_fileId].startBlk;
}

/*
 * Swap two files in eeprom
 */
void EFile::swap(uint8_t i_fileId1, uint8_t i_fileId2)
{
  DirEnt            tmp = eeFs.files[i_fileId1];
  eeFs.files[i_fileId1] = eeFs.files[i_fileId2];
  eeFs.files[i_fileId2] = tmp;

  ENABLE_SYNC_WRITE(true);
  EeFsFlushDirEnt(i_fileId1);
  EeFsFlushDirEnt(i_fileId2);
  ENABLE_SYNC_WRITE(false);
}

void EFile::rm(uint8_t i_fileId)
{
  blkid_t i = eeFs.files[i_fileId].startBlk;
  memclear(&eeFs.files[i_fileId], sizeof(eeFs.files[i_fileId]));
  ENABLE_SYNC_WRITE(true);
  EeFsFlushDirEnt(i_fileId);
  if (i) EeFsFree(i); //chain in
  ENABLE_SYNC_WRITE(false);
}

/*
 * Open file i_fileId for reading.
 * Return the file's type
 */
void EFile::openRd(uint8_t i_fileId)
{
  m_fileId = i_fileId;
  m_pos      = 0;
  m_currBlk  = eeFs.files[m_fileId].startBlk;
  m_ofs      = 0;
  s_write_err = ERR_NONE;       // error reasons */
}

void RlcFile::openRlc(uint8_t i_fileId)
{
  EFile::openRd(i_fileId);
  m_zeroes   = 0;
  m_bRlc     = 0;
}

uint8_t EFile::read(uint8_t *buf, uint8_t i_len)
{
  uint16_t len = eeFs.files[m_fileId].size - m_pos;
  if (i_len > len) i_len = len;

  uint8_t remaining = i_len;
  while (remaining) {
    if (!m_currBlk) break;
  
    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
    if (m_ofs >= BS-sizeof(blkid_t)) {
      m_ofs = 0;
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    remaining--;
  }

  i_len -= remaining;
  m_pos += i_len;
  return i_len;
}

/*
 * Read runlength (RLE) compressed bytes into buf.
 */
uint16_t RlcFile::readRlc(uint8_t *buf, uint16_t i_len)
{
  uint16_t i = 0;
  for( ; 1; ) {
    uint8_t ln = min<uint16_t>(m_zeroes, i_len-i);
    memclear(&buf[i], ln);
    i        += ln;
    m_zeroes -= ln;
    if (m_zeroes) break;

    ln = min<uint16_t>(m_bRlc, i_len-i);
    uint8_t lr = read(&buf[i], ln);
    i        += lr ;
    m_bRlc   -= lr;
    if(m_bRlc) break;

    if (read(&m_bRlc, 1) !=1) break; // read how many bytes to read

    assert(m_bRlc & 0x7f);

    if (m_bRlc&0x80) { // if contains high byte
      m_zeroes  =(m_bRlc>>4) & 0x7;
      m_bRlc    = m_bRlc & 0x0f;
    }
    else if(m_bRlc&0x40) {
      m_zeroes  = m_bRlc & 0x3f;
      m_bRlc    = 0;
    }
  }
  return i;
}

void RlcFile::write1(uint8_t b)
{
  m_write1_byte = b;
  write(&m_write1_byte, 1);
}

void RlcFile::write(uint8_t *buf, uint8_t i_len)
{
  m_write_len = i_len;
  m_write_buf = buf;

  do {
    nextWriteStep();
  } while (IS_SYNC_WRITE_ENABLE() && m_write_len && !s_write_err);
}

void RlcFile::nextWriteStep()
{
  if (!m_currBlk && m_pos==0) {
    eeFs.files[FILE_TMP].startBlk = m_currBlk = eeFs.freeList;
    if (m_currBlk) {
#if defined(CPUARM)
      freeBlocks--;
#endif
      eeFs.freeList = EeFsGetLink(m_currBlk);
      m_write_step |= WRITE_FIRST_LINK;
      EeFsFlushFreelist();
      return;
    }
  }

  if ((m_write_step & 0x0f) == WRITE_FIRST_LINK) {
    m_write_step -= WRITE_FIRST_LINK;
    EeFsSetLink(m_currBlk, 0);
    return;
  }

  while (m_write_len) {
    if (!m_currBlk) {
      s_write_err = ERR_FULL;
      break;
    }
    if (m_ofs >= (BS-sizeof(blkid_t))) {
      m_ofs = 0;
      blkid_t nextBlk = EeFsGetLink(m_currBlk);
      if (!nextBlk) {
        if (!eeFs.freeList) {
          s_write_err = ERR_FULL;
          break;
        }
        m_write_step += WRITE_NEXT_LINK_1;
        EeFsSetLink(m_currBlk, eeFs.freeList);
        // TODO not good
        return;
      }
      m_currBlk = nextBlk;
    }
    switch (m_write_step & 0x0f) {
      case WRITE_NEXT_LINK_1:
        m_currBlk = eeFs.freeList;
#if defined(CPUARM)
        freeBlocks--;
#endif
        eeFs.freeList = EeFsGetLink(eeFs.freeList);
        m_write_step += 1;
        EeFsFlushFreelist();
        return;
      case WRITE_NEXT_LINK_2:
        m_write_step -= WRITE_NEXT_LINK_2;
        EeFsSetLink(m_currBlk, 0);
        return;
    }
    uint8_t tmp = BS-sizeof(blkid_t)-m_ofs; if(tmp>m_write_len) tmp = m_write_len;
    m_write_buf += tmp;
    m_write_len -= tmp;
    m_ofs += tmp;
    m_pos += tmp;
    EeFsSetDat(m_currBlk, m_ofs-tmp, m_write_buf-tmp, tmp);
    return;
  }

  if (s_write_err == ERR_FULL) {
    POPUP_WARNING(STR_EEPROMOVERFLOW);
    m_write_step = 0;
    m_write_len = 0;
    m_cur_rlc_len = 0;
  }
  else if (!IS_SYNC_WRITE_ENABLE()) {
    nextRlcWriteStep();
  }
}

void RlcFile::create(uint8_t i_fileId, uint8_t typ, uint8_t sync_write)
{
  // all write operations will be executed on FILE_TMP
  openRlc(FILE_TMP); // internal use
  eeFs.files[FILE_TMP].typ      = typ;
  eeFs.files[FILE_TMP].size     = 0;
  m_fileId = i_fileId;
  ENABLE_SYNC_WRITE(sync_write);
}

/*
 * Copy file src to dst
 */
bool RlcFile::copy(uint8_t i_fileDst, uint8_t i_fileSrc)
{
  EFile theFile2;
  theFile2.openRd(i_fileSrc);

  create(i_fileDst, FILE_TYP_MODEL/*optimization, only model files are copied. should be eeFs.files[i_fileSrc].typ*/, true);

  uint8_t buf[BS-sizeof(blkid_t)];
  uint8_t len;
  while ((len=theFile2.read(buf, sizeof(buf))))
  {
    write(buf, len);
    if (write_errno() != 0) {
      ENABLE_SYNC_WRITE(false);
      return false;
    }
  }

  blkid_t fri=0;
  if (m_currBlk && (fri=EeFsGetLink(m_currBlk)))
    EeFsSetLink(m_currBlk, 0);

  if (fri) EeFsFree(fri);  //chain in

  eeFs.files[FILE_TMP].size = m_pos;
  EFile::swap(m_fileId, FILE_TMP);

  assert(!m_write_step);

  // s_sync_write is set to false in swap();
  return true;
}

#if defined(SDCARD)
const pm_char * eeBackupModel(uint8_t i_fileSrc)
{
  char *buf = reusableBuffer.modelsel.mainname;
  DIR archiveFolder;
  UINT written;

  // we must close the logs as we reuse the same FIL structure
  closeLogs();

  // check and create folder here
  strcpy_P(buf, STR_MODELS_PATH);
  FRESULT result = f_opendir(&archiveFolder, buf);
  if (result != FR_OK) {
    if (result == FR_NO_PATH)
      result = f_mkdir(buf);
    if (result != FR_OK)
      return SDCARD_ERROR(result);
  }

  buf[sizeof(MODELS_PATH)-1] = '/';
  eeLoadModelName(i_fileSrc, &buf[sizeof(MODELS_PATH)]);
  buf[sizeof(MODELS_PATH)+sizeof(g_model.header.name)] = '\0';

  uint8_t i = sizeof(MODELS_PATH)+sizeof(g_model.header.name)-1;
  uint8_t len = 0;
  while (i>sizeof(MODELS_PATH)-1) {
    if (!len && buf[i])
      len = i+1;
    if (len) {
      if (buf[i])
        buf[i] = idx2char(buf[i]);
      else
        buf[i] = '_'; 	
    }
    i--;
  }

  if (len == 0) {
    uint8_t num = i_fileSrc + 1;
    strcpy_P(&buf[sizeof(MODELS_PATH)], STR_MODEL);
    buf[sizeof(MODELS_PATH) + PSIZE(TR_MODEL)] = (char)((num / 10) + '0');
    buf[sizeof(MODELS_PATH) + PSIZE(TR_MODEL) + 1] = (char)((num % 10) + '0');
    len = sizeof(MODELS_PATH) + PSIZE(TR_MODEL) + 2;
  }

#if defined(RTCLOCK) && LCD_W >= 212
  char * tmp = strAppendDate(&buf[len]);
  len = tmp - buf;
#endif

  strcpy_P(&buf[len], STR_MODELS_EXT);

#ifdef SIMU
  TRACE("SD-card backup filename=%s", buf);
#endif

  result = f_open(&g_oLogFile, buf, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  EFile theFile2;
  theFile2.openRd(FILE_MODEL(i_fileSrc));

  *(uint32_t*)&buf[0] = O9X_FOURCC;
  buf[4] = g_eeGeneral.version;
  buf[5] = 'M';
  *(uint16_t*)&buf[6] = eeModelSize(i_fileSrc);

  result = f_write(&g_oLogFile, buf, 8, &written);
  if (result != FR_OK || written != 8) {
    f_close(&g_oLogFile);
    return SDCARD_ERROR(result);
  }

  while ((len=theFile2.read((uint8_t *)buf, 15))) {
    result = f_write(&g_oLogFile, (uint8_t *)buf, len, &written);
    if (result != FR_OK || written != len) {
      f_close(&g_oLogFile);
      return SDCARD_ERROR(result);
    }
  }

  f_close(&g_oLogFile);
  return NULL;
}

const pm_char * eeRestoreModel(uint8_t i_fileDst, char *model_name)
{
  char *buf = reusableBuffer.modelsel.mainname;
  UINT read;

  // we must close the logs as we reuse the same FIL structure
  closeLogs();

  strcpy_P(buf, STR_MODELS_PATH);
  buf[sizeof(MODELS_PATH)-1] = '/';
  strcpy(&buf[sizeof(MODELS_PATH)], model_name);
  strcpy_P(&buf[strlen(buf)], STR_MODELS_EXT);

  FRESULT result = f_open(&g_oLogFile, buf, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  if (f_size(&g_oLogFile) < 8) {
    f_close(&g_oLogFile);
    return STR_INCOMPATIBLE;
  }

  result = f_read(&g_oLogFile, (uint8_t *)buf, 8, &read);
  if (result != FR_OK || read != 8) {
    f_close(&g_oLogFile);
    return SDCARD_ERROR(result);
  }

  uint8_t version = (uint8_t)buf[4];
  if (*(uint32_t*)&buf[0] != O9X_FOURCC || version < FIRST_CONV_EEPROM_VER || version > EEPROM_VER || buf[5] != 'M') {
    f_close(&g_oLogFile);
    return STR_INCOMPATIBLE;
  }

  if (eeModelExists(i_fileDst)) {
    eeDeleteModel(i_fileDst);
  }

  theFile.create(FILE_MODEL(i_fileDst), FILE_TYP_MODEL, true);

  do {
    result = f_read(&g_oLogFile, (uint8_t *)buf, 15, &read);
    if (result != FR_OK) {
      ENABLE_SYNC_WRITE(false);
      f_close(&g_oLogFile);
      return SDCARD_ERROR(result);
    }
    if (read > 0) {
      theFile.write((uint8_t *)buf, read);
      if (write_errno() != 0) {
        ENABLE_SYNC_WRITE(false);
        f_close(&g_oLogFile);
        return STR_EEPROMOVERFLOW;
      }
    }
  } while (read == 15);

  blkid_t fri=0;
  if (theFile.m_currBlk && (fri=EeFsGetLink(theFile.m_currBlk)))
    EeFsSetLink(theFile.m_currBlk, 0);

  if (fri) EeFsFree(fri);  //chain in

  eeFs.files[FILE_TMP].size = theFile.m_pos;
  EFile::swap(theFile.m_fileId, FILE_TMP); // s_sync_write is set to false in swap();

  f_close(&g_oLogFile);

#if defined(PCBTARANIS)
  if (version < EEPROM_VER) {
    eeCheck(true);
    ConvertModel(i_fileDst, version);
    loadModel(g_eeGeneral.currModel);
  }
#endif

#if defined(CPUARM)
  eeLoadModelHeader(i_fileDst, &modelHeaders[i_fileDst]);
#endif

  return NULL;
}
#endif

void RlcFile::writeRlc(uint8_t i_fileId, uint8_t typ, uint8_t *buf, uint16_t i_len, uint8_t sync_write)
{
  create(i_fileId, typ, sync_write);

  m_write_step = WRITE_START_STEP;
  m_rlc_buf = buf;
  m_rlc_len = i_len;
  m_cur_rlc_len = 0;
#if defined (EEPROM_PROGRESS_BAR)
  m_ratio = (typ == FILE_TYP_MODEL ? 100 : 10);
#endif

  do {
    nextRlcWriteStep();
  } while (IS_SYNC_WRITE_ENABLE() && m_write_step && !s_write_err);
}

void RlcFile::nextRlcWriteStep()
{
  uint8_t cnt    = 1;
  uint8_t cnt0   = 0;
  uint16_t i = 0;

  if (m_cur_rlc_len) {
    uint8_t tmp1 = m_cur_rlc_len;
    uint8_t *tmp2 = m_rlc_buf;
    m_rlc_buf += m_cur_rlc_len;
    m_cur_rlc_len = 0;
    write(tmp2, tmp1);
    return;
  }

  bool run0 = (m_rlc_buf[0] == 0);

  if (m_rlc_len==0) goto close;

  for (i=1; 1; i++) { // !! laeuft ein byte zu weit !!
    bool cur0 = m_rlc_buf[i] == 0;
    if (cur0 != run0 || cnt==0x3f || (cnt0 && cnt==0x0f) || i==m_rlc_len) {
      if (run0) {
        assert(cnt0==0);
        if (cnt<8 && i!=m_rlc_len)
          cnt0 = cnt; //aufbew fuer spaeter
        else {
          m_rlc_buf+=cnt;
          m_rlc_len-=cnt;
          write1(cnt|0x40);
          return;
        }
      }
      else {
        m_rlc_buf+=cnt0;
        m_rlc_len-=cnt0+cnt;
        m_cur_rlc_len=cnt;
        if(cnt0){
          write1(0x80 | (cnt0<<4) | cnt);
        }
        else{
          write1(cnt);
        }
        return;
      }
      cnt=0;
      if (i==m_rlc_len) break;
      run0 = cur0;
    }
    cnt++;
  }

  close:

  switch(m_write_step) {
    case WRITE_START_STEP: {
      blkid_t fri = 0;

      if (m_currBlk && (fri = EeFsGetLink(m_currBlk))) {
        // TODO reuse EeFsFree!!!
        blkid_t prev_freeList = eeFs.freeList;
        eeFs.freeList = fri;
#if defined(CPUARM)
        freeBlocks++;
#endif
        while (EeFsGetLink(fri)) {
          fri = EeFsGetLink(fri);
#if defined(CPUARM)
          freeBlocks++;
#endif
        }
        m_write_step = WRITE_FREE_UNUSED_BLOCKS_STEP1;
        EeFsSetLink(fri, prev_freeList);
        return;
      }
    }

    case WRITE_FINAL_DIRENT_STEP: {
      m_currBlk = eeFs.files[FILE_TMP].startBlk;
      DirEnt & f = eeFs.files[m_fileId];
      eeFs.files[FILE_TMP].startBlk = f.startBlk;
      eeFs.files[FILE_TMP].size = f.size;
      f.startBlk = m_currBlk;
      f.size = m_pos;
      f.typ = eeFs.files[FILE_TMP].typ;
      m_write_step = WRITE_TMP_DIRENT_STEP;
      EeFsFlushDirEnt(m_fileId);
      return;
    }

    case WRITE_TMP_DIRENT_STEP:
      m_write_step = 0;
      EeFsFlushDirEnt(FILE_TMP);
      return;

    case WRITE_FREE_UNUSED_BLOCKS_STEP1:
      m_write_step = WRITE_FREE_UNUSED_BLOCKS_STEP2;
      EeFsSetLink(m_currBlk, 0);
      return;

    case WRITE_FREE_UNUSED_BLOCKS_STEP2:
      m_write_step = WRITE_FINAL_DIRENT_STEP;
      EeFsFlushFreelist();
      return;
  }
}

void RlcFile::flush()
{
#if !defined(CPUARM)
  while (eeprom_buffer_size > 0) wdt_reset();
#endif

  ENABLE_SYNC_WRITE(true);

  while (m_write_len && !s_write_err)
    nextWriteStep();

  while (isWriting() && !s_write_err)
    nextRlcWriteStep();

  ENABLE_SYNC_WRITE(false);
}

#if defined (EEPROM_PROGRESS_BAR)
void RlcFile::DisplayProgressBar(uint8_t x)
{
  if (s_eeDirtyMsk || isWriting() || eeprom_buffer_size) {
    uint8_t len = s_eeDirtyMsk ? 1 : limit((uint8_t)1, (uint8_t)(7 - (m_rlc_len/m_ratio)), (uint8_t)7);
    drawFilledRect(x+1, 0, 5, FH, SOLID, ERASE);
    drawFilledRect(x+2, 7-len, 3, len);
  }
}
#endif

// For conversions ...
#if defined(CPUARM)
void loadGeneralSettings()
{
  memset(&g_eeGeneral, 0, sizeof(g_eeGeneral));
  theFile.openRlc(FILE_GENERAL);
  theFile.readRlc((uint8_t*)&g_eeGeneral, sizeof(g_eeGeneral));
}

void loadModel(int index)
{
  memset(&g_model, 0, sizeof(g_model));
  theFile.openRlc(FILE_MODEL(index));
  theFile.readRlc((uint8_t*)&g_model, sizeof(g_model));
}
#endif

bool eeLoadGeneral()
{
  theFile.openRlc(FILE_GENERAL);
  if (theFile.readRlc((uint8_t*)&g_eeGeneral, 1) == 1 && g_eeGeneral.version == EEPROM_VER) {
    theFile.openRlc(FILE_GENERAL);
    if (theFile.readRlc((uint8_t*)&g_eeGeneral, sizeof(g_eeGeneral)) <= sizeof(EEGeneral) ){ //&& g_eeGeneral.variant == EEPROM_VARIANT) {
      return true; //testbracame
   }
 }

#if defined(PCBTARANIS)
  if (g_eeGeneral.variant != EEPROM_VARIANT) {
    TRACE("EEPROM variant %d instead of %d", g_eeGeneral.variant, EEPROM_VARIANT);
    return false;
  }
  else if (g_eeGeneral.version != EEPROM_VER) {
    TRACE("EEPROM version %d instead of %d", g_eeGeneral.version, EEPROM_VER);
    if (!eeConvert()) {
      return false;
    }
  }
  return true;
#else
  TRACE("EEPROM version %d (%d) instead of %d (%d)", g_eeGeneral.version, g_eeGeneral.variant, EEPROM_VER, EEPROM_VARIANT);
  return false;
#endif
}

void eeLoadModelName(uint8_t id, char *name)
{
  memclear(name, sizeof(g_model.header.name));
  if (id < MAX_MODELS) {
    theFile.openRlc(FILE_MODEL(id));
    theFile.readRlc((uint8_t*)name, sizeof(g_model.header.name));
  }
}

bool eeModelExists(uint8_t id)
{
    return EFile::exists(FILE_MODEL(id));
}

// TODO Now the 2 functions in eeprom_rlc.cpp and eeprom_raw.cpp are really close, should be merged.
void eeLoadModel(uint8_t id)
{
  if (id<MAX_MODELS) {

#if defined(CPUARM)
    watchdogSetTimeout(500/*5s*/);
#endif

#if defined(SDCARD)
    closeLogs();
#endif

    if (pulsesStarted()) {
      pausePulses();
    }

    pauseMixerCalculations();

    theFile.openRlc(FILE_MODEL(id));
    uint16_t sz = theFile.readRlc((uint8_t*)&g_model, sizeof(g_model));

#ifdef SIMU
    if (sz > 0 && sz != sizeof(g_model)) {
      printf("Model data read=%d bytes vs %d bytes\n", sz, (int)sizeof(ModelData));
    }
#endif

    bool newModel = false;

    if (sz < 256) {
      modelDefault(id);
      eeCheck(true);
      newModel = true;
    }

    AUDIO_FLUSH();
    flightReset();
    logicalSwitchesReset();

    if (pulsesStarted()) {
#if defined(GUI)
      if (!newModel) {
        checkAll();
      }
#endif
      resumePulses();
    }

    customFunctionsReset();

    restoreTimers();

#if defined(CPUARM)
    for (int i=0; i<MAX_SENSORS; i++) {
      TelemetrySensor & sensor = g_model.telemetrySensors[i];
      if (sensor.type == TELEM_TYPE_CALCULATED && sensor.persistent) {
        telemetryItems[i].value = sensor.persistentValue;
      }
    }
#endif

    LOAD_MODEL_CURVES();

    resumeMixerCalculations();
    // TODO pulses should be started after mixer calculations ...

#if defined(FRSKY)
    frskySendAlarms();
#endif

#if defined(CPUARM) && defined(SDCARD)
    referenceModelAudioFiles();
#endif

    LOAD_MODEL_BITMAP();
    LUA_LOAD_MODEL_SCRIPTS();
    SEND_FAILSAFE_1S();
    PLAY_MODEL_NAME();
  }
}

void eeErase(bool warn)
{
  generalDefault();

  if (warn) {
    ALERT(STR_EEPROMWARN, STR_BADEEPROMDATA, AU_BAD_EEPROM);
  }

  MESSAGE(STR_EEPROMWARN, STR_EEPROMFORMATTING, NULL, AU_EEPROM_FORMATTING);
  eepromFormat();
  theFile.writeRlc(FILE_GENERAL, FILE_TYP_GENERAL, (uint8_t*)&g_eeGeneral, sizeof(EEGeneral), true);
  modelDefault(0);
  theFile.writeRlc(FILE_MODEL(0), FILE_TYP_MODEL, (uint8_t*)&g_model, sizeof(g_model), true);
}

void eeCheck(bool immediately)
{
  if (immediately) {
    eeFlush();
  }

  if (s_eeDirtyMsk & EE_GENERAL) {
    TRACE("eeprom write general");
    s_eeDirtyMsk -= EE_GENERAL;
    theFile.writeRlc(FILE_GENERAL, FILE_TYP_GENERAL, (uint8_t*)&g_eeGeneral, sizeof(EEGeneral), immediately);
    if (!immediately) return;
  }

  if (s_eeDirtyMsk & EE_MODEL) {
    TRACE("eeprom write model");
    s_eeDirtyMsk = 0;
    theFile.writeRlc(FILE_MODEL(g_eeGeneral.currModel), FILE_TYP_MODEL, (uint8_t*)&g_model, sizeof(g_model), immediately);
  }
}

#if defined(CPUARM)
void eeLoadModelHeader(uint8_t id, ModelHeader *header)
{
  memclear(header, sizeof(ModelHeader));
  if (id < MAX_MODELS) {
    theFile.openRlc(FILE_MODEL(id));
    theFile.readRlc((uint8_t*)header, sizeof(ModelHeader));
  }
}

bool eeCopyModel(uint8_t dst, uint8_t src)
{
  if (theFile.copy(FILE_MODEL(dst), FILE_MODEL(src))) {
    memcpy(&modelHeaders[dst], &modelHeaders[src], sizeof(ModelHeader));
    return true;
  }
  else {
    return false;
  }
}

void eeSwapModels(uint8_t id1, uint8_t id2)
{
  EFile::swap(FILE_MODEL(id1), FILE_MODEL(id2));

  char tmp[sizeof(g_model.header)];
  memcpy(tmp, &modelHeaders[id1], sizeof(ModelHeader));
  memcpy(&modelHeaders[id1], &modelHeaders[id2], sizeof(ModelHeader));
  memcpy(&modelHeaders[id2], tmp, sizeof(ModelHeader));
}

void eeDeleteModel(uint8_t idx)
{
  EFile::rm(FILE_MODEL(idx));
  memset(&modelHeaders[idx], 0, sizeof(ModelHeader));
}
#endif
