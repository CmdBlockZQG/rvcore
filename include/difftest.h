#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include "common.h"

struct DifftestCtx {
  word_t gpr[32];
  vaddr_t pc;
};

#endif
