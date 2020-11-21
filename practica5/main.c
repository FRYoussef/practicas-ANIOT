#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"

#define SENSOR_TIMEOUT 5000

PROCESS(sensor_process, "sensor_process");
PROCESS(print_process, "print_process");
AUTOSTART_PROCESSES(&sensor_process, &print_process);

static process_event_t sensor_event;

PROCESS_THREAD(sensor_process, ev, data)
{
    PROCESS_BEGIN();

    sensor_event = process_alloc_event();

    while(1) {

        process_post_synch(&print_process, sensor_event, NULL);

        delay(SENSOR_TIMEOUT);
    }

    PROCESS_END();
}


PROCESS_THREAD(print_process, ev, data)
{
    PROCESS_BEGIN();

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensor_event);




        printf("\n");
    }

    PROCESS_END();
}
