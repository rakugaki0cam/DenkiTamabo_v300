


#include "loadcell.hpp"

HX711 load1;

void loadcellInit(void){
    // 1. HX711 circuit wiring
    const int LOADCELL_DOUT_PIN = 2;
    const int LOADCELL_SCK_PIN = 3;

    // 2. Adjustment settings
    const long LOADCELL_OFFSET = 50682624;
    const long LOADCELL_DIVIDER = 5895655;

    // 3. Initialize library
    load1.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    load1.set_scale(LOADCELL_DIVIDER);
    load1.set_offset(LOADCELL_OFFSET);

    // 4. Acquire reading
    Serial.print("Weight: ");
    M5.Lcd.clearDisplay(0);
    Serial.println(load1.get_units(10), 2);

}

void dispLoad(void){
    Serial.println(load1.get_units(10), 2);
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
