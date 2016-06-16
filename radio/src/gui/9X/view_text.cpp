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

#include "../../opentx.h"

#define TEXT_FILENAME_MAXLEN  40
#define TEXT_FILE_MAXSIZE     2048

char s_text_file[TEXT_FILENAME_MAXLEN];
char s_text_screen[LCD_LINES-1][LCD_COLS+1];

void readTextFile(int & lines_count)
{
  FIL file;
  int result;
  char c;
  unsigned int sz;
  int line_length = 0;
  int escape = 0;
  char escape_chars[2];
  int current_line = 0;

  memset(s_text_screen, 0, sizeof(s_text_screen));

  result = f_open(&file, s_text_file, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK) {
    for (int i=0; i<TEXT_FILE_MAXSIZE && f_read(&file, &c, 1, &sz)==FR_OK && sz==1 && (lines_count==0 || current_line-menuVerticalOffset<LCD_LINES-1); i++) {
      if (c == '\n') {
        ++current_line;
        line_length = 0;
        escape = 0;
      }
      else if (c!='\r' && current_line>=menuVerticalOffset && current_line-menuVerticalOffset<LCD_LINES-1 && line_length<LCD_COLS) {
        if (c=='\\' && escape==0) {
          escape = 1;
          continue;
        }
        else if (c!='\\' && escape>0 && escape<3) {
          escape_chars[escape-1] = c;
          if (escape == 2 && !strncmp(escape_chars, "up", 2)) {
            c = '\300';
            escape = 0;
          }
          else if (escape == 2 && !strncmp(escape_chars, "dn", 2)) {
            c = '\301';
            escape = 0;
          }
          else {
            escape++;
            continue;
          }
        }
        else if (c=='~') {
          c = 'z'+1;
        }
        else if (c=='\t') {
          c = 0x1D; //tab
        }
        escape = 0;
        s_text_screen[current_line-menuVerticalOffset][line_length++] = c;
      }
    }
    if (c != '\n') {
      current_line += 1;
    }
    f_close(&file);
  }

  if (lines_count == 0) {
    lines_count = current_line;
  }
}

void menuTextView(uint8_t event)
{
  static int lines_count;

  switch (event) {
    case EVT_ENTRY:
      menuVerticalOffset = 0;
      lines_count = 0;
      readTextFile(lines_count);
      break;

    case EVT_KEY_FIRST(KEY_UP):
      if (menuVerticalOffset == 0)
        break;
      else
        menuVerticalOffset--;
        // no break;

    case EVT_KEY_FIRST(KEY_DOWN):
      // if (event == EVT_KEY_BREAK(KEY_DOWN)) {
        if (menuVerticalOffset+LCD_LINES-1 >= lines_count)
          break;
        else
          ++menuVerticalOffset;
      // }
      readTextFile(lines_count);
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      popMenu();
      break;
  }

  for (int i=0; i<LCD_LINES-1; i++) {
    lcdDrawTextAtt(0, i*FH+FH+1, s_text_screen[i], FIXEDWIDTH);
  }

  char *title = s_text_file;
#if defined(SIMU)
  if (!strncmp(title, "./", 2)) title += 2;
#else
  // TODO?
#endif
  lcdDrawText(LCD_W/2-strlen(title)*FW/2, 0, title);
  lcdInvertLine(0);

  if (lines_count > LCD_LINES-1) {
    displayScrollbar(LCD_W-1, FH, LCD_H-FH, menuVerticalOffset, lines_count, LCD_LINES-1);
  }
}

void pushMenuTextView(const char *filename)
{
  if (strlen(filename) < TEXT_FILENAME_MAXLEN) {
    strcpy(s_text_file, filename);
    pushMenu(menuTextView);
  }
}
