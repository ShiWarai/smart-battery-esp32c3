#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <nvs_flash.h>

#ifndef WOKWI
#include <INA226.h>
#else
#include "INA226_wokwi.hpp"
#endif

#ifdef DISPLAY
#include <U8g2lib.h>
#endif

#define FLOAT_MAP(value, in_min, in_max, out_min, out_max) ((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

// Vars
INA226 INA(0x40);
#ifdef WITH_DISPLAY
	U8G2_SSD1306_64X32_1F_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA); // define 64x32 OLED display (with SSD1306 internal controller IC) in I2C mode
#endif

QueueHandle_t settingsQueue;
const int TRANSMISSION_FREQUENCY = 200;
unsigned short BATTERY_ID; // Изначально пустая переменная с нулём
String ACCESS_KEY;

class INA226_DATA {
public:
	INA226_DATA(unsigned short id) {
		this->id = id;
	};
	
	void readData(INA226 *sensor) {
		this->voltage = INA.getBusVoltage();
		this->current = INA.getCurrent();
		this->power = INA.getPower();
		this->capacity = FLOAT_MAP(INA.getBusVoltage(),3.3,4.2,0.0,100.0);
	};

	// String getJSON() {
	// 	return "{\"ID\":" + String(this->id) + ", \"V\":" + this->voltage + ", \"A\":" + this->current + ", \"P\":" + this->power + ", \"C\":" + String(this->capacity) + "}";
	// };

	String getJSON() {
		// Заполняем JSON-документ
		json["ID"] = this->id;
		json["V"] = round2(this->voltage);
		json["A"] = round2(this->current);
		json["P"] = round2(this->power);
		json["C"] = round2(this->capacity);

		// Конвертируем JSON-документ в строку
		serializeJson(json, buffer);
		return buffer;
	};

	unsigned short id;
	float voltage;
	float current;
	float power;
	float capacity;

private:
	JsonDocument json;
	String buffer;

	float round2(float value) {
   		return (int)(value * 100 + 0.5) / 100.0;
	}
};

INA226_DATA *raw_data = nullptr;


// Tasks
void sensorTask(void *pvParameters);
void usbTask(void *pvParameters);
void wirelessTask(void *pvParameters);
#ifdef WITH_DISPLAY
void displayTask(void *pvParameters);
#endif
void preferencesTask(void *pvParameters);


// Functions
void setup();
void loop() {vTaskDelete(NULL);}
#ifdef WITH_DISPLAY
void printStatus(INA226_DATA data, bool changeContrast, byte contrast);
#endif