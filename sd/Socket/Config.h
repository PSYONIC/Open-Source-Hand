#ifndef CONFIG_H
#define CONFIG_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define RESOLUTION 12
#define SLAVE_ADDR 12
#define BUTTON 0

typedef float float32_t;

#define MODE 1//0 - serial, 1 - EMG

typedef enum {TRAIN, CLASSIFY} trainingMode_t;
typedef enum {INITIALIZATION, CALIBRATION, TRAINING, IDLE, RUNNING} operationState_t;
typedef enum {ERROR, SUCCESS} statusCode;

typedef enum {
  CONTROL=0x0,
  CLASS,
  VELOCITY,
  ERR_STATUS
} i2c_registers;

typedef enum {
  ENABLE_OFFSET=0x0,
  TRAIN_OFFSET
} control_state_offset;


extern operationState_t control_state;
extern trainingMode_t training_state;

#endif
