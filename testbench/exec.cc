#include "common.h"
#include "utils.h"

#include "exec.h"
#include "difftest.h"

Core *dut;

int cpu_exec() {
  int ret;
  while (true) {
    try {
      ret = difftest_step();
    } catch (...) {
      ret = CORE_ACT_ABORT;
    }
    switch (ret) {
      case CORE_ACT_NONE:
      case CORE_ACT_SKIP: continue;
      case CORE_ACT_ABORT: {
        Log("rvcore " ANSI_FMT("ABORT", ANSI_FG_RED));
        break;
      }
      case CORE_ACT_GOOD_TRAP: {
        Log("rvcore " ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN));
        break;
      }
      case CORE_ACT_BAD_TRAP: {
        Log("rvcore " ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED));
        break;
      }
      default: assert(0);
    }
    break;
  }
  dut->core_exit();
  difftest_exit();
  return ret != CORE_ACT_GOOD_TRAP;
}
