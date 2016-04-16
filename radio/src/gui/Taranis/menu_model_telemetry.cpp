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

enum menuModelTelemetryItems {
  ITEM_TELEMETRY_PROTOCOL_TYPE,
  ITEM_TELEMETRY_RSSI_LABEL,
  ITEM_TELEMETRY_RSSI_ALARM1,
  ITEM_TELEMETRY_RSSI_ALARM2,
  ITEM_TELEMETRY_SENSORS_LABEL,
  ITEM_TELEMETRY_SENSOR1,
  ITEM_TELEMETRY_SENSOR2,
  ITEM_TELEMETRY_SENSOR3,
  ITEM_TELEMETRY_SENSOR4,
  ITEM_TELEMETRY_SENSOR5,
  ITEM_TELEMETRY_SENSOR6,
  ITEM_TELEMETRY_SENSOR7,
  ITEM_TELEMETRY_SENSOR8,
  ITEM_TELEMETRY_SENSOR9,
  ITEM_TELEMETRY_SENSOR10,
  ITEM_TELEMETRY_SENSOR11,
  ITEM_TELEMETRY_SENSOR12,
  ITEM_TELEMETRY_SENSOR13,
  ITEM_TELEMETRY_SENSOR14,
  ITEM_TELEMETRY_SENSOR15,
  ITEM_TELEMETRY_SENSOR16,
  ITEM_TELEMETRY_SENSOR17,
  ITEM_TELEMETRY_SENSOR18,
  ITEM_TELEMETRY_SENSOR19,
  ITEM_TELEMETRY_SENSOR20,
  ITEM_TELEMETRY_SENSOR21,
  ITEM_TELEMETRY_SENSOR22,
  ITEM_TELEMETRY_SENSOR23,
  ITEM_TELEMETRY_SENSOR24,
  ITEM_TELEMETRY_SENSOR25,
  ITEM_TELEMETRY_SENSOR26,
  ITEM_TELEMETRY_SENSOR27,
  ITEM_TELEMETRY_SENSOR28,
  ITEM_TELEMETRY_SENSOR29,
  ITEM_TELEMETRY_SENSOR30,
  ITEM_TELEMETRY_SENSOR31,
  ITEM_TELEMETRY_SENSOR32,
  ITEM_TELEMETRY_DISCOVER_SENSORS,
  ITEM_TELEMETRY_NEW_SENSOR,
  ITEM_TELEMETRY_DELETE_ALL_SENSORS,
  ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE,
#if defined(VARIO)
  ITEM_TELEMETRY_VARIO_LABEL,
  ITEM_TELEMETRY_VARIO_SOURCE,
  ITEM_TELEMETRY_VARIO_RANGE,
  ITEM_TELEMETRY_VARIO_CENTER,
#endif
  ITEM_TELEMETRY_TOP_BAR_LABEL,
  ITEM_TELEMETRY_TOP_BAR_VOLTAGE,
  ITEM_TELEMETRY_TOP_BAR_ALTITUDE,
  ITEM_TELEMETRY_SCREEN_LABEL1,
  ITEM_TELEMETRY_SCREEN_LINE1,
  ITEM_TELEMETRY_SCREEN_LINE2,
  ITEM_TELEMETRY_SCREEN_LINE3,
  ITEM_TELEMETRY_SCREEN_LINE4,
  ITEM_TELEMETRY_SCREEN_LABEL2,
  ITEM_TELEMETRY_SCREEN_LINE5,
  ITEM_TELEMETRY_SCREEN_LINE6,
  ITEM_TELEMETRY_SCREEN_LINE7,
  ITEM_TELEMETRY_SCREEN_LINE8,
  ITEM_TELEMETRY_SCREEN_LABEL3,
  ITEM_TELEMETRY_SCREEN_LINE9,
  ITEM_TELEMETRY_SCREEN_LINE10,
  ITEM_TELEMETRY_SCREEN_LINE11,
  ITEM_TELEMETRY_SCREEN_LINE12,
  ITEM_TELEMETRY_SCREEN_LABEL4,
  ITEM_TELEMETRY_SCREEN_LINE13,
  ITEM_TELEMETRY_SCREEN_LINE14,
  ITEM_TELEMETRY_SCREEN_LINE15,
  ITEM_TELEMETRY_SCREEN_LINE16,
  ITEM_TELEMETRY_MAX
};

#define TELEM_COL1                    (1*FW)
#define TELEM_COL2                    (16*FW)
#define TELEM_COL3                    (28*FW)
#define TELEM_BARS_COLMIN             TELEM_COL2
#define TELEM_BARS_COLMAX             TELEM_COL3
#define TELEM_SCRTYPE_COL             TELEM_COL2

#define IF_FAS_OFFSET(x)              x,
#define IS_RANGE_DEFINED(k)           (g_model.frsky.channels[k].ratio > 0)

#define SENSOR_ROWS(x)                (isTelemetryFieldAvailable(x) ? (uint8_t)0 : HIDDEN_ROW)
#define SENSORS_ROWS                  LABEL(Sensors), SENSOR_ROWS(0), SENSOR_ROWS(1), SENSOR_ROWS(2), SENSOR_ROWS(3), SENSOR_ROWS(4), SENSOR_ROWS(5), SENSOR_ROWS(6), SENSOR_ROWS(7), SENSOR_ROWS(8), SENSOR_ROWS(9), SENSOR_ROWS(10), SENSOR_ROWS(11), SENSOR_ROWS(12), SENSOR_ROWS(13), SENSOR_ROWS(14), SENSOR_ROWS(15), SENSOR_ROWS(16), SENSOR_ROWS(17), SENSOR_ROWS(18), SENSOR_ROWS(19), SENSOR_ROWS(20), SENSOR_ROWS(21), SENSOR_ROWS(22), SENSOR_ROWS(23), SENSOR_ROWS(24), SENSOR_ROWS(25), SENSOR_ROWS(26), SENSOR_ROWS(27), SENSOR_ROWS(28), SENSOR_ROWS(29), SENSOR_ROWS(30), SENSOR_ROWS(31), 0, 0, 0, 0,
#if defined(VARIO)
  #define VARIO_ROWS                  LABEL(Vario), 0, 1, 2,
#else
  #define VARIO_ROWS
#endif
#define RSSI_ROWS                     LABEL(RSSI), 0, 0,
#if defined(LUA)
  #define SCREEN_TYPE_ROWS            1
#else
  #define SCREEN_TYPE_ROWS            0
#endif
#define VARIO_RANGE_ROWS              3
#define TELEMETRY_TYPE_ROWS           (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM) ? (uint8_t)0 : HIDDEN_ROW,

#if defined(LUA)
  #define TELEMETRY_SCREEN_LINE(x)    ((TELEMETRY_SCREEN_TYPE(x) == TELEMETRY_SCREEN_TYPE_NONE || TELEMETRY_SCREEN_TYPE(x) == TELEMETRY_SCREEN_TYPE_SCRIPT) ? HIDDEN_ROW : (uint8_t)2)
#else
  #define TELEMETRY_SCREEN_LINE(x)    (TELEMETRY_SCREEN_TYPE(x) == TELEMETRY_SCREEN_TYPE_NONE ? HIDDEN_ROW : (uint8_t)2)
#endif
#define TELEMETRY_SCREEN_ROWS(x)      SCREEN_TYPE_ROWS, TELEMETRY_SCREEN_LINE(x), TELEMETRY_SCREEN_LINE(x), TELEMETRY_SCREEN_LINE(x), TELEMETRY_SCREEN_LINE(x)
#define TELEMETRY_CURRENT_SCREEN(k)   (k < ITEM_TELEMETRY_SCREEN_LABEL2 ? 0 : (k < ITEM_TELEMETRY_SCREEN_LABEL3 ? 1 : (k < ITEM_TELEMETRY_SCREEN_LABEL4 ? 2 : 3)))

enum SensorFields {
  SENSOR_FIELD_NAME,
  SENSOR_FIELD_TYPE,
  SENSOR_FIELD_ID,
  SENSOR_FIELD_FORMULA=SENSOR_FIELD_ID,
  SENSOR_FIELD_UNIT,
  SENSOR_FIELD_PRECISION,
  SENSOR_FIELD_PARAM1,
  SENSOR_FIELD_PARAM2,
  SENSOR_FIELD_PARAM3,
  SENSOR_FIELD_PARAM4,
  SENSOR_FIELD_AUTOOFFSET,
  SENSOR_FIELD_ONLYPOSITIVE,
  SENSOR_FIELD_FILTER,
  SENSOR_FIELD_PERSISTENT,
  SENSOR_FIELD_LOGS,
  SENSOR_FIELD_MAX
};

bool isSensorUnit(int sensor, uint8_t unit)
{
  if (sensor <= 0 || sensor > MAX_SENSORS ) return true;
  return g_model.telemetrySensors[--sensor].unit == unit;
}

bool isCellsSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_CELLS);
}

bool isGPSSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_GPS);
}

bool isAltSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_DIST) || isSensorUnit(sensor, UNIT_FEET);
}

bool isVoltsSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_VOLTS) || isSensorUnit(sensor, UNIT_CELLS);
}

bool isCurrentSensor(int sensor)
{
  return isSensorUnit(sensor, UNIT_AMPS);
}

bool isSensorAvailable(int sensor)
{
  if (sensor == 0)
    return true;
  else
    return isTelemetryFieldAvailable(abs(sensor) - 1);
}

#define SENSOR_2ND_COLUMN (12*FW)
#define SENSOR_3RD_COLUMN (18*FW)

#define SENSOR_UNIT_ROWS       ((sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_DIST)) || sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_PREC_ROWS       (sensor->isPrecConfigurable() && sensor->unit != UNIT_FAHRENHEIT  ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_PARAM1_ROWS     (sensor->unit >= UNIT_FIRST_VIRTUAL ? HIDDEN_ROW : (uint8_t)0)
#define SENSOR_PARAM2_ROWS     (sensor->unit == UNIT_GPS || sensor->unit == UNIT_DATETIME || sensor->unit == UNIT_CELLS || (sensor->type==TELEM_TYPE_CALCULATED && (sensor->formula==TELEM_FORMULA_CONSUMPTION || sensor->formula==TELEM_FORMULA_TOTALIZE)) ? HIDDEN_ROW : (uint8_t)0)
#define SENSOR_PARAM3_ROWS     (sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY) ? (uint8_t)0 : HIDDEN_ROW
#define SENSOR_PARAM4_ROWS     (sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY) ? (uint8_t)0 : HIDDEN_ROW
#define SENSOR_AUTOOFFSET_ROWS (sensor->unit != UNIT_RPMS && sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_ONLYPOS_ROWS    (sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_FILTER_ROWS     (sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_PERSISTENT_ROWS ((sensor->type == TELEM_TYPE_CALCULATED && sensor->formula == TELEM_FORMULA_CONSUMPTION) || sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)

void menuModelSensor(uint8_t event)
{
  TelemetrySensor * sensor = & g_model.telemetrySensors[s_currIdx];

  SUBMENU(STR_MENUSENSOR, SENSOR_FIELD_MAX, { 0, 0, sensor->type == TELEM_TYPE_CALCULATED ? (uint8_t)0 : (uint8_t)1, SENSOR_UNIT_ROWS, SENSOR_PREC_ROWS, SENSOR_PARAM1_ROWS, SENSOR_PARAM2_ROWS, SENSOR_PARAM3_ROWS, SENSOR_PARAM4_ROWS, SENSOR_AUTOOFFSET_ROWS, SENSOR_ONLYPOS_ROWS, SENSOR_FILTER_ROWS, SENSOR_PERSISTENT_ROWS, 0 });
  lcd_outdezAtt(PSIZE(TR_MENUSENSOR)*FW+1, 0, s_currIdx+1, INVERS|LEFT);

  putsTelemetryChannelValue(SENSOR_2ND_COLUMN, 0, s_currIdx, getValue(MIXSRC_FIRST_TELEM+3*s_currIdx), LEFT);

  int sub = menuVerticalPosition;

  for (int i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    int k = i + menuVerticalOffset;

    for (int j=0; j<k; j++) {
      if (mstate_tab[j+1] == HIDDEN_ROW) {
        if (++k >= (int)DIM(mstate_tab)) {
          return;
        }
      }
    }

    LcdFlags attr = (sub==k ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch (k) {

      case SENSOR_FIELD_NAME:
        editSingleName(SENSOR_2ND_COLUMN, y, STR_NAME, sensor->label, TELEM_LABEL_LEN, event, attr);
        break;

      case SENSOR_FIELD_TYPE:
        sensor->type = selectMenuItem(SENSOR_2ND_COLUMN, y, NO_INDENT(STR_TYPE), STR_VSENSORTYPES, sensor->type, 0, 1, attr, event);
        if (attr && checkIncDec_Ret) {
          sensor->instance = 0;
          if (sensor->type == TELEM_TYPE_CALCULATED) {
            sensor->param = 0;
            sensor->filter = 0;
            sensor->autoOffset = 0;
          }
        }
        break;

      case SENSOR_FIELD_ID:
        if (sensor->type == TELEM_TYPE_CUSTOM) {
          lcd_putsLeft(y, STR_ID);
          lcd_outhex4(SENSOR_2ND_COLUMN, y, sensor->id, LEFT|(menuHorizontalPosition==0 ? attr : 0));
          lcd_outdezAtt(SENSOR_3RD_COLUMN, y, sensor->instance, LEFT|(menuHorizontalPosition==1 ? attr : 0));
          if (attr) {
            switch (menuHorizontalPosition) {
              case 0:
                sensor->id = checkIncDec(event, sensor->id, 0x0000, 0xffff, INCDEC_REP10|NO_INCDEC_MARKS);
                break;

              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, sensor->instance, 0xff);
                break;
            }
          }
        }
        else {
          sensor->formula = selectMenuItem(SENSOR_2ND_COLUMN, y, STR_FORMULA, STR_VFORMULAS, sensor->formula, 0, TELEM_FORMULA_LAST, attr, event);
          if (attr && checkIncDec_Ret) {
            sensor->param = 0;
            if (sensor->formula == TELEM_FORMULA_CELL) {
              sensor->unit = UNIT_VOLTS;
              sensor->prec = 2;
            }
            else if (sensor->formula == TELEM_FORMULA_DIST) {
              sensor->unit = UNIT_DIST;
              sensor->prec = 0;
            }
            else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
              sensor->unit = UNIT_MAH;
              sensor->prec = 0;
            }
          }
        }
        break;

      case SENSOR_FIELD_UNIT:
        lcd_putsLeft(y, STR_UNIT);
        // TODO flash saving with selectMenuItem where I copied those 2 lines?
        lcd_putsiAtt(SENSOR_2ND_COLUMN, y, STR_VTELEMUNIT, sensor->unit, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR_ZERO(event, sensor->unit, UNIT_MAX);
          if (checkIncDec_Ret) {
            if (sensor->unit == UNIT_FAHRENHEIT) {
              sensor->prec = 0;
            }
            telemetryItems[s_currIdx].clear();
          }
        }
        break;

      case SENSOR_FIELD_PRECISION:
        sensor->prec = selectMenuItem(SENSOR_2ND_COLUMN, y, STR_PRECISION, STR_VPREC, sensor->prec, 0, 2, attr, event);
        if (attr && checkIncDec_Ret) {
          telemetryItems[s_currIdx].clear();
        }
        break;

      case SENSOR_FIELD_PARAM1:
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            lcd_putsLeft(y, STR_CELLSENSOR);
            putsMixerSource(SENSOR_2ND_COLUMN, y, sensor->cell.source ? MIXSRC_FIRST_TELEM+3*(sensor->cell.source-1) : 0, attr);
            if (attr) {
              sensor->cell.source = checkIncDec(event, sensor->cell.source, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isCellsSensor);
            }
            break;
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            lcd_putsLeft(y, STR_GPSSENSOR);
            putsMixerSource(SENSOR_2ND_COLUMN, y, sensor->dist.gps ? MIXSRC_FIRST_TELEM+3*(sensor->dist.gps-1) : 0, attr);
            if (attr) {
              sensor->dist.gps = checkIncDec(event, sensor->dist.gps, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isGPSSensor);
            }
            break;
          }
          else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
            lcd_putsLeft(y, STR_CURRENTSENSOR);
            putsMixerSource(SENSOR_2ND_COLUMN, y, sensor->consumption.source ? MIXSRC_FIRST_TELEM+3*(sensor->consumption.source-1) : 0, attr);
            if (attr) {
              sensor->consumption.source = checkIncDec(event, sensor->consumption.source, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isTelemetryFieldAvailable);
            }
            break;
          }
          else if (sensor->formula == TELEM_FORMULA_TOTALIZE) {
            lcd_putsLeft(y, NO_INDENT(STR_SOURCE));
            putsMixerSource(SENSOR_2ND_COLUMN, y, sensor->consumption.source ? MIXSRC_FIRST_TELEM+3*(sensor->consumption.source-1) : 0, attr);
            if (attr) {
              sensor->consumption.source = checkIncDec(event, sensor->consumption.source, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isTelemetryFieldComparisonAvailable);
            }
            break;
          }
        }
        else {
          if (sensor->unit == UNIT_RPMS) {
            lcd_putsLeft(y, NO_INDENT(STR_BLADES));
            if (attr) sensor->custom.ratio = checkIncDec(event, sensor->custom.ratio, 1, 30000, EE_MODEL|NO_INCDEC_MARKS|INCDEC_REP10);
            lcd_outdezAtt(SENSOR_2ND_COLUMN, y, sensor->custom.ratio, LEFT|attr);
            break;
          }
          else {
            lcd_putsLeft(y, STR_RATIO);
            if (attr) sensor->custom.ratio = checkIncDec(event, sensor->custom.ratio, 0, 30000, EE_MODEL|NO_INCDEC_MARKS|INCDEC_REP10);
            if (sensor->custom.ratio == 0)
              lcd_putcAtt(SENSOR_2ND_COLUMN, y, '-', attr);
            else
              lcd_outdezAtt(SENSOR_2ND_COLUMN, y, sensor->custom.ratio, LEFT|attr|PREC1);
            break;
          }
        }
        // no break

      case SENSOR_FIELD_PARAM2:
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            sensor->cell.index = selectMenuItem(SENSOR_2ND_COLUMN, y, STR_CELLINDEX, STR_VCELLINDEX, sensor->cell.index, 0, 8, attr, event);
            break;
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            lcd_putsLeft(y, STR_ALTSENSOR);
            putsMixerSource(SENSOR_2ND_COLUMN, y, sensor->dist.alt ? MIXSRC_FIRST_TELEM+3*(sensor->dist.alt-1) : 0, attr);
            if (attr) {
              sensor->dist.alt = checkIncDec(event, sensor->dist.alt, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isAltSensor);
            }
            break;
          }
        }
        else if (sensor->unit == UNIT_RPMS) {
          lcd_putsLeft(y, STR_MULTIPLIER);
          if (attr) sensor->custom.offset = checkIncDec(event, sensor->custom.offset, 1, 30000, EE_MODEL|NO_INCDEC_MARKS|INCDEC_REP10);
          lcd_outdezAtt(SENSOR_2ND_COLUMN, y, sensor->custom.offset, LEFT|attr);
          break;
        }
        else {
          lcd_putsLeft(y, NO_INDENT(STR_OFFSET));
          if (attr) sensor->custom.offset = checkIncDec(event, sensor->custom.offset, -30000, +30000, EE_MODEL|NO_INCDEC_MARKS|INCDEC_REP10);
          if (sensor->prec > 0) attr |= (sensor->prec == 2 ? PREC2 : PREC1);
          lcd_outdezAtt(SENSOR_2ND_COLUMN, y, sensor->custom.offset, LEFT|attr);
          break;
        }
        // no break

      case SENSOR_FIELD_PARAM3:
        // no break

      case SENSOR_FIELD_PARAM4:
      {
        putsStrIdx(0, y, NO_INDENT(STR_SOURCE), k-SENSOR_FIELD_PARAM1+1);
        int8_t & source = sensor->calc.sources[k-SENSOR_FIELD_PARAM1];
        if (attr) {
          source = checkIncDec(event, source, -MAX_SENSORS, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isSensorAvailable);
        }
        if (source < 0) {
          lcd_putcAtt(SENSOR_2ND_COLUMN, y, '-', attr);
          putsMixerSource(lcdNextPos, y, MIXSRC_FIRST_TELEM+3*(-1-source), attr);
        }
        else {
          putsMixerSource(SENSOR_2ND_COLUMN, y, source ? MIXSRC_FIRST_TELEM+3*(source-1) : 0, attr);
        }
        break;
      }

      case SENSOR_FIELD_AUTOOFFSET:
        ON_OFF_MENU_ITEM(sensor->autoOffset, SENSOR_2ND_COLUMN, y, STR_AUTOOFFSET, attr, event);
        break;

      case SENSOR_FIELD_ONLYPOSITIVE:
        ON_OFF_MENU_ITEM(sensor->onlyPositive, SENSOR_2ND_COLUMN, y, STR_ONLYPOSITIVE, attr, event);
        break;

      case SENSOR_FIELD_FILTER:
        ON_OFF_MENU_ITEM(sensor->filter, SENSOR_2ND_COLUMN, y, STR_FILTER, attr, event);
        break;
        
      case SENSOR_FIELD_PERSISTENT:
        ON_OFF_MENU_ITEM(sensor->persistent, SENSOR_2ND_COLUMN, y, NO_INDENT(STR_PERSISTENT), attr, event);
        break;

      case SENSOR_FIELD_LOGS:
        ON_OFF_MENU_ITEM(sensor->logs, SENSOR_2ND_COLUMN, y, STR_LOGS, attr, event);
        if (attr && checkIncDec_Ret) {
          closeLogs();
        }
        break;

    }
  }
}

void onSensorMenu(const char *result)
{
  int index = menuVerticalPosition - ITEM_TELEMETRY_SENSOR1;

  if (index < MAX_SENSORS) {
    if (result == STR_EDIT) {
      pushMenu(menuModelSensor);
    }
    else if (result == STR_DELETE) {
      delTelemetryIndex(index);
      index += 1;
      if (index<MAX_SENSORS && isTelemetryFieldAvailable(index))
        menuVerticalPosition += 1;
      else
        menuVerticalPosition = ITEM_TELEMETRY_NEW_SENSOR;
    }
    else if (result == STR_COPY) {
      int newIndex = availableTelemetryIndex();
      
      if (newIndex >= 0) {
        TelemetrySensor & sourceSensor = g_model.telemetrySensors[index];
        TelemetrySensor & newSensor = g_model.telemetrySensors[newIndex];
        newSensor = sourceSensor;
        TelemetryItem & sourceItem = telemetryItems[index];
        TelemetryItem & newItem = telemetryItems[newIndex];
        newItem = sourceItem;
        eeDirty(EE_MODEL);
      } 
      else {
        POPUP_WARNING(STR_TELEMETRYFULL);
      }
    }
  }
}

#if defined(LUA)
void onTelemetryScriptFileSelectionMenu(const char *result)
{
  int sub = menuVerticalPosition;
  int screenIndex = TELEMETRY_CURRENT_SCREEN(sub);

  if (result == STR_UPDATE_LIST) {
    if (!listSdFiles(SCRIPTS_TELEM_PATH, SCRIPTS_EXT, sizeof(g_model.frsky.screens[screenIndex].script.file), NULL)) {
      POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
    }
  }
  else {
    // The user choosed a file in the list
    memcpy(g_model.frsky.screens[screenIndex].script.file, result, sizeof(g_model.frsky.screens[screenIndex].script.file));
    eeDirty(EE_MODEL);
    LUA_LOAD_MODEL_SCRIPTS();
  }
}
#endif

void menuModelTelemetry(uint8_t event)
{
  if (warningResult) {
    warningResult = 0;
    for (int i=0; i<MAX_SENSORS; i++) {
      delTelemetryIndex(i);
    }
  }
  
  MENU(STR_MENUTELEMETRY, menuTabModel, e_Telemetry, ITEM_TELEMETRY_MAX, { TELEMETRY_TYPE_ROWS RSSI_ROWS SENSORS_ROWS VARIO_ROWS LABEL(TopBar), 0, 0, TELEMETRY_SCREEN_ROWS(0), TELEMETRY_SCREEN_ROWS(1), CASE_CPUARM(TELEMETRY_SCREEN_ROWS(2)) CASE_CPUARM(TELEMETRY_SCREEN_ROWS(3)) });

  int sub = menuVerticalPosition;

  for (int i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    int k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j] == HIDDEN_ROW)
        k++;
    }

    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    if (k>=ITEM_TELEMETRY_SENSOR1 && k<ITEM_TELEMETRY_SENSOR1+MAX_SENSORS) {
      int index = k-ITEM_TELEMETRY_SENSOR1;
      lcd_outdezAtt(INDENT_WIDTH, y, index+1, LEFT|attr);
      lcd_putcAtt(lcdLastPos, y, ':', attr);
      lcd_putsnAtt(3*FW, y, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN, ZCHAR);
      if (telemetryItems[index].isFresh()) {
        lcd_putc(10*FW, y, '*');
      }
      TelemetryItem & telemetryItem = telemetryItems[index];
      if (telemetryItem.isAvailable()) {
        bool isOld = telemetryItem.isOld();
        lcdNextPos = TELEM_COL2;
        if (isOld) lcd_putc(lcdNextPos, y, '[');
        putsTelemetryChannelValue(lcdNextPos, y, index, getValue(MIXSRC_FIRST_TELEM+3*index), LEFT);
        if (isOld) lcd_putc(lcdLastPos, y, ']');
      }
      else {
        lcd_putsAtt(TELEM_COL2, y, "---", 0); // TODO shortcut
      }
      TelemetrySensor * sensor = & g_model.telemetrySensors[index];
      if (sensor->type == TELEM_TYPE_CUSTOM && !g_model.ignoreSensorIds) {
        lcd_outdezAtt(TELEM_COL3, y, sensor->instance, LEFT);
      }
      if (attr) {
        s_editMode = 0;
        s_currIdx = index;
        if (event == EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(event);
          POPUP_MENU_ADD_ITEM(STR_EDIT);
          POPUP_MENU_ADD_ITEM(STR_COPY);
          POPUP_MENU_ADD_ITEM(STR_DELETE);
          popupMenuHandler = onSensorMenu;
        }
        else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          pushMenu(menuModelSensor);
        }
      }
    }
    else

    switch (k) {
      case ITEM_TELEMETRY_PROTOCOL_TYPE:
        g_model.telemetryProtocol = selectMenuItem(TELEM_COL2, y, STR_TELEMETRY_TYPE, "\017FrSky S.PORT\0  FrSky D\0       FrSky D (cable)", g_model.telemetryProtocol, PROTOCOL_TELEMETRY_FIRST, g_eeGeneral.serial2Mode==UART_MODE_TELEMETRY ? PROTOCOL_FRSKY_D_SECONDARY : PROTOCOL_FRSKY_D, attr, event);
        break;

      case ITEM_TELEMETRY_SENSORS_LABEL:
        lcd_putsLeft(y, STR_TELEMETRY_SENSORS);
        lcd_putsAtt(TELEM_COL2, y, STR_VALUE, 0);
        if (!g_model.ignoreSensorIds) {
          lcd_putsAtt(TELEM_COL3, y, STR_ID, 0);
        }
        break;

      case ITEM_TELEMETRY_DISCOVER_SENSORS:
        lcd_putsAtt(0, y, allowNewSensors ? STR_STOP_DISCOVER_SENSORS : STR_DISCOVER_SENSORS, attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = 0;
          allowNewSensors = !allowNewSensors;
        }
        break;

      case ITEM_TELEMETRY_NEW_SENSOR:
        lcd_putsAtt(0, y, STR_TELEMETRY_NEWSENSOR, attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = 0;
          int res = availableTelemetryIndex();
          if (res >= 0) {
            s_currIdx = res;
            pushMenu(menuModelSensor);
          }
          else {
            POPUP_WARNING(STR_TELEMETRYFULL);
          }
        }
        break;

      case ITEM_TELEMETRY_DELETE_ALL_SENSORS:
        lcd_putsAtt(0, y, STR_DELETE_ALL_SENSORS, attr);
        if (attr)
          s_editMode = 0;
        if (attr && event==EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(KEY_ENTER);
          POPUP_CONFIRMATION(STR_CONFIRMDELETE);
        }
        break;        

      case ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE:
        ON_OFF_MENU_ITEM(g_model.ignoreSensorIds, TELEM_COL2, y, STR_IGNORE_INSTANCE, attr, event);
        break;

      case ITEM_TELEMETRY_RSSI_LABEL:
        lcd_putsLeft(y, PSTR("RSSI"));
        break;

      case ITEM_TELEMETRY_RSSI_ALARM1:
      case ITEM_TELEMETRY_RSSI_ALARM2: {
        uint8_t alarm = k-ITEM_TELEMETRY_RSSI_ALARM1;
        lcd_putsLeft(y, (alarm==0 ? STR_LOWALARM : STR_CRITICALALARM));
        lcd_outdezNAtt(TELEM_COL2, y, getRssiAlarmValue(alarm), LEFT|attr, 3);
        if (attr && s_editMode>0) {
          CHECK_INCDEC_MODELVAR(event, g_model.frsky.rssiAlarms[alarm].value, -30, 30);
        }
        break;
      }

#if defined(VARIO)
      case ITEM_TELEMETRY_VARIO_LABEL:
        lcd_putsLeft(y, STR_VARIO);
        break;

      case ITEM_TELEMETRY_VARIO_SOURCE:
        lcd_putsLeft(y, STR_SOURCE);
        putsMixerSource(TELEM_COL2, y, g_model.frsky.varioSource ? MIXSRC_FIRST_TELEM+3*(g_model.frsky.varioSource-1) : 0, attr);
        if (attr) {
          g_model.frsky.varioSource = checkIncDec(event, g_model.frsky.varioSource, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isSensorAvailable);
        }
        break;

      case ITEM_TELEMETRY_VARIO_RANGE:
        lcd_putsLeft(y, STR_RANGE);
        lcd_outdezAtt(TELEM_COL2, y, -10+g_model.frsky.varioMin, (menuHorizontalPosition==0 ? attr : 0)|LEFT);
        lcd_outdezAtt(TELEM_COL2+7*FW, y, 10+g_model.frsky.varioMax, (menuHorizontalPosition==1 ? attr : 0)|LEFT);
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.frsky.varioMin, -7, 7);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.frsky.varioMax, -7, 7);
              break;
          }
        }
        break;

      case ITEM_TELEMETRY_VARIO_CENTER:
        lcd_putsLeft(y, STR_CENTER);
        lcd_outdezAtt(TELEM_COL2, y, -5+g_model.frsky.varioCenterMin, (menuHorizontalPosition==0 ? attr : 0)|PREC1|LEFT);
        lcd_outdezAtt(TELEM_COL2+7*FW, y, 5+g_model.frsky.varioCenterMax, (menuHorizontalPosition==1 ? attr : 0)|PREC1|LEFT);
        lcd_putsiAtt(TELEM_COL3, y, STR_VVARIOCENTER, g_model.frsky.varioCenterSilent, (menuHorizontalPosition==2 ? attr : 0));
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.frsky.varioCenterMin, -16, 5+min<int8_t>(10, g_model.frsky.varioCenterMax+5));
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.frsky.varioCenterMax, -5+max<int8_t>(-10, g_model.frsky.varioCenterMin-5), +15);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.frsky.varioCenterSilent, 1);
              break;
          }
        }
        break;

#endif

      case ITEM_TELEMETRY_TOP_BAR_LABEL:
        lcd_putsLeft(y, STR_TOP_BAR);
        break;

      case ITEM_TELEMETRY_TOP_BAR_VOLTAGE:
        lcd_putsLeft(y, STR_VOLTAGE);
        putsMixerSource(TELEM_COL2, y, g_model.frsky.voltsSource ? MIXSRC_FIRST_TELEM+3*(g_model.frsky.voltsSource-1) : 0, attr);
        if (attr) {
          g_model.frsky.voltsSource = checkIncDec(event, g_model.frsky.voltsSource, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isVoltsSensor);
        }
        break;

      case ITEM_TELEMETRY_TOP_BAR_ALTITUDE:
        lcd_putsLeft(y, STR_ALTITUDE);
        putsMixerSource(TELEM_COL2, y, g_model.frsky.altitudeSource ? MIXSRC_FIRST_TELEM+3*(g_model.frsky.altitudeSource-1) : 0, attr);
        if (attr) {
          g_model.frsky.altitudeSource = checkIncDec(event, g_model.frsky.altitudeSource, 0, MAX_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isAltSensor);
        }
        break;

      case ITEM_TELEMETRY_SCREEN_LABEL1:
      case ITEM_TELEMETRY_SCREEN_LABEL2:
      case ITEM_TELEMETRY_SCREEN_LABEL3:
      case ITEM_TELEMETRY_SCREEN_LABEL4:
      {
        uint8_t screenIndex = TELEMETRY_CURRENT_SCREEN(k);
        putsStrIdx(0*FW, y, STR_SCREEN, screenIndex+1);
        TelemetryScreenType oldScreenType = TELEMETRY_SCREEN_TYPE(screenIndex);
        TelemetryScreenType newScreenType = (TelemetryScreenType)selectMenuItem(TELEM_SCRTYPE_COL, y, PSTR(""), STR_VTELEMSCREENTYPE, oldScreenType, 0, TELEMETRY_SCREEN_TYPE_MAX, (menuHorizontalPosition==0 ? attr : 0), event);
        if (newScreenType != oldScreenType) {
          g_model.frsky.screensType = (g_model.frsky.screensType & (~(0x03 << (2*screenIndex)))) | (newScreenType << (2*screenIndex));
          memset(&g_model.frsky.screens[screenIndex], 0, sizeof(g_model.frsky.screens[screenIndex]));
        }
#if defined(LUA)
        if (newScreenType == TELEMETRY_SCREEN_TYPE_SCRIPT) {
          TelemetryScriptData & scriptData = g_model.frsky.screens[screenIndex].script;

          // TODO better function name for ---
          // TODO function for these lines
          if (ZEXIST(scriptData.file))
            lcd_putsnAtt(TELEM_SCRTYPE_COL+7*FW, y, scriptData.file, sizeof(scriptData.file), (menuHorizontalPosition==1 ? attr : 0));
          else
            lcd_putsiAtt(TELEM_SCRTYPE_COL+7*FW, y, STR_VCSWFUNC, 0, (menuHorizontalPosition==1 ? attr : 0));

          if (menuHorizontalPosition==1 && attr && event==EVT_KEY_BREAK(KEY_ENTER) && READ_ONLY_UNLOCKED()) {
            s_editMode = 0;
            if (listSdFiles(SCRIPTS_TELEM_PATH, SCRIPTS_EXT, sizeof(g_model.frsky.screens[screenIndex].script.file), g_model.frsky.screens[screenIndex].script.file)) {
              popupMenuHandler = onTelemetryScriptFileSelectionMenu;
            }
            else {
              POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
            }
          }
        }
        else if (attr) {
          MOVE_CURSOR_FROM_HERE();
        }
#endif
        break;
      }

      case ITEM_TELEMETRY_SCREEN_LINE1:
      case ITEM_TELEMETRY_SCREEN_LINE2:
      case ITEM_TELEMETRY_SCREEN_LINE3:
      case ITEM_TELEMETRY_SCREEN_LINE4:
      case ITEM_TELEMETRY_SCREEN_LINE5:
      case ITEM_TELEMETRY_SCREEN_LINE6:
      case ITEM_TELEMETRY_SCREEN_LINE7:
      case ITEM_TELEMETRY_SCREEN_LINE8:
      case ITEM_TELEMETRY_SCREEN_LINE9:
      case ITEM_TELEMETRY_SCREEN_LINE10:
      case ITEM_TELEMETRY_SCREEN_LINE11:
      case ITEM_TELEMETRY_SCREEN_LINE12:
      case ITEM_TELEMETRY_SCREEN_LINE13:
      case ITEM_TELEMETRY_SCREEN_LINE14:
      case ITEM_TELEMETRY_SCREEN_LINE15:
      case ITEM_TELEMETRY_SCREEN_LINE16:
      {
        uint8_t screenIndex, lineIndex;
        if (k < ITEM_TELEMETRY_SCREEN_LABEL2) {
          screenIndex = 0;
          lineIndex = k-ITEM_TELEMETRY_SCREEN_LINE1;
        }
        else if (k >= ITEM_TELEMETRY_SCREEN_LABEL4) {
          screenIndex = 3;
          lineIndex = k-ITEM_TELEMETRY_SCREEN_LINE13;
        }
        else if (k >= ITEM_TELEMETRY_SCREEN_LABEL3) {
          screenIndex = 2;
          lineIndex = k-ITEM_TELEMETRY_SCREEN_LINE9;
        }
        else {
          screenIndex = 1;
          lineIndex = k-ITEM_TELEMETRY_SCREEN_LINE5;
        }

#if defined(GAUGES)
        if (IS_BARS_SCREEN(screenIndex)) {
          FrSkyBarData & bar = g_model.frsky.screens[screenIndex].bars[lineIndex];
          source_t barSource = bar.source;
          putsMixerSource(TELEM_COL1, y, barSource, menuHorizontalPosition==0 ? attr : 0);
          int barMax = getMaximumValue(barSource);
          int barMin = -barMax;
          if (barSource) {
            if (barSource <= MIXSRC_LAST_CH) {
              putsChannelValue(TELEM_BARS_COLMIN, y, barSource, calc100toRESX(bar.barMin), (menuHorizontalPosition==1 ? attr : 0) | LEFT);
              putsChannelValue(TELEM_BARS_COLMAX, y, barSource, calc100toRESX(bar.barMax), (menuHorizontalPosition==2 ? attr : 0) | LEFT);
            }
            else {
              putsChannelValue(TELEM_BARS_COLMIN, y, barSource, bar.barMin, (menuHorizontalPosition==1 ? attr : 0) | LEFT);
              putsChannelValue(TELEM_BARS_COLMAX, y, barSource, bar.barMax, (menuHorizontalPosition==2 ? attr : 0) | LEFT);
            }
          }
          else if (attr) {
            MOVE_CURSOR_FROM_HERE();
          }
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                bar.source = checkIncDec(event, barSource, 0, MIXSRC_LAST_TELEM, EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable);
                if (checkIncDec_Ret) {
                  if (barSource <= MIXSRC_LAST_CH) {
                    bar.barMin = -100;
                    bar.barMax = 100;
                  }
                  else {
                    bar.barMin = 0;
                    bar.barMax = 0;
                  }
                }
                break;
              case 1:
                bar.barMin = checkIncDec(event, bar.barMin, barMin, bar.barMax, EE_MODEL|NO_INCDEC_MARKS);
                break;
              case 2:
                bar.barMax = checkIncDec(event, bar.barMax, bar.barMin, barMax, EE_MODEL|NO_INCDEC_MARKS);
                break;
            }
          }
        }
        else
#endif
        {
          for (int c=0; c<NUM_LINE_ITEMS; c++) {
            LcdFlags cellAttr = (menuHorizontalPosition==c ? attr : 0);
            source_t & value = g_model.frsky.screens[screenIndex].lines[lineIndex].sources[c];
            const coord_t pos[] = {TELEM_COL1, TELEM_COL2, TELEM_COL3};
            putsMixerSource(pos[c], y, value, cellAttr);
            if (cellAttr && s_editMode>0) {
              value = checkIncDec(event, value, 0, MIXSRC_LAST_TELEM, EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable);
            }
          }
          if (attr && menuHorizontalPosition == NUM_LINE_ITEMS) {
            REPEAT_LAST_CURSOR_MOVE();
          }
        }
        break;
      }
    }
  }
}
