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
#include "graph.hpp"
#include "display.hpp"
#include "sd.hpp"


//global
extern uint8_t fmVer[];
extern uint16_t measCnt;    //測定回数 
extern uint8_t sdStat;      //SDcard
extern const char *TAG;


void vibration(uint16_t);