#include "Vtop__Dpi.h"

#include "common.h"
#include "cpu.h"
#include "soc.h"

int pmem_read(int raddr, int *rdata) {
  raddr = raddr & ~0x3u;
  *reinterpret_cast<word_t *>(rdata) = paddr_read(raddr, 4);
}

void pmem_write(int waddr, int wdata, char wmask) {
  waddr = waddr & ~0x3u;
  if (wmask & 0b0001) paddr_write(waddr + 0, 1, wdata >> 0);
  if (wmask & 0b0010) paddr_write(waddr + 1, 1, wdata >> 8);
  if (wmask & 0b0100) paddr_write(waddr + 2, 1, wdata >> 16);
  if (wmask & 0b1000) paddr_write(waddr + 3, 1, wdata >> 24);
}
