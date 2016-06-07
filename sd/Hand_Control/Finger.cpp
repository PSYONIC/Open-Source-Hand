/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
 * Author: Mary Nguyen <hnguyn10@illinois.edu>
 * Author: Michael Fatina <fatina2@illinois.edu>
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

#include "Finger.h"

/*
 * This class is responsible for controlling each of the fingers in the hand
 * Each instance respresents a finger
 */

Finger::Finger() : target(RESET_POS), position(0), motorDir1Pin (0), motorDir2Pin(0), motorEncoderAPin(0), 
  motorEncoderBPin(0), previousError(0), errorSum(0), currentTime(1), previousTime(0), velocity(0), kp(0), kd(0), ki(0), pcVelocity(1), previousPosition(0),
  previousPositionDelta(0) {
}

/* Basic setup of each finger */
void Finger::setup(int fingerNum) {
    target = RESET_POS;
    position = 0;

    motorDir1Pin = motorDir1Pins[fingerNum];
    motorDir2Pin = motorDir2Pins[fingerNum];

    motorEncoderAPin = encoderAPins[fingerNum];
    motorEncoderBPin = encoderBPins[fingerNum];
    
    previousError = 0;
    errorSum = 0;
    currentTime = 1;
    previousTime = 0;
    velocity = 0;

    pcVelocity = 1;

    kp = KP[fingerNum];
    ki = KI[fingerNum];
    kd = KD[fingerNum];
    
    current_grasp = OPEN;
    last_grasp = OPEN;

    fingerNum_internal = fingerNum;
    dir = FORWARD;
    shouldLimit = false;
}

/* Force a finger to close for zeroing out the encoder position */
void Finger::close() {
  setDutyCycle(motorDir1Pin, 0);
  setDutyCycle(motorDir2Pin, DEFAULT_SPEED);
}

/* Force stop a finger */
void Finger::stop() {
  setDutyCycle(motorDir1Pin, 0);
  setDutyCycle(motorDir2Pin, 0);
}

/* Setup motor pins to their correct input/ouput config */
void Finger::setupMotorPins() {
  pinMode(motorDir1Pin, OUTPUT);
  pinMode(motorDir2Pin, OUTPUT);

  pinMode(motorEncoderAPin, INPUT);
  pinMode(motorEncoderBPin, INPUT);
}

/* Reset finger parameters to their defaults */
void Finger::reset() {
    target = RESET_POS;
    position = 0;
    previousError = 0;
    errorSum = 0;
    velocity = 0;
    current_grasp = NIL;
    last_grasp = NIL;

    setDutyCycle(motorDir1Pins[fingerNum_internal], 0);
    setDutyCycle(motorDir2Pins[fingerNum_internal], 0);
}

/* Called once every event iteration, updates controller */
int Finger::update() {

//    Serial.printf("%d, ", position);
//
//    if (fingerNum_internal == NUM_FINGERS - 1) {
//      Serial.println();
//    }

    int theSpeed = calcSpeed();

//    Serial.print(theSpeed);
    
    velocity = (int)constrain(calcSpeed() * pcVelocity, MIN_SPEED, MAX_SPEED);

    move(velocity);

    return true;
}

/* Sets a target the finger should move to */
void Finger::setTarget(long val) {
    target = constrain(val, ANGLE_MIN, ANGLE_MAX);

    if ((current_grasp != last_grasp) && (current_grasp != NIL)) {
        if ((target - position) > 0) {
          dir = FORWARD;
        } else {
          dir = BACKWARD;
        }
    }

    pcVelocity = 1;
}

/* Sets a target the finger should move to along with a velocity */
void Finger::setTarget(long val, float inVelocity) {
    target = constrain(val, ANGLE_MIN, ANGLE_MAX);

    if ((current_grasp != last_grasp) && (current_grasp != NIL)) {
      if ((target - position) > 0) {
        dir = FORWARD;
      } else {
        dir = BACKWARD;
      }
    }

    pcVelocity = inVelocity;
}

/* Move a finger forward or backwards based on sign of velocity */
void Finger::move(int& v) {
//    v = constrain(v, MIN_SPEED, MAX_SPEED);

    int speed = abs(v);

    if (speed < MOVEMENT_THRESHOLD) {
      setDutyCycle(motorDir1Pin, 0);
        setDutyCycle(motorDir2Pin, 0);
        return;
    }

    int fingerSpeed = DEFAULT_SPEED;

    if (shouldLimit) {
      fingerSpeed = (int)(DEFAULT_SPEED / CONTACT_SPEED[fingerNum_internal]);
    }
    
    switch(dir) {
      case FORWARD:
        if (v > 0) {
          setDutyCycle(motorDir2Pin, 0);
          setDutyCycle(motorDir1Pin, fingerSpeed);
        } else {
          setDutyCycle(motorDir1Pin, 0);
          setDutyCycle(motorDir2Pin, 0);
        }
        break;

      case BACKWARD:
        if (v < 0) {
          setDutyCycle(motorDir1Pin, 0);
          setDutyCycle(motorDir2Pin, fingerSpeed);
        } else {
          setDutyCycle(motorDir1Pin, 0);
          setDutyCycle(motorDir2Pin, 0);
        }
        
    }
}

/* Calculate the speed a finger should move at */
int Finger::calcSpeed() {
    float error = 2.1*((float)target - position);

//    Serial.printf("%d ",target);
//    Serial.printf("%d ",position);

    return error;
}

/* Return the current position of a finger */
long Finger::getPos() {
  return position;
}

/* Set the pwm duty cycle on the i2c pwm driver */
void Finger::setDutyCycle(int pin, int duty) {
  // pwmDriver.setPWM(pin, 0, duty % 4096);
  analogWrite(pin,duty);
}

/* Set the pwmDriver object used by this finger */

classes Finger::getCurrentGrasp() {
  return current_grasp;
}
