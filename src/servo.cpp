/*
 * servo.cpp
 *
 * 
 * 2024.04.20
*/

#include "servo.hpp"


#define PIN_SERVO 33      //G5@stack M5 BASIC v2.7
//LEDC
//#define LEDC_CH     0
#define LEDC_BIT    15  //resolution[bit]
#define LEDC_FREQ   50  //PWM frequency[Hz]
//
#define ARM_LENGTH 9.0f   //サーボホーンアーム長さ[mm]
//
#define degToRad(deg) (deg / 180 * PI)
#define radToDeg(rad) (rad / PI * 180)
//servo
#define DEAD_BAND   1 //[usec]
#define SERVO_SPEED 120000 / 60 //[usec/deg] = 2ms/deg
#define PWM_PERIOD 1000000 / LEDC_FREQ  //[usec] = 20000usec

//global
tama_pos_t tamaPos;  //玉のポジション[mm]

//
Servo servo1;

//local
//servo adjust　-90,0,+90deg --> [usec]
uint16_t  pwM90 = 560;    //-90度   typ.500us
uint16_t  pwN0  = 1420;   //0度     typ.1450us
uint16_t  pwP90 = 2320;   //+90度   typ.2400us
float deadBandPw = 180.0 / (pwP90 - pwM90);   //[deg/us]

//サーボ角度とパルス幅　[deg]
const float centerAngle  = 0;   //この装置での基準角度
const float startAngleD  = -26; //真上からの前進量(マイナス値)[deg]
const float endAngle     = 28;  //真上からの後退量(プラス値)[deg]
float centerAngleS = 60;        //この装置での中点でのサーボの角度 サーボホーンが真上を向く角度...セレーションの影響あり[deg]
float startAngle;
float startAngleS;              //サーボのスタート角度　AngleS　S=Servo[deg]
float endAngleS;                //サーボのエンド角度[deg]
float servoAngleS;              //サーボの現在角度[deg]
float pwPerDeg;                          
//各玉ポジションでのパルス幅 [usec]
uint16_t  startPwidth;
uint16_t  centerPwidth;                                                 
uint16_t  endPwidth;
//玉の位置[mm]
float startPosition;
float endPosition;

//debug
static const char *TAG = "servo";


void servoInit(float setAngle)
{
  //サーボの初期化
  //setAngle 0:default setting, -20~-26:set Angle
  if ((setAngle < -20.0) && (setAngle > -26.0))
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
  pwPerDeg    = (pwP90 - pwM90) / 180.0;      //1度あたりのパルス幅usec                           
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
  servoMove(centerAngle, 1);/////////////////////////
  //
  ESP_LOGI(TAG, "start angle:%5.1fdeg (dx:%6.3fmm)", startAngle, startPosition);
  ESP_LOGI(TAG, "~ end angle:%5.1fdeg (dx:%6.3fmm)", endAngle, endPosition);
}


float endAngleGet(void)
{
  return endAngle;
}

float startAngleGet(void)
{
  return startAngle;
}


void servo1WriteUs(uint32_t usec)
{ //LEDCのPWM dutyでサーボを動かす
  uint32_t dutyTick = usec * ((1 << LEDC_BIT)  / PWM_PERIOD);  // 15bit / 20000usec (50Hz)

  //Serial.printf("pulse:%5d -- dutyTick:%6lu\n", usec, dutyTick);
  //ver.2
  //ledcWrite(LEDC_CH, dutyTick);



  //ver.3
  ledcWrite(PIN_SERVO, dutyTick);






}


float servoMove(float angle, uint16_t speed)
{ //サーボを動かす
  //angle: 装置での角度[deg]（真上がゼロ）　 startMoving ~ endMoving (-26.0 ~ 28.0)
  //ret pos: 玉の位置[mm]

  float prevAngle = servoAngleS;          //現在角度を退避
  servoAngleS = centerAngleS - angle;     //サーボでの角度

  ESP_LOGI(TAG, "angle:%5.1fdeg speed:%5dus", angle, speed);
  //servo1.writeMicroseconds(pw);
  //servo1WriteUs(pw);

  

  uint32_t prevPw = pwPerDeg * prevAngle + pwN0; //usec
  uint32_t toPw = pwPerDeg * servoAngleS + pwN0; //usec

//方向
  int8_t dir = (servoAngleS >= prevAngle) ? +1 : -1;
 

  uint32_t pw;
  //ゆっくりうごかす
  if (dir > 0)
  {
    for (pw = prevPw; pw < toPw; pw++)
    {
      uint32_t dutyTick = pw * (32768.0 / 20000.0); // 15bit / 20000usec (50Hz)
      //ver.3
      ledcWrite(PIN_SERVO, dutyTick);
      ESP_LOGD(TAG, "pw:%dus duty:%04x dir:%d", pw, dutyTick, dir);
      delayMicroseconds(speed);
    }
  }
  else
  {
    for (pw = prevPw; pw > toPw; pw--)
    {
      uint32_t dutyTick = pw * (32768.0 / 20000.0); // 15bit / 20000usec (50Hz)
      //ver.3
      ledcWrite(PIN_SERVO, dutyTick);
      ESP_LOGD(TAG, "pw:%dus duty:%04x dir:%d", pw, dutyTick, dir);
      delayMicroseconds(speed);
    }
  }


  float pos = ARM_LENGTH * sin(degToRad(angle)) - startPosition;

  //Serial.printf("Tamabo angle:%5.1fdeg ", angle);
  ESP_LOGI(TAG, "servo angle:%5.1fdeg  pulse width:%5dus ", servoAngleS, pw);
  ESP_LOGI(TAG, "--> POSITION:%7.3fmm \n", pos);

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
      
      servoMove(startAngle, 20);

      dispTamaPos(tamaPos);       //玉位置表示
      dispBtnB(TO_CENTER);        //ボタンへは次の行先を表示
      break;
    case START_POS:
      //センターへ
      tamaPos = CENTER2_POS;
      
      servoMove(centerAngle, 20);

      dispTamaPos(tamaPos);
      dispBtnB(TO_END);
      break;
    case CENTER2_POS:
      //エンド位置へ
      tamaPos = END_POS;
      
      servoMove(endAngle, 20);

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
      
      servoMove(centerAngle, 20);

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
    servo1WriteUs(pw);
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
    servo1WriteUs(pw);
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
    servo1WriteUs(pw);
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
    servo1WriteUs(pw);
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

