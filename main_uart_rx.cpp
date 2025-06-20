#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define RXD_PIN (GPIO_NUM_16)   // Adjust based on your wiring
#define UART_NUM UART_NUM_2
#define RX_BUF_SIZE 256

static const char *RX_TASK_TAG = "UART_RX";

static void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void rx_task(void *arg)
{
    uint8_t data[RX_BUF_SIZE];

    while (1) {
        int len = uart_read_bytes(UART_NUM, data, RX_BUF_SIZE - 1, pdMS_TO_TICKS(11000));  // 11 sec timeout
        if (len > 0) {
            data[len] = '\0';  // Null-terminate
            printf("Received: %s", (char *)data);
        }
    }
}

extern "C" void app_main(void)
{
    init_uart();
    xTaskCreate(rx_task, "uart_rx_task", 2048, NULL, 10, NULL);
}

