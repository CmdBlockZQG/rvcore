#include "device.h"

Device::Device(const paddr_t base, const paddr_t size): base(base), size(size) { }

Device::~Device() = default;

bool Device::in(const paddr_t addr) const {
  return base <= addr && addr < base + size;
}

paddr_t Device::get_base() const {
  return base;
}
