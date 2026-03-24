#pragma once
#include <cstdint>
#include <cstddef>

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
class PoolAllocator {
  private:
    alignas(alignof(std::max_align_t)) uint8_t pool_[BLOCK_SIZE * NUM_BLOCKS];
    void* free_head_; 
    size_t free_count_; 
    size_t peak_usage_; 
    void init();

  public:
    PoolAllocator(); 
    void* alloc(); 
    void free(void* ptr);
    void reset();
    size_t free_count() const; 
    size_t used_count() const; 
    size_t peak_usage() const; 
    size_t capacity() const; 
}; 

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::PoolAllocator() { init(); }; 

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
void PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::init() {
  free_count_ = capacity(); 
  peak_usage_ = 0; 
  for (size_t i = 0; i < NUM_BLOCKS; i++) {
    void* block = pool_ + i * BLOCK_SIZE; 
    void* next;
    if (i == NUM_BLOCKS - 1) {
      next = nullptr;
    } else {
      next = pool_ + (i + 1) * BLOCK_SIZE; 
    }
    *reinterpret_cast<void**>(block) = next;
  }
  free_head_ = pool_;
}

/*
Reads the pointer stored in free_head_, advances the list
*/
template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
void* PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::alloc() {
  if (!free_head_) {
    return nullptr;
  }
  void* block = free_head_; 
  free_head_ = *reinterpret_cast<void**>(block); 
  free_count_--; 
  if (used_count() > peak_usage_) {peak_usage_ = used_count();}
  return block; 
}

/*
Writes the current free_head_ into ptr, prepends to the list
*/
template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
void PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::free(void* ptr) {
  if (!ptr) {return;}
  free_count_++; 
  *reinterpret_cast<void**>(ptr) = free_head_; 
  free_head_ = ptr;
}

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
void PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::reset() {
  init();
}

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
size_t PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::free_count() const {
  return free_count_; 
}

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
size_t PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::used_count() const {
  return capacity() - free_count_; 
}

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
size_t PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::peak_usage() const {
  return peak_usage_; 
}

template<size_t BLOCK_SIZE, size_t NUM_BLOCKS>
size_t PoolAllocator<BLOCK_SIZE, NUM_BLOCKS>::capacity() const {
  return NUM_BLOCKS;
}