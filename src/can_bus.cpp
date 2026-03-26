#include "can_bus.hpp"

CanBus::CanBus() {
  node_count_ = 0; 
  queue_head_ = 0;
  queue_tail_ = 0;
  queue_count_ = 0;
}

void CanBus::attach(NodeHandle node) {
  if (node_count_ >= 8) return; //bus full
  nodes_[node_count_++] = node; 
}

void CanBus::transmit(const CanFrame& frame) {
  if (queue_count_ >= 16) return; 
  queue_[queue_tail_] = frame; 
  queue_tail_ = (queue_tail_ + 1) % 16; 
  queue_count_++; 
}

void CanBus::tick() {
  if (queue_count_ == 0) return; 
  CanFrame frame = queue_[queue_head_]; 
  queue_head_ = (queue_head_ + 1) % 16; 
  queue_count_--; 

  for (uint8_t i = 0; i < node_count_; i++) {
    if ((frame.id & nodes_[i].filter_mask) == nodes_[i].filter_id) {
      if (nodes_[i].callback) {
        nodes_[i].callback(frame); 
      }
    }
  }
}