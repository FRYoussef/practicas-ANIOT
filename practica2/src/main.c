#include "common.h"
#include "circular_buffer.h"


void sensorTask(void *pvparameters){
    struct SensorArgs *args = (struct SensorArgs *) pvparameters;
    struct SensorSample sample;
    BaseType_t is_write;
    time_t now;

    while(1){
        // just takes values inside int32_t [INT32_MIN, INT32_MAX]
        sample.sample = (int32_t) esp_random();
        time(&now);
        localtime_r(&now, &sample.timestamp);

        //sample.timestamp = esp_log_system_timestamp();

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

    init_buffer(&queue, QUEUE_SIZE);

    while(1){
        // keep trying if queue is empty
        do { is_read = xQueueReceive(args->in_queue, (void *) &sensor_sample, 20); } while(!is_read);
        set_element(&queue, sensor_sample);

        if(size_buffer(&queue) == QUEUE_SIZE) {
            // mean of all samples
            for(i = 0; i < QUEUE_SIZE; i++)
                sample.sample += ((struct SensorSample) get_element(&queue)).sample * WEIGHTS[i];

            // Gets last timestamp
            sample.timestamp = sensor_sample.timestamp;
        }
    }

    free_buffer(&queue);
    vTaskDelete(NULL);
}

/*
 * Example of QueueSet which allows manage multiple queues: 
 * https://github.com/FreeRTOS/FreeRTOS/blob/master/FreeRTOS/Demo/Common/Minimal/QueueSet.c
 */
void controllerTask(void *pvparameters){
    QueueSetHandle_t *set = (QueueSetHandle_t *) pvparameters;
    QueueSetMemberHandle_t which_queue;
    struct FilterSample sample;
    char buff[1024];

    while(1){
        // read from set queue
        which_queue = xQueueSelectFromSet(set, 1);

        if(which_queue != NULL) {
            strftime(buff, sizeof(buff), "%c", &sample.timestamp);
            printf("Task %i has set %5.4f value. The last sample was taken in %s\n", , sample.sample, buff); // TODO: write task number
        }

        // print system tasks info
        printf("\n---------------------------------\n");
        vTaskGetRunTimeStats(&buff);
        printf("%s\n", buff);

        vTaskDelay(CONTROLLER_SLEEP);
    }

    vTaskDelete(NULL);
}


void app_main() {
    // create input args
    struct SensorArgs s_args1, s_args2;
    struct FilterArgs f_args1, f_args2;
    QueueHandle_t s_queue1, s_queue2, f_queue1, f_queue2;
    QueueSetHandle_t set;

    s_queue1 = xQueueCreate(QUEUE_SIZE, sizeof(struct SensorSample));
    s_queue2 = xQueueCreate(QUEUE_SIZE, sizeof(struct SensorSample));
    f_queue1 = xQueueCreate(QUEUE_SIZE, sizeof(struct FilterSample));
    f_queue2 = xQueueCreate(QUEUE_SIZE, sizeof(struct FilterSample));

    // size = size_of(f_queue1) + size_of(f_queue2) -> max uxEventQueueLength
    set      = xQueueCreateSet(QUEUE_SIZE + QUEUE_SIZE); 

    s_args1.milis = CONFIG_S1_MILLIS;
    s_args1.queue = &s_queue1;

    s_args2.milis = CONFIG_S2_MILLIS;
    s_args2.queue = &s_queue2;

    f_args1.in_queue = &s_queue1;
    f_args1.out_queue = &f_queue1;

    f_args2.in_queue = &s_queue2;
    f_args2.out_queue = &f_queue2;

    xQueueAddToSet(f_queue1, set);
    xQueueAddToSet(f_queue2, set);

    xTaskCreatePinnedToCore(&sensorTask, "sensorTask1", 2048, &s_args1, 5, NULL, 0);
    xTaskCreatePinnedToCore(&sensorTask, "sensorTask2", 2048, &s_args2, 5, NULL, 0);
    xTaskCreatePinnedToCore(&filterTask, "filterTask1", 2048, &f_args1, 5, NULL, 1);
    xTaskCreatePinnedToCore(&filterTask, "filterTask2", 2048, &f_args2, 5, NULL, 1);
    xTaskCreatePinnedToCore(&controllerTask, "controllerTask", 2048, &set, 5, NULL, 1);

    while(1) { vTaskDelay(1000); }

    // delete vars
    vQueueDelete(s_queue1);
    vQueueDelete(s_queue2);
    vQueueDelete(f_queue1);
    vQueueDelete(f_queue2);
}
