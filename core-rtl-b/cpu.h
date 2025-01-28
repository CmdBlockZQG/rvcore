#ifndef CORE_RTL_B_CPU_H_
#define CORE_RTL_B_CPU_H_

#include "common.h"

#include "VTop.h"

#define gpr(idx) (idx ? top_module->debugIO_gpr_ ## idx : 0)

extern VTop *top_module;
extern vaddr_t cpu_pc;

enum {
  ST_RUNNING,
  ST_STOP,
  ST_HALT,
  ST_ABORT
};
extern int cpu_state;

void cpu_init();
int cpu_step();
void cpu_exit();

#endif
