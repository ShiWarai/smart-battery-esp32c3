#include "sensor_controller/sensor_controller.hpp"

void SensorController::sensorTask(void *pvParameters) {
	INA226 INA = INA226(0x40);

	INA.begin();

    raw_data = new INA226Data(SETTINGS.battery_id);

	if (!INA.begin() && Serial.isConnected())
		Serial.println("it was not possible to connect to the voltampermeter. Fix the error");
	else
		INA.setMaxCurrentShunt(60, 0.00125, false);

	while(true) {
		raw_data->readData(&INA);

		vTaskDelay(SETTINGS.sensor_read_delay);
	}
}