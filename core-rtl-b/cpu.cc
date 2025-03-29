#include "common.h"
#include "utils.h"
#include "cpu.h"

#include <string>

void Core::init_wave() {
  extern std::string log_dir;
  const std::string wave_filename = log_dir + "rtl-b-wave.vcd";
  contextp->traceEverOn(true);
  wave = new VerilatedVcdC;
  top_module->trace(wave, 99);
  wave->open(wave_filename.c_str());

  Log("Wave is dumped to %s", wave_filename.c_str());
}


void Core::do_eval() const {
  top_module->eval();
  if (wave) {
    contextp->timeInc(1);
    wave->dump(contextp->time());
  }
}

void Core::do_cycle() {
  tot_cycle++;

  top_module->clock = 0; do_eval();
  top_module->clock = 1; do_eval();
}

Core::Core() {
  contextp = new VerilatedContext;
  top_module = new VTop(contextp, "Top");

  top_module->reset = 1;
  int n = 20;
  while (n--) {
    top_module->clock = 0; do_eval();
    top_module->clock = 1; do_eval();
  }
  top_module->reset = 0;
}

Core::~Core() {
  if (wave) wave->close();
  delete top_module;
  // TODO: 在子进程中delete contextp会导致卡死
  // delete contextp;
}

void Core::log_stat() const {
  Log("Total cycles: %ld", tot_cycle);
  Log("Total instructions: %ld", tot_inst);
  Log("Total IPC: %.2f", static_cast<double>(tot_inst) / tot_cycle);
}
