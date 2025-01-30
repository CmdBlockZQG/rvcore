#include "core.h"
#include "exec.h"
#include "difftest.h"
#include "utils.h"

#include <cstring>

static Core *ref = nullptr;

static void difftest_fail(const difftest_ctx_t *ctx_ref, const difftest_ctx_t *ctx_dut) {
  Log("Difftest failed, state diff as follows:");
  _Log("    %-10s %-10s\n", "  ref", "  dut");
  if (ctx_ref->pc == ctx_dut->pc) {
    _Log("pc  " ANSI_FMT("0x%08x 0x%08x\n", ANSI_FG_YELLOW), ctx_ref->pc, ctx_dut->pc);
  } else {
    _Log("pc  " ANSI_FMT("0x%08x 0x%08x\n", ANSI_FG_RED), ctx_ref->pc, ctx_dut->pc);
  }
  for (int i = 0; i < gpr_n; i++) {
    if (ctx_ref->gpr[i] == ctx_dut->gpr[i]) {
      _Log("x%-2d " ANSI_FMT("0x%08x 0x%08x\n", ANSI_FG_YELLOW), i, ctx_ref->gpr[i], ctx_dut->gpr[i]);
    } else {
      _Log("x%-2d " ANSI_FMT("0x%08x 0x%08x\n", ANSI_FG_RED), i, ctx_ref->gpr[i], ctx_dut->gpr[i]);
    }

  }
}

void difftest_init(const char *core_name, const char *img_file) {
  ref = new Core(core_name);
  ref->core_init(img_file, true);
  difftest_ctx_t ctx{};
  dut->difftest_get(&ctx);
  ref->difftest_set(&ctx);

  Log("Difftest ref: " ANSI_FMT("%s", ANSI_FG_MAGENTA), core_name);
}

int difftest_step() {
  const int ret = dut->core_step();
  if (!ref) return ret;

  difftest_ctx_t ref_ctx{}, dut_ctx{};
  if (ret == CORE_ACT_SKIP) {
    dut->difftest_get(&dut_ctx);
    ref->difftest_set(&dut_ctx);
    return CORE_ACT_NONE;
  }

  if (const int t = ref->core_step(); ret != t) {
    Log("Unexpected act %d from dut, expect %d", ret, t);
    return CORE_ACT_ABORT;
  }

  if (ret != CORE_ACT_NONE) return ret;
  assert(ret == CORE_ACT_NONE);

  dut->difftest_get(&dut_ctx);
  ref->difftest_get(&ref_ctx);
  if (memcmp(&ref_ctx, &dut_ctx, sizeof(difftest_ctx_t)) != 0) {
    difftest_fail(&ref_ctx, &dut_ctx);
    return CORE_ACT_ABORT;
  }
  return CORE_ACT_NONE;
}

void difftest_exit() {
  if (!ref) return;
  ref->core_exit();
  delete ref;
}
