#ifndef LDA_H
#define LDA_H

#include "MatrixMath.h"
#include "Config.h"
#include "EMG_CONFIG.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class LDA {
public:
  LDA();
  statusCode train(uint8_t className, float32_t* f_g, uint16_t samples);
  statusCode finalizeTraining();
  //boolean loadTraining();
  uint32_t classify(float32_t* f_g);
  void setMu(uint8_t classNum, float32_t* data);

private:
  uint8_t trainingCount;
  float32_t mu[NUMCLASSES][NUMFEATS];
  float32_t iCOV[NUMFEATS][NUMFEATS];
  float32_t mLDA[NUMCLASSES][NUMFEATS];
  float32_t cLDA[NUMCLASSES];

  //For proportional control
  

};

#endif
