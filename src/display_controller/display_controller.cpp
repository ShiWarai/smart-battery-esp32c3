#include "display_controller/display_controller.hpp"

void DisplayController::displayTask(void *pvParameters) {
	U8G2_SSD1306_64X32_1F_F_HW_I2C oled = U8G2_SSD1306_64X32_1F_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

	bool display_enabled = false;
    const int display_frequency = 200;

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

			for(int i = 0; i < (5000/display_frequency); i++)
			{
				DisplayController::printStatus(&oled, *raw_data);
				vTaskDelay(display_frequency);
			}
		} else {
			oled.clearDisplay();
			digitalWrite(OLED_PWR_PIN, LOW);
			display_enabled = false;
		}
	}
}

void DisplayController::printStatus(U8G2_SSD1306_64X32_1F_F_HW_I2C *oled, INA226Data data, bool changeContrast, byte contrast) // routine for printing simple interface on an OLED display
{
	// clear frame buffer and set display brightness if needed
	oled->clearBuffer();
	if (changeContrast) {
		oled->setContrast(contrast);
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
	oled->setFont(u8g2_font_spleen16x32_mu); // set big font
	if(s.length() == 1) {
		oled->drawStr(34, 20, s.c_str());
	} else if(s.length() == 2) {
		oled->drawStr(18, 20, s.c_str());
	} else {
		oled->drawStr(2, 20, s.c_str());
	}
	oled->drawStr(50, 20, "%");

	// prepare a string with formatted voltage value
	s = String(voltage,2);
	
	// display true battery voltage and fake power value on the bottom
	oled->setFont(u8g2_font_spleen6x12_mr); // set smaller font
	oled->drawStr(0, 31, s.c_str());
	oled->drawStr(24, 31, "v");
	oled->drawStr(33, 31, power.c_str());
	oled->drawStr(57, 31, "w");
	
	// send frame buffer to the display
	oled->sendBuffer();
}