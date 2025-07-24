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
void    getTimeStamp(char*);
void    getTimeText(char*);
void    generateFileName(void);
//time callback
void    timeavailable(struct timeval *t);

//SD card
uint8_t sdInit(void);
void    sdDataSave(char*, uint16_t, uint8_t, float*, float*, uint16_t, float);

//Bluetooth serial
void    bluetoothSerialInit(void);
void    btDataSend(char*, uint16_t, uint8_t, float*, float*, uint16_t, float);
void    btSerialRx(void);

