//
// display.hpp
//
//  2024.04.23
//


#pragma once

#include "header.hpp"
#include "servo_common.hpp"

typedef enum {
  TO_START,
  TO_CENTER,
  TO_END,
} btn_b_name_t;



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
//button B
void  dispBtnB(btn_b_name_t);
void  dispTamaPos(tama_pos_t);
