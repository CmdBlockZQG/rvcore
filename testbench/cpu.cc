#include "cpu.h"

std::unique_ptr<Core> dut;

int cpu_exec() {
  int ret;
  while (true) {
    try {
      ret = dut->step();
    } catch (...) {
      Log("rvcore " ANSI_FMT("ABORT", ANSI_FG_RED));
      ret = -1;
      break;
    }
    if (ret == 0) continue;
    if (ret > 0) {
      Log("rvcore " ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN));
      ret = 0;
    } else {
      Log("rvcore " ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED));
      ret = 1;
    }
    break;
  }
  dut->exit();
  return ret;
}
