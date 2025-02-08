#include "soc.h"
#include "cpu.h"
#include "difftest.h"

extern "C" {

  __EXPORT void core_init(const char *imgfile, bool mute) {
    init_soc(imgfile, mute);
    cpu_init();
  }

  __EXPORT int core_step() {
    return cpu_step();
  }

  __EXPORT void core_exit() {
    cpu_exit();
  }

  __EXPORT void core_stat() {
    cpu_stat();
  }

  __EXPORT void difftest_get(difftest_ctx_t *ctx) {
    #define GET_GPR(i) ctx->gpr[i] = gpr(i);
    #define MAP_GPR(_) \
      _(0) _(1) _(2) _(3) _(4) _(5) _(6) _(7) \
      _(8) _(9) _(10) _(11) _(12) _(13) _(14) _(15) \
      _(16) _(17) _(18) _(19) _(20) _(21) _(22) _(23) \
      _(24) _(25) _(26) _(27) _(28) _(29) _(30) _(31)
    ctx->pc = top_module->debugIO_dnpc;
    ctx->inst = top_module->debugIO_inst;
    ctx->gpr[0] = 0;
    MAP_GPR(GET_GPR);
  }

  __EXPORT void difftest_set(const difftest_ctx_t *ctx) {
    assert(0); // not implemented
  }

}
