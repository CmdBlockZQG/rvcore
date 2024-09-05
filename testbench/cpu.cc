#include "cpu.h"

std::unique_ptr<Core> dut;

static enum {
  ST_RUNNING,
  ST_STOP,
  ST_END,
  ST_ABORT,
  ST_QUIT
} state;

void cpu_exec(uint64_t n) {
  switch (state) {
    case ST_END:
    case ST_ABORT:
      printf("Program execution has ended. To restart the program, exit and run again.\n");
      return;
    default: state = ST_RUNNING;
  }

  int ret;
  while (n--) {
    ret = dut->step();
    if (ret != 0) {
      state = ST_END;
      break;
    }
  }

  switch (state) {
    case ST_RUNNING:
      state = ST_STOP;
    break;
    case ST_ABORT:
      Log("rvcore " ANSI_FMT("ABORT", ANSI_FG_RED));
    break;
    case ST_END:
      if (ret > 0) {
        Log("rvemu " ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN));
      } else {
        Log("rvemu " ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED));
      }
    break;
    default: ;
  }
}
