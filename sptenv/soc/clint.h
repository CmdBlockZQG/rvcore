#ifndef __SPTENV_SOC_CLINT_H__
#define __SPTENV_SOC_CLINT_H__

#include "device.h"

// TEMP: support only 1 hart
class CLINT final : public Device {
  private:
    uint64_t mtime_base;
    uint64_t get_mtime();

    uint64_t mtimecmp = 0;
    bool msip = false;

  public:
    CLINT(paddr_t base);
    virtual ~CLINT() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;

    word_t get_msip();
    word_t get_mtip();
};

#endif
