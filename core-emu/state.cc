#include "common.h"

#include "state.h"

HartState::HartState(int hart_id): csr(hart_id) {
  pc = CONF_RESET_VEC;
  gpr[0] = 0;

  priv = PRIV_M;
}

HartState::~HartState() { }

vaddr_t HartState::get_pc() {
  return pc;
}

void HartState::set_pc(vaddr_t dnpc) {
  pc = dnpc;
}

word_t HartState::gpr_read(int id) {
  if constexpr (rt_check) {
    assert(0 <= id && id < gpr_n);
  }
  return id ? gpr[id] : 0;
}

void HartState::gpr_write(int id, word_t data) {
  if constexpr (rt_check) {
    assert(0 <= id && id < gpr_n);
  }
  if (id) gpr[id] = data;
}

HartCSR::HartCSR(int hart_id) {
  mhartid = hart_id;
}

word_t &HartState::addr_csr(word_t addr) {
  switch (addr & 0xfff) {
    // SRW
    case 0x105: return csr.stvec;
    case 0x140: return csr.sscratch;
    case 0x141: return csr.sepc;
    case 0x142: return csr.scause;
    case 0x143: return csr.stval;
    case 0x180: return csr.satp;
    // MRW
    case 0x300: return csr.mstatus;
    case 0x302: return csr.medeleg;
    case 0x303: return csr.mideleg;
    case 0x304: return csr.mie;
    case 0x312: return csr.medelegh;
    case 0x305: return csr.mtvec;
    case 0x340: return csr.mscratch;
    case 0x341: return csr.mepc;
    case 0x342: return csr.mcause;
    case 0x343: return csr.mtval;
    case 0x344: return csr.mip;
    // MRO
    case 0xf14: return csr.mhartid;

    default:
      Log("Warning: access non-exist CSR: 0x%03x at " FMT_VADDR, addr, get_pc());
      throw 0;
  }
}

static constexpr word_t mstatus_mask = 0x007e19aa;
static constexpr word_t sstatus_mask = 0x000c0122;
static constexpr word_t mie_mask = 0xaaa;
static constexpr word_t sie_mask = 0x222;
// WARN: xenvcfg, mseccfg, PMP未实现
// 会抛出非法指令异常

word_t HartState::csr_read(word_t addr) {
  // 检查当前特权级是否能访问CSR
  word_t csr_priv = bits<9, 8>(addr);
  if (priv < csr_priv) throw 0;

  switch (addr) {
    // sstatus
    case 0x100: return csr.mstatus & sstatus_mask;
    // sie
    case 0x104: return csr.mie & sie_mask;
    // scounteren
    case 0x106: return 0;
    // sip
    case 0x144: return csr.mip & sie_mask;
    // misa
    case 0x301: return 0x40141115; // RV32 IE MAC SU
    // mstatush
    case 0x310: return 0;
    // cycle time instret
    case 0xc00: case 0xc01: case 0xc02: return csr.counter;
    // cycleh timeh instreth
    case 0xc80: case 0xc81: case 0xc82: return csr.counter >> 32;
    // mvendorid
    case 0xf11: return 0x79737978;
    // marchid
    case 0xf12: return 0x15fdeeb;
    // mimpid
    case 0xf13: return 0;
    // mconfigptr
    case 0xF15: return 0;

    default: return addr_csr(addr);
  }
}

void HartState::csr_write(word_t addr, word_t data) {
  // 检查CSR是否只读
  word_t csr_flag = bits<11, 10>(addr);
  if (csr_flag == 0b11) throw 0;
  // 检查当前特权级是否能访问CSR
  word_t csr_priv = bits<9, 8>(addr);
  if (priv < csr_priv) throw 0;

  switch (addr) {
    // sstatus写入实际上是在写入mstatus的子集
    case 0x100:
      csr.mstatus = (csr.mstatus & ~sstatus_mask) | (data & sstatus_mask);
    break;
    // sie写入实际上是在写入mie的子集
    case 0x104:
      csr.mie = (csr.mie & ~sie_mask) | (data & sie_mask);
    break;
    // scounteren 只读0
    case 0x106: break;
    // sip只读
    case 0x144: break;
    // mstatus只支持部分字段，剩余全部硬编码0
    case 0x300: csr.mstatus = data & mstatus_mask; break;
    // mideleg只支持委托s模式中断
    case 0x303: csr.mideleg = data & sie_mask; break;
    // mie只支持除LCOFI之外的标准中断
    case 0x304: csr.mie = data & mie_mask; break;
    // mip中，S模式中断pending可写，其余只读
    case 0x344: csr.mip = data & sie_mask; break;
    // 忽略对misa的写入，不支持指令集功能选择
    case 0x301: break;
    // 忽略对mstatush的写入，仅支持小端序
    case 0x310: break;

    default: addr_csr(addr) = data;
  }
}
