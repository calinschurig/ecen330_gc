#include <stdio.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "watch.h"


#define TIMER_RESOLUTION 1000000
#define ALARM_COUNT 10000
#define ISR_PRINT_COUNT 500

static const char *TAG = "lab03";
volatile uint64_t timer_ticks = 0;
volatile bool running = false;

volatile uint64_t time_start_isr, time_finish_isr;
volatile int32_t isr_cnt;
volatile int64_t isr_max;
void start_time_isr() {
    time_start_isr = esp_timer_get_time();
}
void finish_time_isr() {
    time_finish_isr = esp_timer_get_time();
    int64_t isr_time = time_finish_isr-time_start_isr;
    if ( (isr_time) > isr_max) {
        isr_max = isr_time;
    }
    isr_cnt++;
}
void print_time_isr(const char* section_name) {
    if (isr_cnt > ISR_PRINT_COUNT) {
        printf("%s isr time: %lld microseconds\n", section_name, isr_max);
        isr_cnt = 0;
        isr_max = 0;
    }
}

static bool is_this_a_callback_function(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    start_time_isr();
    if (!pin_get_level(HW_BTN_A)) { // A is pressed
        running = true;
    }
    if (!pin_get_level(HW_BTN_B)) { // B is pressed
        running = false;
    }
    if (!pin_get_level(HW_BTN_START)) { // Start is pressed
        running = false;
        timer_ticks = 0;
    }
    if (running) {
        timer_ticks++;
    }
    finish_time_isr();
    return true;
} 

uint64_t time_start, time_finish;
void start_time() {
    time_start = esp_timer_get_time();
}
void print_time(const char* section_name) {
    time_finish = esp_timer_get_time();
    printf("%s time: %lld microseconds\n", section_name, time_finish-time_start);
}



// Main application
void app_main(void)
{
	ESP_LOGI(TAG, "Starting");

    

    start_time();
    pin_reset(HW_BTN_A);
    pin_input(HW_BTN_A, true);
    pin_reset(HW_BTN_B);
    pin_input(HW_BTN_B, true);
    pin_reset(HW_BTN_START);
    pin_input(HW_BTN_START, true);
    print_time("Configure I/O pins");
    ESP_LOGI(TAG, "Buttons initialized");

    start_time();
    gptimer_handle_t my_gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &my_gptimer));
    // ESP_LOGI(TAG, "Timer initialized");

    gptimer_event_callbacks_t cbs = {
        .on_alarm = is_this_a_callback_function,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(my_gptimer, &cbs, NULL));
    // ESP_LOGI(TAG, "callback function registered? not sure");    

    gptimer_alarm_config_t microsecond_alarm_config = {
        .reload_count = 0,
        .alarm_count = ALARM_COUNT,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(my_gptimer, &microsecond_alarm_config));
    // ESP_LOGI(TAG, "alarm configured");

    ESP_ERROR_CHECK(gptimer_enable(my_gptimer));
    // ESP_LOGI(TAG, "timer enabled");
    ESP_ERROR_CHECK(gptimer_start(my_gptimer));
    // ESP_LOGI(TAG, "timer started");
    print_time("Configure stopwatch timer");

    start_time();
    ESP_LOGI(TAG, "Stopwatch update");
    print_time("ESP_LOGI(TAG, \"Stopwatch update\")");
    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    print_time_isr("interupt");
    for (;;) { // forever update loop
        watch_update(timer_ticks);
        print_time_isr("interrupt");
    }
}




