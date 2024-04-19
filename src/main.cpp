/*
  電気タマボー　
    Platform IO + VScode
    stack M5 CORE2 v1.1 #2

    2024.04.19 uiFlow + stack M5 CORE Basic V2.7から移植





*/


#include "header.hpp"

#define PIN_LOAD  36
#define PIN_BAT_ON  19  //13 BASIC
#define PIN_SERVO 33 // 32??   basic- 5, 2

#define LOADCELL_DOUT_PIN 16////////////2
#define LOADCELL_SCK_PIN  17////////////3


uint8_t fmVer[] = "3.00";
uint8_t cnt = 0;
uint16_t aveload;

Servo servo1;
int16_t val;
int8_t pos;



void setup() {
  pinMode(PIN_LOAD, ANALOG);
  pinMode(PIN_BAT_ON, OUTPUT);

  //ESP32servo
  // Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  servo1.setPeriodHertz(50);    // standard 50 hz servo
  servo1.attach(PIN_SERVO, 1000, 2000); 
  // using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep




  servo1.write(pos);//pos 0-180///////////////////////サーボ角度0〜180

  Serial.begin(115200);
  digitalWrite(PIN_BAT_ON, LOW);    //サーボ用バッテリ電源オフ



  digitalWrite(PIN_BAT_ON, HIGH);    //サーボ用バッテリ電源オン
  delay(500);

  delay(500);

  loadcellInit();
  //sd init

  M5.Display.init();
  M5.Display.setRotation(1);
  M5.Display.setColor(TFT_BLUE);
  M5.Display.fillRect(0, 0, 320, 18);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLUE);
  M5.Display.setCursor(4, 0);
  M5.Display.printf("電気タマボーM5 ver.%s", (char*)fmVer);

}

void loop() {
  dispLoad();  
  delay(1000);







  cnt++;
  if (cnt > 50){
    cnt = 0;
  }
  if ((cnt % 5) == 2){
    //loadcell
    //aveload = readLoad(); ////////////////

  }
  if (cnt == 10){
    //battery
    
  }
  if (cnt == 30){
    //adc    
  }
  if (cnt == 20){
    //battery charge    
  }
}
