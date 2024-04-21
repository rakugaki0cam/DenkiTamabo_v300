/*
    header.hpp
*/

#pragma once

#include <SD.h>
#include <M5Unified.h>
//usr
#include "servo.hpp"
#include "scale.hpp"
#include "measure.hpp"


//global
extern uint16_t measCnt;   //測定回数 
//color
extern uint16_t TFT_ENJI;       //エンジ色
extern uint16_t TFT_BG_SCREEN;  //薄茶
extern uint16_t TFT_BG_TITLE;   //茶色



uint8_t sdInit(void);

//display
void dispTitle(void);
void dispBatV(float);