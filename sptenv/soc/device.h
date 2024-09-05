#ifndef __SPTENV_SOC_DEVICE_H__
#define __SPTENV_SOC_DEVICE_H__

#include "common.h"

class Device {
  protected:
    paddr_t base;
    paddr_t size;
  
  public:
    Device(paddr_t base, paddr_t size);
    virtual ~Device() = 0;

    bool in(paddr_t addr);
    paddr_t get_base();

    virtual void write(paddr_t addr, int len, word_t data) = 0;
    virtual word_t read(paddr_t addr, int len) = 0;
};

#endif
