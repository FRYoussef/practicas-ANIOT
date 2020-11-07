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

#define HALL_MARGIN 100
#define TOUCH_PAD_MARGIN (CONFIG_TOUCH_THRESHOLD / 2)
#define BOUNCE_TIME 200

#define ESP_INTR_FLAG_DEFAULT 0
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TOUCH_THRESH_FACTOR  (0.2f)

#define GPIO_OUTPUT_IO_0 18
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)

#define GPIO_INPUT_IO_0 16
#define GPIO_INPUT_PIN_SEL (1ULL<<GPIO_INPUT_IO_0)

static fsm_event reset_ev = reset;
static char pin_state = 1;
static int32_t pad0_init;

typedef struct Signal {
    QueueHandle_t *queue;
    SemaphoreHandle_t *sem;
};

void touchSensorTask(void *pvparameters);
void timerTask(void *pvparameters);
void FSMTask(void *pvparameters);
void hallSensorResetTask(void *pvparameters);

static void IRAM_ATTR timer_isr_handler(void *arg);
static void touchSensorIsr(void *args);

static void chronoCallback(void*);

#endif