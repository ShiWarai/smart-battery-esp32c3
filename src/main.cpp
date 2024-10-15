#include "main.hpp"


#ifdef WITH_DISPLAY
void printStatus(INA226_DATA data, bool changeContrast = false, byte contrast = 255) // routine for printing simple interface on an OLED display
{
	// clear frame buffer and set display brightness if needed
	oled.clearBuffer();
	if (changeContrast) {
		oled.setContrast(contrast);
	}

	// obtain voltage from power monitor and prepare values
	double voltage = data.voltage;
	double p = data.capacity;
	String power = String(data.power,2);
	power.trim();
	
	// create a string with formatted percentage value
	String s;
	s = String(p,0);
	s.trim();

	// display voltage based battery charge percentage, accounting for the number of digits
	oled.setFont(u8g2_font_spleen16x32_mu); // set big font
	if(s.length() == 1) {
		oled.drawStr(34, 20, s.c_str());
	} else if(s.length() == 2) {
		oled.drawStr(18, 20, s.c_str());
	} else {
		oled.drawStr(2, 20, s.c_str());
	}
	oled.drawStr(50, 20, "%");

	// prepare a string with formatted voltage value
	s = String(voltage,2);
	
	// display true battery voltage and fake power value on the bottom
	oled.setFont(u8g2_font_spleen6x12_mr); // set smaller font
	oled.drawStr(0, 31, s.c_str());
	oled.drawStr(24, 31, "v");
	oled.drawStr(33, 31, power.c_str());
	oled.drawStr(57, 31, "w");
	
	// send frame buffer to the display
	oled.sendBuffer();
}
#endif

void clearInputBuffer() {
    while (Serial.available())
        Serial.read();
}

void settings_menu();

typedef struct {
    String key;
    String value;
} SettingUpdate;

void com_menu() {
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

void settings_menu() {
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

void setup()
{
	settingsQueue = xQueueCreate(10, sizeof(SettingUpdate));
	xTaskCreate(preferencesTask, "Preferences task", 4096, NULL, 1, NULL);

	while(BATTERY_ID == 0) vTaskDelay(100); // Ожидаем загрузки настроек в ОЗУ

	xTaskCreate(sensorTask, "Sensor task", 2048, NULL, 1, NULL);
	vTaskDelay(100);
	xTaskCreate(usbTask, "USB task", 2048, NULL, 1, NULL);
	xTaskCreate(wirelessTask, "Wireless task", 8196, NULL, 1, NULL);
	#ifdef WITH_DISPLAY
	xTaskCreate(displayTask, "Display task", 4096, NULL, 1, NULL);
	#endif
}

void sensorTask(void *pvParameters) {
	raw_data = new INA226_DATA(BATTERY_ID);

	if (!INA.begin() && Serial.isConnected())
		Serial.println("it was not possible to connect to the voltampermeter. Fix the error");
	else
		INA.setMaxCurrentShunt(60, 0.00125, false);

	while(true) {
		raw_data->readData(&INA);

		vTaskDelay(TRANSMISSION_FREQUENCY);
	}
}

void usbTask(void *pvParameters) {
	Serial.begin(115200);

	while(true) {
		if(Serial.isConnected())
		{	
			if(Serial.available()) // Позже сделаем возможность прерывать поток
				com_menu();
			Serial.println(raw_data->getJSON());
		}

		vTaskDelay(TRANSMISSION_FREQUENCY);
	}
}

void wirelessTask(void *pvParameters) {	
	IPAddress HOSTIP;
	WiFiClient client;

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	
	while (WiFi.status() != WL_CONNECTED) vTaskDelay(500);
	// Serial.print("WiFi connected with IP: ");
	// Serial.println(WiFi.localIP());

	while(mdns_init()!= ESP_OK) vTaskDelay(500); // ожидание запуска mDNS

	while(true) {
		// поиск IP-адреса hostname
		#ifdef HOSTNAME
		if (HOSTIP.toString() == "0.0.0.0") {
			HOSTIP = MDNS.queryHost(HOSTNAME);
		} else
		#endif
		if(!client.connected()) // если потеряли связь с клиентом, то устонавливаем её заново
		{
			#ifdef HOSTNAME
				client.connect(HOSTIP, PORT);
			#else
				client.connect("host.wokwi.internal", PORT);
			#endif
		} else {
			client.print(raw_data->getJSON());
		}

		vTaskDelay(TRANSMISSION_FREQUENCY);
	}
}

#ifdef WITH_DISPLAY
void displayTask(void *pvParameters) {
	bool display_enabled = false;

	pinMode(OLED_PWR_PIN, OUTPUT);
	pinMode(BUTTONS_PIN, INPUT_PULLDOWN);
	pinMode(BUZZER_PIN, OUTPUT);

	digitalWrite(OLED_PWR_PIN, HIGH);
	vTaskDelay(100);
	oled.begin();
	vTaskDelay(100);
	digitalWrite(OLED_PWR_PIN, LOW);

	while(true) {
		if(!digitalRead(BUTTONS_PIN))
		{
			if(!display_enabled) {
				digitalWrite(OLED_PWR_PIN, HIGH);
				display_enabled = true;
				vTaskDelay(100);
			}

			for(int i = 0; i < (5000/TRANSMISSION_FREQUENCY); i++)
			{
				printStatus(*raw_data);
				vTaskDelay(TRANSMISSION_FREQUENCY);
			}
		} else {
			oled.clearDisplay();
			digitalWrite(OLED_PWR_PIN, LOW);
			display_enabled = false;
		}
	}
}
#endif

void preferencesTask(void *pvParameters) {
	Preferences preferences;
	SettingUpdate update;

	preferences.begin("pref", false);

	if(!preferences.isKey("id")) { // Первичная инициализация настроек
		preferences.putUShort("id", ID);
		preferences.putString("access_key", DEFAULT_ACCESS_KEY);
	}

	// Загрузка настроек
	if(preferences.isKey("id")) {
		BATTERY_ID = preferences.getUShort("id");
		ACCESS_KEY = preferences.getString("access_key");
	}

	preferences.end();

	while(true) {
		if (xQueueReceive(settingsQueue, &update, portMAX_DELAY)) {
			preferences.begin("pref", false);
			if (update.key == "id" || update.key == "frequency") {
				preferences.putUShort(update.key.c_str(), update.value.toInt());
			} else {
				preferences.putString(update.key.c_str(), update.value);
			}
			preferences.end();
		}

		vTaskDelay(100);
	}
}