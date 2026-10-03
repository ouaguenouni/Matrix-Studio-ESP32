#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace MatrixProtocol {
constexpr size_t WIDTH = 64;
constexpr size_t HEIGHT = 32;
constexpr size_t FRAME_BYTES = WIDTH * HEIGHT * 2;

// One complete RGB565 little-endian frame, received in bounded chunks.
class FrameReceiver {
 public:
  uint8_t bytes[FRAME_BYTES] = {};
  void start() { used_ = 0; failed_ = false; complete_ = false; }
  void reject() { failed_ = true; complete_ = false; }
  void append(const uint8_t* src, size_t size) {
    if (failed_ || complete_) { reject(); return; }
    if (size > FRAME_BYTES - used_) { reject(); return; }
    if (size) memcpy(bytes + used_, src, size);
    used_ += size;
  }
  void finish() { complete_ = !failed_ && used_ == FRAME_BYTES; }
  bool valid() const { return complete_ && !failed_; }
  uint16_t pixel(size_t index) const {
    return uint16_t(bytes[2 * index]) | (uint16_t(bytes[2 * index + 1]) << 8);
  }
 private:
  size_t used_ = 0;
  bool failed_ = true;
  bool complete_ = false;
};
}
