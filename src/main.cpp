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


void setup()
{
	xTaskCreate(sensor_task, "Sensor task", 1024, NULL, 1, NULL);
	xTaskCreate(usb_task, "USB task", 2048, NULL, 1, NULL);
	xTaskCreate(wireless_task, "Wireless task", 4096, NULL, 1, NULL);
	#ifdef WITH_DISPLAY
	xTaskCreate(display_task, "Display task", 4096, NULL, 1, NULL);
	#endif
}

void sensor_task(void *pvParameters) {

	if (!INA.begin() && Serial.isConnected())
		Serial.println("it was not possible to connect to the voltampermeter. Fix the error");
	else
		INA.setMaxCurrentShunt(60, 0.00125, false);

	while(true) {
		raw_data.readData(&INA);

		vTaskDelay(TRANSMISSION_FREQUENCY);
	}
}

void usb_task(void *pvParameters) {
	Serial.begin(115200);

	while(true) {
		if(Serial.isConnected())
		{
			Serial.println(raw_data.getJSON());
		}

		vTaskDelay(TRANSMISSION_FREQUENCY);
	}
}

void wireless_task(void *pvParameters) {	
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
			client.print(raw_data.getJSON());
		}

		vTaskDelay(TRANSMISSION_FREQUENCY);
	}
}

#ifdef WITH_DISPLAY
void display_task(void *pvParameters) {
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
				printStatus(raw_data);
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