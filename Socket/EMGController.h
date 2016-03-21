#ifndef EMG_H
#define EMG_H

#include "EMG_CONFIG.h"
#include "CLASS_CONFIG.h"
#include "Config.h"
#include "Channel.h"
#include "LDA.h"

class EMGController {
public:
    EMGController();

    void setup();

    void train();

    void update();
    
    classes getClass();
    float32_t getVelocity();
    static Channel channel[NUM_CHANNELS];
    
private:

    static void getReadings();

    classes previousClasses[PREVIOUSCLASSES]; //array of previous classes for choosing grip
    classes previousClass; //previous chosen grip
    classes currentClass; //current chosen grip

    float32_t currentVelocity;

    void initFilter();
    
    uint8_t classPointer; //"pointer" to next position to store a previous class
    
    uint16_t collect(float32_t* data, float32_t* sum, uint32_t duration,uint8_t classnum,uint8_t first);

    bool collectSingle(float32_t* data, float32_t* sumMAV);
    
    void setLedColor(ledColor color);
    
    void blink(ledColor color, uint8_t times);
    
    bool isTrained;
        
    LDA classifier;

    //Proportional control variables
    float32_t thresh_min[NUMCLASSES];
    float32_t thresh_max[NUMCLASSES];

};

#endif
