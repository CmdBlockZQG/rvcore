#ifndef __SPTENV_SOC_RAM_H__
#define __SPTENV_SOC_RAM_H__

#include "device.h"

class RAM final : public Device {
  private:
    uint8_t *ptr;
  public:
    RAM(paddr_t base, paddr_t size);
    virtual ~RAM() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;

    void *get_ptr(paddr_t addr);
};

#endif
