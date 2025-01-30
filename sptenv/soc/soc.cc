#include "config.h"
#include "utils.h"
#include "soc.h"

#include "device.h"
#include "ram.h"
#include "uart.h"
#include "clint.h"
#include "plic.h"

#include <cstring>

RAM   mem   {0x80000000, 0x10000000};
UART  uart  {0x10000000};
CLINT clint {0x20000000};
PLIC  plic  {0x0c000000};

static constexpr Device *devices[] = {
  &mem,
  &uart,
  &clint,
  &plic
};

bool soc_mute = false;

static void load_img(const char *filename) {
  if (filename) {
    FILE *fp = fopen(filename, "rb");
    Assert(fp, "Cannot open image file '%s'", filename);

    fseek(fp, 0, SEEK_END);
    const long size = ftell(fp);

    if (!soc_mute) Log("Image file %s, size = %ld", filename, size);

    fseek(fp, 0, SEEK_SET);
    assert(fread(mem.get_ptr(CONF_RESET_VEC), size, 1, fp) == 1);

    fclose(fp);
  } else {
    constexpr uint32_t img [] = {
      0x00500093, // addi x1, x0, 5
      0x008002ef, // jal x5, 8
      0x00608113, // addi x2, x1, 6
      0x00310093, // addi x1, x2, 3
      0x00100073, // ebreak
      0xdeadbeef  // some data
    };
    memcpy(mem.get_ptr(CONF_RESET_VEC), img, sizeof(img));
    if (!soc_mute) Log("No image is given. Use the default built-in image.");
  }
}

void init_soc(const char *imgfile, const bool mute) {
  soc_mute = mute;
  load_img(imgfile);
}

void paddr_write(const paddr_t addr, const int len, const word_t data) {
  for (Device *dev : devices) {
    if (dev->in(addr)) {
      dev->write(addr - dev->get_base(), len, data);
      return;
    }
  }
  Log("Writing invalid paddr: " FMT_PADDR, addr);
  throw 0;
}

word_t paddr_read(const paddr_t addr, const int len) {
  for (Device *dev : devices) {
    if (dev->in(addr)) {
      return dev->read(addr - dev->get_base(), len);
    }
  }
  Log("Reading invalid paddr: " FMT_PADDR, addr);
  throw 0;
}

word_t soc_get_mip() {
  return clint.get_msip() << 3 |
         clint.get_mtip() << 7 |
          plic.get_meip() << 11;
}
