//
// servo.hpp
//
//  2024.04.20
//


#pragma once

//#include "ESP32Servo.h"
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



void servoInit(float setAngle);
//
float servoMove(float angle, uint16_t speed);
void  servo1WriteUs(uint32_t pwUsec);
//btnB
void servoPosition(void);
//get
float endAngleGet(void);
float startAngleGet(void);
//test
void servoAdjust(void);