#include "main.hpp"

void setup()
{
	xTaskCreate(PreferencesController::preferencesTask, "Preferences task", 4096, NULL, 1, NULL);

	while(SETTINGS.battery_id == 0) vTaskDelay(100); // Ожидаем загрузки настроек в ОЗУ

	xTaskCreate(SensorController::sensorTask, "Sensor task", 2048, NULL, 1, NULL);
	vTaskDelay(100);
	xTaskCreate(UsbController::usbTask, "USB task", 2048, NULL, 1, NULL);
	xTaskCreate(WirelessController::wirelessTask, "Wireless task", 8196, NULL, 1, NULL);
	#ifdef WITH_DISPLAY
	xTaskCreate(DisplayController::displayTask, "Display task", 4096, NULL, 1, NULL);
	#endif
}