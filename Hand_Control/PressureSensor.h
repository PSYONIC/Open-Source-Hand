#ifndef PRESSURE_H
#define PRESSURE_H

#include "Config.h"
#include <Wire.h>

#define IO_EXPANDER_ADDR        (0x27)
#define IO_CONFIG_REG           (0x03)
#define IO_OUTPUT_REG           (0x01)


#define MPL_ADDR                (0x60)
#define MPL_COEFF_REG           (0x04)
#define MPL_CONVERT             (0x12)
#define MPL_PRESSURE_REG        (0x00)

class PressureSensor {
public:
    void setup();

    void sample();

    void calculateAverages();

    float* getReadings();
    float* getAverages();

    float getMax();
    float getMin();

    float readings[NUM_SENSORS];
    float average[NUM_SENSORS];

private:
    

    float maxReading;
    float minReading;

    float a0[NUM_SENSORS];
    float b1[NUM_SENSORS];
    float b2[NUM_SENSORS];
    float c12[NUM_SENSORS];

    void enableSensor(int i);
    void readCoefficients(int i);
    void initializeSensors();
    void startConversion();
    void readPressure(int i);

    void writeReg(char port, char reg, char val);
    void readReg(char port, char reg, char val);
};

#endif
