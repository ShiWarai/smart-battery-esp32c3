#include "preferences_controller/preferences_controller.hpp"

void PreferencesController::preferencesTask(void *pvParameters) {
    
	Preferences preferences;
	SettingUpdate update;
	
	preferences.begin("pref", false);

	if(!preferences.isKey(SETTING_NAMES[SETTING_TYPE::battery_id])) { // Первичная инициализация настроек
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::battery_id], ID);
		preferences.putString(SETTING_NAMES[SETTING_TYPE::access_key], DEFAULT_ACCESS_KEY);
		preferences.putString(SETTING_NAMES[SETTING_TYPE::wifi_ssid], WIFI_SSID);
		preferences.putString(SETTING_NAMES[SETTING_TYPE::wifi_password], WIFI_PASSWORD);
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::sensor_delay], 1000);
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::usb_delay], 1000);
		preferences.putUInt(SETTING_NAMES[SETTING_TYPE::wireless_delay], 1000);
	}

	std::variant<float *, String *, uint32_t *, nullptr_t> setting;
	String* s_buffer;
	float* f_buffer;
	uint32_t* u_buffer;

	// Загрузка настроек
	if(preferences.isKey(SETTING_NAMES[SETTING_TYPE::battery_id])) {
		for(unsigned short i = 0; i <= SETTING_TYPE::battery_id; i++)
		{
			setting = getSettingFieldPointer(i);

			if(SETTING_TYPES[i] == "String")
			{
				s_buffer = std::get<String*>(setting);
				*s_buffer = preferences.getString(SETTING_NAMES[i]);
			}	
			else if(SETTING_TYPES[i] == "float") {
				f_buffer = std::get<float*>(setting);
				*f_buffer = preferences.getFloat(SETTING_NAMES[i]);
			} else if (SETTING_TYPES[i] == "uint32_t") {
				u_buffer = std::get<uint32_t*>(setting);
				*u_buffer = preferences.getUInt(SETTING_NAMES[i]);
			}
		}
	}

	preferences.end();

	while(true) {
		if (xQueueReceive(settingUpdateQueue, &update, portMAX_DELAY)) {
			preferences.begin("pref", false);

			setting = getSettingFieldPointer(update.key);

			if(SETTING_TYPES[update.key] == "String")
			{
				s_buffer = std::get<String*>(setting);

				preferences.putString(SETTING_NAMES[update.key], update.value);
				*s_buffer = update.value;
			}	
			else if(SETTING_TYPES[update.key] == "float") {
				f_buffer = std::get<float*>(setting);

				preferences.putFloat(SETTING_NAMES[update.key], update.value.toFloat());
				*f_buffer = update.value.toFloat();
			}
			else if (SETTING_TYPES[update.key] == "uint32_t") {
				u_buffer = std::get<uint32_t*>(setting);

				preferences.putUInt(SETTING_NAMES[update.key], update.value.toInt());
				*u_buffer = update.value.toInt();
			}

			preferences.end();
		}

		vTaskDelay(100);
	}
}