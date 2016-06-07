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
 
#include "MatrixMath.h"
#include "LDA.h"

trainingMode_t training_state;


LDA::LDA() {
  training_state = TRAIN;

  uint8_t i;

  memset(iCOV, 0, sizeof(float32_t) * NUMFEATS * NUMFEATS);

  trainingCount = 0;
}

/* Trains the LDA classifier for a specific class given a feature array and the number samples */
statusCode LDA::train(uint8_t className, float32_t* f_g, uint16_t samples) {
  if (samples <= 0 || className < 0 || className > NUMCLASSES) { //check to make sure the input is valid
    return ERROR;
  }
  
  //initialize temporary matrices for calculations
  float32_t A[samples][NUMFEATS];
  memset(&A, 0, sizeof(float32_t) * samples * NUMFEATS);
  float32_t AT[NUMFEATS][samples];
  memset(&AT, 0, sizeof(float32_t) * samples * NUMFEATS);
  float32_t COV_G[NUMFEATS][NUMFEATS];
  memset(&COV_G, 0,  sizeof(float32_t) * NUMFEATS * NUMFEATS);

  //compute f_g - mu_g
  uint8_t i,j;
  for(i = 0; i < samples; i++) {


    for(j = 0; j < NUMFEATS; j++) {
      A[i][j] = f_g[(i * NUMFEATS) + j] - mu[className][j];
    }
  }

  //compute A transpose
  Matrix.Transpose((float*)A, samples, NUMFEATS, (float*)AT);

  //compute covariance matrix
  Matrix.Multiply((float*)AT,(float*)A, NUMFEATS, samples, NUMFEATS, (float*)COV_G);
  Matrix.Scale((float*)COV_G, NUMFEATS, NUMFEATS, (1.0 / ((samples - 1) * NUMCLASSES)));
  //Matrix.Print((float *)COV_G, NUMFEATS, NUMFEATS, "Covariance");

  //add the computed covariance matrix to the iCov, which will eventually be the inverse of the mean of convariance matrices
  Matrix.Add((float*)COV_G, (float*)iCOV, NUMFEATS, NUMFEATS, (float*)iCOV);

  trainingCount++;

  return SUCCESS;
}

/* Finalize training of the classifer */
statusCode LDA::finalizeTraining() {
  if (trainingCount >= NUMCLASSES) {
    uint8_t i;
    //invert the covariance matrix
    Matrix.Invert((float*)iCOV, NUMFEATS);

    //Matrix.Print((float*)iCOV, NUMFEATS, NUMFEATS, "Inverse Covariance");

    for(i = 0; i < NUMCLASSES; i++) {

      Matrix.Multiply((float*)iCOV, mu[i], NUMFEATS, NUMFEATS, 1, mLDA[i]);

      //Matrix.Print(mLDA[i], 1, NUMFEATS, "Weight Slope");

      //calculate the 'constant' of LDA
      Matrix.Multiply(mLDA[i], mu[i], 1, NUMFEATS, 1, &cLDA[i]);

      cLDA[i] /= 2;
    }
    
    training_state = CLASSIFY;

    return SUCCESS;
  } else {
    return ERROR;
  }
}


/* Return the most likely class from the feature array */
uint32_t LDA::classify(float32_t* f_g) {
  uint8_t i;
  float32_t max = 0;
  uint8_t result;
  float32_t values[NUMCLASSES];

  for (i = 0; i < NUMCLASSES; i++) {
    Matrix.Multiply(f_g, mLDA[i], 1, NUMFEATS, 1, &values[i]);

    values[i] -= cLDA[i];
    if (i == 0) {
      max = values[0];
      result = 0;
    }

    if (values[i] > max) {
      result = i;
      max = values[i];
    }
  }
  //Matrix.Print(values, 1, NUMCLASSES, "Classes");

  return result;
}

/* Set the average vector for calculations */
void LDA::setMu(uint8_t classNum, float32_t* data) {
  for(int i = 0; i < NUMFEATS; i++) {
    mu[classNum][i] = data[i];
  }
}
