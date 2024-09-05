#include "ram.h"

RAM::RAM(paddr_t base, paddr_t size): Device(base, size) {
  ptr = new uint8_t[size];
}

RAM::~RAM() {
  delete[] ptr;
}

void RAM::write(paddr_t addr, int len, word_t data) {
  if constexpr (rt_check) {
    assert(addr < size);
  }
  void *host_addr = ptr + addr;
  switch (len) {
    case 1: *static_cast<uint8_t  *>(host_addr) = data; return;
    case 2: *static_cast<uint16_t *>(host_addr) = data; return;
    case 4: *static_cast<uint32_t *>(host_addr) = data; return;
    case 8:
      if constexpr (xlen == 64) {
        *static_cast<uint64_t *>(host_addr) = data; return;
      } else assert(0);
    default:
      if constexpr (rt_check) assert(0);
  }
}

word_t RAM::read(paddr_t addr, int len) {
  if constexpr (rt_check) {
    assert(addr < size);
  }
  void *host_addr = ptr + addr;
  switch (len) {
    case 1: return *static_cast<uint8_t  *>(host_addr);
    case 2: return *static_cast<uint16_t *>(host_addr);
    case 4: return *static_cast<uint32_t *>(host_addr);
    case 8:
      if constexpr (xlen == 64) {
        return *static_cast<uint64_t *>(host_addr);
      } else assert(0);
    default:
      if constexpr (rt_check) assert(0);
  }
  return 0;
}

void *RAM::get_ptr(paddr_t addr) {
  if constexpr (rt_check) {
    assert(in(addr));
  }
  return ptr + (addr - base);
}
