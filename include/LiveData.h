#pragma once
#include <ArduinoJson.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace LiveData {
struct Weather { float temperature = 0; int code = 0; };
struct Place { float lat = 0, lon = 0; char name[96] = {}; };
struct Youtube {
  uint64_t subs = 0, views = 0, videos = 0;
  bool hidden = false;
  char title[128] = {};
};
inline bool validCode(int c) {
  return c == 0 || c == 1 || c == 2 || c == 3 || c == 45 || c == 48 ||
    c == 51 || c == 53 || c == 55 || c == 56 || c == 57 || c == 61 || c == 63 || c == 65 ||
    c == 66 || c == 67 || c == 71 || c == 73 || c == 75 || c == 77 ||
    c == 80 || c == 81 || c == 82 || c == 85 || c == 86 || c == 95 || c == 96 || c == 99;
}
inline bool weather(JsonVariantConst root, Weather& out) {
  auto current = root["current"];
  if (!current["temperature_2m"].is<float>() || !current["weather_code"].is<int>()) return false;
  Weather candidate;
  candidate.temperature = current["temperature_2m"].as<float>();
  candidate.code = current["weather_code"].as<int>();
  if (!std::isfinite(candidate.temperature) || candidate.temperature < -100 || candidate.temperature > 70 || !validCode(candidate.code)) return false;
  out = candidate;
  return true;
}
inline bool place(JsonVariantConst root, bool ip, Place& out) {
  auto value = ip ? root : root["results"][0];
  if (ip && strcmp(value["status"] | "", "success")) return false;
  auto lat = value[ip ? "lat" : "latitude"], lon = value[ip ? "lon" : "longitude"];
  const char* name = value[ip ? "city" : "name"] | "";
  if (!lat.is<float>() || !lon.is<float>() || !name[0]) return false;
  Place candidate;
  candidate.lat = lat.as<float>(); candidate.lon = lon.as<float>();
  if (!std::isfinite(candidate.lat) || !std::isfinite(candidate.lon) || fabs(candidate.lat) > 90 || fabs(candidate.lon) > 180) return false;
  snprintf(candidate.name, sizeof(candidate.name), "%s", name);
  out = candidate;
  return true;
}
inline bool count(JsonVariantConst value, uint64_t& result) {
  if (!value.is<const char*>()) return false;
  const char* text = value.as<const char*>();
  if (!*text) return false;
  uint64_t number = 0;
  for (; *text; ++text) {
    if (*text < '0' || *text > '9') return false;
    const unsigned digit = *text - '0';
    if (number > (std::numeric_limits<uint64_t>::max() - digit) / 10) return false;
    number = number * 10 + digit;
  }
  result = number; return true;
}
inline bool youtube(JsonVariantConst root, Youtube& out) {
  auto item = root["items"][0];
  auto stats = item["statistics"];
  const char* title = item["snippet"]["title"] | "";
  if (!item.is<JsonObjectConst>() || !stats.is<JsonObjectConst>() || !*title) return false;
  if (!stats["hiddenSubscriberCount"].isNull() && !stats["hiddenSubscriberCount"].is<bool>()) return false;
  Youtube candidate;
  candidate.hidden = stats["hiddenSubscriberCount"] | false;
  if ((!candidate.hidden && !count(stats["subscriberCount"], candidate.subs)) ||
      !count(stats["viewCount"], candidate.views) || !count(stats["videoCount"], candidate.videos)) return false;
  snprintf(candidate.title, sizeof(candidate.title), "%s", title);
  out = candidate; return true;
}
// Return fixed messages only: upstream messages/URLs can contain credentials.
inline const char* apiError(int code, JsonVariantConst root, bool youtubeService) {
  const char* reason = root["error"]["errors"][0]["reason"] | "";
  if (!strcmp(reason, "quotaExceeded") || !strcmp(reason, "dailyLimitExceeded") || code == 429) return "Quota reached. Try again later.";
  if (youtubeService && !strcmp(reason, "invalidParameter")) return "YouTube rejected the request parameters. Check the channel query.";
  if (youtubeService && (code == 400 || code == 401 || code == 403)) return "YouTube rejected the key. Check API access and restrictions.";
  if (code <= 0) return "Network request failed. Check Wi-Fi and internet.";
  return "Service unavailable. Retrying in one minute.";
}
}
