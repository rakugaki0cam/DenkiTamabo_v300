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


void dispInit(void){
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
  //batV
  M5.Display.setFont(&fonts::lgfxJapanGothicP_8);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  M5.Display.setCursor(265, 190);
  M5.Display.print("バッテリ電圧");

  //button
  M5.Display.setTextDatum(BC_DATUM);  //BottomCenter
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_BUTTON);
  //Btn A
  sprintf((char*)text, "　測定開始　");
  M5.Display.drawString((char*)text, 50, 240, &fonts::lgfxJapanGothicP_16);
  //Btn B
  sprintf((char*)text, "スタート位置");
  M5.Display.drawString((char*)text, 160, 240, &fonts::lgfxJapanGothicP_16);
  //Btn C
  sprintf((char*)text, "　ノズル設定");
  M5.Display.drawString((char*)text, 270, 240, &fonts::lgfxJapanGothicP_16);
  //
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft

}

//--- display --------------------

void dispMeasNum(uint16_t val){
  //測定番号
  M5.Display.setTextDatum(TR_DATUM);  //TopRight....print系では効かない
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_TITLE);
  sprintf((char*)text, "　#%3d", val);
  M5.Display.drawString((char*)text, 309, 0, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispPosition(float val){
  //玉位置[mm]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%6.2fmm", val);
  M5.Display.drawString((char*)text, 309, 40, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispLoad(float val){
  //抜き弾力[gf]
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%6.1fgf", val);
  M5.Display.drawString((char*)text, 309, 82, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispZeroSet(void){
  //スケールゼロセット
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BROWN, TFT_BG_SCREEN);
  sprintf((char*)text, "　0セット");
  M5.Display.drawString((char*)text, 309, 82, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft = default
}


void dispLoadMax(float val){
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


void dispBatV(float val){
  //バッテリー電圧[V]
  uint8_t text[20];

  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "　%4.2fV", val);
  M5.Display.drawString((char*)text, 309, 200, &fonts::lgfxJapanGothicP_12);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft

}

void dispSdcardStatus(uint8_t stat){
  //SDカード無しの警告
  #define SD_DISP_X 0
  #define SD_DISP_Y 200

  if (stat){
    //SD fail
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_MAGENTA);
    M5.Display.setCursor(SD_DISP_X, SD_DISP_Y);
    M5.Display.println(" SD Fail! ");
  }else{
    //SD OK!
    M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
    M5.Display.setTextColor(TFT_BLACK, TFT_GREEN);
    M5.Display.setCursor(SD_DISP_X, SD_DISP_Y);
    M5.Display.println(" SDcard OK! ");
  }
}


void dispBtnB(btn_b_name_t name){
  //ボタンBの名前の表示
  uint8_t txt[20];

  M5.Display.setTextDatum(BC_DATUM);  //BottomCenter
  M5.Display.setTextColor(TFT_WHITE, TFT_BG_BUTTON);
  switch(name){
    case TO_START:
      sprintf((char*)txt, "スタート点へ");
      break;
    case TO_CENTER:
      sprintf((char*)txt, "センターへ　");
      break;
    case TO_END:
      sprintf((char*)txt, "エンド位置へ");
      break;
  }
  M5.Display.drawString((char*)txt, 160, 240, &fonts::lgfxJapanGothicP_16);
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft
}


void dispTamaPos(tama_pos_t pos){
  //玉位置の表示
  M5.Display.setCursor(224, 40);
  M5.Display.setFont(&fonts::lgfxJapanGothicP_16);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  switch(pos){
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


