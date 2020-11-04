#ifndef _COMMON_
#define _COMMON_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_intr_alloc.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"

#define EV_ONE_SEC 1
#define EV_START_STOP 2
#define EV_RESTART 3

#define ST_INI 1
#define ST_RUNNING 2
#define ST_STOPED 3

#define CHRONO_TIMER_ID 1
#define RESET_TIMER_ID 2

#define ESP_INTR_FLAG_DEFAULT 0
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

#define GPIO_OUTPUT_IO_0 18
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)

#define GPIO_INPUT_IO_0 4
#define GPIO_INPUT_PIN_SEL (1ULL<<GPIO_INPUT_IO_0)

static uint32_t pad_val;
static QueueHandle_t event_queue;


void eventTaskLogic(uint32_t ev, QueueHandle_t queue, SemaphoreHandle_t sem);
void touchSensorTask(void *pvparameters);
void timerTask(void *pvparameters);
void FSMTask(void *pvparameters);

static void IRAM_ATTR timerIsr(void *args);
static void touchSensorIsr(void *args);

void chronoCallback();
void resetTimerCallback();

#endif