#include "soc.h"
#include "hart.h"

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

}
