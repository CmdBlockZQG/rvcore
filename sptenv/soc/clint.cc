#include "utils.h"
#include "clint.h"

#include <ctime>

static uint64_t get_time() {
  timespec now{};
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  return now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

uint64_t CLINT::get_mtime() const {
  return get_time() - mtime_base;
}

CLINT::CLINT(const paddr_t base): Device(base, 0xc000) {
  mtime_base = get_time();
}

CLINT::~CLINT() = default;

void CLINT::write(paddr_t addr, const int len, const word_t data) {
  if (addr == 0) { // MSIP
    Check(addr + len <= 4);
    msip = data & 1;
  } else if (0x4000 <= addr && addr <= 0x4008) { // MTIMECMP
    Check(addr + len <= 0x4008);
    addr -= 0x4000;
    if constexpr (xlen == 32) {
      Check(len == 4 && (addr == 0 || addr == 4));
      if (addr == 0) mtimecmp = (mtimecmp & ~bit_mask(32)) | data;
      else mtimecmp = (mtimecmp & bit_mask(32)) | (static_cast<uint64_t>(data) << 32);
    } else {
      assert(0);
    }
  } else if (0xbff8 <= addr) { // MTIME
    Check(addr + len <= 0xc000);
    addr -= 0xbff8;
    uint64_t mtime = get_mtime();
    if constexpr (xlen == 32) {
      Check(len == 4 && (addr == 0 || addr == 4));
      if (addr == 0) mtime = (mtime & ~bit_mask(32)) | data;
      else mtime = (mtime & bit_mask(32)) | (static_cast<uint64_t>(data) << 32);
    } else {
      assert(0);
    }
    mtime_base = get_time() - mtime;
  } else {
    Check(0);
  }
}

word_t CLINT::read(paddr_t addr, const int len) {
  if (addr < 0x0004) { // MSIP
    Check(addr + len <= 4);
    return static_cast<uint32_t>(msip) >> (addr * 8);
  }
  if (0x4000 <= addr && addr <= 0x4008) { // MTIMECMP
    Check(addr + len <= 0x4008);
    addr -= 0x4000;
    return mtimecmp >> (addr * 8);
  }
  if (0xbff8 <= addr) { // MTIME
    Check(addr + len <= 0xc000);
    addr -= 0xbff8;
    return get_mtime() >> (addr * 8);
  }
  Check(0);
}

word_t CLINT::get_mtip() const {
  return get_mtime() >= mtimecmp;
}

word_t CLINT::get_msip() const {
  return msip;
}
