#ifndef ION_DEVICE_SHARED_DISPLAY_PRIVILEGED_H
#define ION_DEVICE_SHARED_DISPLAY_PRIVILEGED_H

#include <kandinsky/rect.h>
#include <kandinsky/color.h>
extern "C" {
#include <stddef.h>
}

namespace Ion {
namespace Device {
namespace Display {

void pushRect(KDRect r, const KDColor * pixels);
void pushRectUniform(KDRect r, KDColor c);
void pullRect(KDRect r, KDColor * pixels);
void pullRectSecure(KDRect r, KDColor * pixels);
bool waitForVBlank();

/* TODO Hugo : Implement SVC for all the methods that are exposed to the
 * userland and that are interacting directly with the screen driver :
 * waitForVBlank : Reading pins and using time, should it be implemented in SVC?
 * displayUniformTilingSize10 : Calling functions handled in SVC -> no need
 * displayColoredTilingSize10 : Calling functions handled in SVC -> no need
 * POSTPushMulticolor : Implemented in SVC, see TODO in svc.cpp :
 *     TODO Hugo : [Important]Fix crash on device when this row is uncommented
 */

// For Power On Self tests
void POSTPushMulticolor(int rootNumberTiles, int tileSize);

void init();
void shutdown();

void initDMA();
void initGPIO();
void shutdownGPIO();
void initFSMC();
void shutdownFSMC();
void initPanel();
void shutdownPanel();

enum class Orientation {
  Landscape = 0,
  Portrait = 1
};

void setDrawingArea(KDRect r, Orientation o);
void waitForPendingDMAUploadCompletion();
void pushPixels(const KDColor * pixels, size_t numberOfPixels);
void pushColor(KDColor color, size_t numberOfPixels);
void pullPixels(KDColor * pixels, size_t numberOfPixels);
uint32_t panelIdentifier();

enum class Command : uint16_t {
  Nop = 0x00,
  Reset = 0x01,
  ReadDisplayID = 0x04,
  SleepIn = 0x10,
  SleepOut = 0x11,
  DisplayInversionOff = 0x20,
  DisplayInversionOn = 0x21,
  DisplayOff = 0x28,
  DisplayOn = 0x29,
  ColumnAddressSet = 0x2A,
  PageAddressSet = 0x2B,
  MemoryWrite = 0x2C,
  MemoryRead = 0x2E,
  TearingEffectLineOn = 0x35,
  MemoryAccessControl = 0x36,
  PixelFormatSet = 0x3A,
  FrameRateControl = 0xC6,
  PositiveVoltageGammaControl = 0xE0,
  NegativeVoltageGammaControl = 0xE1
};

constexpr static int FSMCMemoryBank = 1;
constexpr static int FSMCDataCommandAddressBit = 16;

constexpr static uint32_t FSMCBaseAddress = 0x60000000;
constexpr static uint32_t FSMCBankAddress = FSMCBaseAddress + (FSMCMemoryBank-1)*0x04000000;


static volatile Command * const CommandAddress = (Command *)(FSMCBankAddress);
static volatile uint16_t * const DataAddress = (uint16_t *)(FSMCBankAddress | (1<<(FSMCDataCommandAddressBit+1)));

// For Power On Self tests
void pushColorAndContraryPixels(uint16_t color, int count);
int displayUniformTilingSize10(KDColor c);

}
}
}

#endif

