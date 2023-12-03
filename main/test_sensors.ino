/*#include "LightSense.h"
#include "Adafruit_LTR329_LTR303.h"
#include "TempSense.h"

#define TEMPSENSE_PIN A3
#define DESIRED_TEMP 70

LightSense lux_sensor = LightSense();
TempSense temp_sensor = TempSense(TEMPSENSE_PIN);

void setup () {
    lux_sensor.setupLTR();
    temp_sensor.setTemp(DESIRED_TEMP);

    Serial.begin(9600);
}

uint16_t lux_reading;

void loop () {
    lux_reading = lux_sensor.getLux();
    Serial.print("CH0 Visible + IR: ");
    Serial.println(lux_reading);

    float temp_reading = temp_sensor.getTemp();
    Serial.print("Current Temp: ");
    Serial.println(temp_reading);

    float temp_diff = temp_sensor.getTempDiff();
    Serial.print("Current Temp Difference: ");
    Serial.println(temp_diff);

    Serial.println("");

    delay(1000);
}*/
