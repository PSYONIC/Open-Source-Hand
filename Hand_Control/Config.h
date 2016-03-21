#ifndef CONFIG_H
#define CONFIG_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define RESOLUTION 12

#define BUTTON 0

typedef float float32_t;

#define NUM_SENSORS (6)
#define SOCKET_ADDR 12
#define STIM_ADDR 8
#define PRESSURE_THRESHOLD (0.2)

typedef enum {TRAIN, CLASSIFY} trainingMode_t;
typedef enum {INITIALIZATION, CALIBRATION, TRAINING, IDLE, RUNNING} operationState_t;
typedef enum {ERROR, SUCCESS} statusCode;

typedef enum {
  CONTROL=0x0,
  CLASS,
  VELOCITY,
  ERR_STATUS
} i2c_registers;

#endif
