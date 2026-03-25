#pragma once

#include <cstdint>
#include <cstddef>

struct Transition
{
  uint8_t from; 
  uint8_t event; 
  void (*action)(); 
  uint8_t to; 
};

class Fsm
{
private:
  const Transition* table_; 
  size_t table_size_; 
  uint8_t state_;
  uint8_t initial_state_; 

public:
  Fsm(const Transition* tbl, size_t tbl_size, uint8_t initial_state);
  void dispatch(uint8_t event);
  uint8_t state() const; 
  void reset(); 
};