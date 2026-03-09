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

#include <BluetoothSerial.h>


//wifi
uint8_t wifiInit(void);

uint8_t ntpTimeInit(void);
void    printLocalTime(void);
void    getTimeStamp(char* txt);
void    getTimeText(char* txt);
void    generateFileName(void);
//time callback
void    timeavailable(struct timeval *t);

//SD card
uint8_t sdInit(void);
uint8_t fileGenerate(void);
void    sdDataSave(char* time, uint16_t mNum, uint8_t n, float* pos, float* load, float* load2, uint16_t speed, float nukiIntegral);

//Bluetooth serial
void    bluetoothSerialInit(void);
void    btDataSend(char* time, uint16_t mNum, uint8_t n, float* pos, float* load, float* load2, uint16_t speed, float nukiIntegral);
void    btSerialRx(void);

