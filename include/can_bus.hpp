#pragma once
#include <cstdint>
#include "can_frame.hpp"

struct NodeHandle
{
  uint16_t filter_id; 
  uint16_t filter_mask; // 0x7FF = exact match, 0x000 = accept all
  void (*callback)(const CanFrame&); 
};

class CanBus
{
private:
  NodeHandle nodes_[8]; 
  uint8_t node_count_; 
  CanFrame queue_[16]; 
  uint8_t queue_head_;
  uint8_t queue_tail_;
  uint8_t queue_count_;

public:
  CanBus();
  void attach(NodeHandle node); 
  void transmit(const CanFrame& frame); 
  void tick(); 
};
