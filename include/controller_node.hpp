#pragma once 

#include <cstdint>
#include "can_frame.hpp"
#include "can_bus.hpp"
#include "fsm.hpp"

namespace controller
{
  namespace state
  {
    static constexpr uint8_t MONITORING = 0; 
    static constexpr uint8_t EVALUATING = 1; 
    static constexpr uint8_t COMMANDING = 2; 
    static constexpr uint8_t FAULT = 3; 
  } 
  namespace event
  {
    static constexpr uint8_t FRAME_RECEIVED = 0; 
    static constexpr uint8_t TICK = 1; 
    static constexpr uint8_t FAULT = 2; 
    static constexpr uint8_t THRESHOLD_EXCEEDED = 3;
    static constexpr uint8_t THRESHOLD_OK       = 4;
  } 
} 

class ControllerNode
{
private:
  uint16_t threshold_; 
  uint16_t last_reading_; 
  Fsm fsm_;
  CanBus* bus_;
  static const Transition table_[];
public:
  ControllerNode(CanBus* bus, uint16_t threshold);
  static void on_receive(const CanFrame& f); 
  void tick();
  void on_fault();
  uint8_t state() const;
  void reset();
  void set_last_reading(uint16_t lr);
  CanBus* getBus(); 
};
