#include "common.h"


void app_main() {
    int32_t prio = uxTaskPriorityGet(NULL);
    seconds_mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(&timeUpdateTask, "timeUpdateTask", 512, NULL, prio, NULL, 0);
    xTaskCreatePinnedToCore(&switchChronoTask, "switchChronoTask", 512, NULL, prio, NULL, 1);

    while(1) { vTaskDelay(1000); }
}