#include "device.h"

Device::Device(paddr_t base, paddr_t size): base(base), size(size) { }

Device::~Device() { }

bool Device::in(paddr_t addr) {
  return base <= addr && addr < base + size;
}

paddr_t Device::get_base() {
  return base;
}
