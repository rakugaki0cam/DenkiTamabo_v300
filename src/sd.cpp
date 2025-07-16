/*
    sd.cpp
    SD card
      Wifi
      NTP

      2024.04.25
*/
#include "sd.hpp"


//#define TZ_Asia_Tokyo	PSTR("JST-9")

BluetoothSerial SerialBT;

//
uint8_t tamaFileName[] = "/tamabo000000-000000.csv          ";  //後端スペースは保険
File    tamaFile;
bool    timeFlag = 0;     //NTPサーバより時刻取得
const char* week[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
//m5::rtc_datetime_t rtcDateTimeDef;
m5::rtc_date_t rtcDateDef;
m5::rtc_time_t rtcTimeDef;

struct tm timeInfo;       //time information

//---- WiFi ---------------------------------------

uint8_t wifiInit(void){
  //SDに日付を入れるために使用

  //アクセスポイント情報
  const char* ssid = "B0C7456EFFCD";   //ssidを入力
  const char* passwd = "uk5ii9dmj5rxu"; //ネットワークパスワード入力
  
  uint8_t toutCnt = 0;  //タイムアウトカウント
  uint8_t stat = 0;

  //NTP
  sntp_set_time_sync_notification_cb( timeavailable );
  //sntp_servermode_dhcp(1);    // (optional)before WiFi connect....NTP/////
  esp_sntp_servermode_dhcp(1);

  //
  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid, passwd);               //アクセスポイント接続のためのIDとパスワードの設定
  Serial.print("WiFi ");
  dispWifi(TEXT_WIFI);

  while (WiFi.status() != WL_CONNECTED) { //接続状態の確認
    delay(500);                           //接続していなければ0.5秒待つ
    Serial.print(".");                    //接続しなかったらシリアルモニタに「.」と表示
    if (toutCnt % 2 + 1){
      dispWifi(TEXT_DOT);
    }else{
      dispWifi(TEXT_DOT_NONE);
    }
    toutCnt++;
    if (toutCnt > 100){
      //timeout
      Serial.println(" Timeout!");
      dispWifi(TEXT_WIFI_TIMEOUT);
      WiFi.disconnect();
      return 1;
    }
  }

  //通信が可能となったら各種情報を表示する
  Serial.print(" Connected.  ");       //接続したらシリアルモニタに「WiFi Connected」と表示
  dispWifi(TEXT_WIFI_CONECT);
  Serial.print("IP Address: ");          //シリアルモニタに表示
  Serial.println(WiFi.localIP());         //割り当てられたIPアドレスをシリアルモニタに表示
  dispWifi(TEXT_IP);

  stat = ntpTimeInit();   //NTPより時刻取得

  delay(500);
  WiFi.disconnect();
  return stat;
}


//--- NTP time -----------------------------------------------------

uint8_t ntpTimeInit(void){
  //NTPサーバーに接続して時刻を取得しRTCへセット
  const char* ntpServer1 = "ntp.nict.jp";
  const char* ntpServer2 = "ntp.jst.mfeed.ad.jp";
  const long  gmtOffset_sec = 9 * 3600;
  const int   daylightOffset_sec = 0;
  uint8_t toutCnt = 0;  //タイムアウトカウント

  //
  Serial.print("NTP server ");
  //configTzTime(TZ_Asia_Tokyo, ntpServer1, ntpServer2);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  dispWifi(TEXT_NTP);

  toutCnt = 0;
  while(!timeFlag){
    Serial.print(".");
    delay(500);                           //接続していなければ0.5秒待つ
    toutCnt++;
    if (toutCnt > 20){
      //timeout 10秒
      Serial.println(" Timeout!");
      dispWifi(TEXT_NTP_TIMEOUT);
      return 1;
    }
  }
  printLocalTime();
  //RTCに時刻をセット
  time_t ti = time(nullptr) + 1;
  while(ti > time(nullptr)){
    //秒の繰り上げまで待機
  }
  M5.Rtc.setDateTime(localtime(&ti));
  uint8_t text[] = "000000-000000          ";
  getTimeStamp((char*)text);
  Serial.printf("RTC set '%s' OK!\n", text);
  //
  M5.Speaker.tone(1000, 100);

  dispWifi(TEXT_DATE_TIME);
  dispWifi(TEXT_WIFI_OK);
  return 0;
}


//***** NTP & RTC time sub ******************************
void printLocalTime(void){

  if(!getLocalTime(&timeInfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeInfo, "%A, %B %d %Y %H:%M:%S");

  //Serial.printf("NTP: %04d/%02d/%02d(%s) - ", (timeInfo.tm_year + 1900), (timeInfo.tm_mon + 1), timeInfo.tm_mday, week[timeInfo.tm_wday]);
  //Serial.printf("%02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

}


void getTimeStamp(char* txt){
  //タイムスタンプ取得
/*
  //NTPより取得したタイム（WiFi切った後も動いている）
  if(!getLocalTime(&timeInfo)){
    Serial.println("No time available (yet)");
    return;
  } 
  sprintf(txt, "%02d%02d%02d-%02d%02d%02d", (timeInfo.tm_year - 100), (timeInfo.tm_mon + 1), timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  Serial.printf("NTP: %s\n", txt);
*/  
  //RTCより
  M5.Rtc.getDate(&rtcDateDef);
  M5.Rtc.getTime(&rtcTimeDef);
  sprintf(txt, "%02d%02d%02d-%02d%02d%02d", (rtcDateDef.year - 2000), rtcDateDef.month, rtcDateDef.date, rtcTimeDef.hours, rtcTimeDef.minutes, rtcTimeDef.seconds);
  //Serial.printf("RTC: %s\n", txt);
}

void getTimeText(char* txt){
  //タイムテキスト
  M5.Rtc.getDate(&rtcDateDef);
  M5.Rtc.getTime(&rtcTimeDef);
  sprintf(txt, "%02d/%02d/%02d %02d:%02d:%02d", (rtcDateDef.year - 2000), rtcDateDef.month, rtcDateDef.date, rtcTimeDef.hours, rtcTimeDef.minutes, rtcTimeDef.seconds);
  //Serial.printf("RTC: %s\n", txt);
}


void generateFileName(void){
  //filename
  uint8_t fTime[] = "000000-000000          ";

  getTimeStamp((char*)fTime);
  sprintf((char*)tamaFileName, "/tamabo%s.csv", fTime); //Global/////////////

}


// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t){
  Serial.print(" get time :");
  timeFlag = 1;
}


//***** SD card ****************************************************

uint8_t sdInit(void){
  //SDcard init
  //ret --> 2:err, 1:ok, 0:未（初回）

  if (sdStat == 1){
    return 1;
  }

  if (!SD.begin(GPIO_NUM_4, SPI, 20000000)){ //25000000
    Serial.println("SD failed!");
    if (sdStat == 0){
      //最初だけ表示と音で警告
      delay(300);
      dispSdcardStatus(1);
      M5.Speaker.tone(4000, 600);
    }
    return 2;//////////////////////
  }
  //
  generateFileName();
  tamaFile = SD.open((char*)tamaFileName, FILE_WRITE);  //SDカードを抜き差しした後はエラーになってしまう
  //Serial.printf("SDopen:%d\n", tamaFile);/////////
  if (tamaFile != 1){
    Serial.println("SD card error!");
    tamaFile.close();
    return 2;
  }

  tamaFile.println("*** DENKI Tamabo M5 v3 ***");
  //Serial.println("write title");
  tamaFile.close();
  Serial.println("SD card OK!");
  if (sdStat != 0){
    //SD mount OK
    M5.Speaker.tone(1500,30);
    dispSdcardStatus(0);
  }

  return 1;
} 
 

void sdDataSave(char* time, uint16_t mNum, uint8_t n, float* pos, float* load){
  uint16_t i;

  tamaFile = SD.open((char*)tamaFileName, FILE_APPEND);
  //Serial.print("tamaFile = ");
  //Serial.println(tamaFile);
  if (tamaFile != 1){
    Serial.println("SD error");
    //sdStat = 2;
    //エラーの時、SDマウントしなおしたいけれど、SDライブラリ側で再マウントできない仕様らしい
    tamaFile.close();
    dispSdcardStatus(1);
    M5.Speaker.tone(4000, 600);
    return;
  }
  tamaFile.printf("measure #%d\n", mNum);
  tamaFile.printf("%s\n", time);
  tamaFile.printf("measure angle: %4.1f ~ %4.1f deg\n", startAngleGet(), endAngleGet());
  tamaFile.println("#, pos[mm], load[gf]");
  for (i = 0; i < n; i++){
    tamaFile.printf("%5d, %6.3f, %6.2f\n", (i + 1), pos[i], load[i]);
  }
  tamaFile.println();
  tamaFile.close();
  Serial.println("SDcard data saved.");
}

/*
//------------ Bluetooth ----------------------------------------

void bluetoothSerialInit(void){
  //Bluetoothシリアル設定

  dispBtSerial(BTS_INIT);

#define noMACBOOK
#ifdef MACBOOK
  //マックブックへの接続（不安定？？）
  bool connected;
  String slaveName = "MacBook Pro 15";

  SerialBT.begin("DENKI-Tamabo-v3", true);
  connected = SerialBT.connect(slaveName);
  Serial.printf("Bluetooth Serial connect --> BT device\"%s\" ", slaveName.c_str());

  if (connected){
    Serial.println("port: '/dev/cu.Bluetooth-Incoming-Port' ");
    Serial.println(" OK!");
    dispBtSerial(BTS_OK);
    dispBtSerial(BTS_PORT1);

  }else{
    SerialBT.connected(10000));  //timeout 10000msec
    Serial.println("failed.");
    dispBtSerial(BTS_TIMEOUT);
  }
#else
  //接続先を指定しない
  SerialBT.begin("DENKI-Tamabo-v3");
  Serial.print("Bluetooth Serial connecting --> BT device ");
  Serial.println("port: '/dev/cu.DENKI-Tamabo-v3-ESP32SPP'");
  dispBtSerial(BTS_OK);
  dispBtSerial(BTS_PORT2);
#endif
  SerialBT.println("*** DENKI Tamabo M5 v3 ***");

}

void btDataSend(char* time, uint16_t mNum, uint8_t n, float* pos, float* load){
  //Bluetoothシリアルでデータを送信
  uint16_t i;

  SerialBT.printf("measure #%d\n", mNum);
  SerialBT.printf("%s\n", time);
  SerialBT.printf("measure angle: %4.1f ~ %4.1f deg\n", startAngleGet(), endAngleGet());
  SerialBT.println("#, pos[mm], load[gf]");
  for (i = 0; i < n; i++){
    SerialBT.printf("%5d, %6.3f, %6.2f\n", (i + 1), pos[i], load[i]);
  }
  SerialBT.println();
  Serial.println("bluetooth serial data send.");

}


void btSerialRx(void){
  //BTSerial 受信
  if (!SerialBT.available()){
    return;
  }
  while (SerialBT.available()){
    Serial.write(SerialBT.read());
  }
  M5.Speaker.tone(2000, 50);
}


*/