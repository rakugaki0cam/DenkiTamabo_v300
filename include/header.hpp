/*
    header.hpp
*/

#pragma once

#include <SD.h>
#include <M5Unified.h>
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

//usr
#include "servo.hpp"
#include "scale.hpp"
#include "measure.hpp"
#include "graph.hpp"
#include "display.hpp"


//global
extern uint16_t measCnt;   //測定回数 
extern uint8_t fmVer[];

uint8_t wifiInit(void);
void printLocalTime(void);
void timeavailable(struct timeval *t);

uint8_t sdInit(void);

