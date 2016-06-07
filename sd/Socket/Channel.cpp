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

#include "CircularBuffer.h"
#include "Channel.h"

/*
 * This class handles reading a single channel of emg from the electrodes
 */

int32_t Channel::adsBuffer[NUM_CHANNELS + 1] = {0};
SerialEmg* Channel::serialEmg;
bool Channel::adsDataReady = false;

Channel::Channel() : channelNum(0), dcAverage(0), total(0), totalCount(0) {
}

Channel::Channel(uint8_t c) : channelNum(c), dcAverage(0), total(0), totalCount(0) {
}

/* Initial a channel */
void Channel::setup(uint8_t c) {
    channelNum = c;
    arm_biquad_cascade_df2T_init_f32(&S, NUM_IIRS, (float32_t *)&iirCoeffsF32[0], &iirStateF32[0]);
}

/* Sets the dc averages of a channel */
void Channel::setDcAverage() {
    if (totalCount > 0) {
        //set the dc average
        dcAverage = total/totalCount;

        totalCount = 0;
        total = 0;
    }
}

/* Checks if the ads has generate any new data */
bool Channel::hasData() {
    if (serialEmg->hasData(Channel::adsBuffer)) {
        Channel::adsDataReady = true;
    }

    return adsDataReady;
}

/* Set the serial emg object used by this channel for reading from the ads */
void Channel::setSerialEmg(SerialEmg* se) {
    Channel::serialEmg = se;
}

/* Read from the ads and add to the circular buffer of data */
void Channel::sample() {
    if (!rawData.isFull()) {
        if (Channel::adsDataReady) {
            float voltage = (float)(Channel::adsBuffer[channelNum + 1]^SHIFT_RANGE);

            
            rawData.add(voltage);
            
            if (channelNum == NUM_CHANNELS - 1) {
                Channel::adsDataReady = false;
            }
        }
    }
}

/* Filter the data in the emg circular buffer */
void Channel::filterData() {
    if (rawData.isFull()) {
        float temp[WINDOWLENGTH];
        float outputBuffer[WINDOWLENGTH];

        for (int i = 0; i < rawData.size(); ++i) {
            temp[i]= rawData[i];
        }

        arm_biquad_cascade_df2T_f32(&S, temp, outputBuffer, WINDOWLENGTH);

        for (int i = filteredData.size(); i < filteredData.getMaxSize() - filteredData.size(); ++i) {
            filteredData.add(outputBuffer[i]);
        }
    }
}

/* Extract features from the current sampling window */
Channel::Feature Channel::calculateFeatures() {
    if (rawData.isFull()) {
        filterData();
        Feature f;
                
        uint16_t i;
        
        float previousSlope = 0;
        float absSum = 0;

        for(i = 0; i < filteredData.size(); ++i) {
            total += filteredData[i];
        }

        totalCount = filteredData.size();
        setDcAverage();

        for (i = 0; i < WINDOWLENGTH; ++i) {
            absSum += abs(filteredData[i] - dcAverage);
            if (i > 0) {
                float slope = filteredData[i] - filteredData[i - 1];
                
                if (slope * previousSlope < 0) {
                    ++f.ssc;
                }

                if ((filteredData[i] - dcAverage) * (filteredData[i - 1] - dcAverage) < 0) {
                    ++f.zc;
                }

                f.wl += abs(slope);
                
                previousSlope = slope;
            }
        }
        f.mav = absSum / WINDOWLENGTH;
        
        rawData.removeValues(STEPSIZE);
        filteredData.removeValues(STEPSIZE);

        return f;
    }
    return Feature();
}

/* Check whether sampling window is full */
bool Channel::dataAvailable() {
    return rawData.isFull();
}

Channel::Feature::Feature() : mav(0), ssc(0), zc(0), wl(0) {
}

