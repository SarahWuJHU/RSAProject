#include "Adafruit_LTR329_LTR303.h"

class LightSense {
    public:
        LightSense(int pin);
        void setupLTR();
        float getLux();

    private:
        int luxPin;
        uint16_t currentLux;
        Adafruit_LTR329 ltr;
};