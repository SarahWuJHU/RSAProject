#include "LightSense.h"
#include "Adafruit_LTR329_LTR303.h"

LightSense::LightSense() {
    // pinMode(pin, INPUT);
    // luxPin = pin;
}

void LightSense::setupLTR() {
    ltr = Adafruit_LTR303();
    ltr.begin();

    ltr.setGain(LTR3XX_GAIN_2);
    switch (ltr.getGain()) {
        case LTR3XX_GAIN_1: break;
        case LTR3XX_GAIN_2: break;
        case LTR3XX_GAIN_4: break;
        case LTR3XX_GAIN_8: break;
        case LTR3XX_GAIN_48: break;
        case LTR3XX_GAIN_96: break;
    }

    ltr.setIntegrationTime(LTR3XX_INTEGTIME_100);
    switch (ltr.getIntegrationTime()) {
        case LTR3XX_INTEGTIME_50: break;
        case LTR3XX_INTEGTIME_100: break;
        case LTR3XX_INTEGTIME_150: break;
        case LTR3XX_INTEGTIME_200: break;
        case LTR3XX_INTEGTIME_250: break;
        case LTR3XX_INTEGTIME_300: break;
        case LTR3XX_INTEGTIME_350: break;
        case LTR3XX_INTEGTIME_400: break;
    }

    ltr.setMeasurementRate(LTR3XX_MEASRATE_200);
    switch (ltr.getMeasurementRate()) {
        case LTR3XX_MEASRATE_50: break;
        case LTR3XX_MEASRATE_100: break;
        case LTR3XX_MEASRATE_200: break;
        case LTR3XX_MEASRATE_500: break;
        case LTR3XX_MEASRATE_1000: break;
        case LTR3XX_MEASRATE_2000: break;
    }
}

uint16_t LightSense::getLux() {
    bool valid;
    uint16_t visible_plus_ir, infrared;

    if (ltr.newDataAvailable()) {
        valid = ltr.readBothChannels(visible_plus_ir, infrared);
        if (valid) {
            currentLux = visible_plus_ir;
        }
    }

    return currentLux;
}