#ifndef __CORE_EMU_HART_H__
#define __CORE_EMU_HART_H__

#include "common.h"
#include "difftest.h"

#include "state.h"

// 硬件线程
class Hart : HartState {
  [[nodiscard]] paddr_t mmu_translate(vaddr_t vaddr, int acs) const;
  void vaddr_store(vaddr_t vaddr, int len, word_t data) const;
  [[nodiscard]] word_t vaddr_load(vaddr_t vaddr, int len) const;

  word_t inst_fetch();
  void do_inst();
  void mtrap(word_t mcause, word_t mtval);
  void strap(word_t scause, word_t stval);

  public:
    explicit Hart(int id);
    ~Hart();

    int step();
    void difftest_get(difftest_ctx_t *ctx) const;
    void difftest_set(const difftest_ctx_t *ctx);
};

#endif
