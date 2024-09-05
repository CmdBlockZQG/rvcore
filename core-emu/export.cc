#include "soc.h"
#include "hart.h"

Hart cpu {0};

extern "C" {

__EXPORT void init(const char *imgfile, bool mute) {
  init_soc(imgfile, mute);
}

__EXPORT int step() {
  return cpu.step();
}

}
