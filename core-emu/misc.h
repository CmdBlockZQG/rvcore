#ifndef __CORE_EMU_MISC_H__
#define __CORE_EMU_MISC_H__

#include "common.h"

struct Exception {
  word_t cause;
  word_t tval;

  Exception(word_t cause, word_t tval);
};

enum : int {
  ACS_INST  = 0,
  ACS_LOAD  = 1,
  ACS_STORE = 2
};

#endif
