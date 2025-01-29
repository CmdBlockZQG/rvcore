#ifndef __SPTENV_SOC_UART_H__
#define __SPTENV_SOC_UART_H__

#include "device.h"

class UART final : public Device {
  public:
    explicit UART(paddr_t base);
    ~UART() override;

    void write(paddr_t addr, int len, word_t data) override;
    word_t read(paddr_t addr, int len) override;

    // 是否有中断请求
    static word_t get_ip();
};

#endif
