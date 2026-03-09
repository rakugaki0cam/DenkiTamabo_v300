//
// scale.hpp
//
//  2024.04.19
//


#pragma once

#include "HX711.h"
#include "header.hpp"


void  scaleInit(void);
float measLoad(uint8_t ch, uint8_t n);
void  scaleTare(uint8_t ch);