#include "common.h"

void eventTaskLogic(fsm_event *ev, struct Signal *signals) {
    BaseType_t q_ready;

    while(1) {
        // wait till sem is released
        do { q_ready = xSemaphoreTake(*signals->sem, 1000); } while(!q_ready);
        // keep trying if queue is full
        do { q_ready = xQueueSendToFront(*signals->queue, (void *) ev, 100); } while(!q_ready);
    }
}


void timerTask(void *pvparameters) {
    struct Signal *signals = (struct Signal *) pvparameters;
    fsm_event ev = one_sec;
    eventTaskLogic(&ev, signals);
    vTaskDelete(NULL);
}


void touchSensorTask(void *pvparameters) {
    struct Signal *signals = (struct Signal *) pvparameters;
    fsm_event ev = start_stop;
    eventTaskLogic(&ev, signals);
    vTaskDelete(NULL);
}


void FSMTask(void *pvparameters) {
    QueueHandle_t *event_queue = (QueueHandle_t *) pvparameters;
    fsm_state state = initial;
    fsm_event ev;
    BaseType_t q_ready;
    int seconds;
    void (* foo)(int*);

    while(1) {
        do { q_ready = xQueueReceive(*event_queue, (void *) &ev, 1000); } while(!q_ready);

        // next state
        state = state_transitions[state][ev];
        
        // event function
        foo = state_foo[state];
        foo(&seconds);
    }
    vTaskDelete(NULL);
}


static void IRAM_ATTR timer_isr_handler(void *arg) {
    SemaphoreHandle_t *timerSem = (SemaphoreHandle_t *) arg;
    BaseType_t hpTask = pdFALSE;

    xSemaphoreGiveFromISR(*timerSem, &hpTask);

    if (hpTask == pdTRUE)
        portYIELD_FROM_ISR();
}


static void touchSensorIsr(void *args) {
    SemaphoreHandle_t *sem = (SemaphoreHandle_t *) args;
    BaseType_t hpTask = pdFALSE;
    // bool activated = false;
    // uint32_t pad_intr = touch_pad_get_status();
    touch_pad_clear_status();

    // for (int i = 0; i < TOUCH_PAD_MAX; i++)
    //     if ((pad_intr >> i) & 0x01)
    //             activated |= true;

    // if (activated) {
        xSemaphoreGiveFromISR(*sem, &hpTask);

        if (hpTask == pdTRUE)
            portYIELD_FROM_ISR();
    //}
}


static void chronoCallback(void *arg) {
    pin_state = (pin_state + 1) % 2;
    gpio_set_level(GPIO_OUTPUT_IO_0, pin_state%2);
}


static void resetTimerCallback(void *arg) {
    QueueHandle_t *event_queue = (QueueHandle_t *) arg;

    if(hall_sensor_read() < CONFIG_HALL_THRESHOLD) {
        xQueueSendToFront(*event_queue, (void *) &reset_ev, 100);
    }
}


void app_main() {
    int32_t prio = uxTaskPriorityGet(NULL);

    QueueHandle_t event_queue = xQueueCreate(CONFIG_QUEUE_SIZE, sizeof(fsm_event));
    SemaphoreHandle_t touchSensorSem = xSemaphoreCreateBinary();
    SemaphoreHandle_t timerSem = xSemaphoreCreateBinary();
    struct Signal touchSignals, timerSignals;

    // blocks semaphores
    xSemaphoreTake(touchSensorSem, 0);
    xSemaphoreTake(timerSem, 0);

    touchSignals.queue = &event_queue;
    touchSignals.sem = &touchSensorSem;
    timerSignals.queue = &event_queue;
    timerSignals.sem = &timerSem;

    // mandatory for hall sensor
    adc1_config_width(ADC_WIDTH_BIT_12);

    // // gpio configuration
    // output pin
    gpio_config_t io_conf; 
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //input pin
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, timer_isr_handler, (void*) &timerSem);

    // touch pad configuration
    touch_pad_init();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    touch_pad_config(0, CONFIG_TOUCH_THRESHOLD);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    touch_pad_isr_register(touchSensorIsr, &touchSensorSem);
    touch_pad_intr_enable();


    // timers configuration
    const esp_timer_create_args_t chrono_args = {
        .callback = &chronoCallback,
        .name = "chrono_timer"
    };
    const esp_timer_create_args_t reset_args = {
        .callback = &resetTimerCallback,
        .arg = (void *) &event_queue,
        .name = "reset_timer"
    };

    esp_timer_handle_t chrono_timer, reset_timer;
    esp_timer_create(&chrono_args, &chrono_timer);
    esp_timer_create(&reset_args, &reset_timer);

    // tasks configutation
    xTaskCreatePinnedToCore(&touchSensorTask, "touchSensorTask", 3096, &touchSignals, prio, NULL, 0);
    xTaskCreatePinnedToCore(&timerTask, "timerTask", 3096, &timerSignals, prio, NULL, 0);
    xTaskCreatePinnedToCore(&FSMTask, "FSMTask", 3096, &event_queue, prio, NULL, 1);

    // start timers
    esp_timer_start_periodic(chrono_timer, CONFIG_CHRONO_TIME);
    esp_timer_start_periodic(reset_timer, CONFIG_RESET_POLLING_TIME);

    while(1) { vTaskDelay(1000); }

    esp_timer_delete(chrono_timer);
    esp_timer_delete(reset_timer);
}
