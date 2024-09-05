#ifndef __TESTBENCH_CORE_H__
#define __TESTBENCH_CORE_H__

#include "common.h"

class Core {
  public:
    void (*init)(const char *imgfile, bool mute);
    int (*step)();

    Core(const char *name);
};

#endif
