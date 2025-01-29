#ifndef __TESTBENCH_EXEC_H__
#define __TESTBENCH_EXEC_H__

#include "core.h"

#include <memory>

extern std::unique_ptr<Core> dut;

int cpu_exec();

#endif
