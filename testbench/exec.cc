#include "common.h"
#include "utils.h"

#include "exec.h"
#include "difftest.h"
#include "lightsss.h"

Core *dut;

static int lightsss_flag = 1;

int cpu_exec() {
  int ret;
  for (long i = 0; ; ++i) {
    // LightSSS
    if constexpr (ISDEF(CONF_LIGHTSSS)) {
      if (lightsss_flag && i % LIGHTSSS_INTERVAL == 0) {
        if (lightsss_fork() == 0) { // 父进程fork快照
          // 子进程
          lightsss_flag = 0;
          dut->trace_start(); // 打开trace
        }
      }
    }
    // do
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
  dut->core_stat();
  difftest_exit();

  if constexpr (ISDEF(CONF_LIGHTSSS)) {
    if (ret == CORE_ACT_GOOD_TRAP) {
      if (lightsss_flag) lightsss_exit(); // 回收所有子进程
      // 仿真正常结束时不会唤醒子进程
      // 子进程不应执行到这里
      assert(lightsss_flag);
      return 0;
    }
    // 仿真出错
    if (lightsss_flag) { // 父进程
      lightsss_wake(); // 唤醒子进程
    }
    return 1;
  } else {
    return ret != CORE_ACT_GOOD_TRAP;
  }
}
