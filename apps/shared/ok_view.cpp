#include "ok_view.h"

namespace Shared {

const uint8_t okMask[OkView::k_okSize][OkView::k_okSize] = {
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xA9, 0x59, 0x20, 0x06, 0x0C, 0x28, 0x59, 0xAA, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xAE, 0x24, 0x41, 0x97, 0xD8, 0xF5, 0xF5, 0xD8, 0x97, 0x41, 0x24, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFC, 0x76, 0x27, 0xBC, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xBC, 0x27, 0x76, 0xFC, 0xFF, 0xFF},
  {0xFF, 0xFF, 0x76, 0x3D, 0xED, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xEE, 0x3E, 0x76, 0xFF, 0xFF},
  {0xFF, 0xAB, 0x26, 0xEC, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xEC, 0x27, 0xAB, 0xFF},
  {0xF9, 0x1D, 0xC1, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xC2, 0x1C, 0xF9},
  {0xA4, 0x43, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0x43, 0xA3},
  {0x54, 0x98, 0xFB, 0xFB, 0xFB, 0xEC, 0x4C, 0x06, 0x3E, 0xE1, 0xFB, 0x00, 0xFB, 0xE6, 0x00, 0xFB, 0xFB, 0xFB, 0x98, 0x53},
  {0x1F, 0xD8, 0xFB, 0xFB, 0xFB, 0x5D, 0x64, 0xEF, 0x73, 0x53, 0xFB, 0x00, 0xE6, 0x2A, 0x7D, 0xFB, 0xFB, 0xFB, 0xD9, 0x1F},
  {0x07, 0xF5, 0xFB, 0xFB, 0xFB, 0x0F, 0xE9, 0xFB, 0xE8, 0x0C, 0xFB, 0x00, 0x2A, 0x69, 0xFA, 0xFB, 0xFB, 0xFB, 0xF5, 0x06},
  {0x0D, 0xEE, 0xFB, 0xFB, 0xFB, 0x0F, 0xE9, 0xFB, 0xE8, 0x0C, 0xFB, 0x00, 0x7D, 0x29, 0xC4, 0xFB, 0xFB, 0xFB, 0xF5, 0x06},
  {0x26, 0xD4, 0xFB, 0xFB, 0xFB, 0x5E, 0x62, 0xEE, 0x73, 0x53, 0xFB, 0x00, 0xFB, 0x7E, 0x6F, 0xFB, 0xFB, 0xFB, 0xD9, 0x1F},
  {0x54, 0x98, 0xFB, 0xFB, 0xFB, 0xEC, 0x4D, 0x07, 0x3F, 0xE2, 0xFB, 0x00, 0xFB, 0xD2, 0x00, 0xFB, 0xFB, 0xFB, 0x98, 0x53},
  {0xA4, 0x43, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0x43, 0xA3},
  {0xF9, 0x1D, 0xC2, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xC3, 0x1C, 0xF8},
  {0xFF, 0xAA, 0x28, 0xEC, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xEC, 0x27, 0xAA, 0xFF},
  {0xFF, 0xFF, 0x75, 0x3E, 0xEE, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xEE, 0x3E, 0x75, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFC, 0x6C, 0x27, 0xBC, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xBC, 0x27, 0x6B, 0xFC, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xAC, 0x24, 0x43, 0x98, 0xDA, 0xF6, 0xF0, 0xD4, 0x98, 0x43, 0x24, 0xAB, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xA4, 0x53, 0x20, 0x05, 0x05, 0x20, 0x53, 0xA4, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};

void OkView::drawRect(KDContext * ctx, KDRect rect) const {
  KDCoordinate width = bounds().width();
  KDCoordinate height =  bounds().height();
  KDRect frame((width-k_okSize)/2, (height-k_okSize)/2, k_okSize, k_okSize);
  KDColor okWorkingBuffer[OkView::k_okSize*OkView::k_okSize];
  ctx->blendRectWithMask(frame, KDColorBlack, (const uint8_t *)okMask, okWorkingBuffer);
}

KDSize OkView::minimalSizeForOptimalDisplay() const {
  return KDSize(k_okSize, k_okSize);
}

}

