/*
display.cpp


2024.04.23

*/
#include "display.hpp"


uint8_t text[20];             //sprint用


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


void dispZeroSet(void){
  //玉位置ゼロセット
  M5.Display.setTextDatum(TR_DATUM);  //TopRight
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);
  sprintf((char*)text, "ゼロセット");
  M5.Display.drawString((char*)text, 309, 82, &fonts::lgfxJapanGothicP_12);
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






