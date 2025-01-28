#include "cpu.h"

#include <verilated_vcd_c.h>
#include <string>

VTop *top_module;

int cpu_state = ST_RUNNING;
vaddr_t cpu_pc;

static VerilatedContext *contextp;
static VerilatedVcdC *wave = nullptr;

static void init_wave() {
  extern std::string log_dir;
  const std::string wave_filename = log_dir + "rtl-b-wave.vcd";
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
  top_module = new VTop(contextp, "top");

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
  while (true) {
    do_cycle();
    if (top_module->debugIO_ebreak) { // ebreak指令提交
      return gpr(10) ? -1 : 1;
    }
    if (top_module->debugIO_commit) { // 一般指令提交
      cpu_pc = top_module->debugIO_pc;
      return 0;
    }
  }
}

void cpu_exit() {
  if (wave) wave->close();
  delete top_module;
  delete contextp;
}
