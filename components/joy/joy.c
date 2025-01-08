#include "esp_adc/adc_oneshot.h"
#include "joy.h"
#include "esp_log.h"

// Maximum joystick displacement in raw ADC values
// Dividing the values returned from joy_get_displacement()
// by this maximum, will give a proportion between 0 and +/- 1.
#define JOY_MAX_DISP 2048
#define JOY_CLK_SRC 0
#define JOY_ADC_CHANNEL_X ADC_CHANNEL_6
#define JOY_ADC_CHANNEL_Y ADC_CHANNEL_7
#define JOY_INIT_AVG_READS 5
#define JOY_DEBUG 0
adc_oneshot_unit_handle_t joy_adc_handle = NULL;
int32_t joy_center_x = 0;
int32_t joy_center_y = 0;


// Initialize the joystick driver. Must be called before use.
// May be called multiple times. Return if already initialized.
// Return zero if successful, or non-zero otherwise.
int32_t joy_init(void) 
{
    if (joy_adc_handle != NULL) {
        return 1; // already initialized
    }
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .clk_src = JOY_CLK_SRC, // 0 is for default
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &joy_adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(joy_adc_handle, JOY_ADC_CHANNEL_X, &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(joy_adc_handle, JOY_ADC_CHANNEL_Y, &chan_cfg));

    int32_t i = 0;
    int32_t x_reads[JOY_INIT_AVG_READS];
    int32_t y_reads[JOY_INIT_AVG_READS];
    for (i = 0; i < JOY_INIT_AVG_READS; i++) {
        joy_get_displacement(&x_reads[i], &y_reads[i]);
    }
    int64_t x_avg = 0;
    int64_t y_avg = 0;
    for (i = 0; i < JOY_INIT_AVG_READS; i++) {
        x_avg += x_reads[i];
        y_avg += y_reads[i];
    }
    x_avg /= JOY_INIT_AVG_READS;
    y_avg /= JOY_INIT_AVG_READS;
    joy_center_x = (int32_t) x_avg;
    joy_center_y = (int32_t) y_avg; 
    if (JOY_DEBUG) printf("joy_center_x: %d\tjoy_center_y: %d\n", (int) joy_center_x, (int) joy_center_y);
    return 0;
}

// Free resources used by the joystick (ADC unit).
// Return zero if successful, or non-zero otherwise.
int32_t joy_deinit(void)
{
    if ( joy_adc_handle == NULL ) {
        return 1; // already unitialized.
    } else {
        ESP_ERROR_CHECK(adc_oneshot_del_unit(joy_adc_handle));
        joy_adc_handle = NULL;
        return 0;
    }
    
}

// Get the joystick displacement from center position.
// Displacement values range from 0 to +/- JOY_MAX_DISP.
// This function is not safe to call from an ISR context.
// Therefore, it must be called from a software task context.
// *dcx: pointer to displacement in x.
// *dcy: pointer to displacement in y.
void joy_get_displacement(int32_t *dcx, int32_t *dcy)
{
    ESP_ERROR_CHECK(adc_oneshot_read(joy_adc_handle, JOY_ADC_CHANNEL_X, (int*) dcx));
    ESP_ERROR_CHECK(adc_oneshot_read(joy_adc_handle, JOY_ADC_CHANNEL_Y, (int*) dcy));
    *dcx -= joy_center_x;
    *dcy -= joy_center_y;
    if (JOY_DEBUG) printf("x_read: %d\ty_read: %d\n", (int) *dcx, (int) *dcy);
}
