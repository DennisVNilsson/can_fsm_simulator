#include "fsm.hpp"

Fsm::Fsm(const Transition* tbl, size_t tbl_size, uint8_t initial_state)
    : table_(tbl), table_size_(tbl_size), state_(initial_state), initial_state_(initial_state)
{}

void Fsm::dispatch(uint8_t event) {
  for (size_t i = 0; i < table_size_; i++) {
    if (table_[i].from == state_ && table_[i].event == event) {
      if (table_[i].action) {
        table_[i].action();
      }
      state_ = table_[i].to; 
      return; 
    }
  }
}

uint8_t Fsm::state() const { return state_; }

void Fsm::reset() { state_ = initial_state_; } 