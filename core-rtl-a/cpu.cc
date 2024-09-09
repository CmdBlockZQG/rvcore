#include "cpu.h"

#include <verilated_vcd_c.h>
#include <string>

Vtop *top_module;

int cpu_state = ST_RUNNING;
vaddr_t cpu_pc;

static VerilatedContext *contextp;
static VerilatedVcdC *wave = nullptr;

static void init_wave() {
  extern std::string log_dir;
  std::string wave_filename = log_dir + "rtl-a-wave.vcd";
  Verilated::traceEverOn(true);
  wave = new VerilatedVcdC;
  top_module->trace(wave, 99);
  wave->open(wave_filename.c_str());

  Log("Wave is dumped to %s", wave_filename.c_str());
}

static void do_eval() {
  top_module->eval();
  contextp->timeInc(1);
  if (wave) wave->dump(contextp->time());
}

static void do_cycle() {
  top_module->clock = 0; do_eval();
  top_module->clock = 1; do_eval();
}

void cpu_init() {
  contextp = new VerilatedContext;
  contextp->commandArgs(0, static_cast<char **>(nullptr));
  top_module = new Vtop(contextp, "top");

  if constexpr (ISDEF(CONF_WAVE)) {
    init_wave();
  }

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

void cpu_exit() {
  if (wave) wave->close();
  delete top_module;
  delete contextp;
  // 输出性能计数器值到log
  void log_perf_stat();
  log_perf_stat();
}
