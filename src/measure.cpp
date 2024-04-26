/*
 * measure.cpp
 *
 * 2024.04.21
*/

#include "measure.hpp"


float stepMoving = 2.0f;      //測定時の角度増分

void measNukiF(void){
  //抜き弾抵抗力の測定
#define SAMPLE_NUM 50
  float tamaPos[SAMPLE_NUM];    //測定位置
  float load[SAMPLE_NUM];  //抜き弾力測定値
  uint8_t saNum = 0;
  float loadMax;
  float servoAngle;
  float startAngle = startAngleGet();
  float endAngle = endAngleGet();
  uint8_t i;

  measCnt++;    //測定番号
  Serial.println();
  Serial.printf("***** measure Nukidan  #%3d  ********* \n", measCnt);
  Serial.printf("measure angle: %4.1f ~ %4.1f deg\n", startAngle, endAngle);
  dispMeasNum(measCnt);
  dispBtnA(MEAS_RUNNING);
  
  //抜き弾値リセット
  loadMax = 0;
  dispLoadMax(-9999); //表示をリセット

  //スケールのゼロセット
  Serial.print("Zero set --> ");
  servoMove(endAngle);
  delay(200);
  scaleTare();  //スケールゼロ
  Serial.println(" ---> scale zero set.");
  dispLoad(measLoad(10));
  delay(200);

  //スタート位置
  servoAngle = startAngle;
  Serial.print("Start pos --> ");
  dispPosition(servoMove(servoAngle));
  dispLoad(measLoad(10));
  Serial.println(" ---> start position wait.");
  //
  delay(800);   //玉を落ち着かせる

  //測定loop
  while(servoAngle <= endAngle){
    Serial.printf("Measure angle:%5.1fdeg --> ", servoAngle);
    tamaPos[saNum] = servoMove(servoAngle);
    dispPosition(tamaPos[saNum]);
    delay(10);            //サーボが動き終わるまで待つ

    load[saNum] = measLoad(10);
    dispLoad(load[saNum]);       //抵抗力測定
    //
    graphPlot(tamaPos[saNum], load[saNum]);
    //
    Serial.printf(" == LOAD: %6.1fgf ", load[saNum]);
    //簡易グラフ表示
#define A_SCALE 20
    int8_t n = load[saNum] / A_SCALE;   //マイナスの時は表示しない
    if (n <= 0){
      n = 0;
    }

    for(i = 0; i < n; i++){
      Serial.print("*");
    }
    Serial.println();

    //ピーク値のチェック
    if (load[saNum] > loadMax){
      loadMax = load[saNum];
      dispLoadMax(loadMax);
    }

    servoAngle += stepMoving;
    saNum++;
  }
  dispBtnA(MEAS_COMPLETE);
  
  //SDsave
  sdDataSave(measCnt, saNum, tamaPos, load);
  //
  M5.Speaker.tone(1500,100);
  delay(2000);
  dispBtnA(MEAS_START);

}

typedef enum {
  NZFL_IDLE,
  NZFL_BB_SET,
  NZFL_NOZZLE_DETECT,
  NZFL_PACKING_DETECT,
  NZFL_ANGLESET,
} nozzle_stat_t;



void measNozzlePos(void){
  //ノズル位置とパッキンとの隙間距離の測定
  static nozzle_stat_t nozzleStat = NZFL_IDLE;
  float servoAngle;
  
  //グラフエリアをクリア
  void graphAreaClear(void);

  //ノズル設定有効の時
  if (NZFL_ANGLESET == nozzleStat){
    //ノズル位置設定をリセットする
    Serial.print("nozzle position RESET!");
    //servoInit(-26);

    dispBtnC(BTNC_NOZZLE_SET);
    tamaPos = CENTER1_POS;
    servoPosition();
    return;
  }
  Serial.print("nozzle position setting");
  //抜き弾値リセット
  dispLoadMax(-9999); //表示をリセット

  //スケールのゼロセット
  Serial.print("Zero set --> ");
  servoMove(endAngleGet());
  delay(200);
  scaleTare();  //スケールゼロ
  Serial.println(" ---> scale zero set.");
  dispLoad(measLoad(10));
  delay(200);

  //スタート位置
  servoAngle = startAngleGet();
  Serial.print("Start pos --> ");
  dispPosition(servoMove(servoAngle));
  dispLoad(measLoad(10));
  Serial.println(" ---> start position wait.");
  //
  delay(800);   //玉を落ち着かせる







}