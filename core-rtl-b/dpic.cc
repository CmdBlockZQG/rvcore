#include "VTop__Dpi.h"

#include "common.h"
#include "soc.h"

void pmem_read(const int raddr, int *rdata) {
  const paddr_t addr = raddr & ~0x3u;
  *reinterpret_cast<word_t *>(rdata) = paddr_read(addr, 4);
}

void pmem_write(const int waddr, const int wdata, const char wmask) {
  const paddr_t addr = waddr & ~0x3u;
  if (wmask & 0b0001) paddr_write(addr + 0, 1, wdata >> 0);
  if (wmask & 0b0010) paddr_write(addr + 1, 1, wdata >> 8);
  if (wmask & 0b0100) paddr_write(addr + 2, 1, wdata >> 16);
  if (wmask & 0b1000) paddr_write(addr + 3, 1, wdata >> 24);
}
