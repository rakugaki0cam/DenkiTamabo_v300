//
// servo.hpp
//
//  2024.04.20
//


#pragma once

#include "ESP32Servo.h"
#include "header.hpp"
#include "servo_common.hpp"


//global
//servo speed (delay_ms)
#define SPEED_FASTEST 0
#define SPEED_FAST    1
#define SPEED_MID     5
#define SPEED_SLOW    10
#define SPEED_MEAS    20

extern tama_pos_t tamaPos;



void servoInit(float);
//
float endAngleGet(void);
float startAngleGet(void);
float servoMove(float, uint16_t);
//btnB
void servoPosition(void);
//test
void  servo1WriteUs(uint32_t);
void servoAdjust(void);