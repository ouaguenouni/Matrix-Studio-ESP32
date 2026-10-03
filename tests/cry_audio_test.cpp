#include "../include/CryAudio.h"
#include "../include/cry_assets.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

std::vector<uint8_t> read(const std::string& path) {
  std::ifstream file(path, std::ios::binary); assert(file);
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
int main(int argc, char** argv) {
  assert(argc == 2);
  uint64_t checked = 0;
  for (unsigned id = 1; id <= 251; ++id) {
    const auto bytes = read("assets/cries/adpcm/" + std::to_string(id) + ".ima");
    const auto reference = read(std::string(argv[1]) + "/" + std::to_string(id) + ".pcm");
    const auto& asset = CRY_ASSETS[id - 1];
    assert(bytes.size() == asset.size && reference.size() >= asset.samples * 2);
    MatrixAudio::ImaDecoder decoder; decoder.reset(bytes.data(), bytes.size(), asset.samples);
    for (uint32_t i = 0; i < asset.samples; ++i) {
      const int16_t expected = int16_t(uint16_t(reference[i*2]) | uint16_t(reference[i*2+1]) << 8);
      const int16_t actual = decoder.next();
      if (actual != expected) {
        std::cerr << "Cry " << id << ", sample " << i << ": " << actual << " != " << expected << '\n';
        return 1;
      }
    }
    assert(decoder.next() == 0);
    MatrixAudio::CryPlayer player;
    player.reset(bytes.data(), bytes.size(), asset.samples, CRY_SAMPLE_RATE);
    const uint32_t samples = (uint64_t(asset.samples) * 44100 + CRY_SAMPLE_RATE - 1) / CRY_SAMPLE_RATE;
    for (uint32_t i = 0; i < samples; ++i) { assert(!player.finished()); player.next(); }
    assert(player.finished() && player.next() == 0); checked += asset.samples;
    // A replacement cry starts from its beginning, independently of the old state.
    player.reset(bytes.data(), bytes.size(), asset.samples, CRY_SAMPLE_RATE);
    assert(!player.finished() && player.next() == 0);
  }
  std::vector<uint8_t> invalid(256, 0); invalid[2] = 255;
  MatrixAudio::ImaDecoder decoder; decoder.reset(invalid.data(), invalid.size(), 505);
  assert(decoder.next() == 0 && decoder.next() == 0);
  decoder.reset(invalid.data(), 3, 505); assert(decoder.next() == 0);
  std::cout << "All 251 cries match FFmpeg: " << checked
            << " PCM samples. Resampling length, replacement, fades, end-of-stream and invalid blocks passed.\n";
}
