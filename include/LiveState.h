#pragma once
#include <cstdint>
#include <cstring>
namespace LiveState {
static const char* const palettes[] = {"classic", "amber", "neon", "ocean", "mint", "sunset"};
static const char* const motions[] = {"off", "subtle", "full"};
static const char* const modes[] = {"off", "clock", "weather", "youtube"};
static const char* const layouts[3][2] = {{"large", "date"}, {"icon", "detail"}, {"rotate", "subscribers"}};
struct Appearance { int8_t palette = -1; uint8_t layout = 0, motion = 1; };
inline int find(const char* value, const char* const* values, int size) {
  for (int i = 0; i < size; ++i) if (!strcmp(value, values[i])) return i;
  return -1;
}
inline uint8_t migrateTheme(uint8_t old) { return old < 3 ? old : 2; }
inline uint8_t restoredMode(bool chosen, uint8_t saved) { return chosen && saved <= 3 ? saved : 1; }
inline uint8_t effective(uint8_t global, const Appearance& a) { return a.palette < 0 ? global : uint8_t(a.palette); }
struct Service {
  bool valid = false, loading = false, attempted = false, failed = false, force = true;
  uint32_t generation = 0, successAt = 0, attemptAt = 0;
  char error[120] = {};
  bool due(uint32_t now) const { return !loading && (force || !attempted || uint32_t(now - attemptAt) >= (failed ? 60000U : 600000U)); }
  void invalidate(bool clear = false) { ++generation; force = true; if (clear) { valid = false; failed = false; error[0] = 0; } }
  void start(uint32_t now) { loading = true; force = false; attempted = true; attemptAt = now; }
  bool finish(uint32_t token, bool ok, uint32_t now, const char* message) {
    loading = false;
    if (token != generation) return false;
    failed = !ok; attemptAt = now;
    if (ok) { valid = true; successAt = now; error[0] = 0; }
    else { strncpy(error, message, sizeof(error)-1); error[sizeof(error)-1] = 0; }
    return true;
  }
  const char* state(bool online, bool configured, uint32_t now) const {
    if (!configured) return "unconfigured";
    if (valid) return (!online || failed || uint32_t(now - successAt) >= 600000U) ? "stale" : "ready";
    if (!online || failed) return "error";
    return "loading";
  }
};
}
