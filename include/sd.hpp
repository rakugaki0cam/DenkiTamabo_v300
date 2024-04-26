/*
  sd.hpp

  2024.04.25
*/


#pragma once

#include "header.hpp"
#include "sd_common.hpp"

#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

//sd
uint8_t sdInit(void);
void    sdDataSave(uint16_t, uint8_t, float*, float*);




//wifi
uint8_t wifiInit(void);
//time
void printLocalTime(void);
void getTime(char*);
void getFileName(void);

//time callback
void timeavailable(struct timeval *t);
