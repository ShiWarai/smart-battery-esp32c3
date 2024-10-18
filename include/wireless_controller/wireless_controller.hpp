#pragma once

#include <WiFi.h>
#include <ESPmDNS.h>
#include "preferences_controller/settings.hpp"
#include "sensor_controller/INA226Data.hpp"

class WirelessController
{
public:
    static void wirelessTask(void *pvParameters);
};