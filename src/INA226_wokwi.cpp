#include <INA226_wokwi.hpp>

float generateRandomFloat(float min, float max) {
    return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(max-min)));
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
}

//  Core functions
float INA226::getBusVoltage() {
    return generateRandomFloat(3.0f, 4.2f);
}

float INA226::getShuntVoltage() {
    return getBusVoltage()-0.1;
}

float INA226::getCurrent() {
    return generateRandomFloat(0.0f, 10.0f);
}

float INA226::getPower() {
    return getBusVoltage()*getCurrent();
}