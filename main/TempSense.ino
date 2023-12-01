#include "TempSense.h"

TempSense::TempSense(int pin) {
    pinMode(pin, INPUT);
    tempPin = pin;
}

float TempSense::getTemp() {
    //getting the voltage reading from the temperature sensor
    int tempReading = analogRead(tempPin);  
    
    // converting that reading to voltage
    float tempV = tempReading * 5.0 / 1024.0;
    
    // converting from 10 mv per degree wit 500 mV offset to degrees ((voltage - 500mV) times 100)
    float tempC = (tempV - 0.5) * 100 ;
    
    // now convert to Fahrenheit
    float tempF = (tempC * 9.0 / 5.0) + 32.0;

    currentTemp = tempF;
    return tempF;
}

void TempSense::setTemp(int inputTemp) {
    // set the desired temperature
    desTemp = inputTemp;
}

float TempSense::getTempDiff() {
    // calculate the difference between teh current and desired temperature
    float tempDiff = currentTemp - desTemp;

    return tempDiff;
}