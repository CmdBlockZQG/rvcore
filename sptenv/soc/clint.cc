#include "clint.h"

#include <ctime>

static uint64_t get_time() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  return now.tv_sec * 10000000 + now.tv_nsec / 100;
}

uint64_t CLINT::get_mtime() {
  return get_time() - mtime_base;
}

CLINT::CLINT(paddr_t addr): Device(addr, 0xc000) {
  mtime_base = get_time();
}

CLINT::~CLINT() { }

void CLINT::write(paddr_t addr, int len, word_t data) {
  if (addr == 0) { // MSIP
    if constexpr (rt_check) assert(addr + len <= 4);
    msip = data & 1;
  } else if (0x4000 <= addr && addr <= 0x4008) { // MTIMECMP
    if constexpr (rt_check) assert(addr + len <= 0x4008);
    addr -= 0x4000;
    if constexpr (xlen == 32) {
      if constexpr (rt_check) assert(len == 4 && (addr == 0 || addr == 4));
      if (addr == 0) mtimecmp = (mtimecmp & ~bit_mask(32)) | data;
      else mtimecmp = (mtimecmp & bit_mask(32)) | (static_cast<uint64_t>(data) << 32);
    } else {
      if constexpr (rt_check) assert(len == 8);
      mtimecmp = data;
    }
  } else if (0xbff8 <= addr) { // MTIME
    if constexpr (rt_check) assert(addr + len <= 0xc000);
    addr -= 0xbff8;
    uint64_t mtime = get_mtime();
    if constexpr (xlen == 32) {
      if constexpr (rt_check) assert(len == 4 && (addr == 0 || addr == 4));
      if (addr == 0) mtime = (mtime & ~bit_mask(32)) | data;
      else mtime = (mtime & bit_mask(32)) | (static_cast<uint64_t>(data) << 32);
    } else {
      if constexpr (rt_check) assert(len == 8);
      mtime = data;
    }
    mtime_base = get_time() - mtime;
  } else {
    if constexpr (rt_check) assert(0);
  }
}

word_t CLINT::read(paddr_t addr, int len) {
  if (addr < 0x0004) { // MSIP
    if constexpr (rt_check) assert(addr + len <= 4);
    return static_cast<uint32_t>(msip) >> (addr * 8);
  } else if (0x4000 <= addr && addr <= 0x4008) { // MTIMECMP
    if constexpr (rt_check) assert(addr + len <= 0x4008);
    addr -= 0x4000;
    return mtimecmp >> (addr * 8);
  } else if (0xbff8 <= addr) { // MTIME
    if constexpr (rt_check) assert(addr + len <= 0xc000);
    addr -= 0xbff8;
    return get_mtime() >> (addr * 8);
  } else {
    if constexpr (rt_check) assert(0);
  }
  return 0;
}

word_t CLINT::get_mtip() {
  return get_mtime() >= mtimecmp;
}

word_t CLINT::get_msip() {
  return msip;
}
