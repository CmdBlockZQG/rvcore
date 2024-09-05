#include "plic.h"
#include "uart.h"

extern UART uart;

PLIC::PLIC(paddr_t addr): Device(addr, 0x4000000) { }

PLIC::~PLIC() { }

void PLIC::write(paddr_t addr, int len, word_t data) {
  if constexpr (rt_check) assert(len == 4);
  switch (addr) {
    // UART全局中断优先级
    case 0x4: priority = data; break;
    // UART全局中断使能
    case 0x2000: enable = (data >> 1) & 1; break;
    // hart0中断门槛
    case 0x200000: threshold = data; break;
    // hart0 Completion
    case 0x200004:
      if (data == 1) {
        block = 0;
      }
    break;
  }
}

word_t PLIC::read(paddr_t addr, int len) {
  if constexpr (rt_check) assert(len == 4);
  switch (addr) {
    // UART全局中断优先级
    case 0x4: return priority;
    // UART全局中断pending
    case 0x1000: return priority && enable && uart.get_ip() ? 0b10 : 0;
    // UART全局中断使能
    case 0x2000: return enable ? 0b10 : 0;
    // hart0中断门槛
    case 0x200000: return threshold;
    // hart0 Claim
    case 0x200004:
      if (enable && priority && uart.get_ip()) {
        block = 1;
        return 1;
      } else {
        return 0;
      }
  }
  return 0;
}

word_t PLIC::get_meip() {
  return !block && enable && (priority > threshold) && uart.get_ip();
}
