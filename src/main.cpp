#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#ifndef WOKWI
#include <INA226.h>
#else
#include <INA226_wokwi.hpp>
#endif

#ifndef DEBUG
#include <U8g2lib.h>
#endif

const int TRANSMISSION_FREQUENCY = 200;

const int buzzer = D10;
const int btn = D2;
const int oled_pwr = D9;
const int i2c_scl = D5;
const int i2c_sda = D4;


#define SERIALIZE_DATA(ID, V, A, P, C) "{\"ID\":" + ID + ", \"V\":" + V + ", \"A\":" + A + ", \"P\":" + P + ", \"C\":" + C + "}"
#define FLOAT_MAP(value, in_min, in_max, out_min, out_max) ((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

IPAddress HOSTIP;

INA226 INA(0x40);
#ifndef DEBUG
	U8G2_SSD1306_64X32_1F_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, i2c_scl, i2c_sda); // define 64x32 OLED display (with SSD1306 internal controller IC) in I2C mode
#endif

WiFiClient client;
String data;

#ifndef DEBUG
	void print_status(bool changeContrast = false, byte contrast = 255) // routine for printing simple interface on an OLED display
	{
		// clear frame buffer and set display brightness if needed
		oled.clearBuffer();
		if (changeContrast) {
			oled.setContrast(contrast);
		}

		// obtain voltage from power monitor and prepare values
		double voltage = INA.getBusVoltage();
		double p = min(100.0, max(0.0, FLOAT_MAP(voltage,3.2,4.2,0,100)));
		String power = String(INA.getPower(),2);
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

	// helper function for creating a simple fade-in animation on the display (WARNING: need to rewrite this, since using blocking delays!!!)
	void oled_fade_in() {
		oled.setContrast(0);
		oled.setPowerSave(0);
		for (int i = 0; i < 256; i += 10) {
			oled.setContrast(i);
			delay(30);
		}
		oled.setContrast(255);
	}



	// helper function for creating a simple fade-out animation on the display (WARNING: need to rewrite this, since using blocking delays!!!)
	void oled_fade_out() {
		for (int i = 255; i >= 0; i -= 10) {
			oled.setContrast(i);
			delay(30);
		}
		oled.setContrast(0);
		oled.setPowerSave(1);
	}
#endif


void setup()
{
	Serial.begin(115200);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	// Display
	#ifndef DEBUG
		digitalWrite(oled_pwr, HIGH);
		delay(100);
		oled.begin();
	#endif

	// INA226
	if (!INA.begin())
		Serial.println("it was not possible to connect to the voltampermeter. Fix the error");
	INA.setMaxCurrentShunt(60, 0.00125, false);
	
	// подключение к WIFI
	while (WiFi.status() != WL_CONNECTED) {delay(500);}
	Serial.print("WiFi connected with IP: ");
	Serial.println(WiFi.localIP());

	// ожидание запуска mDNS
	while(mdns_init()!= ESP_OK)
		delay(1000);
}

void loop()
{
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
	}

	data = SERIALIZE_DATA(String(ID), INA.getBusVoltage(), INA.getCurrent(), INA.getPower(), FLOAT_MAP(INA.getBusVoltage(),3.3,4.2,0.0,100.0));

	//   \/ сообщение по COM порту \/
	Serial.println(data);

	//     \/ сообщение по WIFI \/
	if(client.connected()) {
		client.print(data);
	}
	
	#ifndef DEBUG
		print_status();
	#endif

	delay(TRANSMISSION_FREQUENCY);
}