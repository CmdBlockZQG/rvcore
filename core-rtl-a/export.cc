#include "soc.h"
#include "cpu.h"

extern "C" {

__EXPORT void init(const char *imgfile, bool mute) {
  init_soc(imgfile, mute);
  cpu_init();
}

__EXPORT int step() {
  return cpu_step();
}

}
