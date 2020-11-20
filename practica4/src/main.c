#include "common.h"

void timerTask(void *pvparameters) {
    struct Signal *signals = (struct Signal *) pvparameters;
    static fsm_event ev = one_sec;
    //eventTaskLogic(&ev, signals);
    BaseType_t q_ready;

    while(1) {
        // wait till sem is released
        do { q_ready = xSemaphoreTake(signals->sem, 1000); } while(!q_ready);
        // keep trying if queue is full
        do { q_ready = xQueueSendToFront(signals->queue, (void *) &ev, 100); } while(!q_ready);
    }
    vTaskDelete(NULL);
}


void touchSensorTask(void *pvparameters) {
    struct Signal *signals = (struct Signal *) pvparameters;
    static fsm_event ev = start_stop;
    //eventTaskLogic(&ev, signals);
    BaseType_t q_ready;
    int32_t value = 0;
    u_char pool_it = 0;

    while(1) {
        // wait till sem is released
        do { q_ready = xSemaphoreTake(signals->sem, 1000); } while(!q_ready);

        // keep trying if queue is full
        do { q_ready = xQueueSendToFront(signals->queue, (void *) &ev, 100); } while(!q_ready);

        // wait till touchpad(0) is released
        touch_pad_read_filtered(0, &value);
        while(value < (pad0_init - TOUCH_PAD_MARGIN) && pool_it < MAX_POOL_IT) {
            vTaskDelay(BOUNCE_TIME);
            touch_pad_read_filtered(0, &value);
            pool_it++;
        }

        // restore touchpad isr
        touch_pad_intr_enable();
        pool_it = 0;
        
    }

    vTaskDelete(NULL);
}


void FSMTask(void *pvparameters) {
    QueueHandle_t *event_queue = (QueueHandle_t *) pvparameters;
    fsm_state state = initial;
    fsm_event ev;
    BaseType_t q_ready;
    int seconds;
    void (* foo)(int *, fsm_event *);

    while(1) {
        do { q_ready = xQueueReceive(*event_queue, (void *) &ev, 1000); } while(!q_ready);

        // next state
        state = state_transitions[state][ev];
        
        // event function
        foo = state_foo[state];
        foo(&seconds, &ev);
    }
    vTaskDelete(NULL);
}


void hallSensorResetTask(void *pvparameters) {
    QueueHandle_t *event_queue = (QueueHandle_t *) pvparameters;
    static fsm_event reset_ev = reset;
    u_char pool_it = 0;

    while(1){
        if(hall_sensor_read() < CONFIG_HALL_THRESHOLD) {
            xQueueSendToFront(*event_queue, (void *) &reset_ev, 100);

            // wait until the sensor stops detecting interaction
            do {
                vTaskDelay(BOUNCE_TIME);
                pool_it++;
            } while(hall_sensor_read() < (CONFIG_HALL_THRESHOLD + HALL_MARGIN) && pool_it < MAX_POOL_IT);

            pool_it = 0;
        }

        vTaskDelay(CONFIG_RESET_POLLING_TIME);
    }
    vTaskDelete(NULL);
}


void infraredSensorResetTask(void *pvparameters) {
    struct InfraredParams *params = (struct InfraredParams *) pvparameters;
    static fsm_event reset_ev = reset;
    u_char pool_it = 0;

    while(1){
        if(get_distance(params->adc_chars) < INFRARED_FIRE_DISTANCE) {
            xQueueSendToFront(params->queue, (void *) &reset_ev, 100);

            // wait until the sensor stops detecting interaction
            do{
                vTaskDelay(BOUNCE_TIME);
                pool_it++;
            }while(get_distance(params->adc_chars) < INFRARED_FIRE_DISTANCE && pool_it < MAX_POOL_IT);

            pool_it = 0;
        }

        vTaskDelay(CONFIG_RESET_POLLING_TIME);
    }
    vTaskDelete(NULL);
}


float get_distance(esp_adc_cal_characteristics_t *adc_chars) {
    uint32_t adc_reading = 0;

    for(int i = 0; i < INFRARED_SAMPLES; i++)
        adc_reading += adc1_get_raw((adc1_channel_t)ADC_CHANNEL_4);

    adc_reading /= INFRARED_SAMPLES;

    float voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    // mV -> V
    voltage /= 1000;

    if (voltage < MIN_VOLTAGE)
        return MAXFLOAT;

    return (12.37 * pow(voltage, -1.1f));
}


static void IRAM_ATTR timer_isr_handler(void *arg) {
    SemaphoreHandle_t *timerSem = (SemaphoreHandle_t *) arg;
    BaseType_t hpTask = pdFALSE;

    xSemaphoreGiveFromISR(*timerSem, &hpTask);

    if (hpTask == pdTRUE)
        portYIELD_FROM_ISR();
}


static void touchSensorIsr(void *args) {
    touch_pad_intr_disable();

    SemaphoreHandle_t *sem = (SemaphoreHandle_t *) args;
    BaseType_t hpTask = pdFALSE;

    touch_pad_clear_status();
    xSemaphoreGiveFromISR(*sem, &hpTask);

    if (hpTask == pdTRUE)
        portYIELD_FROM_ISR();
}


static void chronoCallback(void *arg) {
    pin_state = (pin_state + 1) % 2;
    gpio_set_level(GPIO_OUTPUT_IO_0, pin_state%2);
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

    touchSignals.queue = event_queue;
    touchSignals.sem = touchSensorSem;
    timerSignals.queue = event_queue;
    timerSignals.sem = timerSem;

    // also mandatory for hall sensor
    adc1_config_width(ADC_WIDTH_BIT_12);

#ifndef CONFIG_ENABLE_HALL_SENSOR
    // infrared configuration
    adc1_config_channel_atten(ADC_CHANNEL_4, ADC_ATTEN_DB_11);
    esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    struct InfraredParams ifrared_params;
    ifrared_params.queue = event_queue;
    ifrared_params.adc_chars = adc_chars;
#endif

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

    // touch-pad 0 configuration
    touch_pad_init();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    // first configuration is to get the touchpad idle voltage 
    touch_pad_config(0, 0);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    touch_pad_read_filtered(0, &pad0_init);

    // now config the tochpad to launch the ISR at a margin drop voltage.
    // margin is 10% of touchpad idle voltage
    TOUCH_PAD_MARGIN = (u_int32_t) pad0_init / 10;
    touch_pad_filter_stop();
    touch_pad_config(0, pad0_init - TOUCH_PAD_MARGIN);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);

    touch_pad_isr_register(touchSensorIsr, &touchSensorSem);
    touch_pad_intr_enable();

    // timers configuration
    esp_timer_handle_t chrono_timer;

    const esp_timer_create_args_t chrono_args = {
        .callback = &chronoCallback,
        .name = "chrono_timer"
    };

    esp_timer_create(&chrono_args, &chrono_timer);

    // tasks configutation
    xTaskCreatePinnedToCore(&touchSensorTask, "touchSensorTask", 3096, &touchSignals, prio, NULL, 0);
    xTaskCreatePinnedToCore(&timerTask, "timerTask", 3096, &timerSignals, prio, NULL, 0);
    xTaskCreatePinnedToCore(&FSMTask, "FSMTask", 3096, &event_queue, prio, NULL, 1);

#ifdef CONFIG_ENABLE_HALL_SENSOR
    xTaskCreatePinnedToCore(&hallSensorResetTask, "hallSensorResetTask", 1024, &event_queue, prio, NULL, 1);
#else
    xTaskCreatePinnedToCore(&infraredSensorResetTask, "infraredSensorResetTask", 2048, &ifrared_params, prio, NULL, 1);
#endif

    // start timers
    esp_timer_start_periodic(chrono_timer, CONFIG_CHRONO_TIME);

    while(1) { vTaskDelay(1000); }

    esp_timer_delete(chrono_timer);
#ifndef CONFIG_ENABLE_HALL_SENSOR
    free(adc_chars);
#endif
}
