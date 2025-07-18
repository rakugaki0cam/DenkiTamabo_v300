/*
  電気タマボー　
    Platform IO + VScode
    stack M5 CORE2 v1.1 #2

    2024.04.19  uiFlow + stack M5 CORE Basic V2.7から移植
    2024.04.29  ver.3.02  BTserial仮完成
    2025.07.14  ライブラリを更新（手動でPIOホームから行う）
    2025.07.16  ESP32board ver.3につき変更
                LEDC変更（チャンネル数は不要になった）
                BTserialはIRAMエラーになるため廃止
                WiFi NTP 手順変更
    2025.07.17  サーボをゆっくり動かす
    2025.07.18  抜き弾抵抗力の積分値を表示            


  ///　タッチパネル初期化に失敗していることがある。入力しない。

    
*/

#include "header.hpp"


#define PIN_BAT_V   36    //10k+10k分圧　
#define ANALOG_CH   0     //G36...ADC1_0
#define PIN_BAT_ON  19    //サーボ用バッテリ電源オフ　(G13..stack M5 BASIC v2.7)


//global
uint8_t   fmVer[] = "3.05";
uint16_t  measCnt;      //測定回数 
uint8_t   sdStat = 0;   //SDcard detect 0:未,1:OK,2:fail

//local
uint8_t cnt = 0;        //ループのカウンタ処理カウント用

//debug
const char *TAG = "tamaV3";


void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  //
  pinMode(PIN_BAT_V, ANALOG);   //3.3V-10bit
  pinMode(PIN_BAT_ON, OUTPUT);  //サーボへのバッテリ電圧供給
  digitalWrite(PIN_BAT_ON, LOW);    //サーボ用バッテリ電源オフ
  analogReadResolution(12);   //アナログ12bit
  //
  Serial.begin(115200);

  M5.Speaker.setVolume(30);
  M5.Speaker.tone(1500,300);
  ESP_LOGI(TAG, "*** DENKI Tamabo M5 ver.%s ******************************************", (char*)fmVer);

  dispInit();
  wifiInit();
  sdStat = sdInit();  //dispInitの後に
  //bluetoothSerialInit();
  scaleInit();
  servoInit(0);   //default set
  digitalWrite(PIN_BAT_ON, HIGH);    //サーボ用バッテリ電源オン

  ///////////// test ////////////////////////////////////////////////////////////////
  //servoAdjust();
  //graphInit();
  ESP_LOGI(TAG, "***********************************************************************");

}


void loop()
{
  M5.update();  //ボタンの状態を更新する。

  //button
  if (M5.BtnA.pressedFor(100))
  {  //ms長押し
    //測定
    M5.Speaker.tone(1000,200);
    graphInit();
    measNukiF();
    //玉位置情報更新
    tamaPos = END_POS;
    dispBtnB(TO_CENTER);
  }
  
  if (M5.BtnB.wasPressed())
  {
    //玉位置移動
    M5.Speaker.tone(1320,100);
    servoPosition();          //玉ポジション移動
  }

  if (M5.BtnC.wasPressed())
  {
    //ノズル検出
    M5.Speaker.tone(1760,100);
    measNozzlePos();
    delay(200);
  }


  //routine
  delay(20);
  cnt++;
  cnt = (cnt > 50) ? 0: cnt;  //cycle 1sec

  if ((cnt % 2) == 1)
  {
    //load
    dispLoad(measLoad(10));
  }
  
  switch (cnt)
  {
    case 10:
      //batteryVolt
      dispBatV((float)analogReadMilliVolts(PIN_BAT_V) * 2.0f / 1000);
      break;
    case 20:
      //bat %
      break;
    case 30:
      //battery charge
      break;
    case 40:
      if (sdStat == 2)
      { //SD card
        //抜いたのは検出しない
        //ESP_LOGD(TAG, "SD flag %d", sdFlag);
        sdStat = sdInit();
      }
      break;
  }
  //btSerialRx();
  
}

