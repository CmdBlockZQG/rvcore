#ifndef __CORE_EMU_HART_H__
#define __CORE_EMU_HART_H__

#include "common.h"

#include "state.h"

// 硬件线程
class Hart : private HartState {
  private:
    paddr_t mmu_translate(vaddr_t vaddr, int acs);
    void vaddr_store(vaddr_t vaddr, int len, word_t data);
    word_t vaddr_load(vaddr_t vaddr, int len);
  
    word_t inst_fetch();
    void do_inst();
    void mtrap(word_t mcause, word_t mtval);
    void strap(word_t scause, word_t stval);

  public:
    Hart(int id);
    ~Hart();

    int step();
};

#endif
