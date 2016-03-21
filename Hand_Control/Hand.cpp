/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
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

/* 
 *  This class instantiates many instances of the finger class
 *  to represent a hand as a whole.
 */

#include "Hand.h"

/*
 * Finger array has to be statically declared or else the encoder interrupts
 * would be unable to update the positions
 */
Finger Hand::finger[NUM_FINGERS];

Hand::Hand() {
    for (int i = 0; i < NUM_FINGERS; ++i) {
        finger[i].setup(i);
    }

    pwmDriver = Adafruit_PWMServoDriver(0x5F);
    pwmDriver.begin();
    pwmDriver.setPWMFreq(24);
}

/* Initiate motor pins for all fingers and setup fingers */
void Hand::setupMotorPins(){
  for (int i = 0; i < NUM_FINGERS; ++i) {
        finger[i].setup(i);
  }
    
  for (int i = 0; i < NUM_FINGERS; ++i) {
    finger[i].setupMotorPins();
    finger[i].setPwmDriver(pwmDriver);
  }

}

/* Set the grasp for the hand */
void Hand::setGrasp(classes grasp) {
  for (int i = 0; i < NUM_FINGERS; i++) {
    int cond1 = (finger[i].current_grasp > OPEN && grasp == OPEN);
    int cond2 = (finger[i].current_grasp == OPEN && grasp > OPEN);

    if (((finger[i].current_grasp > OPEN && grasp == OPEN) || (finger[i].current_grasp <= OPEN && grasp >= OPEN)) && ((int)grasp < NUM_CLASSES)) {
      finger[i].last_grasp = finger[i].current_grasp;

      finger[i].current_grasp = grasp;

      finger[i].setTarget(graspToTarget(finger[i].current_grasp, i));
    }
  }
}

/* Set the grasp for the hand with a velocity */
void Hand::setGrasp(classes grasp, float velocity) {
  if (grasp == NIL) {
    return;
  }

  for (int i = 0; i < NUM_FINGERS; i++) {
    if (((finger[i].current_grasp > OPEN && grasp == OPEN) || (finger[i].current_grasp == OPEN && grasp > OPEN)) && ((int)grasp < NUM_CLASSES)) {
      finger[i].last_grasp = finger[i].current_grasp;

      finger[i].current_grasp = grasp;
      
      finger[i].setTarget(graspToTarget(finger[i].current_grasp, i));
    }
  }
}

/* Set the fingers manually based on a target array */
void Hand::setTarget(int * targets) {
    for (int i = 0; i < NUM_FINGERS; ++i) {
        finger[i].setTarget(targets[i]); 
        finger[i].last_grasp = NIL;
    }
} 

/* Enables/Disables contact reflexes */
void Hand::limitSpeed(bool shouldLimit) {
  for (int i = 0; i < NUM_FINGERS; i++) {
    finger[i].shouldLimit = shouldLimit;
  }
}

/* Force the hand to close for calibrations */
void Hand::close() {
  for (int i = 0; i < NUM_FINGERS; i++) {
    finger[i].close();

  delay(3000);

    finger[i].stop();

  }
}

/* Update the control loop for each finger */
void Hand::update() {
    for (int i = 0; i < NUM_FINGERS; ++i) {
        finger[i].update();
    }
}

/* Reset the position and other variables in each finger */
void Hand::reset() {
    for (int i = 0; i < NUM_FINGERS; ++i) {
        finger[i].reset();
    }
}

/* Converts a grasp to the finger targets*/
int Hand::graspToTarget(classes grasp, int fingerNum) {
    if (grasp != NIL && grasp < NUM_CLASSES) {
        return gripTarget[grasp][fingerNum];
    } else {
      return finger[fingerNum].getPos();
    }
}

/* Handles encoder interrupts for finger 0 */
void Hand::Encoder0() {
  Hand::finger[0].position += 1 - ((digitalReadFast(encoderAPins[0]) ^ digitalReadFast(encoderBPins[0])) << 1);
}

/* Handles encoder interrupts for finger 1 */
void Hand::Encoder1() {
  Hand::finger[1].position -= 1 - ((digitalReadFast(encoderAPins[1]) ^ digitalReadFast(encoderBPins[1])) << 1);
}

/* Handles encoder interrupts for finger 2 */
void Hand::Encoder2() {
  Hand::finger[2].position -= 1 - ((digitalReadFast(encoderAPins[2]) ^ digitalReadFast(encoderBPins[2])) << 1);
}

/* Handles encoder interrupts for finger 3 */
void Hand::Encoder3() {
  Hand::finger[3].position -= 1 - ((digitalReadFast(encoderAPins[3]) ^ digitalReadFast(encoderBPins[3])) << 1);
}

/* Handles encoder interrupts for finger 4 */
void Hand::Encoder4() {
  Hand::finger[4].position -= 1 - ((digitalReadFast(encoderAPins[4]) ^ digitalReadFast(encoderBPins[4])) << 1);
}

/* Handles encoder interrupts for finger 5 */
void Hand::Encoder5() {
  Hand::finger[5].position -= 1 - ((digitalReadFast(encoderAPins[5]) ^ digitalReadFast(encoderBPins[5])) << 1);
}
