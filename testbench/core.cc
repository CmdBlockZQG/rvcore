#include "utils.h"
#include "core.h"

#include <cstring>
#include <dlfcn.h>

#define LOAD_FUNC(name) \
  name = reinterpret_cast<decltype(name)>(dlsym(handle, #name)); \
  assert(name);

#define MAP_FUNCS(_) \
  _(core_init) _(core_step) _(core_exit) _(core_stat) \
  _(trace_start) _(difftest_get) _(difftest_set)

Core::Core(const char *name) {
  char so_filename[64] = "build/libcore-";
  strcat(so_filename, name);
  strcat(so_filename, ".so");
  void *handle = dlopen(so_filename, RTLD_LAZY);
  Assert(handle, "Failed to open so file '%s'", so_filename);

  MAP_FUNCS(LOAD_FUNC)
}
