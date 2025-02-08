#ifndef __TESTBENCH_CORE_H__
#define __TESTBENCH_CORE_H__

#include "difftest.h"

class Core {
  public:
    void (*core_init)(const char *img_file, bool mute);
    int (*core_step)();
    void (*core_exit)();
    void (*core_stat)();

    void (*difftest_get)(difftest_ctx_t *ctx);
    void (*difftest_set)(const difftest_ctx_t *ctx);

    explicit Core(const char *name);
};

#endif
