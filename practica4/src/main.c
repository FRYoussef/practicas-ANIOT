#include "common.h"


void eventTaskLogic(uint32_t ev, struct TaskSignals signals) {
    BaseType_t q_ready;

    while(1) {
        // wait till sem is released
        do { q_ready = xSemaphoreTake(signals.sem, 100 ); } while(!q_ready);

        // keep trying if queue is full
        do { q_ready = xQueueSendToFront(signals.queue, (void *) ev, 100); } while(!q_ready);
    }
}


void timerTask(void *pvparameters) {
    struct TaskSignals *signals = (struct TaskSignals *) pvparameters;
    eventTaskLogic(EV_ONE_SEC, *signals);
}


void touchSensorTask(void *pvparameters) {
    struct TaskSignals *signals = (struct TaskSignals *) pvparameters;
    eventTaskLogic(EV_ONE_SEC, *signals);
}


void FSMTask(void *pvparameters) {
    QueueHandle_t *queue = (QueueHandle_t *) pvparameters;

    //TODO
}


static void IRAM_ATTR timerIsr(void *args) {
    SemaphoreHandle_t *timerSem = (SemaphoreHandle_t *) args;
    BaseType_t hpTask = pdFALSE;

    xSemaphoreGiveFromISR(timerSem, &hpTask);

    if (hpTask == pdTRUE)
        portYIELD_FROM_ISR();
}


static void touchSensorIsr(void *args) {
    SemaphoreHandle_t sem = (SemaphoreHandle_t) args;
    BaseType_t hpTask = pdFALSE;
    bool activated = false;
    uint32_t pad_intr = touch_pad_get_status();
    touch_pad_clear_status();

    for (int i = 0; i < TOUCH_PAD_MAX; i++)
        if ((pad_intr >> i) & 0x01)
                activated |= true;

    if (activated) {
        xSemaphoreGiveFromISR(sem, &hpTask);

        if (hpTask == pdTRUE)
            portYIELD_FROM_ISR();
    }
}


void chronoCallback() {
    // TODO
}


void resetTimerCallback() {

    // TODO
}


void app_main() {
    int32_t prio = uxTaskPriorityGet(NULL);

    QueueHandle_t event_queue = xQueueCreate(CONFIG_QUEUE_SIZE, sizeof(unsigned char));
    SemaphoreHandle_t touchSensorSem = xSemaphoreCreateBinary();
    SemaphoreHandle_t timerSem = xSemaphoreCreateBinary();
    struct TaskSignals touch_sig, timer_sig;

    touch_sig.queue = event_queue;
    touch_sig.sem = touchSensorSem;

    timer_sig.queue = event_queue;
    timer_sig.sem = timerSem;

    // block semaphores
    xSemaphoreTake(touchSensorSem, 100);
    xSemaphoreTake(timerSem, 100);

    // gpio configuration
    // output pin
    gpio_config_t io_conf; 
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en= 0;
    gpio_config(&io_conf);

    //input pin
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, timerIsr, (void*) timerSem);

    // touch pad configuration
    touch_pad_init();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);

    for (int i = 0; i< TOUCH_PAD_MAX; i++)
        touch_pad_config(i, 0);

    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    touch_pad_read_filtered(0, &pad_val);
    touch_pad_isr_register(touchSensorIsr, &touchSensorSem);
    touch_pad_intr_enable();

    // timers configuration
    TimerHandle_t chrono = xTimerCreate("chrono",    // Just a text name, not used by the kernel.
                         ( CONFIG_CHRONO_TIME * portTICK_PERIOD_MS ),   // The timer period in ticks.
                         pdTRUE,        // The timers will auto-reload themselves when they expire.
                         ( void * ) CHRONO_TIMER_ID,  // Assign each timer a unique id equal to its array index.
                         chronoCallback // Each timer calls the same callback when it expires.
    );

    TimerHandle_t resetTimer = xTimerCreate("resetTimer",    // Just a text name, not used by the kernel.
                             ( CONFIG_CHRONO_TIME * portTICK_PERIOD_MS ),   // The timer period in ticks.
                             pdTRUE,        // The timers will auto-reload themselves when they expire.
                             ( void * ) RESET_TIMER_ID,  // Assign each timer a unique id equal to its array index.
                             resetTimerCallback // Each timer calls the same callback when it expires.
    );

    // tasks configutation
    xTaskCreatePinnedToCore(&touchSensorTask, "touchSensorTask", 1024, &touch_sig, prio, NULL, 0);
    xTaskCreatePinnedToCore(&timerTask, "timerTask", 1024, &timer_sig, prio, NULL, 0);
    xTaskCreatePinnedToCore(&FSMTask, "FSMTask", 3096, &event_queue, prio, NULL, 1);

    while(1) { vTaskDelay(1000); }
}
