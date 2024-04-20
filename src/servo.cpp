/*
 * servo.cpp
 *
 * 
 * 2024.04.20
*/

#include "servo.hpp"

#define PIN_SERVO 33    //5...stack M5 BASIC v2.7

//サーボの実測パルス幅usec
//#define SERVO_ANGLE_M90 560   
//#define SERVO_ANGLE_0   1420  //0度     typ.1450
//#define SERVO_ANGLE_P90 2320  //+90度   typ.2400
//タマボーへの組み付け角度
//#define NEUTRAL_ANGLE 60      //この装置での中点 サーボホーンが真上を向く角度...セレーションの影響あり
//可動制限
//#define TAMABO_START_ANGLE 26 //真上からの前進量
//#define TAMABO_END_ANGLE 28   //真上からの後退量
#define STEP_MOVING -2        //測定ステップ
//アーム長さ
#define ARM_LENGTH 9.0f       //サーボホーンアーム長さ


Servo servo1;

//global
tama_pos_t tamaPos;  //玉のポジション

//local
//servo adjust　-90,0,+90deg --> [usec]
uint16_t  pwM90 = 560;    //-90度   typ.500
uint16_t  pwN0  = 1420;   //0度     typ.1450
uint16_t  pwP90 = 2320;   //+90度   typ.2400
//サーボ角度とパルス幅　[deg]
uint8_t   centerAngle = 60;   //この装置での中点 サーボホーンが真上を向く角度...セレーションの影響あり
uint8_t   startMoving = 26;   //真上からの前進量
uint8_t   endMoving   = 28;   //真上からの後退量
uint8_t   startAngle  = centerAngle + startMoving;    //最大角度 60+26 = 86
uint8_t   endAngle    = centerAngle - endMoving;      //最小角度 60-28 = 32 (測定時は角度をマイナスさせる方向 86 -> 32)
float     pwPerDeg    = (pwP90 - pwM90) / 180;        //1度あたりのパルス幅usec
                                                      
//各玉ポジションでのパルス幅 [usec]
uint16_t  startPwidth  = pwPerDeg * startAngle  + pwN0;
uint16_t  centerPwidth = pwPerDeg * centerAngle + pwN0;                                                 
uint16_t  endPwidth    = pwPerDeg * endAngle    + pwN0;

float servoAngle;


void servoInit(void){
  // Allow allocation of all timers
  //ESP32PWM::allocateTimer(0);   //0〜3
  servo1.setPeriodHertz(50);    // standard 50 hz servo
  servo1.attach(PIN_SERVO, endPwidth, startPwidth); //パルス幅制限
  
  //中点
  servo1.write(centerPwidth);
}

void servoMove(float angle){
  //角度入力　-90~+90度
    ////////////////////
}


void servoPosition(void){
  //BtnBが押された時に玉の位置を動かす
  switch(tamaPos){
    case CENTER1_POS:
      tamaPos = START_POS;        //スタートポジションへ移動
      servo1.write(startPwidth);
      dispTamaPos(tamaPos);       //ポジション表示
      btnBname(TO_CENTER);        //ボタンへは次の行先を表示
      break;
    case START_POS:
      tamaPos = CENTER2_POS;
      servo1.write(centerPwidth);
      dispTamaPos(tamaPos);
      btnBname(TO_END);
      break;
    case CENTER2_POS:
      tamaPos = END_POS;
      servo1.write(endPwidth);
      dispTamaPos(tamaPos);
      btnBname(TO_CENTER);
      break; 
    case END_POS:
      tamaPos = CENTER1_POS;
      servo1.write(centerPwidth);
      dispTamaPos(tamaPos);
      btnBname(TO_START);
      break;
    default:
      tamaPos = CENTER1_POS;
      btnBname(TO_START);
      break;
  } 


}

void btnBname(btn_b_name_t name){
  //ボタンBの名前の表示
  M5.Display.setCursor(125, 220);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  switch(name){
    case TO_START:
      M5.Display.print("押し込み　");
      break;
    case TO_CENTER:
      M5.Display.print("センタへ　");
      break;
    case TO_END:
      M5.Display.print("抜け出し　");
      break;
  }

}

void dispTamaPos(tama_pos_t pos){
  //玉位置の表示
  M5.Display.setCursor(240, 40);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  switch(pos){
    case START_POS:
      M5.Display.print("スタート");
      break;
    case CENTER1_POS:
    case CENTER2_POS:
      M5.Display.print("センタ　");
      break;
    case END_POS:
      M5.Display.print("エンド　");
      break;
  }

}


//------- TEST ------------------------------------------------------------------------------- 

void servoAdjust(void){
  uint16_t pw;

  ESP32PWM::allocateTimer(0);   //0〜3
  servo1.setPeriodHertz(50);    // standard 50 hz servo
  servo1.attach(PIN_SERVO, 500, 2400); ///////////////////可動域制限///////////
  Serial.println("**** servo adjust **********");

  //
  M5.Display.init();
  M5.Display.setRotation(1);
  M5.Display.setColor(TFT_BROWN);
  M5.Display.fillRect(0, 0, 320, 20);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BROWN);
  M5.Display.setCursor(4, 0);
  M5.Display.printf("サーボの設定　サーボホーンを外す");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(0, 220);
  M5.Display.printf("　　　ー　　　　ＮＥＸＴ　　　　＋");
  //
  while(1){
    M5.update();
    if (M5.BtnB.isPressed()){
      break;
    }
    delay(50);
  }
  M5.Speaker.tone(2000, 50);
  delay(200);

  //
  pw = 2320;
  while(true){
    M5.update();
    servo1.writeMicroseconds(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 30);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("＋９０度（上）パルス幅：%5dusec　", pw);
    if (M5.BtnA.isPressed()){
      pw -= 10;
      if (pw < 500){pw = 500;}
    }
    if (M5.BtnC.isPressed()){
      pw += 10;
      if (pw > 2400){pw = 2400;}
    }
    if (M5.BtnB.isPressed()){
      break;
    }
    delay(100);
  }
  M5.Speaker.tone(2000, 50);
  pwP90 = pw;
  delay(500);

  //時計方向に回る
  pw = 560;
  while(true){
    M5.update();
    servo1.writeMicroseconds(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 50);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("ー９０度（下）パルス幅：%5dusec　", pw);
    if (M5.BtnA.isPressed()){
      pw -= 10;
      if (pw < 500){pw = 500;}
    }
    if (M5.BtnC.isPressed()){
      pw += 10;
      if (pw > 2400){pw = 2400;}
    }
    if (M5.BtnB.isPressed()){
      break;
    }
    delay(100);
  }
  M5.Speaker.tone(2000, 50);
  pwM90 = pw;
  delay(500);
  
  //
  pw = 1420;
  while(true){
    M5.update();
    servo1.writeMicroseconds(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 70);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("　　０度　　　パルス幅：%5dusec　", pw);
    
    if (M5.BtnA.isPressed()){
      pw -= 10;
      if (pw < 500){pw = 500;}
    }
    if (M5.BtnC.isPressed()){
      pw += 10;
      if (pw > 2400){pw = 2400;}
    }
    if (M5.BtnB.isPressed()){
      break;
    }
    delay(100);
  }
  M5.Speaker.tone(2000, 50);
  pwN0 = pw;
  delay(500);
  
  //サーボホーンをセット
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(0, 90);
  M5.Display.println("サーボホーンを真上向きにセットして角度の調整");

  while(true){
    M5.update();
    pw = (pwP90 - pwM90) / 180 * centerAngle + pwN0;
    servo1.writeMicroseconds(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 110);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("　%2d度　　　パルス幅：%5dusec　", centerAngle, pw);
  
    if (M5.BtnA.isPressed()){
      centerAngle -= 1;
      if (centerAngle < 30){centerAngle = 30;}
    }
    if (M5.BtnC.isPressed()){
      centerAngle += 1;
      if (centerAngle > 70){centerAngle = 70;}
    }
    if (M5.BtnB.isPressed()){
      break;
    }
    delay(100);
  }

  M5.Speaker.tone(1500, 300);
  delay(2000);
}