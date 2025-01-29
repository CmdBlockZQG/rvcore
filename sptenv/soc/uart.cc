#include "utils.h"
#include "uart.h"

static auto uart_input_ptr = " "
  "busybox | head -n4\n"
  "ls\n"
  "./hello\n";

static bool input_available() {
  return *uart_input_ptr != '\0';
}

UART::UART(const paddr_t base): Device(base, 0x1000) { }

UART::~UART() = default;

void UART::write(const paddr_t addr, int len, const word_t data) {
  if (addr == 0) {
    if (extern bool soc_mute; !soc_mute) {
      putchar(static_cast<char>(data & 0xff));
      fflush(stdout);
    }
  }
}

word_t UART::read(const paddr_t addr, int len) {
  if (addr == 0) {
    if (input_available()) return *uart_input_ptr++;
    return 0xff;
  }
  if (addr == 5) {
    return 0x60 | input_available();
  }
  return 0;
}

word_t UART::get_ip() {
  return input_available();
}
