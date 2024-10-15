#pragma once

#include <WString.h>
#include <freertos/queue.h>


inline struct Settings
{
    unsigned short battery_id;
    String access_key;
    uint32_t sensor_read_delay;
    uint32_t usb_transmition_delay;
    uint32_t wireless_transmition_delay;
    String wifi_ssid;
    String wifi_password;
} SETTINGS;

typedef struct {
    String key;
    String value;
} SettingUpdate;

inline QueueHandle_t settingsQueue = xQueueCreate(10, sizeof(SettingUpdate));