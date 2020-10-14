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


void *get_mem(int size){
    void *buffer;

    if( (buffer = malloc(size)) == NULL ) {
		fprintf( stderr, "ERROR in memory allocation\n" );
		return( NULL );
	}

    return (buffer);
}


void sensorTask(void *pvparameters){
    struct SensorArgs *args = (struct SensorArgs *) pvparameters;
    int32_t random;

    while(1){
        // just takes values inside int32_t [INT32_MIN, INT32_MAX]
        random = (int32_t) esp_random();
        
        xQueueSendToFront(args->queue, (void *) random, 100);
        vTaskDelay(args->milis);
    }
    vTaskDelete(NULL);
}


void filterTask(void *pvparameters){
    struct FilterArgs *args = (struct FilterArgs *) pvparameters;

    while(1){

    }
    vTaskDelete(NULL);
}


void app_main() {
    // create input args
    struct SensorArgs *s_args1, *s_args2;
    struct FilterArgs *f_args1, *f_args2;
    QueueHandle_t s_queue1, s_queue2, f_queue1, f_queue2;

    s_queue1 = xQueueCreate(5, sizeof(int32_t));
    s_queue2 = xQueueCreate(5, sizeof(int32_t));
    f_queue1 = xQueueCreate(5, sizeof(float));
    f_queue2 = xQueueCreate(5, sizeof(float));

    s_args1 = (struct SensorArgs *) get_mem(sizeof(struct SensorArgs));
    s_args1->milis = CONFIG_S1_MILLIS;
    s_args1->queue = &s_queue1;

    s_args2 = (struct SensorArgs *) get_mem(sizeof(struct SensorArgs));
    s_args2->milis = CONFIG_S2_MILLIS;
    s_args2->queue = &s_queue2;

    f_args1 = (struct FilterArgs *) get_mem(sizeof(struct FilterArgs));
    f_args1->in_queue = &s_queue1;
    f_args1->out_queue = &f_queue1;

    f_args2 = (struct FilterArgs *) get_mem(sizeof(struct FilterArgs));
    f_args2->in_queue = &s_queue2;
    f_args2->out_queue = &f_queue2;

    xTaskCreatePinnedToCore(&sensorTask, "sensorTask1", 2048, s_args1, 5, NULL, 0);
    xTaskCreatePinnedToCore(&sensorTask, "sensorTask2", 2048, s_args2, 5, NULL, 0);
    xTaskCreatePinnedToCore(&filterTask, "filterTask1", 2048, f_args1, 5, NULL, 1);
    xTaskCreatePinnedToCore(&filterTask, "filterTask2", 2048, f_args2, 5, NULL, 1);

    while(1) {}

    // delete vars
    free(s_args1);
    free(s_args2);
    free(f_args1);
    free(f_args2);
    vQueueDelete(s_queue1);
    vQueueDelete(s_queue2);
    vQueueDelete(f_queue1);
    vQueueDelete(f_queue2);
}