/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
 *
 * Derived from https://github.com/adafruit/Adafruit_MPL115A2:
 * Copyright (c) 2012 - Adafruit Industries
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PressureSensor.h"

/* 
 *  Initialize i2c io expander to output mode and get coefficients of 
 *  pressure sensor 
 */
void PressureSensor::setup() {
    writeReg(IO_EXPANDER_ADDR, IO_CONFIG_REG, 0x00);
    writeReg(IO_EXPANDER_ADDR, IO_OUTPUT_REG, 0x00);

    initializeSensors();
}

/* Get readings from all pressure sensors */
void PressureSensor::sample() {
    maxReading = 0;
    minReading = 1023;
    
    for (int i = 0; i < NUM_SENSORS; i++) {
        enableSensor(i);
        readPressure(i);

        startConversion();
    }
}

/* Return minimum reading */
float PressureSensor::getMin() {
  return minReading;
}

/* Return maximum reading */
float PressureSensor::getMax() {
  return maxReading;
}

/* Return array of readings from all pressure sensors */
float* PressureSensor::getReadings() {
    return readings;
}

/* Get average 'steady state' values for each sensor */
float* PressureSensor::getAverages() {
  return average;
}

/* Compute 'steady state' values for each sensor */
void PressureSensor::calculateAverages() {
  for (int i = 0; i < 10; i++) {
    sample();

    for (int j = 0; j < NUM_SENSORS; j++) {
      average[j] += readings[j];
    }
  }

  for (int i = 0; i < NUM_SENSORS; i++) {
    average[i] /= 10;
  }
}

/*
 * Initialize each sensor object by obtaining the
 * coefficients from each corresponding sensor
 */
void PressureSensor::initializeSensors() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        enableSensor(i);
        readCoefficients(i);

        startConversion();
    }
}

/* For a specified sensor, get the coefficients */
void PressureSensor::readCoefficients(int i) {
    int a0coeff;
    int b1coeff;
    int b2coeff;
    int c12coeff;

    Wire.beginTransmission(MPL_ADDR);
    Wire.write(MPL_COEFF_REG);
    Wire.endTransmission();

    Wire.requestFrom(MPL_ADDR, 8);
    a0coeff = ((uint16_t) Wire.read() << 8) | Wire.read();
    b1coeff = ((uint16_t) Wire.read() << 8) | Wire.read();
    b2coeff = ((uint16_t) Wire.read() << 8) | Wire.read();
    c12coeff = (((uint16_t) Wire.read() << 8) | Wire.read()) >> 2;

    a0[i] = (float)a0coeff / 8;
    b1[i] = (float)b1coeff / 8192;
    b2[i] = (float)b2coeff / 16384;
    c12[i] = (float)c12coeff;
    c12[i] /= 4194304.0f;
}

/* Start a pressure conversion for the current enabled sensor */
void PressureSensor::startConversion() {
    Wire.beginTransmission(MPL_ADDR);
    Wire.write(MPL_CONVERT);
    Wire.write(0x00);
    Wire.endTransmission();
}

/* Get the pressure reading from the specified sensor */
void PressureSensor::readPressure(int i) {
    uint16_t pressure, temp;
    float pComp;

    Wire.beginTransmission(MPL_ADDR);
    Wire.write(MPL_PRESSURE_REG);
    Wire.endTransmission();

    Wire.requestFrom(MPL_ADDR, 4);

    pressure = ((uint16_t)Wire.read() << 8) | Wire.read() >> 6;
    temp = (((uint16_t)Wire.read() << 8) | Wire.read()) >> 6;

    pComp = a0[i] + (b1[i] + c12[i] * temp) * pressure + b2[i] * temp;

    readings[i] = (65.0/1023.0) * pComp + 50;

    maxReading = max(pComp, maxReading);
    minReading = min(pComp, minReading);
}

/* Enable the reset pin for the specified sensor */
void PressureSensor::enableSensor(int i) {
    writeReg(IO_EXPANDER_ADDR, IO_OUTPUT_REG, (1 << pressurePins[i]));
}

/* Write a value to an io expander register */
void PressureSensor::writeReg(char port, char reg, char val) {
    Wire.beginTransmission(IO_EXPANDER_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}
