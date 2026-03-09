/*
*   DenkiTamabo
*
*   GRAPH
*
*   2024.04.22
*/
#include "graph.hpp"


//画面上の座標
uint16_t X0_CLS = 0;        //グラフ全面 カラー:TFT_BG_SCREEN
uint16_t Y0_CLS = 20;
uint16_t X_SIZE_CLS = 210;
uint16_t Y_SIZE_CLS = 200;
//グラフ
uint16_t X0 = 30;           //左上座標
uint16_t Y0 = 30;        
uint16_t X_SIZE = 170;      //大きさ
uint16_t Y_SIZE = 170;
uint16_t X1 = X0 + X_SIZE;  //右下座標
uint16_t Y1 = Y0 + Y_SIZE;
//データの数値
uint16_t xScale = 1;
int16_t xMin = 0;
int16_t xMax = 9;
uint16_t yScale = 100;
int16_t yMin = -100;
int16_t yMax = 500;
//縮尺変換係数
float Xconv = (float)X_SIZE / (xMax - xMin);
float Yconv = (float)Y_SIZE / (yMax - yMin);

static int16_t Xbefore[2];  //前回のプロット点
static int16_t Ybefore[2];  //前回のプロット点



void graphInit(void)
{ //graph draw
  uint8_t   text[10];
  int16_t   x, y;     //測定値の値
  uint16_t  Xdisp, Ydisp;  //画面上の座標
  uint16_t  TFT_N_GRAY = M5.Display.color565(0x40, 0x40, 0x40);
  //
  M5.Display.setColor(TFT_BG_SCREEN);
  M5.Display.fillRect(X0_CLS, Y0_CLS, X_SIZE_CLS, Y_SIZE_CLS);
  //
  M5.Display.setColor(TFT_BLACK);
  M5.Display.fillRect(X0, Y0, X_SIZE, Y_SIZE);
  //text
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextColor(TFT_BLACK, TFT_BG_SCREEN);

  //y軸
  M5.Display.setTextDatum(MR_DATUM);  //MiddleRight
  for (y = yMin; y <= yMax; y += yScale)
  {
    Ydisp = Y1 - (float)(y - yMin) * Yconv;
    //ESP_LOGD(TAG, "y:%d Ydisp:%d", y, Ydisp);
    M5.Display.setColor(TFT_N_GRAY);
    M5.Display.drawLine(X0, Ydisp, X1, Ydisp);
    sprintf((char*)text, "%d", y);
    M5.Display.drawString((char*)text, X0, Ydisp, &fonts::Font2);
  }

  //x軸
  M5.Display.setTextDatum(TC_DATUM);  //TopCenter
  for (x = xMin; x <= xMax; x += xScale)
  {
    Xdisp = (float)(x - xMin) * Xconv + X0;
    M5.Display.setColor(TFT_N_GRAY);
    M5.Display.drawLine(Xdisp, Y0, Xdisp, Y1);
    sprintf((char*)text, "%d", x);
    M5.Display.drawString((char*)text, Xdisp, Y1, &fonts::Font2);
  }
  
  M5.Display.setTextDatum(TL_DATUM);  //TopLeft (default)

  //外枠
  M5.Display.setColor(TFT_WHITE);
  M5.Display.drawRect(X0, Y0, X_SIZE, Y_SIZE);

  Xbefore[0] = X0;  //ラインの初期位置
  Ybefore[0] = Y1;
  Xbefore[1] = X0;  //ラインの初期位置
  Ybefore[1] = Y1;

}


void graphPlot(uint8_t ch, float x, float y)
{ //グラフにデータをプロット
  uint16_t Xdisp[2], Ydisp[2];        //画面上の座標

  Xdisp[ch - 1] = (uint16_t)(X0 + (x - xMin) * Xconv);
  //
  y = (y > yMax) ? yMax : y;
  y = (y < yMin) ? yMin : y;
  Ydisp[ch - 1]  = (uint16_t)(Y1 - (y - yMin) * Yconv);
  //
  if (ch == 1)
  {
    M5.Display.setColor(M5.Display.color565(0x33, 0xff, 0x33));
  }
  else if (ch == 2)
  {
    M5.Display.setColor(M5.Display.color565(0x33, 0x33, 0xff));
  }

  M5.Display.drawLine(Xbefore[ch - 1], Ybefore[ch - 1], Xdisp[ch - 1], Ydisp[ch - 1]);
  //
  ESP_LOGD(TAG, "before X,Y %4d %4d   X,Y %4d %4d", Xbefore[ch - 1], Ybefore[ch - 1], Xdisp[ch - 1], Ydisp[ch - 1]);

  Xbefore[ch - 1] = Xdisp[ch - 1];
  Ybefore[ch - 1] = Ydisp[ch - 1];
  
}


void graphAreaClear(void)
{
  M5.Display.setColor(TFT_BG_SCREEN);
  M5.Display.fillRect(X0_CLS, Y0_CLS, X_SIZE_CLS, Y_SIZE_CLS);
}


