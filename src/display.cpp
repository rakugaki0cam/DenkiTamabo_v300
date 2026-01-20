/*
display.cpp


2024.04.23

*/
#include "display.hpp"


uint8_t text[20];             //sprint用
//original color
uint16_t TFT_ENJI      = M5.Display.color565(0x6f, 0x20, 0x20); //エンジ色
uint16_t TFT_BG_SCREEN = M5.Display.color565(0x9e, 0x9d, 0x8c); //薄茶
uint16_t TFT_BG_TITLE  = M5.Display.color565(0x89, 0x5d, 0x37); //茶色
uint16_t TFT_BG_BUTTON = M5.Display.color565(0x60, 0x40, 0x20); //濃い茶色


void dispInit(void)
{
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
  //
  M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  //load
  M5.Display.setCursor(228, 65);
  M5.Display.printf("抜弾抵抗力");
  //tama position
  M5.Display.setCursor(228, 25);
  M5.Display.printf("玉位置");
  //peak load
  M5.Display.setTextColor(TFT_ENJI, TFT_BG_SCREEN);
  M5.Display.setCursor(228, 112);
  M5.Display.printf("抜弾ピーク値");
  dispLoadMax(-9999);
  //nozzle mark
  dispNozzle(NOZ_MARK, 0);
  //batV
  M5.Display.setFont(&fonts::lgfxJapanGothicP_8);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  M5.Display.setCursor(265, 190);
  M5.Display.print("バッテリ電圧");
  //button
  dispBtnA(MEAS_START);
  dispBtnB(TO_START);
  dispBtnC(BTNC_NOZZLE_SET);
}

//--- display --------------------

void dispMeasNum(uint16_t val)
{
  //測定番号
  M5.Display.setTextDatum(TR_DATUM);  //TopRight....print系では効かない
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_TITLE);
  sprintf((char*)text, "　#%3d", val);
  M5.Display.drawString((char*)text, 309, 0, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispPosition(float val)
{
  //玉位置[mm]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%6.2fmm", val);
  M5.Display.drawString((char*)text, 309, 40, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispLoad(float val)
{
  //抜き弾力[gf]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%9.0fgf", val);/////////////////////
  M5.Display.drawString((char*)text, 309, 82, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispZeroSet(void)
{
  //スケールゼロセット
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BROWN, TFT_BG_SCREEN);
  sprintf((char*)text, "　0セット");
  M5.Display.drawString((char*)text, 309, 82, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispLoadMax(float val)
{
  //抜き弾抵抗力ピーク値[gf]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_ENJI, TFT_BG_SCREEN);
  if (val <= -999){
    sprintf((char*)text, "　--- gf");
  }else{
    sprintf((char*)text, "　%5.0fgf", val);
  }
  M5.Display.drawString((char*)text, 309, 130, &fonts::lgfxJapanGothicP_20);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}


void dispNukiInteg(float val)
{
  //抜き弾抵抗力積分値[gf-mm]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%5.0fgf-mm", val);
  M5.Display.drawString((char*)text, 309, 160, &fonts::lgfxJapanGothicP_12);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}


//
void dispBatV(float val)
{
  //バッテリー電圧[V]
  uint8_t text[20];

  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%4.2fV", val);
  M5.Display.drawString((char*)text, 309, 200, &fonts::lgfxJapanGothicP_12);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft

}

void dispSdcardStatus(uint8_t stat)
{
  //SDカード無しの警告
  //stat 0:OK, 1:fail
  #define SD_DISP_X 0
  #define SD_DISP_Y 200

  if (stat)
  {
    //SD fail
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_MAGENTA);
    M5.Display.setCursor(SD_DISP_X, SD_DISP_Y);
    M5.Display.println(" SD Fail! ");
  }
  else
  {
    //SD OK!
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_BLACK, TFT_GREEN);
    M5.Display.setCursor(SD_DISP_X, SD_DISP_Y);
    M5.Display.println(" SDcard OK! ");
  }
}


void dispTamaPos(tama_pos_t pos)
{
  //玉位置の表示
  M5.Display.setCursor(224, 40);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  switch(pos)
  {
    case START_POS:
      M5.Display.print("スタート点　");
      break;
    case CENTER1_POS:
    case CENTER2_POS:
      M5.Display.print("センタ位置　");
      break;
    case END_POS:
      M5.Display.print("エンド位置　");
      break;
  }
}


void dispWifi(wifi_stat_t stat)
{
  //WiFi接続状況
  #define WIFI_DISP_X0  0
  #define WIFI_DISP_X1  40
  #define WIFI_DISP_Y0  30
  #define WIFI_DISP_Y1  50
  #define WIFI_DISP_Y2  80
  #define WIFI_DISP_Y3  100


  switch(stat)
  {
    case TEXT_WIFI:
      //WiFi
      M5.Display.setFont(&fonts::lgfxJapanGothic_16);
      M5.Display.setCursor(WIFI_DISP_X0, WIFI_DISP_Y0);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.print("WiFi ");
      break;
    case TEXT_DOT:
      //waiting1
      M5.Display.setCursor(WIFI_DISP_X1, WIFI_DISP_Y0);
      M5.Display.print("...");
      break;
    case TEXT_DOT_NONE:
      //waiting2
      M5.Display.setCursor(WIFI_DISP_X1, WIFI_DISP_Y0);
      M5.Display.print("　　");
      break;
    case TEXT_WIFI_CONECT:
      //conect
      M5.Display.setCursor(WIFI_DISP_X1, WIFI_DISP_Y0);
      M5.Display.print(".. 接続OK");
      break;
    case TEXT_WIFI_TIMEOUT: 
      //timeout 
      M5.Display.setCursor(WIFI_DISP_X1, WIFI_DISP_Y0);
      M5.Display.print(".. 接続不可!");
      break;
    case TEXT_IP: 
      //IP adress 
      M5.Display.setCursor(WIFI_DISP_X0, WIFI_DISP_Y1);
      M5.Display.print("IP: ");
      M5.Display.println(WiFi.localIP());
      break;
    case TEXT_NTP:
      //NTP date time  
      M5.Display.setCursor(WIFI_DISP_X0, WIFI_DISP_Y2);
      M5.Display.print("NTP->RTC: ");
      break;
    case TEXT_DATE_TIME:
      //date time  
      getTimeStamp((char*)text);
      M5.Display.println((char*)text);
      break;
    case TEXT_NTP_TIMEOUT:
      //timeout
      M5.Display.print("取得不可!");
      break;
    case TEXT_WIFI_OK:
      //OK
      M5.Display.setCursor(WIFI_DISP_X0, WIFI_DISP_Y3);
      M5.Display.print("OK");
      break;  
  }

}


void dispBtSerial(bts_stat_t stat)
{
  //WiFi接続状況
  #define BTS_DISP_X0   0
  #define BTS_DISP_X1   80
  #define BTS_DISP_Y0   120
  
  switch(stat)
  {
    case BTS_INIT:
      //WiFi
      M5.Display.setFont(&fonts::lgfxJapanGothic_16);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(BTS_DISP_X0, BTS_DISP_Y0);
      M5.Display.print("BTserial ");
      break;
    case BTS_OK: 
      //OK 
      M5.Display.setCursor(BTS_DISP_X1, BTS_DISP_Y0);
      M5.Display.println(".. 接続OK");
      break;
    case BTS_TIMEOUT:
      //timeout 
      M5.Display.setCursor(BTS_DISP_X1, BTS_DISP_Y0);
      M5.Display.println(".. 接続不可!");
      break; 
     case BTS_PORT1:
      //ポート名1
      M5.Display.setFont(&fonts::lgfxJapanGothic_12);
      M5.Display.println("ポート:/dev/cu.");
      M5.Display.println("Bluetooth-Incoming-Port");
      break;    
    case BTS_PORT2:
      //ポート名2
      M5.Display.setFont(&fonts::lgfxJapanGothic_12);
      M5.Display.println("ポート:/dev/cu.");
      M5.Display.println("DENKI-Tamabo-v3-ESP32SPP");
      break;
    
  }
}


void dispNozzle(noz_stat_t stat, float val)
{
  //ノズル測定
  #define DISP_X0  0
  #define DISP_X1  40
  #define DISP_X8  226
  #define DISP_Y0  30
  #define DISP_Y1  60
  #define DISP_Y2  76
  #define DISP_Y3  92
  #define DISP_Y4  114
  #define DISP_Y7  190
  #define DISP_Y8  200

  switch(stat)
  {
    case NOZ_TITLE:
      //WiFi
      M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X0, DISP_Y0);
      M5.Display.println("ノズル位置の検出");
      break;
    case NOZ_EXP1:
      //explanation1
      M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X0, DISP_Y1);
      //M5.Display.println("1.ホップ調整を最弱にする。");
      M5.Display.println("ノズル先端に玉が軽く当たるように");
      M5.Display.println("棒の長さを調整する。");
      break;
    case NOZ_ABORT:
      //中止
      M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
      M5.Display.setTextColor(TFT_RED, TFT_BG_SCREEN);

      M5.Display.setCursor(DISP_X0 + 144, DISP_Y0);
      M5.Display.println("中止");
      break;
    case NOZ_MEAS:
      //nozzle position measure
      M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X0, DISP_Y0);
      M5.Display.println("ノズル位置測定中");
      M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
      M5.Display.println("　玉を奥へ移動中");
      break;
    case NOZ_PACKING:
      //パッキン端検出中
      M5.Display.println("　　パッキン端検出中");
      break;
    case NOZ_PACKING_OK:
      //パッキン抜け出し位置表示
      M5.Display.println("　　　パッキンから抜け出しを確認");
      M5.Display.printf("　　　位置:%6.2fmm\n", val);      
      break;
    case NOZ_NOZZLE:
      //ノズル検出中
      M5.Display.println("　　ノズル検出中");
      break;
    case NOZ_DETECT:
      //OK
      M5.Display.println("　　　ノズル先端を確認");
      M5.Display.printf("　　　位置:%6.2fmm\n", val);
      break;
    case NOZ_TUMADUKI:
      M5.Display.printf("　　　つまづき距離:%6.2fmm\n", val); 
      break; 
    case NOZ_ERR:
      M5.Display.println("ノズル先端検出できず");
      break;
    case NOZ_OK:
      M5.Display.println("OK");
      break;
    case NOZ_RESET:
      //ノズル設定をリセット
      M5.Display.setFont(&fonts::lgfxJapanGothicP_12);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X0, DISP_Y0);
      M5.Display.print("ノズル位置設定を無効にしました。");
      break;
    case NOZ_MARK:
      //nozzle set mark
      M5.Display.setFont(&fonts::lgfxJapanGothicP_8);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X8, DISP_Y7);
      M5.Display.print("ノズル");
      break;  
    case NOZ_EN:
      //nozzle set mark enable
      M5.Display.setFont(&fonts::lgfxJapanGothic_16);
      M5.Display.setTextColor(TFT_GREEN, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X8, DISP_Y8);
      M5.Display.print("●");
      break;  
    case NOZ_DIS:
      //nozzle set mark disable
      M5.Display.setFont(&fonts::lgfxJapanGothic_16);
      M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
      M5.Display.setCursor(DISP_X8, DISP_Y8);
      M5.Display.print("◯");
      break;
  }

}


//--------- ボタン -------------------------
void dispBtnA(btn_a_name_t name)
{
  //ボタンAの名前の表示
  M5.Display.setTextDatum(BC_DATUM);  //BottomCenter
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_BUTTON);
  switch(name)
  {
    case MEAS_START:
      sprintf((char*)text, "　測定開始　");
      break;
    case MEAS_READY:
      sprintf((char*)text, "　　移動中　");
      break;
    case MEAS_RUNNING:
      sprintf((char*)text, "　測定中　　");
      break;
    case MEAS_COMPLETE:
      sprintf((char*)text, "　測定完了　");
      break;
  }
  M5.Display.drawString((char*)text, 50, 240, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}


void dispBtnB(btn_b_name_t name)
{
  //ボタンBの名前の表示
  M5.Display.setTextDatum(BC_DATUM);  //BottomCenter
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_BUTTON);
  switch(name){
    case TO_START:
      sprintf((char*)text, "スタート位置");
      break;
    case TO_CENTER:
      sprintf((char*)text, "センターへ　");
      break;
    case TO_END:
      sprintf((char*)text, "エンド位置へ");
      break;
  }
  M5.Display.drawString((char*)text, 160, 240, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}


void dispBtnC(btn_c_name_t name)
{
  //ボタンCの名前の表示
  M5.Display.setTextDatum(BC_DATUM);  //BottomCenter
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_BUTTON);
  switch(name)
  {
    case BTNC_NOZZLE_SET:
      sprintf((char*)text, "ノズル設定　");
      break;
    case BTNC_PUSH_TO_START:
      sprintf((char*)text, "　設定開始　");
      break;
    case BTNC_NULL:
      sprintf((char*)text, "　　　　　　");
      break;
    case BTNC_RUNNING:
      sprintf((char*)text, "ノズル測定中");
      break;
    case BTNC_PACKING:
      sprintf((char*)text, "隙間測定中");
      break;
    case BTNC_NOZZLE_RESET:
      sprintf((char*)text, "　リセット　");
      break;
  }
  M5.Display.drawString((char*)text, 270, 240, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}


