#include "cpu.h"

Vtop *top_module;

int cpu_state = ST_RUNNING;
vaddr_t cpu_pc;

static VerilatedContext *contextp;

static void do_eval() {
  top_module->eval();
  contextp->timeInc(1);
}

static void do_cycle() {
  top_module->clock = 0; do_eval();
  top_module->clock = 1; do_eval();
}

void cpu_init() {
  contextp = new VerilatedContext;
  contextp->commandArgs(0, static_cast<char **>(nullptr));
  top_module = new Vtop(contextp, "top");

  top_module->reset = 1;
  int n = 20;
  while (n--) {
    top_module->clock = 0; do_eval();
    top_module->clock = 1; do_eval();
  }
  top_module->reset = 0;
}

int cpu_step() {
  cpu_state = ST_RUNNING;
  while (cpu_state == ST_RUNNING) {
    do_cycle();
  }
  switch (cpu_state) {
    case ST_STOP: return 0;
    case ST_HALT: return 1;
    case ST_ABORT: return -1;
    default: assert(0);
  }
}
