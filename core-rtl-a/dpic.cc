#include "VTop__Dpi.h"

#include "common.h"
#include "soc.h"
#include <cassert>

void pmem_read(const int raddr, const int rsize, int *rdata) {
  word_t &res = *reinterpret_cast<word_t *>(rdata);
  word_t data;
  switch (rsize) {
    case 0: data = paddr_read(raddr, 1); break;
    case 1: data = paddr_read(raddr & ~0x1u, 2); break;
    case 2: data = paddr_read(raddr & ~0x3u, 4); break;
    default: assert(0);
  }
  data <<= (raddr & 0x3u) * 8;
  res = data;
}

void pmem_write(const int waddr, const int wsize, const int wdata, const char wmask) {
  const int lo = waddr & 0x3;
  const word_t data = wdata >> (lo * 8);
  const word_t mask = wmask >> lo;
  switch (wsize) {
    case 0: if (mask == 0x1) { paddr_write(waddr, 1, data); return; } break;
    case 1: if (mask == 0x3) { paddr_write(waddr, 2, data); return; } break;
    case 2: if (mask == 0xf) { paddr_write(waddr, 4, data); return; } break;
    default: assert(0);
  }
  const paddr_t addr = waddr & ~0x3u;
  if (wmask & 0b0001) paddr_write(addr + 0, 1, wdata >> 0);
  if (wmask & 0b0010) paddr_write(addr + 1, 1, wdata >> 8);
  if (wmask & 0b0100) paddr_write(addr + 2, 1, wdata >> 16);
  if (wmask & 0b1000) paddr_write(addr + 3, 1, wdata >> 24);
}
