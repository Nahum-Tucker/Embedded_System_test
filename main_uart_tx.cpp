#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"  // For esp_timer functions

static const int TX_BUF_SIZE = 256;
#define TXD_PIN (GPIO_NUM_5)
#define UART_NUM UART_NUM_2

static const char *TX_TASK_TAG = "TIMER_TX";
const char characters[] = "ABCDEF";  // Character set to cycle
const size_t num_chars = sizeof(characters) - 1;  // Exclude null terminator

static uint32_t counter = 0;
static size_t char_index = 0;  // Avoid naming conflict with any existing "index" macro

// Initialize UART
static void init_esp32_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM, TX_BUF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

// Send data wrapper
int sendData(const char* logName, const char* data, size_t lenData)
{
    return uart_write_bytes(UART_NUM, data, lenData);
}

// Timer callback function
void periodic_timer_callback(void* arg)
{
    char buffer[64];
    char current_char = characters[char_index];
    int len = snprintf(buffer, sizeof(buffer), "%c,%" PRIu32 "\n", current_char, counter);
    int txBytes = sendData(TX_TASK_TAG, buffer, len);

    if (txBytes > 0) {
        printf("Sent: %s", buffer);
    } else {
        ESP_LOGE(TX_TASK_TAG, "Failed to send data");
    }

    counter++;
    char_index = (char_index + 1) % num_chars;  // Cycle through characters
}

extern "C" void app_main(void)
{
    init_esp32_uart();

    esp_timer_handle_t periodic_timer;

    esp_timer_create_args_t timer_args = {
        .callback = &periodic_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,       // Required field
        .name = "uart_timer",
        .skip_unhandled_events = false,          // Required field
    };

    esp_err_t err = esp_timer_create(&timer_args, &periodic_timer);
    if (err != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to create timer: %s", esp_err_to_name(err));
        return;
    }

    // Start the timer: every 10,000,000 µs = 10 seconds
    esp_timer_start_periodic(periodic_timer, 10000000);
}

