#include "soc.h"
#include "hart.h"
#include "difftest.h"

Hart cpu {0};

extern "C" {

  __EXPORT void core_init(const char *imgfile, bool mute) {
    init_soc(imgfile, mute);
  }

  __EXPORT int core_step() {
    return cpu.step();
  }

  __EXPORT void core_exit() {
  }

  __EXPORT void core_stat() {
  }

  __EXPORT void difftest_get(difftest_ctx_t *ctx) {
    cpu.difftest_get(ctx);
  }

  __EXPORT void difftest_set(const difftest_ctx_t *ctx) {
    cpu.difftest_set(ctx);
  }

}
