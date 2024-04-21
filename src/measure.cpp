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
uint8_t text[20];             //sprint用


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
  Serial.printf("***** measure Fnukidan  ********* #%3d\n", measCnt);
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

  M5.Speaker.tone(880,100);

}


//--- display --------------------
void dispLoadMax(float val){
  //抜き弾抵抗力ピーク値[gf]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight....print系では効かない
  M5.Display.setTextColor(TFT_ENJI, TFT_BG_SCREEN);
  if (val <= -999){
    sprintf((char*)text, "　--- gf");
  }else{
    sprintf((char*)text, "　%5.0fgf", val);
  }
  M5.Display.drawString((char*)text, 309, 130, &fonts::lgfxJapanGothicP_20);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}

void dispLoad(float val){
  //抜き弾力[gf]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%6.1fgf", val);
  M5.Display.drawString((char*)text, 309, 82, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}

void dispPosition(float val){
  //玉位置[mm]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%6.2fmm", val);
  M5.Display.drawString((char*)text, 309, 40, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}

void dispMeasNum(uint16_t val){
  //測定番号
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_TITLE);
  sprintf((char*)text, "　#%3d", val);
  M5.Display.drawString((char*)text, 309, 0, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}

