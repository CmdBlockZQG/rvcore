#include "core.h"
#include "difftest.h"

#include <memory>

static std::unique_ptr<Core> ref;

void difftest_init(const char *core_name, const char *img_file) {
  ref = std::make_unique<Core>(core_name);
  ref->init(img_file, true);
}

void difftest_step() {

}
