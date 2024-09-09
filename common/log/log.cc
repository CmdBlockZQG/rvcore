#include "utils.h"
#include "macro.h"

#include <cstdio>
#include <string>

__EXPORT FILE *log_fp = nullptr;
__EXPORT std::string log_dir = "build/";

__EXPORT void init_log(const char *dir) {
  if (!dir) return;
  log_dir = std::string(dir);
  if (log_dir.back() != '/') log_dir += '/';
  std::string log_filename = log_dir + "rvcore-log.txt";
  const char *filename = log_filename.c_str();
  log_fp = fopen(filename, "w");
  Assert(log_fp, "Cannot open log file '%s'", filename);
  Log("Log is written to %s", filename);
}
