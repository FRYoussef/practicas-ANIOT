#include <stdio.h>
#include "contiki.h"
#include "sys/clock.h"
#include "dev/leds.h"
#include "board-peripherals.h"
#include "sys/ctimer.h"

#define SENSOR_TIMEOUT (5 * CLOCK_SECOND)

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
static struct ctimer mpu_timer;


static void init_mpu_reading(void *not_used) {
  mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}


static void get_mpu_reading(AccData *sample) {
    sample->x = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
    sample->y = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
    sample->z = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);

    SENSORS_DEACTIVATE(mpu_9250_sensor);
    ctimer_set(&mpu_timer, SENSOR_TIMEOUT, init_mpu_reading, NULL);
}


PROCESS_THREAD(print_process, ev, data) {
    PROCESS_BEGIN();

    AccData value;
    leds_init();
    leds_off(LEDS_GREEN | LEDS_RED);

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


PROCESS_THREAD(sensor_process, ev, data) {

    PROCESS_BEGIN();

    SENSORS_ACTIVATE(reed_relay_sensor);
    init_mpu_reading(NULL);

    sensor_event = process_alloc_event();

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

        get_mpu_reading(&sample);
        process_post_synch(&print_process, sensor_event, (void *) &sample);
    }

    PROCESS_END();
}
