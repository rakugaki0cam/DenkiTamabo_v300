//
// servo.hpp
//
//  2024.04.20
//


#pragma once

#include "ESP32Servo.h"
#include "header.hpp"

//global

typedef enum {
  CENTER1_POS,
  START_POS,
  CENTER2_POS,
  END_POS,
} tama_pos_t;
extern tama_pos_t tamaPos;

typedef enum {
  TO_START,
  TO_CENTER,
  TO_END,
} btn_b_name_t;





void servoInit(void);
//
float endAngleGet(void);
float startAngleGet(void);

float servoMove(float);

//
void servoPosition(void);
void btnBname(btn_b_name_t);
void dispTamaPos(tama_pos_t );
//
void servoAdjust(void);