// Test the pinned driver's actual brightness lookup table and DMA bit selection.
#include "cie_luts.h"
#include <cassert>
#include <iostream>

static_assert(PIXEL_COLOR_DEPTH_BITS == 6, "The brightness table must match the six DMA bitplanes.");
static_assert(LUT_NATIVE_BIT_DEPTH == 1, "This test exercises the native brightness table.");

int main() {
  constexpr unsigned mask = (1u << PIXEL_COLOR_DEPTH_BITS) - 1;
  assert(lumConvTab[0] == 0);
  assert(lumConvTab[255] == mask);
  for (unsigned input = 0; input < 256; ++input) {
    const unsigned emitted = lumConvTab[input] & mask;
    assert(emitted == lumConvTab[input]); // No higher bits discarded by DMA.
    if (input) assert(emitted >= (lumConvTab[input - 1] & mask));
  }
  // A warm off-white must retain its channel ordering. The former eight-bit
  // table feeding six bitplanes emitted (26,4,47), turning it purple.
  assert(lumConvTab[240] > lumConvTab[230]);
  assert(lumConvTab[230] > lumConvTab[220]);
  std::cout << "Panel colours: matching LUT/DMA depth, monotonic shades, endpoints, and cream hue passed.\n";
}
