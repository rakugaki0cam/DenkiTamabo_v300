/*
 * servo.cpp
 *
 * 
 * 2024.04.20
*/

#include "servo.hpp"


#define PIN_SERVO   33
//LEDC
#define LEDC_BIT    15  //resolution[bit]
#define LEDC_FREQ   50  //PWM frequency[Hz]
//servo
#define ARM_LENGTH    9.0f        //サーボホーンアーム長さ[mm]
#define DEAD_BAND     1           //PWMデッドバンド[usec]
#define SERVO_SPEED   (120000 / 60)           //[usec/deg] = 2ms/deg
#define PWM_PERIOD    (1000000 / LEDC_FREQ)   //[usec] = 20000usec
//
#define degToRad(deg) (deg / 180 * PI)
#define radToDeg(rad) (rad / PI * 180)


//global
tama_pos_t tamaPos;  //玉のポジション[mm]


//local
Servo servo1;
//servo adjust　-90, 0, +90deg
uint16_t  pwM90 = 560;    //-90度   typ.500us
uint16_t  pwN0  = 1420;   //0度     typ.1450us
uint16_t  pwP90 = 2320;   //+90度   typ.2400us
float deadBandAngle = 180.0 / (pwP90 - pwM90);   // = 180 / (2400 - 500) = 0.108 [deg/us]  分解能0.1°

//サーボ角度とパルス幅　[deg]
const float centerAngle  = 0;   //この装置での基準角度
const float startAngleD  = -26; //真上からの前進量(マイナス値)[deg]
const float endAngle     = 28;  //真上からの後退量(プラス値)[deg]
//
float centerAngleS = 60;        //この装置での中点でのサーボの角度 サーボホーンが真上を向く角度...セレーションの影響あり[deg]
float startAngle;
float startAngleS;                //サーボのスタート角度　AngleS　S=Servo[deg]
float endAngleS;                  //サーボのエンド角度[deg]
float servoAngleS = centerAngleS - 1; //サーボの現在角度[deg]
float pwPerDeg;
//各玉ポジションでのパルス幅 [usec]
uint16_t  startPwidth;
uint16_t  centerPwidth;                                                 
uint16_t  endPwidth;
//玉の位置[mm]
float startPosition;
float endPosition;

//debug
static const char *TAG = "サーボ";


void servoInit(float setAngle)
{ //サーボの初期化
  //setAngle 0:default setting, -20~-26:set Angle
  if ((setAngle < -20.0) && (setAngle > startAngleD))
  {  //スタート角度を変更（ノズル検出による変更）
    startAngle = setAngle;
  }
  else
  {
    startAngle = startAngleD;
  }
  ESP_LOGI(TAG, "Servo start angle:%6.1fdeg", startAngle);
  startAngleS  = centerAngleS - startAngle;   //最大角度 60-(-26) = 86
  endAngleS    = centerAngleS - endAngle;     //最小角度 60-28 = 32 (測定時は角度をマイナスさせる方向 86 -> 32)
  pwPerDeg    = (pwP90 - pwM90) / 180.0;      //1度あたりのパルス幅usec  約　9.78usec/deg                          
  //各玉ポジションでのパルス幅 [usec]
  startPwidth  = pwPerDeg * startAngleS  + pwN0;
  centerPwidth = pwPerDeg * centerAngleS + pwN0;                                                 
  endPwidth    = pwPerDeg * endAngleS    + pwN0;
  ESP_LOGI(TAG, "Servo start pw:%5dus center pw:%5dus end pw:%5dus", startPwidth, centerPwidth, endPwidth);
  //玉の位置[mm]
  startPosition = ARM_LENGTH * sin(degToRad(startAngle));
  endPosition = ARM_LENGTH * sin(degToRad(endAngle));

  //PWM(LEDC) init
  ledcAttach(PIN_SERVO, LEDC_FREQ, LEDC_BIT); //ver.3
  //
  servoMove(centerAngle, SPEED_SLOW); /////////////////////////もとの位置がわからないので最大速度で動くことがある
  //
  ESP_LOGI(TAG, "start angle:%5.1fdeg (dx:%6.3fmm) ~ end angle:%5.1fdeg (dx:%6.3fmm)", startAngle, startPosition, endAngle, endPosition);
}


float servoMove(float angle, uint16_t speed)
{ //サーボをゆっくり動かす
  //angle: 装置での角度[deg]（真上がゼロ）　 startMoving ~ endMoving (-26.0 ~ 28.0)
  //speed: 0=最高速, μsのディレイが入る
  //ret pos: 玉の位置[mm]

  static float prevAngleS;          //初期値（物理位置は不明）
  
  prevAngleS = servoAngleS;          //現在角度を退避
  servoAngleS = centerAngleS - angle;     //サーボでの角度
  uint32_t prevPw = pwPerDeg * prevAngleS + pwN0; //usec
  uint32_t toPw = pwPerDeg * servoAngleS + pwN0; //usec

  ESP_LOGD(TAG, "servo angle:%5.1fdeg (pw:%5d) -> %5.1fdeg (pw:%5d)   speed:%5dms", prevAngleS, prevPw, servoAngleS, toPw, speed);
  //方向
  int8_t dir = (servoAngleS >= prevAngleS) ? +1 : -1;
 
  uint32_t pw;
  //ゆっくりうごかす
  if (dir > 0)
  { //+方向
    for (pw = prevPw; pw < toPw; pw++)
    {
      servo1WriteUs(pw);
      delay(speed); //msec
    }
  }
  else
  { //-方向
    for (pw = prevPw; pw > toPw; pw--)
    {
      servo1WriteUs(pw);
      delay(speed); //msec
    }
  }

  float pos = ARM_LENGTH * sin(degToRad(angle)) - startPosition;
  ESP_LOGD(TAG, "servo angle:%5.1fdeg  pulse width:%5dms --> POSITION:%7.3fmm", servoAngleS, pw, pos);

  return pos;
}


void servo1WriteUs(uint32_t pwUsec)
{ //LEDCのPWM dutyでサーボを動かす
  constrain(pwUsec, endPwidth, startPwidth); //制限
  uint32_t dutyTick = pwUsec * (1 << LEDC_BIT)  / PWM_PERIOD;  // 32768(15bit) / 20000usec (50Hz)
  ESP_LOGD(TAG, "pw:%dus dutytick:%04x", pwUsec, dutyTick);
  //ver.2
  //ledcWrite(LEDC_CH, dutyTick);
  //ver.3
  ledcWrite(PIN_SERVO, dutyTick);
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
      
      servoMove(startAngle, SPEED_SLOW);

      dispTamaPos(tamaPos);       //玉位置表示
      dispBtnB(TO_CENTER);        //ボタンへは次の行先を表示
      break;
    case START_POS:
      //センターへ
      tamaPos = CENTER2_POS;
      
      servoMove(centerAngle, SPEED_MID);

      dispTamaPos(tamaPos);
      dispBtnB(TO_END);
      break;
    case CENTER2_POS:
      //エンド位置へ
      tamaPos = END_POS;
      
      servoMove(endAngle, SPEED_MID);

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
      
      servoMove(centerAngle, SPEED_SLOW);

      dispTamaPos(tamaPos);
      dispBtnB(TO_START);
      break;
    default:
      tamaPos = CENTER1_POS;
      dispBtnB(TO_START);
      break;
  } 
}


//get 
float endAngleGet(void)
{
  return endAngle;
}


float startAngleGet(void)
{
  return startAngle;
}


//------- TEST ------------------------------------------------------------------------------- 
void servoAdjust(void)
{
  //サーボホーンの組み付け調整
  uint16_t pw;

  //ESP32PWM::allocateTimer(0);   //0〜3
  //servo1.setPeriodHertz(50);    // standard 50 hz servo
  //servo1.attach(PIN_SERVO, 500, 2400); ///////////////////可動域制限///////////
  ESP_LOGI(TAG, "**** servo adjust **********");

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
    ESP_LOGI(TAG, "pulse width:%d", pw);
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
    ESP_LOGI(TAG, "pulse width:%d", pw);
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
    ESP_LOGI(TAG, "pulse width:%d", pw);
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
    ESP_LOGI(TAG, "pulse width:%d", pw);
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

