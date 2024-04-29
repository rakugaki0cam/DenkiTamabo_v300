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
void  dispLoadMax(float);
void  dispLoad(float);
void  dispZeroSet(void);
void  dispPosition(float);
void  dispMeasNum(uint16_t);
void  dispBatV(float);
void  dispSdcardStatus(uint8_t);
void  dispTamaPos(tama_pos_t);
void  dispWifi(wifi_stat_t);
void  dispBtSerial(bts_stat_t);
void  dispNozzle(noz_stat_t, float);

//button 
void  dispBtnA(btn_a_name_t);
void  dispBtnB(btn_b_name_t);
void  dispBtnC(btn_c_name_t);

