#ifndef __SPTENV_SOC_H__
#define __SPTENV_SOC_H__

#include "common.h"

void init_soc(const char *imgfile, bool mute);

void paddr_write(paddr_t addr, int len, word_t data);
word_t paddr_read(paddr_t addr, int len);

word_t soc_get_mip();

#endif
