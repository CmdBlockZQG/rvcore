#ifndef __SPTENV_SOC_RAM_H__
#define __SPTENV_SOC_RAM_H__

#include "device.h"

class RAM final : public Device {
  uint8_t *ptr;
  public:
    RAM(paddr_t base, paddr_t size);
    ~RAM() override;

    void write(paddr_t addr, int len, word_t data) override;
    word_t read(paddr_t addr, int len) override;

    [[nodiscard]] void *get_ptr(paddr_t addr) const;
};

#endif
