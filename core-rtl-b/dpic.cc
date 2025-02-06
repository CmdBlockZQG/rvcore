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
  const word_t data = wdata >> ((waddr & 0x3u) * 8);
  const word_t mask = wmask >> (waddr & 0x3u);
  switch (wsize) {
    case 0: assert(mask == 0x1); paddr_write(waddr, 1, data); break;
    case 1: assert(mask == 0x3); paddr_write(waddr, 2, data); break;
    case 2: assert(mask == 0xf); paddr_write(waddr, 4, data); break;
    default: assert(0);
  }
}
