#ifndef __CORE_RTL_A_CPU_H__
#define __CORE_RTL_A_CPU_H__

#include "common.h"

#include "Vtop.h"
#include "Vtop_top.h"
#include "Vtop_CPU.h"
#include "Vtop_GPR.h"

#define cpu_module top_module->top->cpu
#define gpr(idx) (idx ? cpu_module->gpr->r[idx - 1] : 0)

extern Vtop *top_module;
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

#endif
