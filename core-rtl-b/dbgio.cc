#include "common.h"
#include "utils.h"
#include "cpu.h"

#define RAT(x) (top_module->dbgOut_rat_ ## x)
#define PRF(x) (top_module->dbgOut_prf_ ## x)

#define MAP_32(_) \
  _(0) _(1) _(2) _(3) _(4) _(5) _(6) _(7) \
  _(8) _(9) _(10) _(11) _(12) _(13) _(14) _(15) \
  _(16) _(17) _(18) _(19) _(20) _(21) _(22) _(23) \
  _(24) _(25) _(26) _(27) _(28) _(29) _(30) _(31)

#define MAP_64(_) MAP_32(_) \
  _(32) _(33) _(34) _(35) _(36) _(37) _(38) _(39) \
  _(40) _(41) _(42) _(43) _(44) _(45) _(46) _(47) \
  _(48) _(49) _(50) _(51) _(52) _(53) _(54) _(55) \
  _(56) _(57) _(58) _(59) _(60) _(61) _(62) _(63)

#define COMP_RAT(i) assert(RAT(i) == rat[i]);
#define GET_PRF(i) prf[i] = PRF(i);

#define ROB_VALID(i) (top_module->dbgOut_robValid_ ## i)
#define ROB_ENTRY(i) ((RobEntry){ \
  .pc = top_module->dbgOut_robEntry_ ## i ##_dp_pc_h << 2,\
  .arf_rd = top_module->dbgOut_robEntry_ ## i ## _dp_arfRd,\
  .prf_rd = top_module->dbgOut_robEntry_ ## i ## _dp_prfRd,\
  .ebreak = static_cast<bool>(top_module->dbgOut_robEntry_ ## i ## _dp_ebreak),\
  .jmp = !static_cast<bool>(top_module->dbgOut_robEntry_ ## i ## _wb_trivial) && \
         !static_cast<bool>(top_module->dbgOut_robEntry_ ## i ## _wb_mmio),\
  .jmp_addr = top_module->dbgOut_robEntry_ ## i ## _wb_addr,\
})

int Core::do_step() {
  tot_inst++;
  if (rob.empty()) {
    // RAT比较
    MAP_32(COMP_RAT);
    // 驱动仿真
    do {
      do_cycle();
    } while (!top_module->dbgOut_commit);
    // 读取PRF
    MAP_64(GET_PRF);
    // 填装ROB
    if (ROB_VALID(0)) rob.push(ROB_ENTRY(0));
    if (ROB_VALID(1)) rob.push(ROB_ENTRY(1));
  }
  // 新一条提交的指令
  const RobEntry entry = rob.front();
  rob.pop();
  // 更新PC
  assert(pc == entry.pc);
  if (entry.jmp) {
    pc = entry.jmp_addr;
  } else {
    pc += 4;
  }
  // 更新RAT
  rat[entry.arf_rd] = entry.prf_rd;
  // 返回指令类型
  if constexpr (ISDEF(CONF_TRAP)) {
    if (entry.ebreak) { // ebreak指令提交
      return gpr(10) ? CORE_ACT_BAD_TRAP : CORE_ACT_GOOD_TRAP;
    }
  }
  // TODO: CORE_ACT_SKIP
  return CORE_ACT_NONE;
}

word_t Core::gpr(const int i) const {
  assert(0 <= i && i < 32);
  return prf[rat[i]];
}
