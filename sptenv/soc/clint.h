#ifndef __SPTENV_SOC_CLINT_H__
#define __SPTENV_SOC_CLINT_H__

#include "device.h"

// TEMP: support only 1 hart
class CLINT final : public Device {
  uint64_t mtime_base;
    [[nodiscard]] uint64_t get_mtime() const;

    uint64_t mtimecmp = 0;
    bool msip = false;

  public:
    explicit CLINT(paddr_t base);
    ~CLINT() override;

    void write(paddr_t addr, int len, word_t data) override;
    word_t read(paddr_t addr, int len) override;

    [[nodiscard]] word_t get_msip() const;
    [[nodiscard]] word_t get_mtip() const;
};

#endif
