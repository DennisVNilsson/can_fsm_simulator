#pragma once

#include <cstdint>

struct CanFrame
{
  uint16_t id; 
  uint8_t dlc; 
  uint8_t data[8]; 
  bool rtr; 
};

namespace can {
    static constexpr uint16_t ID_SENSOR     = 0x100;
    static constexpr uint16_t ID_CONTROLLER = 0x200;
    static constexpr uint16_t ID_ACTUATOR   = 0x300;
    static constexpr uint16_t ID_FAULT      = 0x7FF;
}