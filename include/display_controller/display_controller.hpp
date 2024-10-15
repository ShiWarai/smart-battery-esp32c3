#pragma once

#include <U8g2lib.h>
#include "sensor_controller/INA226Data.hpp"


class DisplayController
{
    public:
        static void displayTask(void *pvParameters);
    private:
        static void printStatus(U8G2_SSD1306_64X32_1F_F_HW_I2C *oled, INA226Data data, bool changeContrast = false, byte contrast = 255);
};