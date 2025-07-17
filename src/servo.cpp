/*
 * servo.cpp
 *
 * 
 * 2024.04.20
*/

#include "servo.hpp"


#define PIN_SERVO 33      //G5@stack M5 BASIC v2.7
//LEDC
#define LEDC_CH     0
#define LEDC_BIT    15  //resolution
#define LEDC_FREQ   50  //Hz
//
#define ARM_LENGTH 9.0f   //サーボホーンアーム長さ
//
#define degToRad(deg) (deg / 180 * PI)
#define radToDeg(rad) (rad / PI * 180)

//global
tama_pos_t tamaPos;  //玉のポジション

//
Servo servo1;

//local
//servo adjust　-90,0,+90deg --> [usec]
uint16_t  pwM90 = 560;    //-90度   typ.500
uint16_t  pwN0  = 1420;   //0度     typ.1450
uint16_t  pwP90 = 2320;   //+90度   typ.2400
//サーボ角度とパルス幅　[deg]
const float centerAngle  = 0;   //この装置での基準角度
const float startAngleD  = -26; //真上からの前進量(マイナス値)
const float endAngle     = 28;  //真上からの後退量(プラス値)
float centerAngleS = 60;        //この装置での中点でのサーボの角度 サーボホーンが真上を向く角度...セレーションの影響あり
float startAngle;
float startAngleS;              //サーボのスタート角度　AngleS　S=Servo
float endAngleS;                //サーボのエンド角度
float pwPerDeg;                          
//各玉ポジションでのパルス幅 [usec]
uint16_t  startPwidth;
uint16_t  centerPwidth;                                                 
uint16_t  endPwidth;
//玉の位置[mm]
float startPosition;
float endPosition;


void servoInit(float setAngle)
{
  //サーボの初期化
  //setAngle 0:default setting, -20~-26:set Angle
  if ((setAngle < -20) && (setAngle > -26))
  {
    //スタート角度を変更（ノズル検出による変更）
    startAngle = setAngle;
    Serial.printf("CHANGE start angle:%6.1fdeg\n", setAngle);
  }
  else
  {
    startAngle = startAngleD;
  }
  startAngleS  = centerAngleS - startAngle;   //最大角度 60-(-26) = 86
  endAngleS    = centerAngleS - endAngle;     //最小角度 60-28 = 32 (測定時は角度をマイナスさせる方向 86 -> 32)
  pwPerDeg    = (pwP90 - pwM90) / 180;        //1度あたりのパルス幅usec                           
  //各玉ポジションでのパルス幅 [usec]
  startPwidth  = pwPerDeg * startAngleS  + pwN0;
  centerPwidth = pwPerDeg * centerAngleS + pwN0;                                                 
  endPwidth    = pwPerDeg * endAngleS    + pwN0;
  //玉の位置[mm]
  startPosition = ARM_LENGTH * sin(degToRad(startAngle));
  endPosition = ARM_LENGTH * sin(degToRad(endAngle));

  //PWM(LEDC) init
  //ver.2
  //ledcSetup(LEDC_CH, LEDC_FREQ, LEDC_BIT);
  //ledcAttachPin(PIN_SERVO, LEDC_CH);
  //ver.3
  ledcAttach(PIN_SERVO, LEDC_FREQ, LEDC_BIT);
  //
  servo1WriteMs(centerPwidth);
  //
  Serial.printf("start angle:%5.1fdeg (dx:%6.3fmm) ", startAngle, startPosition);
  Serial.printf("~ end angle:%5.1fdeg (dx:%6.3fmm) \n", endAngle, endPosition);
}


float endAngleGet(void)
{
  return endAngle;
}

float startAngleGet(void)
{
  return startAngle;
}


void servo1WriteMs(uint32_t usec)
{
  uint32_t dutyTick = usec * (32768.0 / 20000.0);
  //Serial.printf("pulse:%5d -- dutyTick:%6lu\n", usec, dutyTick);
  //ver.2
  //ledcWrite(LEDC_CH, dutyTick);
  //ver.3
  ledcWrite(PIN_SERVO, dutyTick);
}


float servoMove(float angle)
{
  //角度入力　startMoving ~ endMoving (-26 ~ 28)装置での角度（真上がゼロ）
  float angleS = centerAngleS - angle;     //サーボでの角度
  uint32_t pw = pwPerDeg * angleS + pwN0; //usec

  //servo1.writeMicroseconds(pw);
  servo1WriteMs(pw);
  float pos = ARM_LENGTH * sin(degToRad(angle)) - startPosition;

  //Serial.printf("Tamabo angle:%5.1fdeg ", angle);
  Serial.printf("servo angle:%5.1fdeg  pulse width:%5dus ", angleS, pw);
  Serial.printf("--> POSITION:%7.3fmm ", pos);

  return pos;
}


// button B -----------------------------------------------------------------

void servoPosition(void)
{
  //ボタンBが押された時に玉の位置を動かす
  switch(tamaPos)
  {
    case CENTER1_POS:
      //スタート位置へ
      tamaPos = START_POS;        //スタート位置へ移動
      //servo1.write(startPwidth);
      servo1WriteMs(startPwidth);
      dispTamaPos(tamaPos);       //玉位置表示
      dispBtnB(TO_CENTER);        //ボタンへは次の行先を表示
      break;
    case START_POS:
      //センターへ
      tamaPos = CENTER2_POS;
      //servo1.write(centerPwidth);
      servo1WriteMs(centerPwidth);
      dispTamaPos(tamaPos);
      dispBtnB(TO_END);
      break;
    case CENTER2_POS:
      //エンド位置へ
      tamaPos = END_POS;
      //servo1.write(endPwidth);
      servo1WriteMs(endPwidth);
      dispTamaPos(tamaPos);
      delay(300);
      //ゼロセット
      dispZeroSet();
      scaleTare();
      M5.Speaker.tone(1000,500);
      delay(2000);
      //
      dispBtnB(TO_CENTER);
      break; 
    case END_POS:
      //センターへ
      tamaPos = CENTER1_POS;
      //servo1.write(centerPwidth);
      servo1WriteMs(centerPwidth);
      dispTamaPos(tamaPos);
      dispBtnB(TO_START);
      break;
    default:
      tamaPos = CENTER1_POS;
      dispBtnB(TO_START);
      break;
  } 
}



//------- TEST ------------------------------------------------------------------------------- 

void servoAdjust(void)
{
  //サーボホーンの組み付け調整
  uint16_t pw;

  //ESP32PWM::allocateTimer(0);   //0〜3
  //servo1.setPeriodHertz(50);    // standard 50 hz servo
  //servo1.attach(PIN_SERVO, 500, 2400); ///////////////////可動域制限///////////
  Serial.println("**** servo adjust **********");

  //display init
  M5.Display.clearDisplay(TFT_BLACK);
  M5.Display.setColor(TFT_BROWN);
  M5.Display.fillRect(0, 0, 320, 20);
  //title bar
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BROWN);
  M5.Display.setCursor(4, 0);
  M5.Display.printf("サーボの設定　サーボホーンを外す");
  //button
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(0, 220);
  M5.Display.printf("　　　ー　　　　ＮＥＸＴ　　　　＋");
  //
  while(1)
  {
    M5.update();
    if (M5.BtnB.isPressed())
    {
      break;
    }
    delay(50);
  }
  M5.Speaker.tone(2000, 50);
  delay(200);

  //
  pw = 2320;
  while(true)
  {
    M5.update();
    //servo1.writeMicroseconds(pw);
    servo1WriteMs(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 30);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("＋９０度（上）パルス幅：%5dusec　", pw);
    if (M5.BtnA.isPressed())
    {
      pw -= 10;
      if (pw < 500){pw = 500;}
    }
    if (M5.BtnC.isPressed())
    {
      pw += 10;
      if (pw > 2400){pw = 2400;}
    }
    if (M5.BtnB.isPressed())
    {
      break;
    }
    delay(100);
  }
  M5.Speaker.tone(2000, 50);
  pwP90 = pw;
  delay(500);

  //時計方向に回る
  pw = 560;
  while(true)
  {
    M5.update();
    //servo1.writeMicroseconds(pw);
    servo1WriteMs(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 50);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("ー９０度（下）パルス幅：%5dusec　", pw);
    if (M5.BtnA.isPressed())
    {
      pw -= 10;
      if (pw < 500){pw = 500;}
    }
    if (M5.BtnC.isPressed())
    {
      pw += 10;
      if (pw > 2400){pw = 2400;}
    }
    if (M5.BtnB.isPressed())
    {
      break;
    }
    delay(100);
  }
  M5.Speaker.tone(2000, 50);
  pwM90 = pw;
  delay(500);
  
  //
  pw = 1420;
  while(true)
  {
    M5.update();
    //servo1.writeMicroseconds(pw);
    servo1WriteMs(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 70);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("　　０度　　　パルス幅：%5dusec　", pw);
    
    if (M5.BtnA.isPressed())
    {
      pw -= 10;
      if (pw < 500){pw = 500;}
    }
    if (M5.BtnC.isPressed())
    {
      pw += 10;
      if (pw > 2400){pw = 2400;}
    }
    if (M5.BtnB.isPressed())
    {
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

  while(true)
  {
    M5.update();
    pw = (pwP90 - pwM90) / 180 * centerAngleS + pwN0;
    //servo1.writeMicroseconds(pw);
    servo1WriteMs(pw);
    Serial.printf("pulse width:%d \n", pw);
    M5.Display.setCursor(0, 110);
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("　%2d度　　　パルス幅：%5dusec　", centerAngleS, pw);
  
    if (M5.BtnA.isPressed())
    {
      centerAngleS -= 1;
      if (centerAngleS < 30){centerAngleS = 30;}
    }
    if (M5.BtnC.isPressed())
    {
      centerAngleS += 1;
      if (centerAngleS > 70){centerAngleS = 70;}
    }
    if (M5.BtnB.isPressed())
    {
      break;
    }
    delay(100);
  }

  M5.Speaker.tone(1500, 300);
  delay(2000);
}

