/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
 * Author: Aadeel Akhtar <aakhta3@illinois.edu>
 * Author: Mary Nguyen <hnguyn10@illinois.edu>
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

#include <i2c_t3.h>
 
#include "Config.h"
#include "Finger.h"
#include "Hand.h"
#include "PressureSensor.h"
#include "HAND_CONFIG.h"

Hand hand;
PressureSensor pressureSensors;

int targets[NUM_FINGERS] = {};
int serialTarget = 0;
classes prevClass = NIL;

elapsedMicros deltaT;

int errorCount = 0;

bool socketDisconnect = false;

/* Initate everything */
void setup() {
//    Wire.setDefaultTimeout(10000);
//    Wire.begin();
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
    analogWriteResolution(10);
    
    Serial.begin(9600);
    delay(1000);

    pinMode(12, OUTPUT);
    pinMode(28, OUTPUT);
    pinMode(27, OUTPUT);
    
    digitalWrite(12, HIGH);
    deltaT = 0;
    hand.setupMotorPins();

    pressureSensors.setup();
    pressureSensors.calculateAverages();
    
    delay(2000);
    Serial.println("reset start");
//    hand.reset();

    

    for (int i = 0; i < NUM_FINGERS; i++) {
      pinMode(encoderAPins[i], INPUT);
      pinMode(encoderBPins[i], INPUT);
    }
//= {13, 2, 7, 31, 24, 17};
//{11, 14, 8, 26, 33, 16}
    //For some reason the pins had to be set explicitly instead of from the encoder pin array directly
    attachInterrupt(digitalPinToInterrupt(13), Hand::Encoder0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(14), Hand::Encoder1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(8), Hand::Encoder2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(26), Hand::Encoder3, CHANGE);
    attachInterrupt(digitalPinToInterrupt(33), Hand::Encoder4, CHANGE);
    attachInterrupt(digitalPinToInterrupt(16), Hand::Encoder5, CHANGE);

    Serial.println("Encoder interrupts attached");

        hand.close();
//Serial.println("Encoder positions calibrated.");
}

/* Event loop */
void loop() {
  pressureSensors.sample();
  
  if (Serial.available() > 0) {
      handleSerial();
  }

  handlePressure();

  updateSocketStatus();

  hand.update();

  //Handle any serial events
  switch (serialTarget) {
    case 1:
      hand.setTarget(targets);
      Serial.print("Set Target");
      break;
      
    case 2:
      hand.setGrasp((classes)targets[0]);
      Serial.print("Set Grasp");
      break;
      
    case 3:
      train();
      break;
  }
  serialTarget = 0;

}

/* Get pressure readings and decide if contact reflexes needed  */
void handlePressure() {
  float * readings = pressureSensors.getReadings();
  float * averages = pressureSensors.getAverages();

  bool cond = 0;

  for (int i = 0; i < NUM_SENSORS; i++) {
    float ratio = abs(readings[i] - averages[i]) / averages[i];

    if (ratio > PRESSURE_THRESHOLD) {
      cond = 1;
      break;
    }
  }


  hand.limitSpeed(cond);

  if (socketDisconnect) {
    cond = 0;
  }
//  Wire.beginTransmission(STIM_ADDR);
//  Wire.write(cond);
//  Wire.endTransmission(I2C_STOP, 50000);
}

/* Reads a register from the socket */
char readFromRegister(char readRegister) {
  Wire.beginTransmission(SOCKET_ADDR);
  Wire.write(readRegister);
  Wire.endTransmission(I2C_STOP, 50000);

  Wire.requestFrom(SOCKET_ADDR, 1, I2C_STOP, 50000);

  return Wire.read(); 
}

/* Initiate training in the socket */
void train() {
  Wire.beginTransmission(SOCKET_ADDR);
  Wire.write(0);
  Wire.write(0);
  Wire.endTransmission();
}

/* Check the status of the socket and update the hand accordingly */
void updateSocketStatus() {
  operationState_t state = (operationState_t)readFromRegister(CONTROL);
  classes handGrasp = (classes)readFromRegister(CLASS);
  int timeE = deltaT;
  Serial.println(timeE);
  deltaT = 0;
//  float velocity = (float)(readFromRegister(VELOCITY)/255);
socketDisconnect = false;

if (getI2CStatus() == -1) {
  socketDisconnect = true;
  digitalWrite(28, HIGH);

  Wire.beginTransmission(STIM_ADDR);
  Wire.write(0);
  Wire.endTransmission(I2C_STOP, 50000);

  while(true);

} else {
//  errorCount = 0;
  digitalWrite(28, LOW);
  
    switch(state) {
      case TRAINING:
        if (prevClass != handGrasp) { //check if we have switched classes
          hand.setGrasp((handGrasp));
        }
      break;
  
      case RUNNING:
        hand.setGrasp(handGrasp);
      break;
  
      default: break;
    }
  }
}

/* Handle any serial commands from the computer */
void handleSerial() {
    int32_t val = 0;
    int i = 0;
    if (Serial.available()) {
        int temp = Serial.read();

        switch(temp) {
        case '(':
            temp = '0';
            serialTarget = 2;
            while(temp != ')' && i < NUM_FINGERS) {
                if (Serial.available()) {
                    val *= 10;
                    val += temp - '0';
                    temp = Serial.read();
                }
                if (temp == ',') {
                    targets[i] = val;
                    val = 0;
                    i++;
                    temp = '0';
                }
            }

            if (i == NUM_FINGERS - 1) {
                serialTarget = 1;
                targets[i] = val;
            }

            switch (serialTarget) {
            case 1:
                Serial.printf("Target set to (");
                for (int j = 0; j <= i; ++j) {
                    Serial.print(targets[j]);
                    if (j < i)
                        Serial.print(", ");
                    else
                        Serial.println(")");
                }
                break;           
            case 2:
                targets[0] = val;
                Serial.printf("Target set to (%d)\n", targets[0]);
                break;

            } // end switch serialTarget
            break;

        case 't': //train
            serialTarget = 3;
            break; 
        case 'r': //reset
            hand.close();
//            hand.reset();
            break; 
        case 'p':
            hand.printPos();
            break;
            
        } // end switch temp
        
    } // end if Serial.available
} // end handleSerial

int getI2CStatus() {
  int status = 0;
  switch (Wire.status()) {
    case I2C_ADDR_NAK:
    case I2C_DATA_NAK:
    case I2C_ARB_LOST:
    case I2C_TIMEOUT:
    case I2C_BUF_OVF:
      status = -1;
      break;
    }

  return status;
}

