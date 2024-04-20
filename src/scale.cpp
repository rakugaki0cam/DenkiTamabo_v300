
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

void scaleInit(void){
    //const long LOADCELL_OFFSET = 50682624;
    const long LOADCELL_DIVIDER = 3192.f;   //adjust by 20gf,45gf,193gf OK!

    scale.begin(PIN_HX711_DOUT, PIN_HX711_SCK);
    scale.set_scale(LOADCELL_DIVIDER);
    //scale.set_offset(LOADCELL_OFFSET);
    scale.tare();   //zero set

    //Serial.print("Weight: ");
    //Serial.println(scale.get_units(5), 2);  // print the average of 5 readings from the ADC minus tare weight, divided by the SCALE parameter set with set_scale

/*
if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
*/


}

float measLoad(uint8_t n){
    float load = scale.get_units(10);
    Serial.println(load, 2); 
    return load;
}


/*
//non blocking

// 4. Acquire reading without blocking
if (loadcell.wait_ready_timeout(1000)) {
    long reading = loadcell.get_units(10);
    Serial.print("Weight: ");
    Serial.println(reading, 2);
} else {
    Serial.println("HX711 not found.");
}
*/
