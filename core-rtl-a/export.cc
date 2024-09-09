#include "soc.h"
#include "cpu.h"

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

}
