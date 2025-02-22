#include "isr.h"
#include <userland/drivers/board.h>
#include <userland/drivers/display.h>

extern "C" {
extern const void * _process_stack_start;
typedef void (*ISR)(void);
}

ISR InitialisationVector[] __attribute__((section(".isr_vector_table"), used)) = {
  reinterpret_cast<ISR>(&_process_stack_start), // Stack start
  start, // Reset service routine,
  reinterpret_cast<ISR>(Ion::Device::Board::heapRange),
  reinterpret_cast<ISR>(Ion::Display::drawString)
};
