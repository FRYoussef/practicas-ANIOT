#include <stdio.h>
#include "contiki.h"
#include "sys/clock.h"
#include "dev/leds.h"

#define SENSOR_TIMEOUT 5

PROCESS(sensor_process, "Sensor process");
PROCESS(print_process, "Print process");
AUTOSTART_PROCESSES(&print_process, &sensor_process);

static process_event_t sensor_event;
static int32_t example = 0;

PROCESS_THREAD(print_process, ev, data)
{
    PROCESS_BEGIN();
    leds_init();
    leds_off(LEDS_GREEN);
    int32_t value;

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensor_event);
        value = *((int32_t *)data);

        printf("%ld\n", value);
        leds_toggle(LEDS_GREEN);
    }

    PROCESS_END();
}

PROCESS_THREAD(sensor_process, ev, data)
{
    PROCESS_BEGIN();

    sensor_event = process_alloc_event();

    while(1) {
        
        example++;
        process_post_synch(&print_process, sensor_event, (void *) &example);
        clock_wait(SENSOR_TIMEOUT * CLOCK_SECOND);
    }

    PROCESS_END();
}
