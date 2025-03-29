#ifndef __CORE_RTL_B_CPU_H__
#define __CORE_RTL_B_CPU_H__

#include "VTop.h"
#include "config.h"

#include <verilated_vcd_c.h>
#include <queue>

struct RobEntry {
  vaddr_t pc;
  word_t arf_rd;
  word_t prf_rd;
  bool ebreak;
  bool jmp;
  vaddr_t jmp_addr;
};

class Core {
  VTop *top_module;
  VerilatedContext *contextp;
  VerilatedVcdC *wave = nullptr;

  void do_eval() const;
  void do_cycle();
  void next_commit();

  word_t rat[32] = {};
  word_t prf[64] = {};
  std::queue<RobEntry> rob;

public:
  long tot_cycle = 0;
  long tot_inst = 0;
  vaddr_t pc = CONF_RESET_VEC;

  Core();
  ~Core();
  void init_wave();
  int do_step();
  [[nodiscard]] word_t gpr(int i) const;
  void log_stat() const;
};

extern VTop *top_module;

void cpu_init();
int cpu_step();
void cpu_exit();
void cpu_stat();

void init_wave();

#endif
