#include "preferences_controller/preferences_controller.hpp"

void PreferencesController::preferencesTask(void *pvParameters) {
    
	Preferences preferences;
	SettingUpdate update;
	
	preferences.begin(SETTINGS_SPACE_NAME, false);

	if(!preferences.isKey(SETTING_NAMES[SETTING_TYPE::battery_id])) { // Первичная инициализация настроек
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::battery_id], ID);
		preferences.putString(SETTING_NAMES[SETTING_TYPE::access_key], DEFAULT_ACCESS_KEY);
		preferences.putString(SETTING_NAMES[SETTING_TYPE::wifi_ssid], WIFI_SSID);
		preferences.putString(SETTING_NAMES[SETTING_TYPE::wifi_password], WIFI_PASSWORD);
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::sensor_delay], 1000);
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::usb_delay], 1000);
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::wireless_delay], 1000);
	}

	DECLARE_SETTING_TYPES_LINKS_VARIANT(UNIQUE_SETTINGS_TYPES) setting;
	void* buffer;

	// Загрузка настроек
	if(preferences.isKey(SETTING_NAMES[SETTING_TYPE::battery_id])) {
		GEN_READ_SETTINGS_CYCLE(preferences, buffer, setting, UNIQUE_SETTINGS_TYPES)
	}

	preferences.end();

	while(true) {
		if (xQueueReceive(settingUpdateQueue, &update, portMAX_DELAY)) {
			preferences.begin(SETTINGS_SPACE_NAME, false);
			GEN_UPDATE_ITER(preferences, buffer, setting, update, UNIQUE_SETTINGS_TYPES)
			preferences.end();
		}

		vTaskDelay(100);
	}
}