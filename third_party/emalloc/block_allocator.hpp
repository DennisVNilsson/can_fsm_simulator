#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <new>

struct BlockHeader {
  size_t size; 
  bool is_free; 
  BlockHeader* next;
  BlockHeader* prev; 
  static constexpr uint32_t MAGIC = 0xDEADBEEF;
  uint32_t magic;
}; 

struct HeapStat {
  size_t total_bytes; 
  size_t used_bytes; 
  size_t free_bytes; 
  size_t largest_free_block; 
  size_t allocation_count; 
  float fragmentation; // In range [0,1]
};

template<size_t HEAP_SIZE>
class BlockAllocator {
  private:
    uint8_t heap_[HEAP_SIZE];
    BlockHeader* head_; 
    void init(); 

  public:
    BlockAllocator(); 
    void* alloc(size_t size);
    void free(void* ptr); 
    void reset();
    void verify();
    HeapStat stats() const;
    void print_stats() const;
    static constexpr uint32_t CANARY = 0xCAFEBABE; 
};

template<size_t HEAP_SIZE>
BlockAllocator<HEAP_SIZE>::BlockAllocator() { init(); }

template<size_t HEAP_SIZE>
void BlockAllocator<HEAP_SIZE>::init() { 
  BlockHeader* h = new (heap_) BlockHeader(); // Constructs a BlockHeader at the adress heap_ 
  h -> size = HEAP_SIZE - sizeof(BlockHeader) - sizeof(uint32_t);
  h -> is_free = true; 
  h -> next = nullptr;
  h -> prev = nullptr; 
  h -> magic = h -> BlockHeader::MAGIC; 
  head_ = h; 
}

// alloc(size_t size)
//
// Walks the block list from head_ looking for the first free block large
// enough to satisfy the request (first-fit algorithm).
//
// If a suitable block is found and has enough leftover space, it is split
// into two blocks: one exactly 'size' bytes (handed to the caller) and one
// covering the remainder (left free for future allocations). This prevents
// wasting large blocks on small requests.
//
// The pointer returned points to the data region AFTER the BlockHeader —
// the caller never sees the header itself.
//
// Returns nullptr if no free block large enough exists.
template<size_t HEAP_SIZE>
void* BlockAllocator<HEAP_SIZE>::alloc(size_t size) {
  BlockHeader* current = head_; 
  while(current) {
    if (current->is_free && current->size >= size) {
      if (current->size >= size + sizeof(BlockHeader) + sizeof(uint32_t) + 1) {
        uint8_t* new_header_addr = reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader) + size + sizeof(uint32_t); 
        BlockHeader* new_block = new (new_header_addr) BlockHeader(); 
        new_block->size = current->size - size - sizeof(BlockHeader) - sizeof(uint32_t); 
        new_block->is_free = true; 
        new_block->prev = current; 
        new_block->next = current->next; 
        if (new_block->next) {
          new_block->next->prev = new_block;
        }
        new_block->magic = BlockHeader::MAGIC;  
        current->size = size;
        current->next = new_block; 
      }
      uint32_t* canary_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader) + current->size);
      *canary_ptr = CANARY; 
      current->is_free = false;
      return reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader);
    }
    current = current->next; 
  }
  return nullptr; 
}

// free(void* ptr)
//
// Accepts a pointer previously returned by alloc() and marks the
// corresponding block as free.
//
// The BlockHeader sits exactly sizeof(BlockHeader) bytes before ptr —
// since alloc() returns (header address + sizeof(BlockHeader)), this
// operation is the exact reverse: (ptr - sizeof(BlockHeader)).
//
// After marking free, checks if the immediately following block is also
// free. If so, the two blocks are merged into one (coalescing) to prevent
// fragmentation building up over time.
//
// Passing nullptr is a safe no-op. Passing a pointer with a corrupted
// magic number is silently ignored — it did not come from this allocator.
template<size_t HEAP_SIZE>
void BlockAllocator<HEAP_SIZE>::free(void* ptr) {
  if (!ptr) return; 
  BlockHeader* h = reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(BlockHeader)); 
  if (h->magic != BlockHeader::MAGIC) return;  // not a valid block
  uint32_t* canary_ptr = reinterpret_cast<uint32_t*>(
    reinterpret_cast<uint8_t*>(h) + sizeof(BlockHeader) + h->size
  ); 
  if (*canary_ptr != CANARY) { 
    printf("[emalloc] CORRUPTION DETECTED at %p size=%lu\n", ptr, (unsigned long)h->size);
    return;
  }
  h->is_free = true; 
  if (h->next && h->next->is_free) {
    h->size = h->size + sizeof(BlockHeader) + h->next->size;
    h->next = h->next->next;
    if (h->next) {
        h->next->prev = h;  
    }
  }
  if (h->prev && h->prev->is_free) {
    h->prev->size = h->prev->size + sizeof(BlockHeader) + sizeof(uint32_t) + h->size;
    h->prev->next = h->next; 
    if (h->next) {
      h->next->prev = h->prev; 
    }
    h->magic = 0; // poison - this header no longer exists
  }
} 

template<size_t HEAP_SIZE>
void BlockAllocator<HEAP_SIZE>::verify() {
  BlockHeader* current = head_;
  const uint8_t* heap_start = heap_;
  const uint8_t* heap_end   = heap_ + HEAP_SIZE;
  while (current) {
    const uint8_t* addr = reinterpret_cast<const uint8_t*>(current);
    if (addr < heap_start || addr >= heap_end) {
      break;
    }
    if (!current->is_free) {
      uint32_t* canary_ptr = reinterpret_cast<uint32_t*>(
        reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader) + current->size
      );
      if (*canary_ptr != CANARY) {
        void* data_ptr = reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader);
        printf("[emalloc] CORRUPTION DETECTED at %p size=%lu\n", data_ptr, (unsigned long)current->size);
      }
    }
    if (current->next && current->next->prev != current) {
      printf("[emalloc] LINKED LIST CORRUPTION at %p\n",
      reinterpret_cast<void*>(current));
    }
    current = current->next;
  }
}

template<size_t HEAP_SIZE>
void BlockAllocator<HEAP_SIZE>::reset() {
  init(); 
}

template<size_t HEAP_SIZE>
HeapStat BlockAllocator<HEAP_SIZE>::stats() const {
    HeapStat s{};  // zero-initialise all fields
    s.total_bytes = HEAP_SIZE;

    BlockHeader* current = head_;
    while (current) {
        // accumulate fields based on current->is_free and current->size
        if (current->is_free) {
          s.free_bytes += current->size; 
          if (s.largest_free_block < current->size) {
            s.largest_free_block = current->size; 
          }
        } else {
          s.used_bytes += current->size; 
          s.allocation_count++; 
        }
        
        current = current->next;
    }

    // calculate fragmentation after the loop
    s.fragmentation = (s.free_bytes != 0) ? 1.0f - (static_cast<float>(s.largest_free_block) / static_cast<float>(s.free_bytes)) : 0.0f; 

    return s;
}

template<size_t HEAP_SIZE>
void BlockAllocator<HEAP_SIZE>::print_stats() const {
    HeapStat s = stats();
    printf("── Heap Stats ───────────────────────\n");
    printf("  total       : %lu bytes\n",  (unsigned long)s.total_bytes);
    printf("  used        : %lu bytes\n",  (unsigned long)s.used_bytes);
    printf("  free        : %lu bytes\n",  (unsigned long)s.free_bytes);
    printf("  largest free: %lu bytes\n",  (unsigned long)s.largest_free_block);
    printf("  allocations : %lu\n",        (unsigned long)s.allocation_count);
    printf("  fragmentation: %.1f%%\n",   s.fragmentation * 100.0f);
    printf("─────────────────────────────────────\n");
}