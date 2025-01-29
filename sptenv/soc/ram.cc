#include "utils.h"
#include "ram.h"

RAM::RAM(const paddr_t base, const paddr_t size): Device(base, size) {
  ptr = new uint8_t[size];
}

RAM::~RAM() {
  delete[] ptr;
}

void RAM::write(const paddr_t addr, const int len, const word_t data) {
  Check(addr < size);
  void *host_addr = ptr + addr;
  switch (len) {
    case 1: *static_cast<uint8_t  *>(host_addr) = data; return;
    case 2: *static_cast<uint16_t *>(host_addr) = data; return;
    case 4: *static_cast<uint32_t *>(host_addr) = data; return;
    case 8: assert(0);
    default:
      Check(0);
  }
}

word_t RAM::read(const paddr_t addr, const int len) {
  Check(addr < size);
  void *host_addr = ptr + addr;
  switch (len) {
    case 1: return *static_cast<uint8_t  *>(host_addr);
    case 2: return *static_cast<uint16_t *>(host_addr);
    case 4: return *static_cast<uint32_t *>(host_addr);
    case 8: assert(0);
    default:
      Check(0);
  }
}

void *RAM::get_ptr(const paddr_t addr) const {
  Check(in(addr));
  return ptr + (addr - base);
}
