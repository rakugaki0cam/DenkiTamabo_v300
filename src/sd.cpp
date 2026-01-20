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
//debug
static const char *TAG = "SDカード";


//---- WiFi ---------------------------------------
uint8_t wifiInit(void)
{ //SDに日付を入れるために使用
  //ret:  0..OK
  //      1..wifi接続不可
  //      2..SDに接続情報なし　

  //アクセスポイント情報をSDカードから読み出すようにする
  uint8_t ssidFileName[] = "/AccessPoint/ssid.txt";  //アクセスポイント情報ファイル
  String  readText;
  String  ssid;
  String  password;

  tamaFile = SD.open((char*)ssidFileName, FILE_READ);  //SDカードを抜き差しした後はエラーになってしまう
  if (tamaFile != 1)
  {
    ESP_LOGE(TAG, "SD card error!");
    tamaFile.close();
    return 2;
  }
  ESP_LOGI(TAG, "SD open %s OK.", (char*)ssidFileName);

  while (tamaFile.available()) 
  {
    readText = tamaFile.readStringUntil('\n');                //改行までを読み出し
    readText = readText.substring(0, readText.indexOf('#'));  //コメント
    ssid = readText.substring(0, readText.indexOf(' '));      //１文字目から　’スペース’までを検索して　最初の単語を切り出す
    ssid.trim();                                              //前後のスペースを取り除く
    password = readText.substring(readText.indexOf(' '));     //スペース以降を切り出す
    password.trim();                                          //前後のスペースを取り除く
    if (ssid.length() != 0) 
    {
      ESP_LOGI(TAG, "WiFi SSID:%s password:%s ", ssid.c_str(), password.c_str());
      //WiFi.begin(ssid.c_str(), password.c_str());  
      break;
    }
  }

  /*
  SDカードへ書き込んでおくファイル
  -----------------------------------------------------
  /AccessPoint/ssid.txt

  # WiFi アクセスポイント設定ファイル
  # 2.4GHzのみ　　5.0GHzは不可

  # SSID パスワードを間にスペースを入れて、パスワードの後は改行して記入する

  # 例
  # Baffalo-G-1234 abc27833ad
  #

  B0C7456EFFCD uk5ii9dmj5rxu
  #B0C7456EFFgD uk5ii9dmj5r test

  ＃はコメント行
  -----------------------------------------------------
  */


  uint8_t toutCnt = 0;  //タイムアウトカウント
  uint8_t stat = 0;
  bool done = true;
  //
  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid.c_str(), password.c_str()); //アクセスポイント接続のためのIDとパスワードの設定
  Serial.print("WiFi ");
  dispWifi(TEXT_WIFI);
  //NTP
  esp_sntp_servermode_dhcp(1);  //(optional)
  
  while (done) 
  {
    Serial.print("WiFi connecting");
    auto last = millis() + 2000;
    while ((WiFi.status() != WL_CONNECTED) && ( millis() < last)) 
    {
      delay(500);
      Serial.print(".");
      dispWifi(TEXT_DOT);
    }
    if (WiFi.status() == WL_CONNECTED) 
    {
      done = false;
    } 
    else 
    {
      Serial.println("retry");
      WiFi.disconnect();
      WiFi.reconnect();
    }
    toutCnt++;
    if (toutCnt > 2)
    {
      //timeout
      ESP_LOGE(TAG, "Timeout!");
      dispWifi(TEXT_WIFI_TIMEOUT);
      WiFi.disconnect(true);  //wifiをオフ
      WiFi.mode(WIFI_OFF); 
      ESP_LOGE(TAG, "WiFi not connect!");
      //接続しなかった時の時刻はM5内蔵のRTCによる
      dispWifi(TEXT_RTC); //RTCタイム
      dispWifi(TEXT_DATE_TIME);

      return 1;
    }
  }

  //通信が可能となったら各種情報を表示する
  Serial.print("WiFi connected.");
  dispWifi(TEXT_WIFI_CONECT);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());//割り当てられたIPアドレスをシリアルモニタに表示 
  dispWifi(TEXT_IP);
  //NTP
  sntp_set_time_sync_notification_cb(timeavailable); //NTPサーバーからの時刻取得完了時に呼び出されるコールバック関数を設定
  stat = ntpTimeInit();   //NTPより時刻取得

  delay(500);
   
  WiFi.disconnect(true);  //wifiをオフ
  return stat;
}


//--- NTP time -----------------------------------------------------

uint8_t ntpTimeInit(void){
  //NTPサーバーに接続して時刻を取得しRTCへセット
  const char* ntpServer1 = "ntp.nict.jp";
  const char* ntpServer2 = "ntp.jst.mfeed.ad.jp";
  const long  gmtOffset_sec = 9 * 3600;
  const int   daylightOffset_sec = 0;
  uint8_t     toutCnt = 0;  //タイムアウトカウント

  //
  Serial.print("NTP server ");
  //configTzTime(TZ_Asia_Tokyo, ntpServer1, ntpServer2);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  dispWifi(TEXT_NTP);

  toutCnt = 0;
  while(!timeFlag)
  {
    Serial.print(".");
    delay(500);                           //接続していなければ0.5秒待つ
    toutCnt++;
    if (toutCnt > 40)
    {
      //timeout 20秒
      Serial.println(" Timeout!");
      dispWifi(TEXT_NTP_TIMEOUT);
      return 1;
    }
  }
  printLocalTime();
  //RTCに時刻をセット
  time_t ti = time(nullptr) + 1;
  while(ti > time(nullptr))
  {
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
void printLocalTime(void)
{
  if(!getLocalTime(&timeInfo))
  {
    ESP_LOGE(TAG, "No time available (yet)");
    return;
  }
  Serial.println(&timeInfo, "%A, %B %d %Y %H:%M:%S");   //特別な書き方　printfとかにするとリセットかかったりするので注意
  //普通の書き方
  //ESP_LOGI(TAG, "NTP: %04d/%02d/%02d(%s) - ", (timeInfo.tm_year + 1900), (timeInfo.tm_mon + 1), timeInfo.tm_mday, week[timeInfo.tm_wday]);
  //ESP_LOGI(TAG, "%02d:%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

}


void getTimeStamp(char* txt)
{ //タイムスタンプ取得
/*
  //NTPより取得したタイム（WiFi切った後も動いている）
  if(!getLocalTime(&timeInfo)){
    ESP_LOGI(TAG, "No time available (yet)");
    return;
  } 
  sprintf(txt, "%02d%02d%02d-%02d%02d%02d", (timeInfo.tm_year - 100), (timeInfo.tm_mon + 1), timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  ESP_LOGI(TAG, "NTP: %s", txt);
*/  
  //RTCより
  M5.Rtc.getDate(&rtcDateDef);
  M5.Rtc.getTime(&rtcTimeDef);
  sprintf(txt, "%02d%02d%02d-%02d%02d%02d", (rtcDateDef.year - 2000), rtcDateDef.month, rtcDateDef.date, rtcTimeDef.hours, rtcTimeDef.minutes, rtcTimeDef.seconds);
  //ESP_LOGI(TAG, "RTC: %s", txt);
}

void getTimeText(char* txt)
{
  //タイムテキスト
  M5.Rtc.getDate(&rtcDateDef);
  M5.Rtc.getTime(&rtcTimeDef);
  sprintf(txt, "%02d/%02d/%02d %02d:%02d:%02d", (rtcDateDef.year - 2000), rtcDateDef.month, rtcDateDef.date, rtcTimeDef.hours, rtcTimeDef.minutes, rtcTimeDef.seconds);
  //ESP_LOGI(TAG, "RTC: %s", txt);
}


void generateFileName(void)
{ //filename
  uint8_t fTime[] = "000000-000000          ";

  getTimeStamp((char*)fTime);
  sprintf((char*)tamaFileName, "/tamabo%s.csv", fTime); //Global/////////////
}


// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.print(" get time:  ");
  timeFlag = 1;
}


//***** SD card ****************************************************

uint8_t sdInit(void)
{ //SDcard init
  //ret --> 2:err, 1:ok, 0:未（初回）

  if (sdStat == 1)
  { //SD準備設定済み
    return 1;
  }

  if (!SD.begin(GPIO_NUM_4, SPI, 20000000))
  { //25000000
    ESP_LOGI(TAG, "SD failed!");
    if (sdStat == 0)
    { //最初だけ表示と音で警告
      delay(300);
      dispSdcardStatus(1);
      M5.Speaker.tone(4000, 600);
    }
    return 2;//////////////////////
  }
  else
  {
    ESP_LOGI(TAG, "SD card OK!");
    if (sdStat != 0)  // == 2 とおなじこと
    { //SD mount OK
      M5.Speaker.tone(1500,30);
      dispSdcardStatus(0);
    }
  }

  fileGenerate();

  return 1;
}



uint8_t fileGenerate(void)
{ //ファイルネームを生成
  generateFileName();
  tamaFile = SD.open((char*)tamaFileName, FILE_WRITE);  //SDカードを抜き差しした後はエラーになってしまう
  //ESP_LOGI(TAG, "SDopen:%d", tamaFile);/////////
  if (tamaFile != 1)
  {
    ESP_LOGI(TAG, "SD card error!");
    tamaFile.close();
    return 2;
  }

  tamaFile.printf("DENKI,Tamabo,v.%s\n", (char*)fmVer);
  tamaFile.close();
  return 1;
} 
 

void sdDataSave(char* time, uint16_t mNum, uint8_t n, float* pos, float* load, uint16_t speed, float nukiIntegral)
{
  uint16_t i;

  tamaFile = SD.open((char*)tamaFileName, FILE_APPEND);
  if (tamaFile != 1)
  {
    ESP_LOGI(TAG, "SD error");
    sdStat = 2;
    SD.end();
    //エラーの時、SDマウントしなおしたいけれど、SDライブラリ側で再マウントできない仕様らしい
    tamaFile.close();
    dispSdcardStatus(1);
    M5.Speaker.tone(4000, 600);
    return;
  }
  time[8] = ',';  //日付と時刻を分ける
  tamaFile.printf("measure #,%d\n", mNum);
  tamaFile.printf("%s\n", time);
  tamaFile.printf("angle:,%4.1f ~,%4.1f deg\n", startAngleGet(), endAngleGet());
  tamaFile.printf("speed:,%6d,msec\n", speed);
  tamaFile.println("#, pos[mm], load[gf]");
  for (i = 0; i < n; i++)
  {
    tamaFile.printf("%5d, %6.3f, %6.2f\n", (i + 1), pos[i], load[i]);
  }
  tamaFile.printf("nukidan Integral:,%7.1f,gf-mm\n", nukiIntegral);
  tamaFile.println();
  tamaFile.close();
  ESP_LOGI(TAG, "SDcard data saved.");
}

/*
//------------ Bluetooth ----------------------------------------

void bluetoothSerialInit(void)
{
  //Bluetoothシリアル設定

  dispBtSerial(BTS_INIT);

#define noMACBOOK
#ifdef MACBOOK
  //マックブックへの接続（不安定？？）
  bool connected;
  String slaveName = "MacBook Pro 15";

  SerialBT.begin("DENKI-Tamabo-v3", true);
  connected = SerialBT.connect(slaveName);
  ESP_LOGI(TAG, "Bluetooth Serial connect --> BT device\"%s\" ", slaveName.c_str());

  if (connected)
  {
    ESP_LOGI(TAG, "port: '/dev/cu.Bluetooth-Incoming-Port' ");
    ESP_LOGI(TAG, " OK!");
    dispBtSerial(BTS_OK);
    dispBtSerial(BTS_PORT1);

  }
  else
  {
    SerialBT.connected(10000));  //timeout 10000msec
    ESP_LOGI(TAG, "failed.");
    dispBtSerial(BTS_TIMEOUT);
  }
#else
  //接続先を指定しない
  SerialBT.begin("DENKI-Tamabo-v3");
  ESP_LOGI(TAG, "Bluetooth Serial connecting --> BT device ");
  ESP_LOGI(TAG, "port: '/dev/cu.DENKI-Tamabo-v3-ESP32SPP'");
  dispBtSerial(BTS_OK);
  dispBtSerial(BTS_PORT2);
#endif
  SerialBT.println("*** DENKI Tamabo M5 v3 ***");

}

void btDataSend(char* time, uint16_t mNum, uint8_t n, float* pos, uint16_t speed, float* load)
{
  //Bluetoothシリアルでデータを送信
  uint16_t i;

  SerialBT.printf("measure #%d\n", mNum);
  SerialBT.printf("%s\n", time);
  SerialBT.printf("measure angle: %4.1f ~ %4.1f deg\n", startAngleGet(), endAngleGet());
  SerialBT.printf("speed: %6d msec\n", speed);
  SerialBT.println("#, pos[mm], load[gf]");
  for (i = 0; i < n; i++)
  {
    SerialBT.printf("%5d, %6.3f, %6.2f\n", (i + 1), pos[i], load[i]);
  }
  SerialBT.println();
  SerialBT.printf("nukidan Integral: %7.1fgf-mm\n", nukiIntegral);
  ESP_LOGI(TAG, "bluetooth serial data send.");

}


void btSerialRx(void)
{
  //BTSerial 受信
  if (!SerialBT.available())
  {
    return;
  }
  while (SerialBT.available())
  {
    Serial.write(SerialBT.read());
  }
  M5.Speaker.tone(2000, 50);
}


*/