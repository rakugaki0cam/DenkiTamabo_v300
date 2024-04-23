/*
 * measure.cpp
 *
 * 2024.04.21
*/

#include "measure.hpp"

#define SAMPLE_NUM 50

float nukiPos[SAMPLE_NUM];    //測定位置
float nukiForce[SAMPLE_NUM];  //抜き弾力測定値
float stepMoving = 2.0f;      //測定時の角度増分


void measNukiF(void){
  //抜き弾抵抗力の測定
  float servoAngle;
  float load;
  float loadMax;
  float tamaPos;
  uint8_t i;
  float startAngle = startAngleGet();
  float endAngle = endAngleGet();

  measCnt++;
  //番号
  Serial.println();
  Serial.printf("***** measure Nukidan  #%3d  ********* \n", measCnt);
  Serial.printf("measure angle: %4.1f ~ %4.1f deg\n", startAngle, endAngle);
  dispMeasNum(measCnt);

  //SDへヘッダ書き出し
  //

  
  //抜き弾値リセット
  loadMax = 0;
  dispLoadMax(-9999); //表示をリセット

  //スケールのゼロセット
  Serial.print("Zero set --> ");
  servoMove(endAngle);
  delay(200);
  scaleTare();  //スケールゼロ
  Serial.println(" ---> scale zero set.");
  load = measLoad(10);
  dispLoad(load);
  delay(200);

  //スタート位置
  servoAngle = startAngle;
  Serial.print("Start pos --> ");
  tamaPos = servoMove(servoAngle);
  dispPosition(tamaPos);
  load = measLoad(10);
  dispLoad(load);
  Serial.println(" ---> start position wait.");

  //
  delay(800);   //玉を落ち着かせる

  //測定loop
  while(servoAngle <= endAngle){
    Serial.printf("Measure angle:%5.1fdeg --> ", servoAngle);
    tamaPos = servoMove(servoAngle);
    dispPosition(tamaPos);
    delay(10);            //サーボが動き終わるまで待つ
    load = measLoad(10);
    dispLoad(load);       //抵抗力測定
    //
    graphPlot(tamaPos, load);
    //
    Serial.printf(" == LOAD: %6.1fgf ", load);
    //簡易グラフ表示
    #define A_SCALE 20
    uint8_t n = load / A_SCALE;
    for(i = 0; i < n; i++){
      Serial.print("*");
    }
    Serial.println();

    //ピーク値のチェック
    if (load > loadMax){
      loadMax = load;
      dispLoadMax(loadMax);
    }

    //data
    //SDsave
    servoAngle += stepMoving;
    
  }
}

