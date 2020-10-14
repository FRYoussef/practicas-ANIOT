#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


struct SensorArgs {
   int32_t  milis;
   QueueHandle_t *queue;
};

struct FilterArgs {
   QueueHandle_t *in_queue;
   QueueHandle_t *out_queue;
}; 


void sensorTask(void *pvparameters){
    struct SensorArgs *args = (struct SensorArgs *) pvparameters;

    while(1){
        vTaskDelay(args->milis);
    }
    vTaskDelete(NULL);
}


void filterTask(void *pvparameters){
    while(1){

    }
    vTaskDelete(NULL);
}


void app_main() {
    // create input args
    struct SensorArgs *s_args1, *s_args2;
    struct FilterArgs *f_args1, *f_args2;
    QueueHandle_t s_queue1, s_queue2, f_queue1, f_queue2;

    s_queue1 = xQueueCreate(5, sizeof(uint32_t));
    s_queue2 = xQueueCreate(5, sizeof(uint32_t));
    f_queue1 = xQueueCreate(5, sizeof(float));
    f_queue2 = xQueueCreate(5, sizeof(float));

    s_args1->milis = S1_MILIS;
    s_args1->queue = &s_queue1;

    s_args2->milis = S2_MILIS;
    s_args2->queue = &s_queue2;

    f_args1->in_queue = &s_queue1;
    f_args1->out_queue = &f_queue1;

    f_args2->in_queue = &s_queue2;
    f_args2->out_queue = &f_queue2;

    xTaskCreatePinnedToCore(&sensorTask, "sensorTask1", 16, s_args1, 5, NULL, 0);
    xTaskCreatePinnedToCore(&sensorTask, "sensorTask2", 16, s_args2, 5, NULL, 0);
    xTaskCreatePinnedToCore(&filterTask, "filterTask1", 16, f_args1, 5, NULL, 0);
    xTaskCreatePinnedToCore(&filterTask, "filterTask2", 16, f_args2, 5, NULL, 0);

    while(1) {}

    // delete vars
}