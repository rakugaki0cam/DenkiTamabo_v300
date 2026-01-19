/*
  電気タマボー　
    Platform IO + VScode
    stack M5 CORE2 v1.1 #2
    stack M5 CORE2 v1.0 #1も追加　ー　タマボー2号機


    2024.04.19  uiFlow + stack M5 CORE Basic V2.7から移植
    2024.04.29  ver.3.02  BTserial仮完成
    2025.07.14  ライブラリを更新（手動でPIOホームから行う）
    2025.07.16  ESP32board ver.3につき変更
                LEDC変更（チャンネル数は不要になった）
                BTserialはIRAMエラーになるため廃止
                WiFi NTP 手順変更
    2025.07.17  サーボをゆっくり動かす
    2025.07.18  抜き弾抵抗力の積分値を表示  
    2026.01.14  ファイルネームの時刻取得、SSIDの保存場所等変更          


  タッチパネル初期化に失敗していることがある。入力しない。


  STACK M-BUS
  CORE2では以前のものとピン配置が違っているところがあるので注意
  CORE2 (CORE & PROTO)   
  *LEFT
  GND   (GND)   - GND   アース
  GND   (GND)
  GND   (GND)
  G23   (G23)
  G38   (G19)*
  G18   (G18)
  G3    (G3)
  G13   (G16)*  - GPIO_IN/ HX711_DOUT　ロードセルIC通信データ（独自プロトコル）
  G21   (G21)
  G32   (G2)*
  G27   (G12)*
  G2    (G15)*
  NC    (HPWR)
  NC    (HPWR)
  NC    (HPWR)

  *RIGHT
  G35   (G35)
  G36   (G36)   - ANALOG_IN(CH0)/ BAT_V_1/2   バッテリー電圧測定
  EN    (EN)
  G25   (G25)
  G26   (G26)
  3V3   (3V3)   - 3.3V POWER  ロードセル電源
  G1    (G1)
  G14   (G17)*  - GPIO_IN/ HX711_SCK  ロードセルIC通信クロック
  G22   (G22)
  G33   (G5)*
  G19   (G13)*  - GPIO_OUT/ BAT_ON　　ロードスイッチでサーボ電源のオンオフ
  G0    (G0)  
  G34   (G34)
  5V    (5V)
  BAT   (BAT)   - BAT_POWER   バッテリー電源


*/

#include "header.hpp"

#define PIN_BAT_V   36    //10k+10k分圧　バッテリー電圧の1/2
#define ANALOG_CH   0     //GPIO36...ADC1_0
#define PIN_BAT_ON  19    //サーボ用バッテリ電源オンオフ　(GPIO13..stack M5 BASIC v2.7 & PROTO MODULE SYLK#)


//global
uint8_t   fmVer[] = "3.06";
uint16_t  measCnt;      //測定回数 
uint8_t   sdStat = 0;   //SDcard detect 0:未,1:OK,2:fail

//local
uint8_t cnt = 0;        //ループのカウンタ処理カウント用
uint8_t wifiConnected = 0;   //wifi未接続フラグ

//debug
static const char *TAG = "メイン";


void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  //
  pinMode(PIN_BAT_V, ANALOG);     //バッテリー電圧の1／2  （3.3V=10bit）
  pinMode(PIN_BAT_ON, OUTPUT);    //サーボへのバッテリ電圧供給ロードスイッチ
  digitalWrite(PIN_BAT_ON, LOW);  //サーボ用バッテリ電源をオフ
  analogReadResolution(12);       //アナログ12bit
  //
  Serial.begin(115200);

  M5.Speaker.setVolume(30);
  M5.Speaker.tone(1500,300);
  ESP_LOGI(TAG, "*** DENKI Tamabo M5 ver.%s ******************************************", (char*)fmVer);

  dispInit();
  sdStat = sdInit();  //dispInitの後に
  wifiConnected = wifiInit();
  filenameInit();
  //bluetoothSerialInit();
  scaleInit();
  digitalWrite(PIN_BAT_ON, HIGH);    //サーボ用バッテリ電源オン
  servoInit(0);   //default set

  // test ----------
  //servoAdjust();
  //graphInit();
  ESP_LOGI(TAG, "***********************************************************************");

}


void loop()
{
  M5.update();  //ボタンの状態を更新する。

  m5::touch_detail_t pos = M5.Touch.getDetail();
  auto x = pos.distanceX();
  auto y = pos.distanceY();

  //button
  if (M5.BtnA.pressedFor(50))
  {  //ms長押し
    vibration(100);
    //測定
    M5.Speaker.tone(1000, 200);
    graphInit();
    measNukiF();
    //玉位置情報更新
    tamaPos = END_POS;
    dispBtnB(TO_CENTER);
  }
  
  if (M5.BtnB.wasPressed())
  {
    vibration(100);
    //玉位置移動
    M5.Speaker.tone(1320, 100);
    servoPosition();          //玉ポジション移動
  }

  if (M5.BtnC.pressedFor(200))
  {
    vibration(100);
    //ノズル検出
    M5.Speaker.tone(1760, 100);
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
      dispBatV((float)analogReadMilliVolts(PIN_BAT_V) * 2.0f / 1000);///電源ICから読めるはずーーーーーーーーーーーーーーーー
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


//
void vibration(uint16_t timeMs)
{ //バイブレーションモーター
  //電源IC AXP2101につながっている
  M5.Power.Axp2101.setDLDO1(3300);
  delay(timeMs);
  M5.Power.Axp2101.setDLDO1(0);  //DLDO1 off
}