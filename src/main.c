#include "freertos/FreeRTOS.h"

#include "nvs_flash.h"
#include "esp_event.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "defines.h"

static void uart_init() {
	exchange_t* gExchangeData = initExchange();

	uart_config_t uart_config = {
		.baud_rate = 9600,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	uart_param_config(UART_NUM_1, &uart_config);
	uart_driver_install(UART_NUM_1, 129, 0, 10, &(gExchangeData->uartQueue), 0);
	uart_set_pin(UART_NUM_1, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void app_main() {
	uart_init();

	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( esp_event_loop_create_default() );

	// ToDo: xTaskCreate(<UART read loop>, "gps_read", 4*1024, (void*)(exchangeData), 5, NULL);
}