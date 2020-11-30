#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/adc.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_pm.h"

#define N_SAMPLES 5
#define SAMPLES_TO_GO_SLEEP 5
#define CONFIG_EXAMPLE_MAX_CPU_FREQ_MHZ
static const char* TAG = "Practica6";


void print_wakeup_cause(esp_sleep_wakeup_cause_t cause){
    switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:
        ESP_LOGI(TAG, "Processor has been waked up by a timer.");
        break;
    
    default:
        ESP_LOGI(TAG, "Processor has been waked up by an undefined cause.");
        break;
    }
}


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
#elif CONFIG_LIGHT_SLEEP
    esp_light_sleep_start();
    print_wakeup_cause(esp_sleep_get_wakeup_cause());
#endif

}


void app_main() {
#ifdef CONFIG_DEEP_SLEEP
    print_wakeup_cause(esp_sleep_get_wakeup_cause());
#endif

    float sample;
    int32_t sample_counter = 0;

    // mandatory for hall sensor
    adc1_config_width(ADC_WIDTH_BIT_12);

#ifdef CONFIG_PM_ENABLE
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = CONFIG_MAX_CPU_FREQ_MHZ,
        .min_freq_mhz = CONFIG_MIN_CPU_FREQ_MHZ,
        .light_sleep_enable = true };

    esp_pm_configure(&pm_config);
#endif

    while(1) {
        sample = measure_hall_sensor();
        sample_counter++;

        ESP_LOGI(TAG, "Sample: %f", sample);

#ifndef NON_SLEEP
        if(sample_counter % SAMPLES_TO_GO_SLEEP == 0)
            go_low_energy_mode();
#endif
        vTaskDelay(pdMS_TO_TICKS(CONFIG_SAMPLE_DELAY));
#ifdef CONFIG_PM_ENABLE
        print_wakeup_cause(esp_sleep_get_wakeup_cause());
#endif
    }
}