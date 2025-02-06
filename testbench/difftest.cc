#include "core.h"
#include "exec.h"
#include "difftest.h"
#include "utils.h"

#include <cstring>

static Core *ref = nullptr;

static const char *gpr_name[] = {
  "zero(x 0)", "ra(x 1)", "sp(x 2)", "gp(x 3)", "tp(x 4)", "t0(x 5)", "t1(x 6)", "t2(x 7)",
  "s0/fp(x 8)", "s1(x 9)", "a0(x10)", "a1(x11)", "a2(x12)", "a3(x13)", "a4(x14)", "a5(x15)",
  "a6(x16)", "a7(x17)", "s2(x18)", "s3(x19)", "s4(x20)", "s5(x21)", "s6(x22)", "s7(x23)",
  "s8(x24)", "s9(x25)", "s10(x26)", "s11(x27)", "t3(x28)", "t4(x29)", "t5(x30)", "t6(x31)"
};

static void diff_line(const char *name, const uint32_t ref, const uint32_t dut) {
  if (ref == dut) {
    _Log("%10s " ANSI_FMT("0x%08x 0x%08x\n", ANSI_FG_YELLOW), name, ref, dut);
  } else {
    _Log("%10s " ANSI_FMT("0x%08x 0x%08x\n", ANSI_FG_RED), name, ref, dut);
  }
}

static void difftest_fail(const difftest_ctx_t *ctx_ref, const difftest_ctx_t *ctx_dut) {
  Log("Difftest failed, state diff as follows:");
  _Log("%-10s %-10s %-10s\n", "", "  ref", "  dut");
  diff_line("inst", ctx_ref->inst, ctx_dut->inst);
  diff_line("pc", ctx_ref->pc, ctx_dut->pc);
  for (int i = 0; i < gpr_n; i++) {
    diff_line(gpr_name[i], ctx_ref->gpr[i], ctx_dut->gpr[i]);
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
