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

//global
uint8_t fmVer[] = "3.00";
uint16_t measCnt;   //測定回数 

//local
uint8_t sdStat = 0;       //SDcard detect 0:未,1:OK,2:fail
uint8_t cnt = 0;          //ループのカウンタ処理カウント用
bool    timeFlag = 0;     //NTPサーバより時刻取得


void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  //
  pinMode(PIN_BAT_V, ANALOG);   //3.3V-10bit
  pinMode(PIN_BAT_ON, OUTPUT);  //サーボへのバッテリ電圧供給
  digitalWrite(PIN_BAT_ON, LOW);    //サーボ用バッテリ電源オフ
  analogReadResolution(12);   //アナログ12bit
  //
  Serial.begin(115200);

  M5.Speaker.setVolume(48);
  M5.Speaker.tone(1500,300);
  dispInit();
  wifiInit(); ////////////////
  sdStat = sdInit();  //dispInitの後に
  scaleInit();
  servoInit();
  digitalWrite(PIN_BAT_ON, HIGH);    //サーボ用バッテリ電源オン

  ///////////// test ////////////////////////////////////////////////////////////////
  //servoAdjust();
  //graphClear();

}


void loop() {
  M5.update();  //ボタンの状態を更新する。

  //button
  if (M5.BtnA.pressedFor(150)){  //ms長押し
    //測定
    //Serial.println("Btn A");
    M5.Speaker.tone(1000,200);
    graphClear();
    measNukiF();
    M5.Speaker.tone(1500,100);
    //玉位置情報更新
    tamaPos = END_POS;
    dispBtnB(TO_CENTER);
  }
  
  if (M5.BtnB.wasPressed()){
    //玉位置移動
    //Serial.println("Btn B");
    M5.Speaker.tone(1320,100);
    servoPosition();          //玉ポジション移動
  }
  if (M5.BtnC.wasPressed()){
    //ノズル検出
    //Serial.println("Btn C");
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
    dispLoad(measLoad(10));
  }else{
    //
  }
  
  if (cnt == 10){
    //batteryVolt
    dispBatV((float)analogReadMilliVolts(PIN_BAT_V) * 2.0f / 1000);
  }
  if (cnt == 20){
    //bat %
  }
  if (cnt == 30){
    //battery charge    
  }
  if ((cnt == 40) && (sdStat == 2)){
    //SD card
    //抜いたのは検出しない
    //Serial.printf("SD flag %d\n", sdFlag);
    sdStat = sdInit();
  }
  
}

//----WiFi init--------
uint8_t wifiInit(void){
  //SDに日付を入れるために使用
  //アクセスポイント情報
  const char* ssid = "B0C7456EFFCD";   //ssidを入力
  const char* passwd = "uk5ii9dmj5rxu"; //ネットワークパスワード入力
  const char* ntpServer1 = "pool.ntp.org";
  const char* ntpServer2 = "time.nist.gov";
  const long  gmtOffset_sec = 9 * 3600;
  const int   daylightOffset_sec = 0;
  uint8_t toutCnt = 0;  //タイムアウトカウント


  // set notification call-back function
  sntp_set_time_sync_notification_cb( timeavailable );

  /**
   * NTP server address could be aquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: configTime() function call if made AFTER DHCP-client run
   * will OVERRIDE aquired NTP server address
   */
  sntp_servermode_dhcp(1);    // (optional)


  #define TZ_Asia_Tokyo	PSTR("JST-9")
  configTzTime(TZ_Asia_Tokyo, ntpServer1, ntpServer2);
  //
  WiFi.begin(ssid, passwd);               //アクセスポイント接続のためのIDとパスワードの設定
  while (WiFi.status() != WL_CONNECTED) { //接続状態の確認
    delay(300);                           //接続していなければ0.3秒待つ
    Serial.print(".");                    //接続しなかったらシリアルモニタに「.」と表示
    toutCnt++;
    if (toutCnt > 30){
      //timeout
      Serial.println("WiFi Timeout!");
      return 1;
    }
  }
  //通信が可能となったら各種情報を表示する
  Serial.print("WiFi Connected   ");       //接続したらシリアルモニタに「WiFi Connected」と表示
  Serial.print("IP Address : ");          //シリアルモニタに表示
  Serial.println(WiFi.localIP());         //割り当てられたIPアドレスをシリアルモニタに表示
  toutCnt = 0;
  while(!timeFlag){
    //time
    delay(300);                           //接続していなければ0.3秒待つ
    toutCnt++;
    if (toutCnt > 30){
      //timeout
      Serial.println("NTP Timeout!");
      return 1;
    }
  }
  
  return 0;
}


void printLocalTime(void){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t){
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
  timeFlag = 1;
}


//------ SD ------------
File tamaFile;

uint8_t sdInit(void){
  //SDcard init
  //ret --> 2:err, 1:ok, 0:未（初回）
  if (sdStat == 1){
    return 1;
  }

  if (SD.begin(GPIO_NUM_4, SPI, 20000000)){ //25000000
    Serial.println("SD OK!");
    if (sdStat != 0){
      M5.Speaker.tone(1500,80);
      dispSdcardStatus(0);
    }

    //test read write
    tamaFile = SD.open("/test.txt", FILE_WRITE);
    tamaFile.println("test test test 123456");
    Serial.println("/test.txt <--write- 'test test test 123456'");
    tamaFile.close();
    //reopen read
    tamaFile = SD.open("/test.txt");
    Serial.print("/test.txt -read--> '");
    while(tamaFile.available()){
      Serial.write(tamaFile.read());
    }
    Serial.println("'");
    tamaFile.close();
    //

    return 1;
  }
    Serial.println("SD failed!");
    if (sdStat == 0){
      //最初だけ表示と音で警告
      delay(300);
      dispSdcardStatus(1);
      M5.Speaker.tone(4000, 600);
    }
    return 2;
 }

