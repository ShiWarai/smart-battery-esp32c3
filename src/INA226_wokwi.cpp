#include <INA226_wokwi.hpp>

const int MAX_COUNTER = 100;
int sin_generator_counter = 0;
int cos_generator_counter = 0;

float generateSinusoidalValue(float min_v, float max_v) {
    float amplitude = (max_v - min_v) / 2.0;
    float offset = min_v + amplitude;
    float radians = static_cast<float>(sin_generator_counter) * M_PI / 50.0; // преобразование счетчика в радианы

    sin_generator_counter = (sin_generator_counter + 1) % MAX_COUNTER;
    return offset + amplitude * sin(radians);
}

float generateCosinusoidalValue(float min_v, float max_v) {
    float amplitude = (max_v - min_v) / 2.0;
    float offset = min_v + amplitude;
    float radians = static_cast<float>(cos_generator_counter) * M_PI / 50.0; // преобразование счетчика в радианы

    cos_generator_counter = (cos_generator_counter + 1) % MAX_COUNTER;
    return offset + amplitude * cos(radians);
}

INA226::INA226(const uint8_t address, TwoWire *wire) {
    std::srand(static_cast<unsigned int>(std::time(0)));

    this->_address = address;
    this->_wire = wire;
}

bool INA226::begin() {
    this->connected = true;
    return true;
}

bool INA226::isConnected() {
    return this->connected;
}

int INA226::setMaxCurrentShunt(float maxCurrent, float shunt, bool normalize) {
    this->_maxCurrent = maxCurrent;
    this->_shunt = shunt;
    this->_current_LSB = 1.0;

    return 1;
}

//  Core functions
float INA226::getBusVoltage() {
    return generateSinusoidalValue(3.9f, 4.0f);
}

float INA226::getShuntVoltage() {
    return getBusVoltage()-0.1;
}

float INA226::getCurrent() {
    return generateCosinusoidalValue(0.5f, 1.0f);
}

float INA226::getPower() {
    return getBusVoltage()*getCurrent();
}