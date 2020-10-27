#ifndef _COMMON_
#define _COMMON_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

static TimerHandle_t chrono;
static TimerHandle_t resetTimer;
static TimerHandle_t printTimer;

static int32_t seconds;
static SemaphoreHandle_t seconds_mutex;

void timeUpdateTask(void *pvparameters);
void switchChronoTask(void *pvparameters);
void timerISR();
void touchSensorISR();
void chronometerCallback();
void resetTimerCallback();
void printTimerCallback();

#endif