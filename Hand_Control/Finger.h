#ifndef FINGER_H
#define FINGER_H

#include "HAND_CONFIG.h"
#include "CLASS_CONFIG.h"
#include "Config.h"

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

typedef enum {FORWARD, BACKWARD} direction;

class Finger {
public:
    Finger();
       
    int update();
    
    void setTarget(long val);
    void setTarget(long val, float velocity);
    long getPos();
    void reset();
    void close();
    void stop();
    void setup(int fingerNum);
    void setupMotorPins();
    void setPwmDriver(Adafruit_PWMServoDriver& driver);
    
    volatile long position;
    
    classes current_grasp;
    classes last_grasp;

    direction dir;

    bool shouldLimit;

private:
    Adafruit_PWMServoDriver pwmDriver;

    void setDutyCycle(int pin, int duty);

    long getEncoderValue();

    void move(int& v);
    int calcSpeed();

    float pcVelocity;

    int velocity;
    float previousError;
    float errorSum;
    
    long previousPosition;
    long previousPositionDelta;

    int fingerNum_internal;
    
    int target;
    
    int motorDir1Pin;
    int motorDir2Pin;

    int motorEncoderAPin;
    int motorEncoderBPin;

    uint32_t currentTime;
    uint32_t previousTime;
    
    float kp;
    float kd;
    float ki;
};

#endif
