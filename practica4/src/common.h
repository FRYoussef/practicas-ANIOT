#ifndef _COMMON_
#define _COMMON_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_intr_alloc.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "driver/adc.h"
#include "fsm/fsm.h"
#include "esp_timer.h"

#define CHRONO_TIMER_ID 1
#define RESET_TIMER_ID 2

#define ESP_INTR_FLAG_DEFAULT 0
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

#define GPIO_OUTPUT_IO_0 18
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)

#define GPIO_INPUT_IO_0 4
#define GPIO_INPUT_PIN_SEL (1ULL<<GPIO_INPUT_IO_0)

static uint32_t pad_val;
static fsm_event reset_ev = reset;
static char pin_state = 1;

typedef struct Signal {
    QueueHandle_t *queue;
    SemaphoreHandle_t *sem;
};

void eventTaskLogic(fsm_event *ev, struct Signal *signals);
void touchSensorTask(void *pvparameters);
void timerTask(void *pvparameters);
void FSMTask(void *pvparameters);

static void IRAM_ATTR timer_isr_handler(void *arg);
static void touchSensorIsr(void *args);

static void chronoCallback(void*);
static void resetTimerCallback(void*);

#endif