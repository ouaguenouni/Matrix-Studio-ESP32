#pragma once
#include <stddef.h>
#include <stdint.h>

namespace MatrixAudio {
static constexpr int8_t IMA_INDEX[8] = {-1,-1,-1,-1,2,4,6,8};
static constexpr int16_t IMA_STEPS[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,
    73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,
    408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,
    1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,
    5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,
    18500,20350,22385,24623,27086,29794,32767
  };

// Mono WAV IMA ADPCM: 256-byte independent blocks, low nibble first.
// Reads immutable flash directly; no filesystem, allocation or decoder task.
class ImaDecoder {
 public:
  void reset(const uint8_t* data, size_t size, uint32_t samples) {
    data_ = data; size_ = size; left_ = samples; position_ = 0; high_ = false;
  }
  int16_t next() {
    if (!left_) return 0;
    if (position_ % 256 == 0) {
      if (position_ + 256 > size_ || data_[position_ + 2] > 88) {
        left_ = 0; return 0;
      }
      predictor_ = int16_t(uint16_t(data_[position_]) | uint16_t(data_[position_ + 1]) << 8);
      index_ = data_[position_ + 2]; position_ += 4; high_ = false; --left_;
      return int16_t(predictor_);
    }
    const uint8_t code = high_ ? data_[position_++] >> 4 : data_[position_] & 15;
    high_ = !high_;
    const int step = IMA_STEPS[index_];
    int difference = step >> 3;
    if (code & 1) difference += step >> 2;
    if (code & 2) difference += step >> 1;
    if (code & 4) difference += step;
    predictor_ += (code & 8) ? -difference : difference;
    if (predictor_ > 32767) predictor_ = 32767;
    if (predictor_ < -32768) predictor_ = -32768;
    index_ += IMA_INDEX[code & 7];
    if (index_ < 0) index_ = 0;
    if (index_ > 88) index_ = 88;
    --left_; return int16_t(predictor_);
  }
 private:
  const uint8_t* data_ = nullptr;
  size_t size_ = 0, position_ = 0;
  uint32_t left_ = 0;
  int predictor_ = 0, index_ = 0;
  bool high_ = false;

};

// Linear interpolation preserves the cry's pitch in A2DP's 44.1 kHz PCM.
class CryPlayer {
 public:
  static constexpr uint32_t OUTPUT_RATE = 44100;
  void reset(const uint8_t* data, size_t size, uint32_t samples, uint32_t sourceRate) {
    decoder_.reset(data, size, samples); sourceRate_ = sourceRate; phase_ = 0;
    total_ = sourceRate ? (uint64_t(samples) * OUTPUT_RATE + sourceRate - 1) / sourceRate : 0;
    left_ = total_; a_ = decoder_.next(); b_ = decoder_.next();
  }
  int16_t next() {
    if (!left_) return 0;
    int32_t sample = a_ + int64_t(int32_t(b_) - a_) * phase_ / OUTPUT_RATE;
    // Short fades avoid clicks when switching sprites or stopping playback.
    const uint32_t elapsed = total_ - left_;
    const uint32_t edge = elapsed < left_ ? elapsed : left_;
    if (edge < 220) sample = sample * int32_t(edge) / 220;
    --left_; phase_ += sourceRate_;
    while (phase_ >= OUTPUT_RATE) { phase_ -= OUTPUT_RATE; a_ = b_; b_ = decoder_.next(); }
    return int16_t(sample);
  }
  bool finished() const { return left_ == 0; }
 private:
  ImaDecoder decoder_;
  uint32_t phase_ = 0, sourceRate_ = 8000, left_ = 0, total_ = 0;
  int16_t a_ = 0, b_ = 0;
};
} // namespace MatrixAudio
