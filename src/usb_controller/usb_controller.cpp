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
    unsigned short id;
    char hostname[50];
    unsigned short frequency;
    char wifi[50];
	SettingUpdate update;

    while (true) {
        // Вывод меню настроек
        Serial.println("Настройки:");
        Serial.println("1) ID");
        Serial.println("2) hostname");
        Serial.println("3) transmission frequency");
        Serial.println("4) WiFi");
        Serial.println("0) Назад");
        Serial.println("Выберите вариант: ");
        
        while (!Serial.available()); // Ожидание ввода
        setting_choice = Serial.parseInt();
        Serial.read(); // Очистка буфера

        
        switch (setting_choice) {
            case 1:
                Serial.print("Введите новый ID: ");
                while (!Serial.available());
                id = Serial.parseInt();

                update.key = "id";
                update.value = id;
                xQueueSend(settingsQueue, &update, portMAX_DELAY);
                vTaskDelay(10);
                break;
            case 2:
                Serial.print("Введите новый hostname: ");
                while (!Serial.available());
                Serial.readBytesUntil('\n', hostname, sizeof(hostname));

                update.key = "hostname";
                update.value = hostname;
                xQueueSend(settingsQueue, &update, portMAX_DELAY);
                break;
            case 3:
                Serial.print("Введите новую частоту передачи: ");
                while (!Serial.available());
                frequency = Serial.parseInt();

                update.key = "frequency";
                update.value = frequency;
                xQueueSend(settingsQueue, &update, portMAX_DELAY);
                break;
            case 4:
                Serial.print("Введите новый WiFi: ");
                while (!Serial.available());
                Serial.readBytesUntil('\n', wifi, sizeof(wifi));

                update.key = "wifi";
                update.value = wifi;
                xQueueSend(settingsQueue, &update, portMAX_DELAY);
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

		vTaskDelay(SETTINGS.usb_transmition_delay);
	}
}