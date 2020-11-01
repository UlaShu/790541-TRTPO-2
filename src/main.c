#include <freertos/FreeRTOS.h>

#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_wifi.h>

#include <driver/uart.h>
#include <driver/gpio.h>

#include "defines.h"
#include "cfg.h"


static esp_err_t event_handler(void *ctx, system_event_t *event) {
	static uint8_t apStationsCount=0;
	exchange_t* gExchangeData;

	while ( (gExchangeData=initExchange()) == NULL )
		vTaskDelay(100 / portTICK_PERIOD_MS);

	while (gExchangeData->mainEventGroup == NULL)
		vTaskDelay(100 / portTICK_PERIOD_MS);

	wifi_mode_t wMode;
	esp_wifi_get_mode(&wMode);

	switch (event->event_id) {
	case SYSTEM_EVENT_AP_STACONNECTED:
		apStationsCount++;
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		apStationsCount--;
		if(apStationsCount == 0 && wMode == WIFI_MODE_AP) {
			xEventGroupSetBits(gExchangeData->mainEventGroup, WIFI_DISCONNECT_BIT);
		}
		break;
	case SYSTEM_EVENT_AP_STAIPASSIGNED:
		xEventGroupSetBits(gExchangeData->mainEventGroup, WIFI_CONNECTED_BIT);
		break;
	default:
		break;
	}

	return ESP_OK;
}


static void wifi_init(void) {
	tcpip_adapter_init();
	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

	wifi_config_int_t *wifiConfigInt = config_wifi();
	if (wifiConfigInt->mode != WIFI_MODE_AP && wifiConfigInt->mode != WIFI_MODE_APSTA)
		wifiConfigInt->mode = WIFI_MODE_AP;

	ESP_ERROR_CHECK( esp_wifi_set_mode(wifiConfigInt->mode) );

	static wifi_config_t wifi_esp_config;
	strncpy((char*)wifi_esp_config.ap.ssid, (char*)wifiConfigInt->ap.ssid, sizeof(wifi_esp_config.ap.ssid));
	strncpy((char*)wifi_esp_config.ap.password, (char*)wifiConfigInt->ap.password, sizeof(wifi_esp_config.ap.password));

	wifi_esp_config.ap.ssid_len = strnlen((char*)wifiConfigInt->ap.ssid, 32);
	wifi_esp_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

	if (strnlen((char*)wifiConfigInt->ap.password, 16) == 0)
		wifi_esp_config.ap.authmode = WIFI_AUTH_OPEN;

	wifi_esp_config.ap.ssid_hidden = false;
	wifi_esp_config.ap.max_connection = 3;

	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_esp_config) );

	ESP_ERROR_CHECK( esp_wifi_start() );
	ESP_ERROR_CHECK( esp_wifi_set_max_tx_power(-128) );
}


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

	wifi_init();

	// ToDo: xTaskCreate(<UART read loop>, "gps_read", 4*1024, (void*)(exchangeData), 5, NULL);
}
