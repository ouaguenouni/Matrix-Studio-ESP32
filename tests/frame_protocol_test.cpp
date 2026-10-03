#include "../include/FrameProtocol.h"
#include <cassert>
#include <vector>
#include <iostream>
using namespace MatrixProtocol;
int main() {
  FrameReceiver frame;
  std::vector<uint8_t> bytes(FRAME_BYTES, 0);
  bytes[0] = 0x00; bytes[1] = 0xF8; // red, little endian
  bytes[2] = 0xE0; bytes[3] = 0x07; // green
  bytes[4] = 0x1F; bytes[5] = 0x00; // blue
  assert(!frame.valid());
  for (size_t chunk : {size_t(1), size_t(37), size_t(1436), FRAME_BYTES}) {
    frame.start();
    for (size_t p = 0; p < bytes.size();) {
      size_t n = std::min(chunk, bytes.size() - p);
      frame.append(bytes.data() + p, n); p += n;
    }
    frame.finish(); assert(frame.valid());
    assert(frame.pixel(0) == 0xF800 && frame.pixel(1) == 0x07E0 && frame.pixel(2) == 0x001F);
    assert(frame.pixel(2047) == 0);
  }
  frame.start(); frame.append(bytes.data(), FRAME_BYTES - 1); frame.finish(); assert(!frame.valid());
  frame.start(); frame.append(bytes.data(), FRAME_BYTES); uint8_t extra=1; frame.append(&extra, 1);
  frame.finish(); assert(!frame.valid());
  frame.start(); frame.append(bytes.data(), 50); frame.reject(); frame.append(bytes.data(), FRAME_BYTES);
  frame.finish(); assert(!frame.valid());
  frame.start(); frame.finish(); assert(!frame.valid());
  frame.start(); frame.append(bytes.data(), FRAME_BYTES); frame.finish(); assert(frame.valid());
  std::cout << "Frame protocol: chunking, byte order, bounds, abort, and recovery passed.\n";
}
