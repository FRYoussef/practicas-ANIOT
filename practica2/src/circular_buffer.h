#ifndef _CBUFFER_
#define _CBUFFER_

#include "common.h"

typedef struct CircularBuffer {
   int32_t  counter;
   int32_t size;
   struct SensorSample *values;
};

void init_buffer(struct CircularBuffer *queue, int32_t size);
void free_buffer(struct CircularBuffer *queue);
int32_t size_buffer(struct CircularBuffer *queue);
struct SensorSample get_element(struct CircularBuffer *queue);
void set_element(struct CircularBuffer *queue, struct SensorSample element);

#endif