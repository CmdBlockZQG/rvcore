#include "common.h"
#include "soc.h"
#include "hart.h"

#include "decode.h"
#include "misc.h"

Hart::Hart(int id): HartState(id) { }

Hart::~Hart() { }

paddr_t Hart::mmu_translate(vaddr_t vaddr, int acs) {
  word_t satp = csr.satp;
  // MPRV=1时，load&store使用mstatus.MPP特权级
  word_t act_priv = mstatus_MPRV && (acs != ACS_INST) ? mstatus_MPP : priv;
  if (act_priv == PRIV_M || !(satp >> 31)) return vaddr;

  // page fault在不同访问时的cause编号
  static constexpr word_t pf_cause[3] = {12, 13, 15};
  // access fault在不同访问时的cause编号
  static constexpr word_t af_cause[3] = {1, 5, 7};
  #define EXC_PF ((Exception {pf_cause[acs], vaddr}))
  #define EXC_AF ((Exception {af_cause[acs], vaddr}))

  // vpn在虚拟地址中的低位
  constexpr int vpn_off[] = {12, 22};

  paddr_t pt_addr = satp << 12;
  word_t pte;
  int i = 1;
  for (i = 1; i >= 0; --i) {
    const paddr_t vpn = (vaddr >> vpn_off[i]) & 0x3ff;
    const paddr_t pte_addr = pt_addr | (vpn << 2);
    try {
      pte = paddr_read(pte_addr, 4);
    } catch (...) {
      throw EXC_AF;
    }
    // 检查页表项是否非法
    if (!(pte & 1) || ((pte >> 1) & 0b11) == 0b10) {
      throw EXC_PF;
    }
    // 检查是否指向子页表
    if ((pte & 0b1010) == 0) {
      pt_addr = (pte << 2) & ~0x3ff;
      continue;
    }
    // 找到叶子页表项
    // 检查特权级权限
    bool flag_u = (pte >> 4) & 1; // U == 1 ?
    if (act_priv == PRIV_U) {
      if (!flag_u) throw EXC_PF; // U模式仅能访问U=1的页面
    } else if (act_priv == PRIV_S) {
      // S模式不能从U页面取指
      if (flag_u && acs == ACS_INST) throw EXC_PF;
      // 只有当SUM=1时，访问U页面合法
      if (flag_u && !mstatus_SUM) throw EXC_PF;
    } else assert(0);
    // 检查访问模式权限
    bool pf = true;
    switch (acs) {
      case ACS_INST:
        if ((pte >> 3) & 1) pf = false;
      break;
      case ACS_LOAD:
        if ((pte >> 1) & 1) pf = false;
        // MXR=1，X页面可读
        if (mstatus_MXR && ((pte >> 3) & 1)) pf = false;
      break;
      case ACS_STORE:
        if ((pte >> 2) & 1) pf = false;
      break;
      default: assert(0);
    }
    if (pf) throw EXC_PF;
    // 检查通过
    // 4MB大页的ppn[0]应该是0，否则大页就没有正确对齐
    if (i > 0 && bits<19, 10>(pte)) throw EXC_PF;
    // TEMP: 严格内存序，单hart，故更新页表AD位时不检查pte匹配情况
    // 更新页表flag
    word_t pte_up = pte | (1 << 6); // 将A位设为1
    if (acs == ACS_STORE) pte_up |= 1 << 7; // store则将D位设为1
    if (pte_up != pte) { // 更新页表项
      try {
        paddr_write(pte_addr, 4, pte_up);
      } catch (...) {
        // 读取时没有触发access fault，写入时也不应该 
        assert(0);
      }
    }
    break; // 地址翻译成功完成
  }
  if (i < 0) throw EXC_PF; // 叶子哪去了？

  paddr_t paddr;
  if (i > 0) { // 4MB大页
    paddr = ((pte << 2) & ~0xfff) | bits<21, 0>(vaddr);
  } else { // 4KB一般页
    paddr = ((pte << 2) & ~0xfff) | bits<11, 0>(vaddr);
  }
  return paddr;
}

void Hart::vaddr_store(vaddr_t vaddr, int len, word_t data) {
  if (vaddr & (len - 1)) throw Exception {6, vaddr};
  word_t paddr = mmu_translate(vaddr, ACS_STORE);
  try {
    paddr_write(paddr, len, data);
  } catch (...) {
    throw Exception {7, vaddr};
  }
}

word_t Hart::vaddr_load(vaddr_t vaddr, int len) {
  if (vaddr & (len - 1)) throw Exception {4, vaddr};
  word_t paddr = mmu_translate(vaddr, ACS_LOAD);
  word_t data;
  try {
    data = paddr_read(paddr, len);
  } catch (...) {
    throw Exception {5, vaddr};
  }
  return data;
}

word_t Hart::inst_fetch() {
  // Log_write("pc: %08x\n", get_pc());
  vaddr_t vaddr = get_pc();
  paddr_t paddr = mmu_translate(vaddr, ACS_INST);
  word_t data;
  try {
    data = paddr_read(paddr, 4);
  } catch (...) {
    throw Exception {1, vaddr};
  }
  return data;
}
