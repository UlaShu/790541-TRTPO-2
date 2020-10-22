#include "defines.h"

exchange_t* initExchange() { // Синглтон на структуру очередей
	static exchange_t* exchangeData = NULL;
	
	if(exchangeData == NULL) {
		exchangeData = malloc(sizeof(exchange_t));
		exchangeData->mainEventGroup = xEventGroupCreate();
		// exchangeData->gpsQueue = xQueueCreate(1, ToDo ); // Последняя валидная точка от GPS
		// exchangeData->odoQueue = xQueueCreate(1, ToDo ); // Последнее значение от одометра
		exchangeData->oilTimerQueue = xQueueCreate(5, sizeof(uint16_t)); // до 5 (навскидку) команд в очереди насосу
		exchangeData->humidityTimerQueue = xQueueCreate(1, sizeof(uint8_t)); // только последнее значение от датчика влажности 
		exchangeData->uartTS = 0;
		exchangeData->nmeaTS = 0;
	}

	return exchangeData;
}

// Обновление таймаутов. Сдвиг на 20 примерно равен делению на миллион, но быстрее
// esp_timer_get_time отдает значения в микросекундах

void ts_update(int64_t *ts) { 
	*ts = esp_timer_get_time() >> 20;
}
int64_t ts_get(int64_t ts) {
	return (esp_timer_get_time() >> 20) - ts;
}
int64_t ts_current() {
	return (esp_timer_get_time() >> 20);
}
