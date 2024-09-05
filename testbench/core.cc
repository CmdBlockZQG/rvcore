#include "core.h"

#include <cstring>
#include <dlfcn.h>

Core::Core(const char *name) {
  char so_filename[64] = "build/libcore-";
  strcat(so_filename, name);
  strcat(so_filename, ".so");
  void *handle = dlopen(so_filename, RTLD_LAZY);
  Assert(handle, "Failed to open so file '%s'", so_filename);

  init = reinterpret_cast<decltype(init)>(dlsym(handle, "init"));
  assert(init);
  step = reinterpret_cast<decltype(step)>(dlsym(handle, "step"));
  assert(step);
}
