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
extern tama_pos_t tamaPos;


void servoInit(float);
//
float endAngleGet(void);
float startAngleGet(void);
void  servo1WriteMs(uint32_t);
float servoMove(float);
//btnB
void servoPosition(void);
//test
void servoAdjust(void);