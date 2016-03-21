/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
 * Author: Aadeel Akhtar <aakhta3@illinois.edu>
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

#include <SPI.h>
#include <Servo.h>
#include <Wire.h>

#include "Channel.h"
#include "LDA.h"
#include "Config.h"
#include "EMGController.h"
#include "SerialEmg.h"

SerialEmg adsEMG;
EMGController emg;

int serialTarget = 0;
bool isCalibrated = false;
volatile char readRegister = 0;
volatile char writeRegister = 0;
volatile char writeVal = 0;;
volatile boolean needsWrite = 0;

operationState_t control_state = IDLE;

/* Initial setup of socket */
void setup() {
    control_state = INITIALIZATION;
   
    Wire.begin(SLAVE_ADDR);
  
    Serial.begin(9600);
    delay(1000);
    
    emg.setup();
    Channel::setSerialEmg(&adsEMG);
    delay(2000);

    Serial.println("start");

    Wire.onReceive(handleWrite);
    Wire.onRequest(handleRead);
    adsEMG.setupADS();
    control_state = IDLE;
}


/* Event loop for controlling all socket activity */
void loop() {
    handleSerial();
    handleRegisterWrite();

    switch (serialTarget) {
        case 1:
            isCalibrated = false;
            control_state = TRAINING;
            Serial.println("Training...");
            emg.train();
            serialTarget = 0;
            isCalibrated = true;
            control_state = RUNNING;
            break;
    }
    /* comment out for future addition of training button */
/*
    if (digitalRead(BUTTON)) {
        isCalibrated = false;
        Serial.println("Training...");
        emg.train();
        serialTarget = 0;
        isCalibrated = true;
    }*/

    if (isCalibrated) {
        emg.update();
    }

    delay(50);
}

/* Handles any serial events from the computer */
void handleSerial() {
    if (Serial.available()) {
        int temp = Serial.read();

        switch(temp) {
          case 't': //train
              serialTarget = 1;
              break;
        } // end switch temp

    } // end if Serial.available
} // end handleSerial

/* Handles any i2c writes from the master */
void handleRegisterWrite() {
  if (needsWrite) {
    needsWrite = 0;

    if (writeRegister == CONTROL) {
      if (!(writeVal & (1 << (int)TRAIN_OFFSET))) {
        serialTarget = 1;
      }
    }
    
  }
}

/* Interrupt handler for i2c writes */
void handleWrite(int n) {
  if (n > 1) {
    writeRegister = Wire.read();
    writeVal = Wire.read();

    needsWrite = 1;
  } else {
    readRegister = Wire.read();
  }

  
}

/* Interrupt handler for i2c reads */
void handleRead() {
  char buffer = 0;
  
  switch (readRegister) {
    case CONTROL:
      buffer = (char)control_state;
    break;

    case CLASS:
    if (isCalibrated) {
      buffer = (char)emg.getClass();
    }
    break;

    case VELOCITY:
      buffer = (char)(255 * emg.getVelocity());
    case ERR_STATUS:
    break;
    
  }
  Wire.write(buffer);
}

