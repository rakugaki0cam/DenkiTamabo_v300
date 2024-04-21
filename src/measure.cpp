/*
 * measure.cpp
 *
 * 2024.04.21
*/

#include "measure.hpp"

#define SAMPLE_NUM 50

float loadMax = 0;              //ピーク値
float nukiPos[SAMPLE_NUM];      //測定位置
float nukiForce[SAMPLE_NUM];    //抜き弾力測定値
float stepMoving = -2.0f;


void measNukiF(void){
  //抜き弾抵抗力の測定
  float servoAngle;
  float load;
  float tamaPos;
  //SDへヘッダ書き足し
  Serial.println("*** measure *****");
  //
  M5.Display.setFont(&fonts::lgfxJapanGothicP_20);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(220, 125);
  M5.Display.printf("%5.0fgf　", loadMax);

  float startAngle = startAngleGet();
  float endAngle = endAngleGet();

  Serial.printf("measure angle: %4.1f ~ %4.1f deg\n", startAngle, endAngle);

  //スケールのゼロセット
  Serial.print("Zero set --> ");
  servoMove(endAngle);
  delay(200);
  scaleTare();
  Serial.println("scale ZeroSET");
  delay(200);
  //
  servoAngle = startAngle;
  Serial.print("Start pos --> ");
  tamaPos = servoMove(servoAngle);
  M5.Display.setCursor(240, 40);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.printf("%6.3fmm　", tamaPos);
  delay(800);   //玉を落ち着かせる
  //
  load = measLoad(10);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(240, 82);
  M5.Display.printf("%6.1fgf　", load);
  //

  //測定loop
  while(servoAngle <= endAngle){
    Serial.printf("measure:%3.0fdeg --> ", servoAngle);
    tamaPos = servoMove(servoAngle);  
    M5.Display.setCursor(240, 40);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("%6.3fmm　", tamaPos);
    delay(3);
    load = measLoad(10);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(240, 82);
    M5.Display.printf("%6.1fgf　", load);
    //max
    if (load > loadMax){
      loadMax = load;
      M5.Display.setFont(&fonts::lgfxJapanGothicP_20);
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(220, 125);
      M5.Display.printf("%5.0fgf　", loadMax);
    }
    //data
    //SDsave
    servoAngle -= stepMoving;
    
  }

  M5.Speaker.tone(880,100);

}