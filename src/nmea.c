#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "driver/uart.h"

#include "defines.h"
#include "nmea.h"

#define TAG_NMEA "nmea"

void nmea_read_task(void *pvData) {
	exchange_t *exData = initExchange(); // Структура для обмена данными
    uart_event_t event; // Событие от UART

	static uint8_t* buf = NULL; // Буфер для входящей строки 

	while (1) {
		// ожидание 200мс для получения события от UART
		if(xQueueReceive(exData->uartQueue, (void * )&event, (TickType_t)(200 / portTICK_PERIOD_MS)) == pdTRUE) {
            switch(event.type) {
                case UART_DATA: // Пришли данные
					buf = (uint8_t*) realloc(buf, event.size); 

					ts_update(&(exData->uartTS)); // обновили таймстамп данных от GPS приемника
                    uart_read_bytes(UART_NUM, buf, event.size, portMAX_DELAY); // прочитали данные
					ESP_LOGV(TAG_NMEA, "%d: %s", event.size, buf); // Вывели строку в лог

					for(uint16_t i=0; i<event.size; i++) {
						// ToDo: отдать символ buf[i] gps-парсеру, проверить валидность возвращенной точки
						// и если ок - запихать точку в exData->gpsQueue
					}

					break;
				case UART_FIFO_OVF: // События переполнения буфера
				case UART_BUFFER_FULL:
					ESP_LOGE(TAG_NMEA, "UART_BUFFER_FULL | UART_FIFO_OVF");
                    uart_flush_input(UART_NUM); // сбросили буфер
                    xQueueReset(exData->uartQueue); // Сбросили очередь
					break;
				default: // все остальные события просто логировать
					ESP_LOGD(TAG_NMEA, "UART event %d: ", event.type);
					break;

			}
		}
	}
	
	free(buf);
	vTaskDelete(NULL);
}
