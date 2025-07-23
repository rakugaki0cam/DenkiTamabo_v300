/*
 * measure.cpp
 *
 * 2024.04.21
*/

#include "measure.hpp"


float stepMoving = 2.0f;      //測定時の角度増分

void measNukiF(void)
{
  //抜き弾抵抗力の測定
#define SAMPLE_NUM 50
  static float tamaPos[SAMPLE_NUM];    //測定位置
  static float load[SAMPLE_NUM];  //抜き弾力測定値
  uint8_t saNum = 0;
  float loadMax;
  float servoAngle;
  float startAngle = startAngleGet();
  float endAngle = endAngleGet();
  float nukiInteg = 0;  //抜き弾抵抗値の積分値
  uint8_t i;
  uint8_t measTime[] ="00/00/00 00:00:00     ";

  measCnt++;    //測定番号
  getTimeText((char*)measTime);
  ESP_LOGI(TAG, "***** measure Nukidan  #%3d  (%s) ********* ", measCnt, measTime);
  ESP_LOGI(TAG, "measure angle: %4.1f ~ %4.1f deg", startAngle, endAngle);
  dispMeasNum(measCnt);
  dispBtnA(MEAS_RUNNING);
  
  //抜き弾値リセット
  loadMax = 0;
  dispLoadMax(-9999); //最大抵抗力をリセット

  //スケールのゼロセット
  ESP_LOGI(TAG, "Scale zero set.");
  servoMove(endAngle, SPEED_FAST);
  delay(500);
  scaleTare();  //スケールゼロ
  dispLoad(measLoad(10));
  delay(200);

  //スタート位置
  servoAngle = startAngle;
  ESP_LOGI(TAG, "Move start position & wait.");
  dispPosition(servoMove(servoAngle, SPEED_SLOW));
  dispLoad(measLoad(10));
  delay(1200);   //玉を落ち着かせる

  //測定loop
  while(servoAngle <= endAngle)
  {
    Serial.printf("angle:%5.1fdeg", servoAngle);
    tamaPos[saNum] = servoMove(servoAngle, SPEED_MEAS);
    dispPosition(tamaPos[saNum]);

    load[saNum] = measLoad(5);//(10);
    dispLoad(load[saNum]);       //抵抗力測定
    //
    graphPlot(tamaPos[saNum], load[saNum]);
    //
    Serial.printf(" LOAD: %6.1fgf ", load[saNum]);
    //簡易グラフ表示
    #define A_SCALE 20

    int8_t n = load[saNum] / A_SCALE;
    n = (n <= 0) ? 0 : n;   //マイナスの時は表示しない

    for(i = 0; i < n; i++)
    {
      Serial.print("*");
    }
    Serial.println();

    //ピーク値のチェック
    if (load[saNum] > loadMax)
    {
      loadMax = load[saNum];
      dispLoadMax(loadMax);
    }

    //積分値の計算
    if ((saNum >= 1) && (load[saNum] > 0))
    {
      nukiInteg += (tamaPos[saNum] - tamaPos[saNum - 1]) * (load[saNum] + load[saNum - 1]) / 2;
    }
    dispNukiInteg(nukiInteg);

    servoAngle += stepMoving;
    saNum++;
    delay(50);
  }
  ESP_LOGI(TAG, "Nuki load integral: %7.1fgf-mm", nukiInteg);

  dispBtnA(MEAS_COMPLETE);
  
  //SDsave BTserialsend
  sdDataSave((char*)measTime, measCnt, saNum, tamaPos, load, nukiInteg);
  //btDataSend((char*)measTime, measCnt, saNum, tamaPos, load, nukiInteg);
  //
  M5.Speaker.tone(1500,100);
  delay(500);
  dispBtnA(MEAS_START);

}


//------ nozzle set ----------------------------------------------------------------------------

typedef enum 
{
  NZFL_IDLE,
  NZFL_BB_SET,
  NZFL_NOZZLE_DETECT,
  NZFL_PACKING_DETECT,
  NZFL_ANGLESET,
} nozzle_stat_t;


void measNozzlePos(void)
{
  //ノズル位置とパッキンとの隙間距離の測定
  static nozzle_stat_t nozzleStat = NZFL_IDLE;
  float   servoAngle;
  int16_t toCnt;
  int8_t  blinkCnt;

  //グラフエリアをクリア
  graphAreaClear();
  //ESP_LOGD(TAG, "nozzle status:%d", nozzleStat);

  //ノズル設定が有効になっている時ーーーーーーーーーーーーーーーー
  if (NZFL_ANGLESET == nozzleStat)
  {
    //ノズル位置設定をリセットする
    M5.Speaker.tone(2000, 800);
    servoInit(0);   //default set
    ESP_LOGI(TAG, "Nozzle position RESET!");
    dispNozzle(NOZ_RESET, 0);
    dispBtnC(BTNC_NOZZLE_SET);

    tamaPos = CENTER2_POS;
    servoPosition();
    nozzleStat = NZFL_IDLE;   
    dispNozzle(NOZ_DIS, 0);
    delay(200);
    
    return;
  }

  //ノズル位置の測定ーーーーーーーーーーーーーーーーーーーーーーー
  dispNozzle(NOZ_TITLE, 0);
  ESP_LOGI(TAG, "Nozzle position setting。");
  dispLoadMax(-9999); //最大値をリセット
  //スケールのゼロセット
  ESP_LOGI(TAG, "Scale zero set。");
  servoMove(endAngleGet(), SPEED_MID);
  delay(200);
  scaleTare();  //スケールゼロ
  dispLoad(measLoad(10));
  delay(200);

  //棒の長さを調整する
  servoAngle = startAngleGet() + 3;   //最前位置より3度戻し
  ESP_LOGI(TAG, "Start pos set --> ");
  dispPosition(servoMove(servoAngle, SPEED_MID));
  dispLoad(measLoad(10));
  dispNozzle(NOZ_EXP1, 0);
  delay(800);   //玉を落ち着かせる
  //調整後ボタンCを押す
  ESP_LOGI(TAG, "wait for button C");
  toCnt = 600;     //timeout カウンタ 5分
  blinkCnt = 0;

  while(true)
  {
    M5.update();  //ボタンの状態を更新する。
    //ボタンを点滅させる
    if (blinkCnt < 6)
    {
      dispBtnC(BTNC_PUSH_TO_START);   
    }
    else
    {
      dispBtnC(BTNC_NULL);
    }
    dispLoad(measLoad(10));
    blinkCnt++;
    if(blinkCnt >= 10)
    {
      blinkCnt = 0;   //点滅カウントリセット
    }
    toCnt--;
    if (toCnt < 0)
    {
      //タイムアウト
      M5.Speaker.tone(1000, 500);
      ESP_LOGI(TAG, "Timeout! Nozzle position setting canceled.");
      dispNozzle(NOZ_DIS, 0);
      dispBtnC(BTNC_NOZZLE_SET);
      nozzleStat = NZFL_IDLE;
      dispNozzle(NOZ_ABORT, 0);
      delay(1000);
      tamaPos = END_POS;
      servoPosition();
      return;
    }
    delay(50);

    if (M5.BtnC.isPressed())
    {
      M5.Speaker.tone(1760,100);
      vibration(100);
      break;
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
  const float nozStep = -0.2;   //deg 測定ステップ角度
  uint8_t stat;
  graphAreaClear();

  dispBtnC(BTNC_RUNNING);
  ESP_LOGI(TAG, "measure Packing Free & Nozzle position.");
  dispNozzle(NOZ_MEAS, 0);
  servoAngle += 10;           //10度戻す
  servoMove(servoAngle, SPEED_SLOW);//////////////////////////////////////////////////
  ESP_LOGI(TAG, "measure start position set.");
  delay(1500);
  //
  ESP_LOGI(TAG, "--> TAMA goes to right.");
  stat = 0;
  dispNozzle(NOZ_PACKING, 0);

  while(servoAngle > -26)
  {
    //玉を奥へ進める
    pos = servoMove(servoAngle, SPEED_SLOW);
    dispPosition(pos);
    delay(100);    /////////////

    nLoad = measLoad(10);
    if (abs(nLoad) < minLoad)
    {
      minLoad = abs(nLoad);
    }
    dispLoad(nLoad);
    ESP_LOGI(TAG, "Angle:%5.1fdeg LOAD:%6.1fgf", servoAngle, nLoad);

    if ((stat == 0) && (nLoad > -3.0) && (nLoad < 3.0))
    {
      //パッキンの抵抗から抜けた位置
      paFreePos = pos;
      paFreeAngle = servoAngle;
      M5.Speaker.tone(1500, 100);
      ESP_LOGI(TAG, "***** PACKING FREE  %6.2fdeg - %6.3fmm **********", paFreeAngle, paFreePos);
      dispNozzle(NOZ_PACKING_OK, paFreePos);
      stat = 1;
      delay(1000);
      dispNozzle(NOZ_NOZZLE, 0);
    }
    if ((stat == 1) && (nLoad < -20.0))
    {
      //ノズル先端位置を検出
      nozPos = pos;
      nozAngle = servoAngle;
      M5.Speaker.tone(1500, 100);
      ESP_LOGI(TAG, "***** NOZZLE FIND  %6.2fdeg - %6.3fmm ***********", nozAngle, nozPos);
      dispNozzle(NOZ_DETECT, nozPos);
      nozAngle -= nozStep;
      stat = 9;
      float tumaDuki = paFreePos - nozPos;
      dispNozzle(NOZ_TUMADUKI, tumaDuki);
      break;
    }
    //
    servoAngle += nozStep;
  }

  ESP_LOGI(TAG, "min Load (absolute):%6.1f", minLoad);

  if (stat == 9)
  {
    //正常測定完了
    ESP_LOGI(TAG, "OK");
    dispNozzle(NOZ_OK, 0);
    servoInit(nozAngle);  //start angle set
    ESP_LOGI(TAG, " Start angle set OK");
    dispNozzle(NOZ_EN, 0);
    dispBtnC(BTNC_NOZZLE_RESET);
    nozzleStat = NZFL_ANGLESET;
    //ESP_LOGI(TAG, "nozzle status:%d", nozzleStat);
  }
  else
  {
    //ノズル検出不可
    ESP_LOGI(TAG, "Nozzle detect error!");
    dispNozzle(NOZ_ERR, 0);
    servoInit(0);   //default set
    dispNozzle(NOZ_DIS, 0);
    dispBtnC(BTNC_NOZZLE_SET);
    nozzleStat = NZFL_IDLE; 
  }

  delay(1000);
  tamaPos = END_POS;
  servoPosition();
}

