#ifndef CHANNEL_H
#define CHANNEL_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "CircularBuffer.h"
#include "Config.h"
#include "SerialEmg.h"
#include "EMG_CONFIG.h"
#include "arm_math.h"
#include "math_helper.h"

#define NUM_IIRS 4 //num sections (rows) in iirCoeffsF32
#define SHIFT_RANGE 0x800000

const float32_t iirGain = 0.6402719075f; //For accurate values, multiply the filtered result by this. Not necessary though for classification
const float32_t iirCoeffsF32[NUM_IIRS * 5] = {
        1.0000000000f, 2.0000000000f, 1.0000000000f, -1.5591411117f, -0.6418402553f,
        1.0000000000f, -2.0000000000f, 1.0000000000f, 1.7340413998f, -0.7662533367f,
        1.0000000000f, -1.8632295168f, 1.0000000000f, 1.7406414099f, -0.9057410725f,
        1.0000000000f, -1.8632297429f, 1.0000000000f, 1.8206378153f, -0.9243057168f
};


class Channel {
private:
    CircularBuffer<float> rawData;
    CircularBuffer<float> filteredData;
    float dcAverage;
    float total;
    uint16_t totalCount;
    uint8_t assignedPin;
    uint8_t channelNum;

    

    float32_t iirStateF32[2 * NUM_IIRS];
    arm_biquad_cascade_df2T_instance_f32 S;

    void filterData();

public:

    class Feature {
    public:
        Feature();
        
        uint16_t mav;
        uint16_t ssc;
        uint16_t zc;
        float wl;
    };    

    static int32_t adsBuffer[NUM_CHANNELS + 1];
    static SerialEmg* serialEmg;
    static bool adsDataReady;
    
    Channel();
    Channel(uint8_t channelNum);
    
    void sample();
    void setDcAverage();
    void setup(uint8_t channelNum);
    
    Feature calculateFeatures();
   
    bool dataAvailable();
    
    float dcThreshold;

    static bool hasData();
    static void setSerialEmg(SerialEmg* se);
  };
//TODO: burn this code
#endif
