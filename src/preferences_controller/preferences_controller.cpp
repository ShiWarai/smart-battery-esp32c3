#include "preferences_controller/preferences_controller.hpp"

void PreferencesController::preferencesTask(void *pvParameters) {
    
	Preferences preferences;
	SettingUpdate update;
	
	preferences.begin("pref", false);

	if(!preferences.isKey("id")) { // Первичная инициализация настроек
		preferences.putUShort("id", ID);
		preferences.putString("access_key", DEFAULT_ACCESS_KEY);
		preferences.putString("wifi_ssid", WIFI_SSID);
		preferences.putString("wifi_password", WIFI_PASSWORD);
		preferences.putUInt("sensor_delay", 1000);
		preferences.putUInt("usb_delay", 1000);
		preferences.putUInt("wireless_delay", 1000);
	}

	// Загрузка настроек
	if(preferences.isKey("id")) {
		SETTINGS.access_key = preferences.getString("access_key");
		SETTINGS.wifi_ssid = preferences.getString("wifi_ssid");
		SETTINGS.wifi_password = preferences.getString("wifi_password");
		SETTINGS.sensor_read_delay = preferences.getUInt("sensor_delay");
		SETTINGS.usb_transmition_delay = preferences.getUInt("usb_delay");
		SETTINGS.wireless_transmition_delay = preferences.getUInt("wireless_delay");

		SETTINGS.battery_id = preferences.getUShort("id");
	}

	preferences.end();

	while(true) {
		if (xQueueReceive(settingsQueue, &update, portMAX_DELAY)) {
			preferences.begin("pref", false);
			if (update.key == "id") {
				preferences.putUShort(update.key.c_str(), update.value.toInt());
				SETTINGS.battery_id = update.value.toInt();
			} else {
				preferences.putString(update.key.c_str(), update.value);
			}
			preferences.end();
		}

		vTaskDelay(100);
	}
}