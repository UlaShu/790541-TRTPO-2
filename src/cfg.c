#include <nvs.h>
#include <esp_system.h>
#include <string.h>

#include "cfg.h"

nvs_handle config_wifi_open() {
	static nvs_handle wifiStorage = -1;
	esp_err_t err;

	if (wifiStorage == -1) {
		if ( (err=nvs_open(NVS_WIFI_NS, NVS_READWRITE, &wifiStorage))!=ESP_OK ) {
			return -1;
		}
	}

	return wifiStorage;
}

int8_t config_wifi_read(wifi_config_int_t *wCfg) {
	nvs_handle wifiStorage = config_wifi_open();
	if (wifiStorage==-1) return -1;

	esp_err_t err;
	size_t len;

	if (wCfg == NULL) return -1;

	err = nvs_get_blob(wifiStorage, NVS_WIFI_AP_CFG, &(wCfg->ap), &len);
	
	if (err != ESP_OK || strnlen((char*)wCfg->ap.ssid,16)<2) {
		strcpy((char*)wCfg->ap.ssid, "ilv-oiler-2");
		strcpy((char*)wCfg->ap.password, "00000000");
		wCfg->mode = WIFI_MODE_AP;
	}

	int8_t mode;
	err = nvs_get_i8(wifiStorage, NVS_WIFI_MODE_CFG, &mode );
	if (err == ESP_ERR_NVS_NOT_FOUND){
		wCfg->mode = WIFI_MODE_AP;
	} else {
		wCfg->mode = mode;
	}

	err = nvs_get_blob(wifiStorage, NVS_WIFI_STA_CFG, &(wCfg->sta), &len);
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		memset((void*)&(wCfg->sta), 0, sizeof(wCfg->sta));
	}

	return ESP_OK;
}

wifi_config_int_t* config_wifi() {
	nvs_handle wifiStorage = config_wifi_open();
	if (wifiStorage==-1) return NULL;

	static wifi_config_int_t *wCfg = NULL;

	if (wCfg == NULL) {
		wCfg = malloc(sizeof(wifi_config_int_t));
		config_wifi_read(wCfg);
	}

	return wCfg;
}

nvs_handle config_oil_open() {
	static nvs_handle oilStorage = -1;
	esp_err_t err;

	if (oilStorage == -1) {
		if ( (err=nvs_open(NVS_OILER_NS, NVS_READWRITE, &oilStorage))!=ESP_OK ) {
			return -1;
		}
	}

	return oilStorage;
}

int8_t config_oil_read(oiler_config_t *oCfg) {
	nvs_handle storage = config_oil_open();
	if (storage==-1) return -1;

	esp_err_t err;
	size_t len;

	if (oCfg == NULL) return -1;

	err = nvs_get_blob(storage, NVS_OILER_NS, oCfg, &len);
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		
		oCfg->pulseTime = 100;
		oCfg->baseDistance = 1000;

		oCfg->hwSpeed = 100;
		oCfg->hwKf = 90;

		oCfg->dewValue = 95; // approx. 100% humidity
		oCfg->dewKf = 90;

		oCfg->gpsTimeout = 180; // 3 minutes without GPS
		oCfg->timerPulseDelay = 120; // 2 minutes between pulses, in case of GPS fail
	}

	return ESP_OK;
}

oiler_config_t* config_oil() {
	nvs_handle storage = config_oil_open();
	if (storage==-1) return NULL;

	static oiler_config_t *oCfg = NULL;

	if (oCfg == NULL) {
		oCfg = malloc(sizeof(oiler_config_t));
		config_oil_read(oCfg);
	}

	return oCfg;
}


esp_err_t config_save() {
	esp_err_t err = ESP_OK;
	
	nvs_handle wifiStorage = config_wifi_open();
	if (wifiStorage==-1) return ESP_ERR_NVS_NOT_INITIALIZED;

	wifi_config_int_t *wCfg = config_wifi();
	if(wCfg == NULL) return ESP_ERR_INVALID_ARG;

	wifi_config_int_t currentConfig;
	config_wifi_read(&currentConfig);

	if ( memcmp((void*)&(wCfg->ap), (void*)&(currentConfig.ap), sizeof(currentConfig.ap)) != 0 ) {
		err = nvs_set_blob(wifiStorage, NVS_WIFI_AP_CFG, (void*)&(wCfg->ap), sizeof(wifi_sta_config_int_t));
	}

	if (wCfg->mode != currentConfig.mode) {
		err = nvs_set_i8(wifiStorage, NVS_WIFI_MODE_CFG, wCfg->mode);
	}

	if ( memcmp((void*)&(wCfg->sta), (void*)&(currentConfig.sta), sizeof(currentConfig.sta)) != 0 ) {
		err = nvs_set_blob(wifiStorage, NVS_WIFI_STA_CFG, (void*)&(wCfg->sta), sizeof(wifi_sta_config_int_t));
	}

	oiler_config_t oilCurrentConfig;
	oiler_config_t *oilNewConfig = config_oil();

	nvs_handle oilStorage = config_oil_open();
	if (oilStorage==-1) {
		return ESP_ERR_NVS_NOT_INITIALIZED;
	}
	config_oil_read(&oilCurrentConfig);

	if ( memcmp((void*)oilNewConfig, (void*)&oilCurrentConfig, sizeof(oiler_config_t)) != 0 ) {
		err = nvs_set_blob(oilStorage, NVS_OILER_NS, (void*)oilNewConfig, sizeof(oiler_config_t));
	}


	return err;
}

void config_oil_reset() {
	nvs_handle oilStorage = config_oil_open();
	if (oilStorage==-1) {
		return;
	}

	nvs_erase_all(oilStorage);
	nvs_commit(oilStorage);
	esp_restart();
}