#ifndef __DEFINES_H
#define __DEFINES_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_event_legacy.h"

#include "driver/gpio.h"
#define UART_TX GPIO_NUM_17
#define UART_RX GPIO_NUM_16

#define _sysName "oiler v2 rtos esp32"

typedef struct { // Очереди сообщений и событие
	EventGroupHandle_t mainEventGroup; // Основные события системы
	QueueHandle_t gpsQueue; // Очередь точек GPS
	QueueHandle_t uartQueue; // Очередь UART
	QueueHandle_t odoQueue; // Очередь сообщений одометра
	QueueHandle_t oilTimerQueue; // Очередь команд насосу
	QueueHandle_t humidityTimerQueue; // Очередь значений от датчика влажности
	int64_t uartTS; // Время последнего сообщения от UART, для контроля состояния связи с приемником
	int64_t nmeaTS; // Время последней валидной точки GPS
} exchange_t;

// Флаги для обмена событиями - подключение\отключение клиента
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_DISCONNECT_BIT BIT1

extern exchange_t* initExchange();
extern void ts_update(int64_t *ts);
extern int64_t ts_get(int64_t ts);
extern int64_t ts_current();

#endif