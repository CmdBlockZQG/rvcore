#include "soc.h"
#include "cpu.h"
#include "difftest.h"

static Core *cpu = nullptr;

extern "C" {

  __EXPORT void core_init(const char *imgfile, bool mute) {
    init_soc(imgfile, mute);
    cpu = new Core();
  }

  __EXPORT int core_step() {
    return cpu->do_step();
  }

  __EXPORT void core_exit() {
    delete cpu;
  }

  __EXPORT void core_stat() {
    cpu->log_stat();
  }

  __EXPORT void trace_start() {
    cpu->init_wave();
  }

  __EXPORT void difftest_get(difftest_ctx_t *ctx) {
    assert(cpu);
    ctx->pc = cpu->pc;
    for (int i = 0; i < 32; ++i) {
      ctx->gpr[i] = cpu->gpr(i);
    }
  }

  __EXPORT void difftest_set(const difftest_ctx_t *ctx) {
    assert(0); // not implemented
  }

}
