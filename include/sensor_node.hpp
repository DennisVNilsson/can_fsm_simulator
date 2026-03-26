#pragma once

#include <cstdint>
#include "fsm.hpp"
#include "can_bus.hpp"

namespace sensor
{
  namespace state
  {
    static constexpr uint8_t IDLE = 0;
    static constexpr uint8_t SAMPLING = 1;
    static constexpr uint8_t TRANSMITTING = 2;
    static constexpr uint8_t FAULT = 3; 
  } // namespace state
  namespace event
  {
    static constexpr uint8_t TICK = 0;
    static constexpr uint8_t FAULT = 1;
  } // namespace event  
} // namespace sensor


class SensorNode
{
private:
  Fsm fsm_; 
  CanBus* shared_bus_; 
  uint16_t sensor_value_; 
  static const Transition table_[]; 
public:
  SensorNode(CanBus* bus); 
  void tick(); 
  void on_fault();
  uint16_t get_sensor_value() const; 
  void set_sensor_value(uint16_t sv); 
  CanBus* get_shared_bus() const; 
  uint8_t state() const;
};