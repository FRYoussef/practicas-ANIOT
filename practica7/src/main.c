#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_timer.h"

#define N_SAMPLES 5
static float sample;
static const char* TAG = "Practica7";
// Mount path for the partition
const char *base_path = "/spiflash";
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;


float measure_hall_sensor() {
    int32_t m = 0;

    for(int i = 0; i < N_SAMPLES; i++)
        m += hall_sensor_read();

    return (float)(m / N_SAMPLES);
}


void monitor_sensor() {
    sample = measure_hall_sensor();
    ESP_LOGI(TAG, "Sample: %f", sample);
}


void app_main() {
    esp_timer_handle_t timer;

    // mandatory for hall sensor
    adc1_config_width(ADC_WIDTH_BIT_12);

    const esp_timer_create_args_t timer_args = {
        .callback = &monitor_sensor,
        .name = "timer"
    };

    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, CONFIG_TIMER_DELAY);

    while(1) { vTaskDelay(pdMS_TO_TICKS(CONFIG_TIMER_DELAY)); }

    esp_timer_delete(timer);
}