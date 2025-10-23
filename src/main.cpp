// #include <Arduino.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/timer.h"
// #include "driver/gpio.h"     // for gpio_set_level
// #include "esp_timer.h"       // for esp_timer_get_time


// #define LED_GPIO        2               // Change if your board's LED is different
// #define TIMER_GROUP     TIMER_GROUP_0
// #define TIMER_INDEX     TIMER_0
// #define TIMER_DIVIDER   80              // 80 MHz / 80 = 1 MHz (1 tick = 1 µs)
// #define LED_PERIOD_US   100000          // 100,000 µs = 0.1 s toggle => 10 Hz toggle => 5 Hz blink
// void IRAM_ATTR onTimer(void*);          // forward declaration 

// void IRAM_ATTR onTimer(void* arg){
//     (void)arg; // avoid unused parameter warning
//     static bool ledState = false;
//     ledState = !ledState;
//     gpio_set_level((gpio_num_t)LED_GPIO, ledState);
//     timer_group_clr_intr_status_in_isr(TIMER_GROUP, TIMER_INDEX);
//     timer_group_enable_alarm_in_isr(TIMER_GROUP, TIMER_INDEX);
// }

// void timer_stamp_task(void *pvparameters){       // task 1 1khz timestam printer
//     const TickType_t xFrequency = pdMS_TO_TICKS(1);
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     uint64_t t0 = esp_timer_get_time();

//     while(1){
//         uint64_t now_us = esp_timer_get_time() - t0;
//         float now_s = now_us / 1e6f;
//         Serial.printf("Timer : %.3f s\n",now_s);
//         vTaskDelayUntil(&xLastWakeTime,xFrequency);   //wait 1 ms from last wakeup
//     }
// }

// void setup() {
//   Serial.begin(115200);
//   pinMode(LED_GPIO, OUTPUT);
//   digitalWrite(LED_GPIO, LOW);

//   // Configure the hardware timer
//   timer_config_t cfg;
//   cfg.divider = TIMER_DIVIDER;
//   cfg.counter_dir = TIMER_COUNT_UP;
//   cfg.counter_en = TIMER_PAUSE;
//   cfg.alarm_en = TIMER_ALARM_EN;
//   cfg.auto_reload = TIMER_AUTORELOAD_EN;
// #if (ESP_IDF_VERSION_MAJOR >= 4)
//   cfg.intr_type = TIMER_INTR_LEVEL;
//   cfg.clk_src = TIMER_SRC_CLK_APB;
// #endif
//   timer_init(TIMER_GROUP, TIMER_INDEX, &cfg);
//   timer_set_counter_value(TIMER_GROUP, TIMER_INDEX, 0);
//   timer_set_alarm_value(TIMER_GROUP, TIMER_INDEX, LED_PERIOD_US);
//   timer_enable_intr(TIMER_GROUP, TIMER_INDEX);
//   // Register ISR in IRAM
//   timer_isr_register(TIMER_GROUP, TIMER_INDEX, onTimer, NULL, ESP_INTR_FLAG_IRAM, NULL);
//   timer_start(TIMER_GROUP, TIMER_INDEX);

//   // Create the 1 kHz timestamp task on core 1 (avoid Wi-Fi on core 0)
//   xTaskCreatePinnedToCore(timer_stamp_task, "TimestampTask", 4096, NULL, 1, NULL, 1);
// }

// void loop(){

// }