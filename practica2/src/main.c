#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define QUEUE_SIZE 5

static float WEIGHTS[] = {0.05, 0.1, 0.15, 0.25, 0.45};

typedef struct CircularBuffer {
   int32_t  counter;
   struct SensorSample *values;
};

void init_buffer(struct CircularBuffer *queue){
    int i;

    queue->values = (int32_t *) malloc(QUEUE_SIZE * sizeof(struct SensorSample));
    queue->counter = 0;

    for(i = 0; i < QUEUE_SIZE; i++)
        queue->values[i].sample = 0;
}

void free_buffer(struct CircularBuffer *queue){
    free(queue->values);
}

int32_t size_buffer(struct CircularBuffer *queue){
    return queue->counter;
}

struct SensorSample get_element(struct CircularBuffer *queue){
    queue->counter %= QUEUE_SIZE;
    return queue->values[queue->counter++];
}

void set_element(struct CircularBuffer *queue, struct SensorSample element){
    queue->counter %= QUEUE_SIZE;
    queue->values[queue->counter++] = element;
}


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
   char *timestamp;
};

typedef struct FilterSample {
   float sample;
   char *timestamp;
};


void sensorTask(void *pvparameters){
    struct SensorArgs *args = (struct SensorArgs *) pvparameters;
    struct SensorSample sample;
    BaseType_t is_write;

    while(1){
        // just takes values inside int32_t [INT32_MIN, INT32_MAX]
        sample.sample = (int32_t) esp_random();
        sample.timestamp = esp_log_system_timestamp();

        // keep trying if queue is full
        do { is_write = xQueueSendToFront(args->queue, (void *) &sample, 20); } while(!is_write);

        vTaskDelay(args->milis);
    }
    vTaskDelete(NULL);
}


void filterTask(void *pvparameters){
    int i;
    struct FilterArgs *args = (struct FilterArgs *) pvparameters;
    struct FilterSample sample;
    struct CircularBuffer queue;
    BaseType_t is_read;
    struct SensorSample sensor_sample;

    init_buffer(&queue);

    while(1){
        // keep trying if queue is empty
        do { is_read = xQueueReceive(args->in_queue, (void *) &sensor_sample, 20); } while(!is_read);
        set_element(&queue, sensor_sample);

        if(size_buffer(&queue) == QUEUE_SIZE) {
            // mean of all samples
            for(i = 0; i < QUEUE_SIZE; i++)
                sample.sample += ((struct SensorSample) get_element(&queue)).sample * WEIGHTS[i];

            // TODO: check how to select a timestamp
            sample.timestamp = sensor_sample.timestamp;
        }
    }

    free_buffer(&queue);
    vTaskDelete(NULL);
}


void app_main() {
    // create input args
    struct SensorArgs s_args1, s_args2;
    struct FilterArgs f_args1, f_args2;
    QueueHandle_t s_queue1, s_queue2, f_queue1, f_queue2;

    s_queue1 = xQueueCreate(QUEUE_SIZE, sizeof(struct SensorSample));
    s_queue2 = xQueueCreate(QUEUE_SIZE, sizeof(struct SensorSample));
    f_queue1 = xQueueCreate(QUEUE_SIZE, sizeof(struct FilterSample));
    f_queue2 = xQueueCreate(QUEUE_SIZE, sizeof(struct FilterSample));

    s_args1.milis = CONFIG_S1_MILLIS;
    s_args1.queue = &s_queue1;

    s_args2.milis = CONFIG_S2_MILLIS;
    s_args2.queue = &s_queue2;

    f_args1.in_queue = &s_queue1;
    f_args1.out_queue = &f_queue1;

    f_args2.in_queue = &s_queue2;
    f_args2.out_queue = &f_queue2;

    xTaskCreatePinnedToCore(&sensorTask, "sensorTask1", 2048, &s_args1, 5, NULL, 0);
    xTaskCreatePinnedToCore(&sensorTask, "sensorTask2", 2048, &s_args2, 5, NULL, 0);
    xTaskCreatePinnedToCore(&filterTask, "filterTask1", 2048, &f_args1, 5, NULL, 1);
    xTaskCreatePinnedToCore(&filterTask, "filterTask2", 2048, &f_args2, 5, NULL, 1);

    while(1) {}

    // delete vars
    vQueueDelete(s_queue1);
    vQueueDelete(s_queue2);
    vQueueDelete(f_queue1);
    vQueueDelete(f_queue2);
}
