#ifndef HAND_H
#define HAND_H

#include "HAND_CONFIG.h"
#include "CLASS_CONFIG.h"
#include "Finger.h"

class Hand {
public:
    Hand();
    
    void setupMotorPins();
        
    void setGrasp(classes grasp);

    void setGrasp(classes grasp, float velocity);

    void setTarget(int * targets); //expects array of NUM_FINGERS + 1 targets (fingers + thumb)
  
    void limitSpeed(bool shouldLimit);

    void close();
        
    void update();
    
    void reset();

    void printPos();

    bool checkFinalPosition();

    static Finger finger[NUM_FINGERS];

    static void Encoder0();
    static void Encoder1();
    static void Encoder2();
    static void Encoder3();
    static void Encoder4();
    static void Encoder5();
    
private:
    int graspToTarget(classes grasp, int fingerNum);

    bool inFinalPosition;
};

#endif
