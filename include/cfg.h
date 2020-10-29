#ifndef __CFG_H
#define __CFG_H

#include <stdint.h>
#include <esp_err.h>
#include <esp_wifi_types.h>

// Неймспейсы и имена данных для NVS
#define NVS_WIFI_NS "wifi"
#define NVS_WIFI_AP_CFG     "ap"
#define NVS_WIFI_STA_CFG    "sta"
#define NVS_WIFI_MODE_CFG   "mode"

#define NVS_OILER_NS "oiler"

// Данные точки доступа\клиента
typedef struct {
	uint8_t ssid[16];      // SSID of target AP
	uint8_t password[16];  // password of target AP
} wifi_sta_config_int_t;

// Общий конфиг вайфая
typedef struct {
	wifi_sta_config_int_t	ap;  // configuration of AP
	wifi_mode_t 			mode;
	wifi_sta_config_int_t	sta; // configuration of STA
} wifi_config_int_t;

// Конфигурация смазчика
typedef struct {
	uint16_t pulseTime;    // Pulse time for pump, ms

	uint16_t baseDistance; // Distance between pulses, meters

	uint8_t hwSpeed;  // min speed to switch to 'highway' mode, km\h
	uint8_t hwKf;     // Koeff to change distance in highway mode, 1/100 (should be divided to 100)

	uint8_t dewValue; // Min humidity apply dewKf to pulse distance, %
	uint8_t dewKf;    // Koeff to change distance due to humidyty, 1/100 (should be divided to 100)

	uint8_t  gpsTimeout;      // nmea timeout to start oil by timer, seconds
	uint8_t  timerPulseDelay; // delay between pulses when GPS is lost, seconds
} oiler_config_t;

// синглтон конфигурации WiFi
extern wifi_config_int_t* config_wifi();
// синглтон конфигурации смазчика
extern oiler_config_t* config_oil();

// сброс конфигурации к значениям по умолчанию
extern void config_oil_reset();

// сохранение конфигурации
extern esp_err_t config_save();

#endif