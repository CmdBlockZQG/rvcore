#include "Vtop__Dpi.h"

#include "common.h"
#include "cpu.h"
#include "soc.h"

void halt() {
  if (gpr(10)) cpu_state = ST_ABORT;
  else cpu_state = ST_HALT;
}

void inst_complete(int new_pc, int new_inst) {
  cpu_pc = new_pc;
  cpu_state = ST_STOP;
}

void event_mem_read(int raddr, int rsize, int rdata) {
}

void event_mem_write(int waddr, int wsize, int wdata) {
}

int pmem_read(int raddr) {
  raddr = raddr & ~0x3u;
  word_t rdata = paddr_read(raddr, 4);
  return *reinterpret_cast<int *>(&rdata);
}

void pmem_write(int waddr, int wdata, char wmask) {
  waddr = waddr & ~0x3u;
  if (wmask & 0b0001) paddr_write(waddr + 0, 1, wdata >> 0);
  if (wmask & 0b0010) paddr_write(waddr + 1, 1, wdata >> 8);
  if (wmask & 0b0100) paddr_write(waddr + 2, 1, wdata >> 16);
  if (wmask & 0b1000) paddr_write(waddr + 3, 1, wdata >> 24);
}
