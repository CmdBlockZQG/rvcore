#ifndef __CORE_RTL_B_CPU_H__
#define __CORE_RTL_B_CPU_H__

#include "VTop.h"

#define gpr(idx) (idx ? top_module->debugIO_gpr_ ## idx : 0)

extern VTop *top_module;

void cpu_init();
int cpu_step();
void cpu_exit();
void cpu_stat();

#endif
