#include "utils.h"
#include "macro.h"

#include <cstdio>

FILE *log_fp = nullptr;

__EXPORT void init_log(const char *filename) {
  if (!filename) return;
  log_fp = fopen(filename, "w");
  Assert(log_fp, "Cannot open log file '%s'", filename);
  Log("Log is written to %s", filename);
}
