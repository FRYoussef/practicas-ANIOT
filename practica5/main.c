#include <stdio.h>
#include "contiki.h"
#include "sys/clock.h"
#include "dev/leds.h"

#define SENSOR_TIMEOUT 5

typedef struct {
    int32_t  x;
    int32_t  y;
    int32_t  z;
} AccData;

PROCESS(sensor_process, "Sensor process");
PROCESS(print_process, "Print process");
AUTOSTART_PROCESSES(&print_process, &sensor_process);

static process_event_t sensor_event;
static AccData sample;
static int counter;

PROCESS_THREAD(print_process, ev, data)
{
    PROCESS_BEGIN();
    leds_init();
    leds_off(LEDS_GREEN | LEDS_RED);
    AccData value;

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensor_event);
        value = *((AccData *)data);

        printf("x = %ld, y = %ld, z = %ld\n", value.x, value.y, value.z);

        if(value.z >= 0){
            printf("The SensorTag is face up.\n\n");
            leds_on(LEDS_GREEN);
            leds_off(LEDS_RED);
        }
        else {
            printf("The SensorTag is face down.\n\n");
            leds_on(LEDS_RED);
            leds_off(LEDS_GREEN);
        }
    }

    PROCESS_END();
}

PROCESS_THREAD(sensor_process, ev, data)
{
    PROCESS_BEGIN();

    sensor_event = process_alloc_event();

    while(1) {
        
        counter++;
        if(counter % 2 == 0)
            sample.z = 1;
        else
            sample.z = -1;

        process_post_synch(&print_process, sensor_event, (void *) &sample);
        clock_wait(SENSOR_TIMEOUT * CLOCK_SECOND);
    }

    PROCESS_END();
}
