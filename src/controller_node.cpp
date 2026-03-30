#include "controller_node.hpp"
#include "can_frame.hpp"

static ControllerNode* g_instance = nullptr;


static void do_send_command() {
    CanFrame cf{};
    cf.id    = can::ID_CONTROLLER;
    cf.dlc   = 1;
    cf.data[0] = 0x01;  // command byte — "actuate"
    cf.rtr   = false;
    g_instance->getBus()->transmit(cf); 
}

const Transition ControllerNode::table_[] = {
  {controller::state::MONITORING,   controller::event::FRAME_RECEIVED,      nullptr,          controller::state::EVALUATING},
  {controller::state::EVALUATING,   controller::event::THRESHOLD_EXCEEDED,  do_send_command,  controller::state::COMMANDING},
  {controller::state::EVALUATING,   controller::event::THRESHOLD_OK,        nullptr,          controller::state::MONITORING},
  {controller::state::COMMANDING,   controller::event::TICK,                nullptr,          controller::state::MONITORING},
  {controller::state::MONITORING,   controller::event::FAULT,               nullptr,          controller::state::FAULT },
  {controller::state::EVALUATING,   controller::event::FAULT,               nullptr,          controller::state::FAULT },
  {controller::state::COMMANDING,   controller::event::FAULT,               nullptr,          controller::state::FAULT },
}; 

ControllerNode::ControllerNode(CanBus* bus, uint16_t threshold)
  : threshold_(threshold),
      last_reading_(0),
      fsm_(table_, 7, controller::state::MONITORING),
      bus_(bus) {
  g_instance = this;
  if (bus_) {
      bus_->attach({can::ID_SENSOR, 0x7FF, &ControllerNode::on_receive});
  }
}

void ControllerNode::on_receive(const CanFrame& f) {
  if (!g_instance) return;

  if (f.dlc >= 2) {
      const auto reading = static_cast<uint16_t>((static_cast<uint16_t>(f.data[0]) << 8) | f.data[1]);
      g_instance->set_last_reading(reading);
      g_instance->fsm_.dispatch(controller::event::FRAME_RECEIVED);
  }

}


void ControllerNode::tick() {
    if (fsm_.state() == controller::state::EVALUATING) {
        if (last_reading_ > threshold_) {
            fsm_.dispatch(controller::event::THRESHOLD_EXCEEDED);
        } else {
            fsm_.dispatch(controller::event::THRESHOLD_OK);
        }
    } else {
        fsm_.dispatch(controller::event::TICK);
    }
}

void ControllerNode::on_fault() { fsm_.dispatch(controller::event::FAULT); }

uint8_t ControllerNode::state() const { return fsm_.state(); }

void ControllerNode::reset() { fsm_.reset(); }

void ControllerNode::set_last_reading(uint16_t lr) { last_reading_ = lr; }

CanBus* ControllerNode::getBus() { return bus_; }