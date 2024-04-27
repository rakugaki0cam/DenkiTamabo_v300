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
    delay(20);            //サーボが動き終わるまで待つ

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
    delay(50);
  }
  dispBtnA(MEAS_COMPLETE);
  
  //SDsave
  sdDataSave(measCnt, saNum, tamaPos, load);
  //
  M5.Speaker.tone(1500,100);
  delay(500);
  dispBtnA(MEAS_START);

}


//------ nozzle set ----------------------------------------------------------

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
  float   servoAngle;
  int16_t toCnt;
  int8_t  blinkCnt;

  //グラフエリアをクリア
  graphAreaClear();
  //Serial.printf("nozzle status:%d\n", nozzleStat);

  //ノズル設定が有効になっている時ーーーーーーーーーーーーーーーー
  if (NZFL_ANGLESET == nozzleStat){
    //ノズル位置設定をリセットする
    M5.Speaker.tone(2000, 800);

    //servoInit(-26);/////////////////////////////////////////
    Serial.println("nozzle position RESET!");
    dispNozzle(NOZ_RESET);
    dispBtnC(BTNC_NOZZLE_SET);
    tamaPos = CENTER1_POS;
    servoPosition();
    nozzleStat = NZFL_IDLE;   
    dispNozzle(NOZ_DIS);
    delay(500);
    return;
  }

  //ノズル位置の測定ーーーーーーーーーーーーーーーーーーーーーーー
  dispNozzle(NOZ_TITLE);
  Serial.println("nozzle position setting");
  //抜き弾値リセット
  dispLoadMax(-9999); //表示をリセット
  //スケールのゼロセット
  Serial.print("Scale zero set --> ");
  servoMove(endAngleGet());
  delay(200);
  scaleTare();  //スケールゼロ
  dispLoad(measLoad(10));
  Serial.println();
  delay(200);

  //棒の長さを調整する
  servoAngle = startAngleGet() + 2;   //最前位置より2度戻し
  Serial.print("Start pos set --> ");
  dispPosition(servoMove(servoAngle));
  dispLoad(measLoad(10));
  Serial.println();
  dispNozzle(NOZ_EXP1);
  delay(800);   //玉を落ち着かせる
  //調整後ボタンCを押す
  Serial.println("wait for button C");
  toCnt = 1200;     //timeout カウンタ 10分 -----> 測定に入ってしまう／／／／／／／／／／／／／／／／／／／／／／／／／／
  while(toCnt > 0){
    blinkCnt = 6;
    dispBtnC(BTNC_PUSH_TO_START);   //ボタンに文字を表示
    while(blinkCnt){
      M5.update();  //ボタンの状態を更新する。
      dispLoad(measLoad(10));
      if (M5.BtnC.wasPressed()){
        M5.Speaker.tone(1760,100);
        toCnt = -1;
        break;
      }    
      blinkCnt--;
      delay(50);
    }
    blinkCnt = 4;
    dispBtnC(BTNC_NULL);   //ボタンを空白にして点滅させる
    while(blinkCnt){
      M5.update();  //ボタンの状態を更新する。
      dispLoad(measLoad(10));
      blinkCnt--;
      if (M5.BtnC.wasPressed()){
        M5.Speaker.tone(1760,100);
        toCnt = 0;
        break;
      }
      toCnt--;
      delay(50);      
    }
  }
  //測定開始
  float nLoad;
  float minLoad = 999;
  float pos;
  float paFreePos;  //パッキンから抜けた位置
  float paFreeAngle;
  float nozPos;     //ノズル位置
  float nozAngle;
  uint8_t stat;
  graphAreaClear();

  dispBtnC(BTNC_RUNNING);
  Serial.println();
  Serial.println("measure Packing Free & Nozzle position.");
  dispNozzle(NOZ_MEAS);
  servoAngle += 10;           //10度戻す
  servoMove(servoAngle);
  Serial.println("measure start position set.");
  delay(1500);
  //
  Serial.println("--> TAMA goes to right.");
  stat = 0;
  while(servoAngle > -26){
    //玉を奥へ進める
    Serial.printf("Angle:%5.1fdeg --> ", servoAngle);
    pos = servoMove(servoAngle);
    dispPosition(pos);
    delay(50);

    nLoad = measLoad(10);
    if (abs(nLoad) < minLoad){
      minLoad = abs(nLoad);
    }
    dispLoad(nLoad);
    Serial.printf(" == LOAD:%6.1fgf \n", nLoad);

    if ((stat == 0) && (nLoad > -3.0) && (nLoad < 3.0)){
      //パッキンの抵抗から抜けた位置
      paFreePos = pos;
      paFreeAngle = servoAngle;
      M5.Speaker.tone(1500, 100);
      Serial.printf("***** PACKING FREE  %6.2fdeg - %6.3fmm **********\n", paFreeAngle, paFreePos);

      stat = 1;
      delay(1000);
    }
    if ((stat == 1) && (nLoad < -20.0)){
      nozPos = pos;
      nozAngle = servoAngle;
      M5.Speaker.tone(1500, 100);
      Serial.printf("***** NOZZLE FIND  %6.2fdeg - %6.3fmm ***********\n", nozAngle, nozPos);

      stat = 9;
      break;
    }
    servoAngle -= 0.1;
  }
  Serial.printf("min Load (absolute):%6.1f\n", minLoad);

  if (stat == 9){
    //正常測定完了
    //angle set

    //servoInit(-26);/////////////////////
    dispBtnC(BTNC_NOZZLE_RESET);
    dispNozzle(NOZ_EN);
    nozzleStat = NZFL_ANGLESET; 
    //Serial.printf("nozzle status:%d\n", nozzleStat);

  }else{
    //ノズル検出不可
  //dispNozzle(NOZ_PACKING);

  

  }

  delay(1000);

  tamaPos = END_POS;
  servoPosition();

}