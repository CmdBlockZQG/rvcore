#ifndef __SPTENV_SOC_PLIC_H__
#define __SPTENV_SOC_PLIC_H__

#include "device.h"

// TEMP: 只支持一个硬件线程和一个中断源（UART）
class PLIC final : public Device {
  uint32_t priority = 0; // UART全局中断优先级
  bool enable = false; // UART全局中断使能
  uint32_t threshold = 0; // hart0中断门槛
  bool block = false; // hart0屏蔽UART中断

  public:
    explicit PLIC(paddr_t base);
    ~PLIC() override;

    void write(paddr_t addr, int len, word_t data) override;
    word_t read(paddr_t addr, int len) override;

    [[nodiscard]] word_t get_meip() const; // hart0中断pending
};

#endif
