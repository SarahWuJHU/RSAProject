#ifndef TEMPSENSE_H
#define TEMPSENSE_H

class TempSense {
    public:
        TempSense(int pin);
        float getTemp();
        void setTemp(int inputTemp);
        float getTempDiff();

    private:
        int tempPin;
        float currentTemp;
        float desTemp;
};

#endif