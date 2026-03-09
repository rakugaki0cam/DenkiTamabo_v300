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
    2026.01.14  ver.3.07  ファイルネームの時刻取得、SSIDの保存場所等変更

    ***** ロードセルを2つ　hx711を2つにして　ホップ押し垂直抗力を測定
    2026.03.08  ver.4.00  f_hop ブランチ　抜き弾抵抗力とホップ押し垂直抗力の2つの荷重を測定する    
    
    


  タッチパネル初期化に失敗していることがある。入力しない。


  STACK M-BUS　(ESP32無印)
  CORE2では以前のものとピン配置が違っているところがあるので注意
  CORE2 (CORE & PROTO)   
  *LEFT
  GND   (GND)   - GND   アース
  GND   (GND)
  GND   (GND)
  G23   (G23)   - SPI MOSI/ LCD ILI9342C, SD
  G38   (G19)*  - SPI MISO/ LCD ILI9342C, SD
  G18   (G18)   - SPI SCK/ LCD ILI9342C, SD
  G3    (G3)    - SERIAL_TXD
  G13   (G16)*  - GPIO_IN/ HX711_DOUT　抜き弾ロードセル1IC通信データ（独自プロトコル）
  G21   (G21)   - I2C SDA/ CAP_TOUCH, MPU6886, BM8563
  G32   (G2)*   - PORT A/ SDA
  G27   (G12)*
  G2    (G15)*  - SP_NS4168_DATA
  NC    (HPWR)
  NC    (HPWR)
  NC    (HPWR)

  *RIGHT
  G35   (G35)
  G36   (G36)   - ANALOG_IN(CH0)/ BAT_V_1/2   バッテリー電圧測定
  EN    (EN)    - RESET
  G25   (G25)   - GPIO_IN/ HX711_SCK_2　垂直抗力ロードセル2IC通信クロック
  G26   (G26)   - GPIO_IN/ HX711_DOUT_2　垂直抗力ロードセル2IC通信データ（独自プロトコル）
  3V3   (3V3)   - 3.3V POWER  ロードセル電源
  G1    (G1)    - SERIAL_RXD
  G14   (G17)*  - GPIO_IN/ HX711_SCK  抜き弾ロードセル1IC通信クロック
  G22   (G22)   - I2C SCL/ CAP_TOUCH, MPU6886, BM8563
  G33   (G5)*   - PORT A/ SCL
  G19   (G13)*  - GPIO_OUT/ BAT_ON　　ロードスイッチでサーボ電源のオンオフ
  G0    (G0)    - SP_NS4168_LRCK/ MIC_CLK
  G34   (G34)   - MIC_DATA
  5V    (5V)
  BAT   (BAT)   - BAT_POWER   バッテリー電源

 *PIN 無し
  G4            - SD_CS
  G5            - GPIO_OUT/ LCD_CS
  G12           - SP_NS4168_BCLK
  G15           - GPIO_OUT/ LCD_DC
  G16
  G17
  G39           - GPIO_IN/ CAP_TOUCH_INT
*/

#include "header.hpp"

//
#define N_MEAS_10 10

//pin
#define PIN_BAT_V   36    //10k+10k分圧　バッテリー電圧の1/2
#define ANALOG_CH   0     //GPIO36...ADC1_0
#define PIN_BAT_ON  19    //サーボ用バッテリ電源オンオフ　(GPIO13..stack M5 BASIC v2.7 & PROTO MODULE SYLK#)


//global
uint8_t   fmVer[] = "4.01";
uint16_t  measCnt;      //測定回数 
uint8_t   sdStat = 0;   //SDcard detect 0:未,1:OK,2:fail

//local
uint8_t cnt = 0;            //ループのカウンタ処理カウント用
uint8_t wifiConnected = 0;  //wifi未接続フラグ

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
  sdStat = sdInit();
  wifiConnected = wifiInit();
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

  //タッチボタン入力
  if (M5.BtnA.pressedFor(50))
  { //50ms長押し
    vibration(100);
    //測定へ
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
  { //200msec長押し
    vibration(100);
    //ノズル検出へ
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
    dispLoad(measLoad(1, N_MEAS_10));
  }
  else
  {
    dispLoad2(measLoad(2, N_MEAS_10));
  }
  
  switch (cnt)
  {
    case 10:
      //batteryVolt
      dispBatV((float)analogReadMilliVolts(PIN_BAT_V) * 2.0f / 1000);   ///電源ICから読めるみたい？？ーーーーーーーーーーーーーーーー未確認
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
        //抜いたのは検出できてない
        //ESP_LOGD(TAG, "SD flag %d", sdFlag);
        sdStat = sdInit();  //挿入検出したら、新しいファイルネームに
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