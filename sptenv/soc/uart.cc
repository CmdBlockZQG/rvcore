#include "uart.h"

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

static const char *uart_input_ptr = " "
                                    "busybox | head -n4\n"
                                    "ls\n";

static inline bool input_available() {
  return *uart_input_ptr != '\0';
}

UART::UART(paddr_t addr): Device(addr, 0x1000) { }

UART::~UART() { }

void UART::write(paddr_t addr, int len, word_t data) {
  if (addr == 0) {
    extern bool soc_mute;
    if (!soc_mute) {
      putchar(data & 0xff);
      fflush(stdout);
    }
  }
}

word_t UART::read(paddr_t addr, int len) {
  if (addr == 0) {
    if (input_available()) {
      return *uart_input_ptr++;
    } else {
      return 0xff;
    }
  } else if (addr == 5) {
    return 0x60 | input_available();
  }
  return 0;
}

word_t UART::get_ip() {
  return input_available();
}
