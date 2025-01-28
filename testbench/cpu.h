#ifndef __TESTBENCH_CPU_H__
#define __TESTBENCH_CPU_H__

#include "common.h"
#include "core.h"

#include <memory>

extern std::unique_ptr<Core> dut;

int cpu_exec();

#endif
