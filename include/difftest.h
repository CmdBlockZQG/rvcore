#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include "common.h"

struct difftest_ctx_t {
  word_t gpr[32];
  vaddr_t pc;
};

void difftest_init(const char *core_name, const char *img_file);
int difftest_step();
void difftest_exit();

#endif
