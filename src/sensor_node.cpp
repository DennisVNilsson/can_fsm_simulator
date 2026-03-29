#include "sensor_node.hpp"
#include <cstdlib>  // for rand()

// Global pointer so static action functions can access the node
static SensorNode* g_instance = nullptr;

// ── Actions ───────────────────────────────────────────────────────────────
static void do_sample() {
  g_instance->set_sensor_value(rand() % 1024); 
}

static void do_transmit() {
  auto sensor = g_instance->get_sensor_value();
  CanFrame cf{};
  cf.id = can::ID_SENSOR;
  cf.dlc = 2;
  cf.data[0] = static_cast<uint8_t>((sensor >> 8) & 0xFF);
  cf.data[1] = static_cast<uint8_t>(sensor & 0xFF);
  cf.rtr = false;
  g_instance->get_shared_bus()->transmit(cf); 
}

void SensorNode::tick()     { fsm_.dispatch(sensor::event::TICK);  }

void SensorNode::on_fault() { fsm_.dispatch(sensor::event::FAULT); }

// ── Transition table ──────────────────────────────────────────────────────
const Transition SensorNode::table_[] = {
    { sensor::state::IDLE,         sensor::event::TICK,  do_sample,   sensor::state::SAMPLING     },
    { sensor::state::SAMPLING,     sensor::event::TICK,  do_transmit, sensor::state::TRANSMITTING },
    { sensor::state::TRANSMITTING, sensor::event::TICK,  nullptr,     sensor::state::IDLE         },
    { sensor::state::IDLE,         sensor::event::FAULT, nullptr,     sensor::state::FAULT        },
};

// ── Constructor ───────────────────────────────────────────────────────────
SensorNode::SensorNode(CanBus* bus)
    : fsm_(table_, 4, sensor::state::IDLE)
    , shared_bus_(bus)
    , sensor_value_(0)
{
    g_instance = this;
}

uint16_t SensorNode::get_sensor_value() const { return sensor_value_; }

void SensorNode::set_sensor_value(uint16_t sv) { sensor_value_ = sv; }

CanBus* SensorNode::get_shared_bus() const { return shared_bus_; }

uint8_t SensorNode::state() const { return fsm_.state(); }

void SensorNode::reset() { fsm_.reset(); }