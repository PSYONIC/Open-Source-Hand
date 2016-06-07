#ifndef HANDCONF_H
#define HANDCONF_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Config.h"

#define NUM_FINGERS 6
#define MAX_SPEED 1000 // PWM
#define MIN_SPEED (-1000) // PWM
#define DEFAULT_SPEED 1023 // PWM
#define ANGLE_MIN (1) // Deg
#define ANGLE_MAX 10000 // Deg
#define THUMB_MAX 55 // Deg
#define RESET_POS ANGLE_MIN // Deg
#define PWM_FREQUENCY 93750 //187500
#define CLOSE_DIR 1
#define OPEN_DIR 0

#define DIR_THRESHOLD 100

#define CONTACT_REFLEX 0.3f

#define MOVEMENT_THRESHOLD 15

#define PID_INTEGRATOR_CLAMP 75 // limit of sum of error for anti-windup in PID controller
#define POS_SLACK 100
#define POS_CHANGE_SLACK 0 //change in deg/cycle
#define RESELECT_SLACK 5
//
//const uint8_t motorDir2Pins[NUM_FINGERS] = {32, 22, 20, 21, 9, 4};
//const uint8_t motorDir1Pins[NUM_FINGERS] = {25, 23, 6, 5, 10, 3};
//
//const uint8_t encoderBPins[NUM_FINGERS] = {16, 11, 14, 8, 26, 33};
//const uint8_t encoderAPins[NUM_FINGERS] = {17, 13, 2, 7, 31, 24};

const uint8_t motorDir1Pins[NUM_FINGERS] = {22, 20, 21, 9, 4, 32};
const uint8_t motorDir2Pins[NUM_FINGERS] = {23, 6, 5, 10, 3, 25};

const uint8_t encoderAPins[NUM_FINGERS] = {13, 14, 8, 26, 33, 16};
const uint8_t encoderBPins[NUM_FINGERS] = {11, 2, 7, 31, 24, 17};

const float KP[NUM_FINGERS] = {2.445, 2.445, 2.445, 2.445, 2.445, 2.445};
const float KI[NUM_FINGERS] = {0.25, 0.25, 0.25, 0.25, 0.25, 0.25};
const float KD[NUM_FINGERS] = {500, 500, 500, 500, 500, 500};

const float CONTACT_SPEED[NUM_FINGERS] = {5.0, 5.0, 5.0, 7.0, 5.0, 5.0};

const int16_t gripTarget[7][NUM_FINGERS] = { // in deg
  {0,0,0,0,0,0}, /* Dummy for NIL */
  {1500, 3500, 3500, 3500, 3200, 3500}, /* OPEN */
//  {0,0,0,2250,0,0},
  {1500,0,0,0,0,3500}, /* POWER */
  {1600,2200,2000,3600,3600,2000}, /* CHUCK */
  {1600,2200,3500,3600,3600,2400}, /* PINCH */
  
  {0,0,0,0,0,0}, /* TOOL */ //3160
  {0,0,0,0,0,0} /* KEY */
  };
//{1000, 2700, 3500, 3500, 3200, 2500}, /* PINCH */
const int16_t gripTargetSpeed[7][NUM_FINGERS] = { // PWM
  {0,0,0,0,0}, /* NIL */
  {0,0,0,0,0}, /* OPEN - Once open, hand won't move. Speed while opening determined by grasp, hand was in before opening */ 
  {250, 1023, 1023, 1023, 1023},/*POWER*/  
  {1023, 1023, 1023, 0, 0}, /* CHUCK */ 
  {1023, 1023, 0, 0, 0}, /* PINCH */
  {100, 400, 1023, 1023, 1023}, /* TOOL */
  {900, 900, 900, 900, 900}, /* KEY */
};

#endif
