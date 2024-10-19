#include "usb_controller/usb_controller.hpp"


void UsbController::clearInputBuffer() {
    while (Serial.available())
        Serial.read();
}

void UsbController::com_menu() {
	clearInputBuffer();

    int choice;
    while (true) {
        // Вывод меню
        Serial.println("Меню:");
        Serial.println("1) Настройки");
        Serial.println("2) Сброс");
        Serial.println("0) Выйти");
        Serial.print("Выберите вариант: ");
        
        while (!Serial.available()); // Ожидание ввода
        choice = Serial.parseInt();
        Serial.read(); // Очистка буфера

        switch (choice) {
            case 1:
                settings_menu();
                break;
            case 2:
				nvs_flash_erase();
				nvs_flash_init();
                ESP.restart(); // Перезапуск ESP32
                break;
            default:
				clearInputBuffer();
                return;
        }
    }
}

void UsbController::settings_menu() {
    int setting_choice;
    DECLARE_SETTING_TYPES_VARIANT(UNIQUE_SETTINGS_TYPES) buffer;

	SettingUpdate update;

    while (true) {
        // Вывод меню настроек
        Serial.println("Настройки:");
        Serial.println("1) ID");
        Serial.println("0) Назад");
        Serial.println("Выберите вариант: ");
        
        while (!Serial.available()); // Ожидание ввода
        setting_choice = Serial.parseInt();
        Serial.read(); // Очистка буфера

        
        switch (setting_choice) {
            case 1:
                Serial.print("Введите новый ID: ");
                while (!Serial.available());

                update.value = (uint32_t)Serial.parseInt();
                update.key = SETTING_TYPE::battery_id;

                xQueueSend(settingUpdateQueue, &update, portMAX_DELAY);
                vTaskDelay(10);
                break;
            default:
                clearInputBuffer();
				return;
        }
    }
}

void UsbController::usbTask(void *pvParameters) {
	Serial.begin(115200);

	while(true) {
		if(Serial.isConnected())
		{	
			if(Serial.available()) // Позже сделаем возможность прерывать поток
				com_menu();
			Serial.println(raw_data->getJSON());
        }

		vTaskDelay(settings.usb_delay);
	}
}