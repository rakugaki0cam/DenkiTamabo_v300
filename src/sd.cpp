/*
    sd.cpp
    SD card
      Wifi
      NTP

      2024.04.25
*/
#include "sd.hpp"


#define TZ_Asia_Tokyo	PSTR("JST-9")
//
uint8_t tamaFileName[] = "/tamabo000000-000000.csv          ";  //後端スペースは保険
File    tamaFile;
bool    timeFlag = 0;     //NTPサーバより時刻取得
struct tm rt;             //time information


//---- WiFi init ---------------------------------------

uint8_t wifiInit(void){
  //SDに日付を入れるために使用

  //アクセスポイント情報
  const char* ssid = "B0C7456EFFCD";   //ssidを入力
  const char* passwd = "uk5ii9dmj5rxu"; //ネットワークパスワード入力
  //
  const char* ntpServer1 = "pool.ntp.org";
  const char* ntpServer2 = "time.nist.gov";
  const long  gmtOffset_sec = 9 * 3600;
  const int   daylightOffset_sec = 0;
  uint8_t toutCnt = 0;  //タイムアウトカウント

  sntp_set_time_sync_notification_cb( timeavailable );
  sntp_servermode_dhcp(1);    // (optional)before WiFi connect

  //
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
  //time
    configTzTime(TZ_Asia_Tokyo, ntpServer1, ntpServer2);
    dispWifi(TEXT_NTP);
  toutCnt = 0;
  while(!timeFlag){
    //time
    delay(500);                           //接続していなければ0.5秒待つ
    toutCnt++;
    if (toutCnt > 20){
      //timeout 10秒
      Serial.println("NTP Timeout!");
      dispWifi(TEXT_NTP_TIMEOUT);
      return 1;
    }
  }
  printLocalTime();
  dispWifi(TEXT_DATE_TIME);
  WiFi.disconnect();
  //
  M5.Speaker.tone(1000, 100);
  return 0;
}


//***** time ******************************
void printLocalTime(void){
  if(!getLocalTime(&rt)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&rt, "%A, %B %d %Y %H:%M:%S");
}


void getTime(char* txt){
  //タイムスタンプ取得 
  if(!getLocalTime(&rt)){
    Serial.println("No time available (yet)");
    return;
  } 
  sprintf(txt, "%02d%02d%02d-%02d%02d%02d", (rt.tm_year - 100), (rt.tm_mon + 1), rt.tm_mday, rt.tm_hour, rt.tm_min, rt.tm_sec);
  //Serial.println(txt);
}


void getFileName(void){
  //filename
  uint8_t fTime[] = "000000-000000";

  getTime((char*)fTime);
  sprintf((char*)tamaFileName, "/tamabo%s.csv", fTime); //Global/////////////
}


// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t){
  Serial.println("Got time adjustment from NTP!");
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
  getFileName();
  tamaFile = SD.open((char*)tamaFileName, FILE_WRITE);  //SDカードを抜き差しした後はエラーになってしまう
  //Serial.printf("SDopen:%d\n", tamaFile);/////////
  if (tamaFile != 1){
    Serial.println("SD error!");
    tamaFile.close();
    return 2;
  }

  tamaFile.println("*** DENKI Tamabo M5 v3 ***");
  //Serial.println("write title");
  tamaFile.close();
  Serial.println("SD OK!");
  if (sdStat != 0){
    //SD mount OK
    M5.Speaker.tone(1500,30);
    dispSdcardStatus(0);
  }

  return 1;

} 
 

void sdDataSave(uint16_t mNum, uint8_t n, float* pos, float* load){
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
  tamaFile.println();
  tamaFile.printf("measure #%d, ", mNum);
  tamaFile.println("position[mm], load[gf]");
  for (i = 0; i < n; i++){
    tamaFile.printf("%5d, %6.3f, %7.3f\n", (i + 1), pos[i], load[i]);
  }
  tamaFile.println();
  tamaFile.close();
  Serial.println("data saved");
}

