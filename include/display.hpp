//
// display.hpp
//
//  2024.04.23
//


#pragma once

#include "header.hpp"
#include "servo_common.hpp"
#include "sd_common.hpp"
#include "measure_common.hpp"


//button
typedef enum {
  MEAS_START,
  MEAS_READY,
  MEAS_RUNNING,
  MEAS_COMPLETE,
} btn_a_name_t;

typedef enum {
  TO_START,
  TO_CENTER,
  TO_END,
} btn_b_name_t;

typedef enum {
  BTNC_NOZZLE_SET,
  BTNC_PUSH_TO_START,
  BTNC_RUNNING,
  BTNC_NULL,
  BTNC_PACKING,
  BTNC_NOZZLE_RESET,
} btn_c_name_t;


//color
extern uint16_t TFT_ENJI;       //エンジ色
extern uint16_t TFT_BG_SCREEN;  //薄茶
extern uint16_t TFT_BG_TITLE;   //茶色
extern uint16_t TFT_BG_BUTTON;  //濃い茶色


//display
void  dispInit(void);
//
void  dispMeasNum(uint16_t val);
void  dispPosition(float val);
void  dispLoad(float val);
void  dispLoad2(float val);
void  dispLoadMax(float val);
void  dispZeroSet(void);
void  dispNukiInteg(float val);
//
void  dispBatV(float val);
void  dispSdcardStatus(uint8_t stat);
void  dispTamaPos(tama_pos_t pos);
void  dispWifi(wifi_stat_t stat);
void  dispBtSerial(bts_stat_t stat);
void  dispNozzle(noz_stat_t stat, float val);

//button 
void  dispBtnA(btn_a_name_t name);
void  dispBtnB(btn_b_name_t name);
void  dispBtnC(btn_c_name_t name);
