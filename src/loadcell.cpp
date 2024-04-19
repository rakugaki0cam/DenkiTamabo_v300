


#include "loadcell.hpp"

HX711 scale;

void loadcellInit(void){
    // 1. HX711 circuit wiring
    const int LOADCELL_DOUT_PIN = 13;
    const int LOADCELL_SCK_PIN = 14;

    // 2. Adjustment settings
    //const long LOADCELL_OFFSET = 50682624;
    const long LOADCELL_DIVIDER = 3192.f;

    // 3. Initialize library
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(LOADCELL_DIVIDER);
    //scale.set_offset(LOADCELL_OFFSET);
    scale.tare();   //zero set

    // 4. Acquire reading
    Serial.print("Weight: ");
    M5.Lcd.clearDisplay(0);
    Serial.println(scale.get_units(5), 2);// print the average of 5 readings from the ADC minus tare weight, divided by the SCALE parameter set with set_scale

/*
if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
*/


}

void dispLoad(void){
    Serial.println(scale.get_units(10), 2);
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
