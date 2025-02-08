#include "common.h"
#include "utils.h"
#include "cpu.h"

#include <verilated_vcd_c.h>
#include <string>

VTop *top_module;
static VerilatedContext *contextp;
static VerilatedVcdC *wave = nullptr;

static long tot_cycle = 0;
static long tot_inst = 0;

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
  tot_cycle++;
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
  tot_inst++;
  while (true) {
    do_cycle();
    if (top_module->debugIO_ebreak) { // ebreak指令提交
      return gpr(10) ? CORE_ACT_BAD_TRAP : CORE_ACT_GOOD_TRAP;
    }
    if (top_module->debugIO_commit) { // 一般指令提交
      return top_module->debugIO_skip ? CORE_ACT_SKIP : CORE_ACT_NONE;
    }
  }
}

void cpu_exit() {
  if (wave) wave->close();
  delete top_module;
  delete contextp;
}

void cpu_stat() {
  Log("Total cycles: %ld", tot_cycle);
  Log("Total instructions: %ld", tot_inst);
  Log("Total IPC: %.2f", static_cast<double>(tot_inst) / tot_cycle);
}
