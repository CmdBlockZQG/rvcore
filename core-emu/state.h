#ifndef __CORE_EMU_STATE_H__
#define __CORE_EMU_STATE_H__

#include "common.h"

// 硬件线程特权级
enum : word_t {
  PRIV_U = 0, // User
  PRIV_S = 1, // Supervisior
  PRIV_M = 3  // Machine
};

// 硬件线程CSR
class HartCSR {
  public:
    word_t mstatus;

    word_t mhartid;
    word_t mtvec, mepc, mcause = 0, mtval;
    word_t mie = 0, mip = 0;
    word_t medeleg = 0, mideleg = 0, medelegh = 0;

    word_t satp = 0;
    word_t mscratch, sscratch;

    word_t stvec, sepc, scause, stval;

    uint64_t counter = 0; // time cycle instret

    HartCSR(int hart_id);
};

// 硬件线程状态
class HartState {
  private:
    vaddr_t pc;
    word_t gpr[MUXDEF(CONF_RVE, 16, 32)];

    word_t &addr_csr(word_t addr);
  
  public:
    HartState(int hart_id);
    ~HartState();

    HartCSR csr;
    word_t priv;

    vaddr_t get_pc();
    void set_pc(vaddr_t dnpc);

    word_t gpr_read(int id);
    void gpr_write(int id, word_t data);

    word_t csr_read(word_t addr);
    void csr_write(word_t addr, word_t data);
};

#endif
