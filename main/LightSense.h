#include "Adafruit_LTR329_LTR303.h"
#ifndef LIGHTSENSE_H
#define LIGHTSENSE_H

class LightSense {
    public:
        // LightSense(int pin);
        LightSense();
        void setupLTR();
        uint16_t getLux();

    private:
        int luxPin;
        uint16_t currentLux;
        Adafruit_LTR303 ltr;
};

#endif
