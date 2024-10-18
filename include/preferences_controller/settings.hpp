#pragma once

#include <WString.h>
#include <freertos/queue.h>
#include <variant>
#include <typeindex>
#include <typeinfo>


// Макрос для определения полей с указанием типа
#define DECLARE_SETTINGS_FIELD(TYPE, NAME) TYPE NAME;
#define REMOVE_TYPE(TYPE, NAME) NAME,
#define REMOVE_TYPE_STR(TYPE, NAME) #NAME,
#define REMOVE_TYPE_INDEX(TYPE, NAME) #TYPE,

#define DECLARE_SWITCH_CASE(TYPE, NAME) \
    case SETTING_TYPE::NAME: return &settings.NAME;

// Определяем структуру и генерируем enum
#define DECLARE_SETTINGS(NAME, FIELDS) \
    inline struct NAME { \
        FIELDS(DECLARE_SETTINGS_FIELD) \
    } settings; \
    \
    enum SETTING_TYPE { \
        FIELDS(REMOVE_TYPE) \
    }; \
    \
    constexpr const char* SETTING_NAMES[] = { FIELDS(REMOVE_TYPE_STR) }; \
    constexpr  const char* SETTING_TYPES[] = { FIELDS(REMOVE_TYPE_INDEX) }; \
    \
    inline std::variant< \
        float*, String*, uint32_t*, nullptr_t \
    > getSettingFieldPointer(unsigned short type) { \
    \
        switch (type) { \
            FIELDS(DECLARE_SWITCH_CASE) \
            default: return nullptr; \
        } \
    }

// Определяем поля структуры с указанием типа и названия настройки
#define SETTINGS_FIELDS(FIELD) \
    FIELD(String, access_key) \
    FIELD(uint32_t, sensor_delay) \
    FIELD(uint32_t, usb_delay) \
    FIELD(uint32_t, wireless_delay) \
    FIELD(String, wifi_ssid) \
    FIELD(String, wifi_password) \
    FIELD(uint32_t, battery_id) \

// Генерируем структуру и enum
DECLARE_SETTINGS(SETTINGS, SETTINGS_FIELDS)

typedef struct {
    SETTING_TYPE key;
    String value;
} SettingUpdate;

inline QueueHandle_t settingUpdateQueue = xQueueCreate(10, sizeof(SettingUpdate));