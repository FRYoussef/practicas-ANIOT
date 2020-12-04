#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_timer.h"

#define N_SAMPLES 5
#define MAX_LINE_LENGTH 80

static const char* TAG = "Practica7";
// Mount path for the partition
const char *base_path = "/spiflash";
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;


int log_printf(const char *fmt, va_list args) {
    int result;
    FILE *f = fopen("/spiflash/log.txt", "a");

    result = vfprintf(f, fmt, args);

    fclose(f);
    return result;
}


float measure_hall_sensor() {
    int32_t m = 0;

    for(int i = 0; i < N_SAMPLES; i++)
        m += hall_sensor_read();

    return (float)(m / N_SAMPLES);
}


void monitor_sensor() {
    ESP_LOGI(TAG, "Sample: %f", measure_hall_sensor());
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

    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
    
    esp_log_set_vprintf(&log_printf);

    // store time before launch the timer
    bool wake_up = false;
    int init_time = time(NULL);
    esp_timer_start_periodic(timer, CONFIG_TIMER_DELAY);

    while(!wake_up) {
        vTaskDelay(pdMS_TO_TICKS(500));
        
        if((time(NULL) - init_time) >= CONFIG_TIME_TO_STOP) {
            esp_timer_stop(timer);
            wake_up = true;
        }
    }

    // read and print first n samples
    FILE *f = fopen("/spiflash/log.txt", "r");
    char line[MAX_LINE_LENGTH] = {0};
    int nsamples = 0;

    // Ends when it found n samples or with EOF.
    while((nsamples < CONFIG_SHOW_N_SAMPLES) && fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        printf("%s", line);
        nsamples++;
    }

    fclose(f);
    esp_timer_delete(timer);
    esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle);
}