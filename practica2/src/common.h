#ifndef _COMMON_
#define _COMMON_

#include <stdio.h>
#include <time.h>
#include <float.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define QUEUE_SIZE 5
#define QUEUE_SET_SIZE (QUEUE_SIZE + QUEUE_SIZE)
#define CONTROLLER_SLEEP 1000
static float WEIGHTS[] = {0.05, 0.1, 0.15, 0.25, 0.45};

typedef struct SensorArgs {
   int32_t  milis;
   QueueHandle_t *queue;
};

typedef struct FilterArgs {
   QueueHandle_t *in_queue;
   QueueHandle_t *out_queue;
};

typedef struct SensorSample {
   int32_t sample;
   time_t timestamp;
};

typedef struct FilterSample {
   char *name;
   float sample;
   time_t timestamp;
};

#endif