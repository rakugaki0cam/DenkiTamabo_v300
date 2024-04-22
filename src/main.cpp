/*
  電気タマボー　
    Platform IO + VScode
    stack M5 CORE2 v1.1 #2

    2024.04.19 uiFlow + stack M5 CORE Basic V2.7から移植

*/

#include "header.hpp"

#define PIN_BAT_V   36    //10k+10k分圧　
#define ANALOG_CH   0     //G36...ADC1_0
#define PIN_BAT_ON  19    //サーボ用バッテリ電源オフ　(G13..stack M5 BASIC v2.7)


uint8_t fmVer[] = "3.00";
//global
uint16_t measCnt;   //測定回数  
//local
uint8_t sdFlag = 0;       //SDcard detect
uint8_t sdCnt = 0;
uint8_t cnt = 0;
uint16_t aveload;
float  batV;
float  load;


void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  //
  pinMode(PIN_BAT_V, ANALOG);   //3.3V-10bit
  pinMode(PIN_BAT_ON, OUTPUT);  //サーボへのバッテリ電圧供給
  digitalWrite(PIN_BAT_ON, LOW);    //サーボ用バッテリ電源オフ
  analogReadResolution(12);   //アナログ12bit
  Serial.begin(115200);

  dispTitle();

  M5.Speaker.setVolume(64);
  M5.Speaker.tone(1500,300);
  
  if (sdInit()){
    //SD fail
    M5.Speaker.tone(660,800);
  }else{
    //SD OK
    sdFlag = 1;
  }

  scaleInit();
  servoInit();
  digitalWrite(PIN_BAT_ON, HIGH);    //サーボ用バッテリ電源オン

  ///////////// test ////////////////////////////////////////////////////////////////
  //servoAdjust();
  graphClear();

  

}


void loop() {
  M5.update();  //ボタンの状態を更新する。

  //button
  if (M5.BtnA.pressedFor(200)){  //ms長押し
    //測定
    Serial.println("Btn A");
    M5.Speaker.tone(1000,200);
    graphClear();
    measNukiF();
    M5.Speaker.tone(1500,100);
    //玉位置情報更新
    tamaPos = END_POS;
    btnBname(TO_CENTER);
  }
  
  if (M5.BtnB.wasPressed()){
    //玉位置移動
    Serial.println("Btn B");
    M5.Speaker.tone(1320,100);
    servoPosition();          //玉ポジション移動
  }
  if (M5.BtnC.wasPressed()){
    //ノズル検出
    Serial.println("Btn C");
    M5.Speaker.tone(1760,100);
    delay(200);
  }

  //routine
  delay(20);
  cnt++;
  if (cnt > 50){
    cnt = 0;
  }
  if ((cnt % 2) == 1){
    //load
    load = measLoad(10);
    dispLoad(load);
  }else{
    //
  }
  
  if (cnt == 10){
    //batteryVolt
    batV = (float)analogReadMilliVolts(PIN_BAT_V) * 2.0f / 1000;
      dispBatV(batV);
  }
  if (cnt == 20){
    //bat %
  }
  if (cnt == 30){
    //battery charge    
  }
  if ((cnt == 40) && (sdFlag == 0)){
    //SD card
    //抜いたのは検出しない
    sdCnt++;
    if (sdCnt > 4){
      //Serial.printf("SD flag %d\n", sdFlag);
      sdFlag = !sdInit();
      sdCnt = 0;
    }
  }
  
}


 uint8_t sdInit(void){
  //SDcard init
  //ret --> 1:err, 0:ok
  if(SD.begin(GPIO_NUM_4, SPI, 25000000)){
    Serial.println("SD OK!");
    M5.Speaker.tone(1500,80);
    M5.Display.setColor(TFT_BG_SCREEN);
    M5.Display.fillRect(22, 195, 64, 20);
    return 0;
  }
  Serial.println("SD failed!");
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_MAGENTA);
  M5.Display.setCursor(22, 195);
  M5.Display.println(" SD Fail! ");
  return 1;
 }

//--display-----------------------

//original color
uint16_t TFT_ENJI = M5.Display.color565(0x6f, 0x20, 0x20);    //エンジ色
uint16_t TFT_BG_SCREEN = M5.Display.color565(0x9e, 0x9d, 0x8c); //薄茶
uint16_t TFT_BG_TITLE = M5.Display.color565(0x89, 0x5d, 0x37);  //茶色


void dispTitle(void){
  //スクリーン画面

  //init
  M5.Display.init();
  M5.Display.setRotation(1);
  M5.Display.clearDisplay(TFT_BG_SCREEN);
  //title bar
  M5.Display.setColor(TFT_BG_TITLE);
  M5.Display.fillRect(0, 0, 320, 20);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_TITLE);
  M5.Display.setCursor(4, 0);
  M5.Display.printf("電気タマボーM5　ver.%s", (char*)fmVer);
  //load
  M5.Display.setCursor(228, 65);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  M5.Display.printf("抜弾抵抗力");
  //tama position
  M5.Display.setCursor(228, 25);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  M5.Display.printf("玉位置");
  //peak load
  M5.Display.setCursor(228, 112);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
  M5.Display.setTextColor(TFT_ENJI, TFT_BG_SCREEN);
  M5.Display.printf("抜弾ピーク値");
  dispLoadMax(-9999);
  //batV
  M5.Display.setFont(&fonts::lgfxJapanGothicP_8);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  M5.Display.setCursor(265, 190);
  M5.Display.print("バッテリ電圧");
  //button
  //M5.Display.setTextDatum(TC_DATUM);  //TopCenter
  //Btn A
  M5.Display.setCursor(0, 220);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.printf("　測定開始　");
  //Btn B
  M5.Display.setCursor((16*7), 220);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.printf("スタート位置");
  //Btn C
  M5.Display.setCursor((16*14), 220);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.printf("ノズル設定　");

  //M5.Display.setTextDatum(TL_DATUM);  //TopLeft

}

void dispBatV(float val){
  //バッテリー電圧[V]
  uint8_t text[20];

  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%4.2fV", val);
  M5.Display.drawString((char*)text, 309, 200, &fonts::lgfxJapanGothicP_12);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft

}


