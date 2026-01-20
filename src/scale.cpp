
/*
 * loadcell.cpp
 *
 * HX711 library for Arduino
 * https://github.com/bogde/HX711
 *
 * 
 * 2024.04.19
*/

#include "scale.hpp"

#define PIN_HX711_DOUT 13   //16..stack M5 BASIC v2.7
#define PIN_HX711_SCK  14   //17..stack M5 BASIC v2.7

HX711 scale;

void scaleInit(void)
{
  //[Full scale 500gf]
  const long LOADCELL_DIVIDER = 3192.f;   //adjust by 20gf,45gf,193gf OK! 
  const long LOADCELL_OFFSET = 0;

  //[Full scale 10kg]
  //const long LOADCELL_DIVIDER = 200; 
  //const long LOADCELL_OFFSET = 315600;

  scale.begin(PIN_HX711_DOUT, PIN_HX711_SCK);
  scale.set_scale(LOADCELL_DIVIDER);
  scale.set_offset(LOADCELL_OFFSET);
  scaleTare();   //zero set
}


float measLoad(uint8_t n)
{
  float load = scale.get_units(n);
  //ESP_LOGI(TAG, "Load: %.2f", load);
  return load;
}

void scaleTare(void)
{ //ゼロセット
  scale.tare(); 
}

