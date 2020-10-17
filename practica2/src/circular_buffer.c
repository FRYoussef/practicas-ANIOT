#include "common.h"
#include "circular_buffer.h"

void init_buffer(struct CircularBuffer *queue, int32_t size){
    int i;

    queue->size = size;
    queue->values = (struct SensorSample *) malloc(queue->size * sizeof(struct SensorSample));
    queue->counter = 0;

    for(i = 0; i < queue->size; i++)
        queue->values[i].sample = 0;
}

void free_buffer(struct CircularBuffer *queue){
    free(queue->values);
}

int32_t buffer_elements(struct CircularBuffer *queue){
    return queue->counter;
}

struct SensorSample get_element(struct CircularBuffer *queue){
    queue->counter %= queue->size;
    return queue->values[queue->counter++];
}

void set_element(struct CircularBuffer *queue, struct SensorSample element){
    queue->counter %= queue->size;
    queue->values[queue->counter++] = element;
}