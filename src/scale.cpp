
/*
 * loadcell.cpp
 *
 * HX711 library for Arduino
 * https://github.com/bogde/HX711
 *
 * 
 * 2024.04.19
 * 
 * 2026.03.08 loadcell2追加
*/

#include "scale.hpp"

#define PIN_HX711_DOUT 13   //16..stack M5 BASIC v2.7
#define PIN_HX711_SCK  14   //17..stack M5 BASIC v2.7
#define PIN_HX711_2_DOUT 26
#define PIN_HX711_2_SCK  25

HX711 scale;
HX711 scale2;


void scaleInit(void)
{
  //[Full scale 500gf]
  const long LOADCELL_DIVIDER = 3192.f;   //adjust by 20gf,45gf,193gf OK! 
  const long LOADCELL_OFFSET = 0;
  //scale2  [Full scale 500gf]
  const long LOADCELL2_DIVIDER = 3192.f;   //adjust by 20gf,45gf,193gf OK! 
  const long LOADCELL2_OFFSET = 0;

  //[Full scale 10kg]
  //const long LOADCELL_DIVIDER = 200; 
  //const long LOADCELL_OFFSET = 315600;

  scale.begin(PIN_HX711_DOUT, PIN_HX711_SCK);
  scale.set_scale(LOADCELL_DIVIDER);
  scale.set_offset(LOADCELL_OFFSET);

  scale2.begin(PIN_HX711_2_DOUT, PIN_HX711_2_SCK);
  scale2.set_scale(LOADCELL2_DIVIDER);
  scale2.set_offset(LOADCELL2_OFFSET);

  //
  scaleTare(1);   //zero set
  scaleTare(2);   //zero set
}


float measLoad(uint8_t ch, uint8_t n)
{
  float load;

  if (ch == 1)
  {
    load = scale.get_units(n);
  }
  if (ch == 2)
  {
    load = scale.get_units(n);
  }
  //ESP_LOGI(TAG, "Load: %.2f", load);
  return load;
}


void scaleTare(uint8_t ch)
{ //ゼロセット
  if (ch == 1)
  {
    scale.tare();
    return;
  }
  if (ch == 2)
  {
    scale2.tare(); 
    return;
  }
}

