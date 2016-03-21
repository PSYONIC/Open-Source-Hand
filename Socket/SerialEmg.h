#ifndef S_EMG_H
#define S_EMG_H

#include "EMG_CONFIG.h"
#include "ADS129X.h"

class SerialEmg {
public:
    SerialEmg();
    
    void setupADS();
        
    bool hasData(long* data);
        
private:
    ADS129X ADS = ADS129X(ADS_DRDY, ADS_CS);

    long buffer[NUM_CHANNELS + 1];
};

#endif
