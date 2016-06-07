/*
 * Copyright (C) 2016 - PSYONIC Inc.
 * Author: Edward Wu <elwu2@illinois.edu>
 * Author: Aadeel Akhtar <aakhta3@illinois.edu>
 * Author: Alvin Wu <alvinwu2@illinois.edu>
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

#include "EMGController.h"

Channel EMGController::channel[NUM_CHANNELS];

EMGController::EMGController() {
}

/* Setup the controller we use for emg data */
void EMGController::setup() {
    for (int i = 0; i < NUM_CHANNELS; ++i) {
        EMGController::channel[i].setup(i);
    }

    isTrained = false;

    classPointer = 0; //class pointer for previous classes
    for (int i = 0; i < PREVIOUSCLASSES; i++) { //initialize array of previous classes to nil class
        previousClasses[i] = NIL;
    }
    previousClass = NIL; //initialize previous class to NIL
    currentClass = NIL; //initialize current class to NIL

    currentVelocity = 0;

    stimState = false;

}

/* Initialize the filter used in each channel to prevent an initial spike */
void EMGController::initFilter() {
    IntervalTimer samplingTimer; //initialize a sampling tmer
    samplingTimer.begin(getReadings, PERIOD); //begin taking samples

    uint8_t i;
    uint32_t duration = 2000000;
    uint32_t currentTime = micros();
    
    while(micros() - currentTime <duration) { //while within the time limit
        if (EMGController::channel[NUM_CHANNELS - 1].dataAvailable()) { //when a sampling window is complete the data available flag is set to true
                for (i = 0; i < NUM_CHANNELS; i++) { //iterate thorugh all of the emg channels        
                    EMGController::channel[i].calculateFeatures(); //call process readings to get a pointer to the current sampling window
                }

        }
    }
    samplingTimer.end();
}

/* Train the LDA classifier with user input */
void EMGController::train() {
    isTrained = false;

    //loop to train every class
    for(uint8_t i = 0; i < NUMCLASSES; i++) {
        Serial.printf("Training class: %d\n", i);

        blink(GREEN, i+1);

        currentClass = (classes)i;

        float32_t featAverage[NUMFEATS]; //array to hold average of each feature
        float32_t trainingData[2600]; //array to hold all training data
        uint16_t index = 0; //variable to keep track of number of samples for calculating average

        memset(featAverage, 0, 32 * sizeof(float32_t));

        delay(2000); 

        for (uint8_t j = 0; j < 3; j++) {
            Serial.printf("Iteration: %d\n", j);
            setLedColor(RED);

            delayMicroseconds(750000);

            //call collect which will store feature data into training data array and also store the sum of that array into feat average
            uint16_t sampleCount = collect(trainingData + (index * NUMFEATS), featAverage, 2000000,i,j);


            //update count of samples
            index += sampleCount; //update index to the number of samples
            setLedColor(GREEN);
            delayMicroseconds(1000000);
        }

        //finalize calculating TH_min,i and TH_max,i (PC)
        float32_t mav_min = thresh_min[i];
        float32_t mav_max = thresh_max[i];

        thresh_min[i] = (1-PMINTHRESH/100) * mav_min + PMINTHRESH * mav_max / 100;
        thresh_max[i] = (1-PMAXTHRESH/100) * mav_min + PMAXTHRESH * mav_max / 100;


        for (int j = 0; j < NUMFEATS; j++) { //loop to divide feataverage by number of samples to get the actual average of the array
            featAverage[j] /= index;
        }

        Serial.printf("%d) Samples: %d\n", i, index);

        if (i == NIL) { //if the current class is nil, then also calculate the threshold using the average matrix, feat average
            for (int j = 0; j < NUM_CHANNELS; j++) {
                EMGController::channel[j].dcThreshold = (THRESHOLD_MULT) * featAverage[j * 4];
                Serial.printf("%d) Threshold: %f\n", j, EMGController::channel[j].dcThreshold);
            }
        }

        //print the two matrices
        Matrix.Print(trainingData, index, NUMFEATS, "feature matrix");
        Matrix.Print(featAverage, 1, NUMFEATS, "average vector");

        //print the min and max threshold values for class i
        Serial.printf("\nThreshold Min for Proportional Control:%f\n",thresh_min[i]);
       Serial.printf("Threshold Max for Proportional Control:%f\n",thresh_max[i]);

        //set the average vector of the current class being trained to feataverage
        classifier.setMu(i, featAverage);
        //train the current class with the training data just collected
        classifier.train(i, trainingData, index);

        setLedColor(OFF);

        delayMicroseconds(2000000);
    }



    classifier.finalizeTraining();

    isTrained = true;
} // end train()

/*
 * Collect data from all of the channels for a specified duration
 * Data is stored into data and returns the number of samples collected
 */
uint16_t EMGController::collect(float32_t* data, float32_t* sum, uint32_t duration,uint8_t classnum,uint8_t first) {
    IntervalTimer samplingTimer; //initialize a sampling tmer
    samplingTimer.begin(getReadings, PERIOD); //begin taking samples
    float32_t sumMAV;

    volatile uint16_t sampleCount = 0;
    uint8_t i;
    uint32_t currentTime = micros();
   

    while(micros() - currentTime <duration) { //while within the time limit
        if (EMGController::channel[NUM_CHANNELS - 1].dataAvailable()) { //when a sampling window is complete the data available flag is set to true
                uint8_t passes = true;
                sumMAV = 0;
                
                for (i = 0; i < NUM_CHANNELS; i++) { //iterate thorugh all of the emg channels        
                    Channel::Feature currentWindow = EMGController::channel[i].calculateFeatures(); //call process readings to get a pointer to the current sampling window
                        //store into data the respective features of each channel

//FILTER SPIKE
                        
                    if (currentWindow.mav > 10000) {
                        passes = false; //set to true to ignore cheap fix
                    } else {
                        data[(sampleCount * NUMFEATS) + (i * 4)] = currentWindow.mav;
                        data[(sampleCount * NUMFEATS) + (i * 4) + 1] = currentWindow.ssc;
                        data[(sampleCount * NUMFEATS) + (i * 4) + 2] = currentWindow.zc;
                        data[(sampleCount * NUMFEATS) + (i * 4) + 3] = currentWindow.wl;
    
                        //add to the sum, each feature
                        sum[(i * 4)] += currentWindow.mav;
                        sum[(i * 4) + 1] += currentWindow.ssc;
                        sum[(i * 4) + 2] += currentWindow.zc;
                        sum[(i * 4) + 3] += currentWindow.wl;

                        //add mav to sum_MAV (PC)
                        sumMAV += currentWindow.mav;
                    }
                
                }

                //update min and max for current class (PC)
                //if first time calling collect 

                if (passes) { 
                    if(0==first++){
                        thresh_min[classnum] = sumMAV;
                        thresh_max[classnum] = sumMAV;
                    }
                    else{
                        //check if new min or new max
                        if(sumMAV<thresh_min[classnum]){
                            thresh_min[classnum] = sumMAV;
                        }
                        else if(sumMAV>thresh_max[classnum]){
                            thresh_max[classnum] = sumMAV;
                        }
                    }
            
                    //increase the count of number of samples
                    sampleCount++;
                }

        }
    }
    samplingTimer.end();

    //return the number of samples sampled
    return sampleCount;
}

/* Static function for sampling from the channels by an interval timer */
void EMGController::getReadings() {
    if (Channel::hasData()) {
        for (int i = 0; i < NUM_CHANNELS; ++i) {
            EMGController::channel[i].sample();
        }
    }
}

/* Sets the color of an LED to indicate current socket status */
void EMGController::setLedColor(ledColor color) {
    //Currently disabled due to lack of led in hand
    //set the led according to the color sent
    /*switch(color) {
    case RED:
        digitalWrite(LEDPIN1, HIGH);
        digitalWrite(LEDPIN2, LOW);
        break;
    case GREEN:
        digitalWrite(LEDPIN1, LOW);
        digitalWrite(LEDPIN2, HIGH);
        break;
    case OFF:
        digitalWrite(LEDPIN1, LOW);
        digitalWrite(LEDPIN2, LOW);
        break;
    }*/
}

/* Blinks the led the specified number of times */
void EMGController::blink(ledColor color, uint8_t times) {
    uint8_t i;

    setLedColor(OFF);
    delayMicroseconds(10000);
    //loop to repeat the blinking of the led the specified number of times
    for(i = 0; i < times; i++) {
        setLedColor(color);
        delayMicroseconds(250000);
        setLedColor(OFF);
        delayMicroseconds(250000);
    }
}

/* Collects a single window of features and store it into the memory address specified by the pointer data */
bool EMGController::collectSingle(float32_t* data,float32_t* sumMAV) {
    IntervalTimer samplingTimer; //initialize sampling timer
    samplingTimer.begin(getReadings, PERIOD); //start the sampling timer

    uint8_t i;
    boolean passesThreshold = false;

    while (! EMGController::channel[NUM_CHANNELS - 1].dataAvailable()); //since the channels will set the flag datavailable as true when an emg window's worth of data has been collected, wait until the last emg channel sets dataavailable as true

        if (EMGController::channel[NUM_CHANNELS - 1].dataAvailable()) {
        samplingTimer.end();

        for (i = 0; i < NUM_CHANNELS; i++) { //loop through all emg windows
            Channel::Feature currentWindow = EMGController::channel[i].calculateFeatures(); //get pointer to current window
            if (currentWindow.mav > EMGController::channel[i].dcThreshold) { //check to see if any of the channels mean absolute values surpass the dc threshold
                passesThreshold = true; //if so, set the passthreshold flag as true
            }
            data[(i * 4)] = currentWindow.mav; //store the features into the data array
            data[(i * 4) + 1] = currentWindow.ssc;
            data[(i * 4) + 2] = currentWindow.zc;
            data[(i * 4) + 3] = currentWindow.wl;

            *sumMAV += currentWindow.mav; //sum mavs (PC)
        }
    }

    return passesThreshold; //return whether or not this window of data passes the threshold
}

/* Updates the emg controller with new data from EMG */
void EMGController::update() {
    float32_t f_g[NUMFEATS];
    //variable to hold the determined class
    uint32_t classNum = 0;
    float32_t sumMAV = 0;
    float32_t velocity = 0;

    //call collect single to collect a single window of features and returns true if the collected data passes the threshold determined in training
    boolean passesThreshold = collectSingle(f_g,&sumMAV);

    if (passesThreshold) {
        //call the classifier to determine the most likely class from this set of features
        classNum = classifier.classify(f_g);

        //calculate velocity (PC)
        velocity = constrain((sumMAV - thresh_min[classNum])/(thresh_max[classNum]-thresh_min[classNum]), 0, 1);

        //add this determined class to the circular buffer of previous classes
        previousClasses[classPointer] = (classes) classNum;
        classPointer ++;
        classPointer = classPointer % PREVIOUSCLASSES;
    }

    //print the features and what the current and previous chosen class were
    //Matrix.Print(f_g, 1, NUMFEATS, "f_g");
    Serial.printf("Classified as class: %d\n", classNum);
//    Serial3.print(classNum + '0');
    //Serial.printf("Previous class: %d\n", previousClass);
    Serial.printf("Velocity of class: %f\n",velocity);

    uint8_t i;
    uint8_t count[NUMCLASSES];
    memset(count, 0, sizeof(uint8_t) * NUMCLASSES);
    classes c = previousClasses[0];

    //loop to count the number of times each class appeared in the circular buffer
    for (i = 0; i < PREVIOUSCLASSES; i++) {
        count[previousClasses[i]]++;
    }

    //print out that count
    for (i = 0; i < NUMCLASSES; i++) {
        //Serial.printf("%d,", count[i]);
    }
    //Serial.println();


    uint8_t max = count[0];
    uint8_t maxIndex = 0;
    //loop to determine the mode of the circular buffer using the count we just determined
    for (i = 1; i < NUMCLASSES; i++) {
        if (count[i] > max) {
            maxIndex = i;
            max = count[i];
        }
    }

    Serial.printf("Mode of previous classes is: %d\n", maxIndex);

    classes temp = (classes) maxIndex;
    //since we are only changing grips to and from an open grip, check if the current grip will be open or that the previous grip was open or that the current grip is the same as the previous grip
    if (previousClass == OPEN || temp == OPEN || temp == NIL || temp == previousClass) {
        currentClass = temp;
        if (temp != NIL) { //set the previous class as the current class if the current class isn't null, so if the grip was stopped halfway, we won't have to reopen the hand to do the same grip again
            previousClass = currentClass;
        }
        currentVelocity = velocity;
    }
}  


/* Returns the current classified grasp */
classes EMGController::getClass() {
    if (currentClass >= POWER) {
      currentClass = POWER;
    }
    return currentClass;   
}

/* Returns current proportional velocity */
float EMGController::getVelocity() {
    return currentVelocity;
}

