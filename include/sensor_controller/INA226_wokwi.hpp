#pragma once

#include "Arduino.h"
#include <Wire.h>
#include <cstdlib>
#include <ctime>


class INA226
{
public:
  //  address between 0x40 and 0x4F
  explicit INA226(const uint8_t address, TwoWire *wire = &Wire);

  bool     begin();
  bool     isConnected();
  uint8_t  getAddress();


  //  Core functions
  float    getBusVoltage();       //  Volt
  float    getShuntVoltage();     //  Volt
  float    getCurrent();          //  Ampere
  float    getPower();            //  Watt
  //  See #35
  bool     isConversionReady();   //  conversion ready flag is set.
  bool     waitConversionReady(uint32_t timeout = 10);


  //  Scale helpers milli range
  float    getBusVoltage_mV()   { return getBusVoltage()   * 1e3; };
  float    getShuntVoltage_mV() { return getShuntVoltage() * 1e3; };
  float    getCurrent_mA()      { return getCurrent()      * 1e3; };
  float    getPower_mW()        { return getPower()        * 1e3; };
  //  Scale helpers micro range
  float    getBusVoltage_uV()   { return getBusVoltage()   * 1e6; };
  float    getShuntVoltage_uV() { return getShuntVoltage() * 1e6; };
  float    getCurrent_uA()      { return getCurrent()      * 1e6; };
  float    getPower_uW()        { return getPower()        * 1e6; };


  //  Configuration
  bool     reset();
  bool     setAverage(uint8_t avg = 10.0);
  uint8_t  getAverage();
  bool     setBusVoltageConversionTime(uint8_t bvct = 10.0);
  uint8_t  getBusVoltageConversionTime();
  bool     setShuntVoltageConversionTime(uint8_t svct = 10.0);
  uint8_t  getShuntVoltageConversionTime();


  //  Calibration
  //  mandatory to set these!
  //  shunt * maxCurrent < 81 mV
  //  maxCurrent >= 0.001
  //  shunt      >= 0.001
  int      setMaxCurrentShunt(float maxCurrent = 20.0, float shunt = 0.002, bool normalize = true);
  bool     isCalibrated()     { return _current_LSB != 0.0; };

  //  These functions return zero if not calibrated!
  float    getCurrentLSB()    { return _current_LSB;       };
  float    getCurrentLSB_mA() { return _current_LSB * 1e3; };
  float    getCurrentLSB_uA() { return _current_LSB * 1e6; };
  float    getShunt()         { return _shunt;             };
  float    getMaxCurrent()    { return _maxCurrent;        };


  //  Operating mode
  bool     setMode(uint8_t mode = 7);  //  default ModeShuntBusContinuous
  uint8_t  getMode();
  bool     shutDown()                  { return setMode(0); };
  bool     setModeShuntTrigger()       { return setMode(1); };
  bool     setModeBusTrigger()         { return setMode(2); };
  bool     setModeShuntBusTrigger()    { return setMode(3); };
  bool     setModeShuntContinuous()    { return setMode(5); };
  bool     setModeBusContinuous()      { return setMode(6); };
  bool     setModeShuntBusContinuous() { return setMode(7); };  //  default.


  //  Alert
  //  - separate functions per flag?
  //  - what is a reasonable limit?
  //  - which units to define a limit per mask ?
  //    same as voltage registers ?
  //  - how to test
  bool     setAlertRegister(uint16_t mask);
  uint16_t getAlertFlag();
  bool     setAlertLimit(uint16_t limit);
  uint16_t getAlertLimit();


  //  Meta information
  uint16_t getManufacturerID();   //  should return 0x5449
  uint16_t getDieID();            //  should return 0x2260


  //  DEBUG
  uint16_t getRegister(uint8_t reg)  { return _readRegister(reg); };


private:

  uint16_t _readRegister(uint8_t reg);
  uint16_t _writeRegister(uint8_t reg, uint16_t value);
  float    _current_LSB;
  float    _shunt;
  float    _maxCurrent;

  uint8_t   _address;
  TwoWire * _wire;
  bool connected = false;
};