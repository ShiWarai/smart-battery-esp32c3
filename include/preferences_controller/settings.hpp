#pragma once

#include <WString.h>
#include <freertos/queue.h>
#include <variant>
#include <functional>
#include <unordered_map>
#include <Preferences.h>

// Объявляем все доступные типы переменных
#define DECLARE_TYPE_LINK(TYPE, F1, F2, ...) TYPE*,
#define DECLARE_TYPE(TYPE, F1, F2, ...) TYPE,

#define DECLARE_SETTING_TYPES_LINKS_VARIANT(TYPES) std::variant<TYPES(DECLARE_TYPE_LINK) nullptr_t>
#define DECLARE_SETTING_TYPES_VARIANT(TYPES) std::variant<TYPES(DECLARE_TYPE) nullptr_t>

#define DECLARE_GET_ITER(TYPE, F1, F2, PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER) \
if (SETTING_TYPES[i] == #TYPE) { \
    BUFFER_POINTER = std::get<TYPE*>(SETTING_POINTER); \
    *(TYPE*)BUFFER_POINTER = PREFERENCES_OBJ.F1(SETTING_NAMES[i]); \
    continue; \
} 

// #define DECLARE_PUT_ITER(TYPE, F1, F2, PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER) \
// if(SETTING_TYPES[UPDATE_QUEUE.key] == #TYPE) \
// { \
//     buffer = std::get<TYPE*>(SETTING_POINTER); \
//     PREFERENCES_OBJ.F2(SETTING_NAMES[UPDATE_QUEUE.key], UPDATE_QUEUE.value); \
//     *(TYPE*)buffer = UPDATE_QUEUE.value; \
//     continue; \
// } \

#define DECLARE_UPDATE_ITER(TYPE, F1, F2, PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER, UPDATE_QUEUE) \
if(SETTING_TYPES[UPDATE_QUEUE.key] == #TYPE) \
{ \
    BUFFER_POINTER = std::get<TYPE*>(SETTING_POINTER); \
    PREFERENCES_OBJ.F2(SETTING_NAMES[UPDATE_QUEUE.key], std::get<TYPE>(UPDATE_QUEUE.value)); \
    *(TYPE*)BUFFER_POINTER = std::get<TYPE>(UPDATE_QUEUE.value); \
    continue; \
}


#define GEN_READ_SETTINGS_CYCLE(PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER, TYPES) \
for (unsigned short i = 0; i < SETTING_TYPE::SETTINGS_COUNT; i++) { \
    SETTING_POINTER = getSettingFieldPointer(i); \
    TYPES(DECLARE_GET_ITER, PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER) \
}

#define GEN_UPDATE_ITER(PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER, UPDATE_QUEUE, TYPES) \
SETTING_POINTER = getSettingFieldPointer(UPDATE_QUEUE.key); \
TYPES(DECLARE_UPDATE_ITER, PREFERENCES_OBJ, BUFFER_POINTER, SETTING_POINTER, UPDATE_QUEUE) \

#define UNIQUE_SETTINGS_TYPES(TYPE_F1_F2, ...) \
    TYPE_F1_F2(String, getString, putString, __VA_ARGS__ ) \
    TYPE_F1_F2(float, getFloat, putFloat, __VA_ARGS__ ) \
    TYPE_F1_F2(uint32_t, getUInt, putUInt, __VA_ARGS__ )




// Макрос для определения полей с указанием типа
#define DECLARE_SETTINGS_FIELD(TYPE, NAME) TYPE NAME;
#define REMOVE_TYPE(TYPE, NAME) NAME,
#define REMOVE_TYPE_STR(TYPE, NAME) #NAME,
#define REMOVE_TYPE_INDEX(TYPE, NAME) #TYPE,

#define DECLARE_SWITCH_CASE(TYPE, NAME) \
    case SETTING_TYPE::NAME: return &settings.NAME;

// Определяем структуру и генерируем enum
#define GEN_SETTINGS(NAME, FIELDS) \
    inline struct NAME { \
        FIELDS(DECLARE_SETTINGS_FIELD) \
    } settings; \
    \
    enum SETTING_TYPE { \
        FIELDS(REMOVE_TYPE) \
        SETTINGS_COUNT \
    }; \
    \
    constexpr const char* SETTING_NAMES[] = { FIELDS(REMOVE_TYPE_STR) }; \
    constexpr const char* SETTING_TYPES[] = { FIELDS(REMOVE_TYPE_INDEX) }; \
    \
    inline DECLARE_SETTING_TYPES_LINKS_VARIANT(UNIQUE_SETTINGS_TYPES) \
    getSettingFieldPointer(unsigned short type) { \
    \
        switch (type) { \
            FIELDS(DECLARE_SWITCH_CASE) \
            default: return nullptr; \
        } \
    }

// Определяем поля структуры с указанием типа и названия настройки
#define SETTINGS_FIELDS(TYPE_AND_NAME) \
    TYPE_AND_NAME(String, access_key) \
    TYPE_AND_NAME(uint32_t, sensor_delay) \
    TYPE_AND_NAME(uint32_t, usb_delay) \
    TYPE_AND_NAME(uint32_t, wireless_delay) \
    TYPE_AND_NAME(String, wifi_ssid) \
    TYPE_AND_NAME(String, wifi_password) \
    TYPE_AND_NAME(uint32_t, battery_id) 

// Генерируем структуру и enum
GEN_SETTINGS(SETTINGS, SETTINGS_FIELDS)


typedef struct {
    SETTING_TYPE key;
    DECLARE_SETTING_TYPES_VARIANT(UNIQUE_SETTINGS_TYPES) value;
} SettingUpdate;

inline QueueHandle_t settingUpdateQueue = xQueueCreate(10, sizeof(SettingUpdate));