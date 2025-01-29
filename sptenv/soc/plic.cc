#include "utils.h"
#include "plic.h"
#include "uart.h"

extern UART uart;

PLIC::PLIC(const paddr_t base): Device(base, 0x4000000) { }

PLIC::~PLIC() = default;

void PLIC::write(const paddr_t addr, const int len, const word_t data) {
  Check(len == 4);
  switch (addr) {
    // UART全局中断优先级
    case 0x4: priority = data; break;
    // UART全局中断使能
    case 0x2000: enable = data >> 1 & 1; break;
    // hart0中断门槛
    case 0x200000: threshold = data; break;
    // hart0 Completion
    case 0x200004:
      if (data == 1) {
        block = false;
      }
    break;
    default: break;
  }
}

word_t PLIC::read(const paddr_t addr, const int len) {
  Check(len == 4);
  switch (addr) {
    // UART全局中断优先级
    case 0x4: return priority;
    // UART全局中断pending
    case 0x1000: return priority && enable && UART::get_ip() ? 0b10 : 0;
    // UART全局中断使能
    case 0x2000: return enable ? 0b10 : 0;
    // hart0中断门槛
    case 0x200000: return threshold;
    // hart0 Claim
    case 0x200004: {
      if (enable && priority && UART::get_ip()) {
        block = true;
        return 1;
      }
      return 0;
    }
    default: return 0;
  }
}

word_t PLIC::get_meip() const {
  return !block && enable && priority > threshold && UART::get_ip();
}
