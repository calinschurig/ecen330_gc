#include <driver/uart.h>
#include "hw.h"
#include "pin.h"
#include "com.h"

#define PORT_NUM      (UART_NUM_2)
#define BAUD_RATE     (115200)
#define RX_BUFFER_SIZE    (UART_HW_FIFO_LEN(PORT_NUM)*2)

#define TX_PIN HW_EX8
#define RX_PIN HW_EX7
#define RTS_PIN 0
#define CTS_PIN 0

// Initialize the communication channel.
// Return zero if successful, or non-zero otherwise.
int32_t com_init(void) {

    // initializing the configuration for the uart. 
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int32_t intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_set_pin(PORT_NUM, TX_PIN, RX_PIN, RTS_PIN, CTS_PIN));

    ESP_ERROR_CHECK(uart_driver_install(PORT_NUM, RX_BUFFER_SIZE, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(PORT_NUM, &uart_config));

    pin_pullup(RX_PIN, true);
    return !uart_is_driver_installed(PORT_NUM);
}

// Free resources used for communication.
// Return zero if successful, or non-zero otherwise.
int32_t com_deinit(void) {
    if (uart_is_driver_installed(PORT_NUM)) {
        uart_driver_delete(PORT_NUM);
        return 0;
    }
    return 1;
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size) {
    return uart_tx_chars(PORT_NUM, buf, size);
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size) {
    return uart_read_bytes(PORT_NUM, buf, size, 0);
}
