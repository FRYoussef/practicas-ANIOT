#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#define N_SAMPLES 5
#define SAMPLES_TO_GO_SLEEP 5
static const char* TAG = "Practica6";

float measure_hall_sensor() {
    int32_t m = 0;

    for(int i = 0; i < N_SAMPLES; i++)
        m += hall_sensor_read();

    return (float)(m / N_SAMPLES);
}


void go_low_energy_mode(){
    esp_sleep_enable_timer_wakeup(CONFIG_TIMER_DELAY);

#ifdef CONFIG_DEEP_SLEEP
    esp_deep_sleep_start();
#else
    esp_light_sleep_start();
#endif
}


void app_main() {
    float sample;
    int32_t sample_counter = 0;

    // mandatory for hall sensor
    adc1_config_width(ADC_WIDTH_BIT_12);

    while(1) {
        sample = measure_hall_sensor();
        sample_counter++;

        ESP_LOGI(TAG, "Sample: %f", sample);

        if(sample_counter % SAMPLES_TO_GO_SLEEP == 0)
            go_low_energy_mode();

        vTaskDelay(pdMS_TO_TICKS(CONFIG_SAMPLE_DELAY));
    }
}