#include "common.h"
#include "circular_buffer.h"

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