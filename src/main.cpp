// Classic ESP32-WROOM + one Waveshare 64x32 HUB75 panel.
// Build and upload with PlatformIO; the browser UI is embedded from include/.
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <esp_system.h>
#include <driver/gpio.h>
#include <rom/gpio.h>
#include <soc/gpio_sig_map.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "FrameProtocol.h"
#include "web_ui.h"

WebServer server(80);
Preferences settings;
MatrixPanel_I2S_DMA* panel = nullptr;
MatrixProtocol::FrameReceiver incomingFrame;
String hostname, apName, apPassword, savedSSID, savedPassword;
bool apRunning = false, wasConnected = false, mdnsRunning = false;
bool connecting = false, changeWiFiPending = false, receivedContent = false;
uint32_t connectStarted = 0, changeWiFiAt = 0, framesReceived = 0;
uint8_t brightness = 30;
unsigned uploadFiles = 0;
gpio_drive_cap_t clockDrive = GPIO_DRIVE_CAP_0;
bool runtimeClockPhase = false;

enum SignalDriveMode : uint8_t { SIGNALS_DEFAULT, CONTROL_SOFT, ALL_SOFT };
SignalDriveMode signalDriveMode = SIGNALS_DEFAULT;

struct TimingProfile {
  char id;
  bool phase;
  uint8_t blanking;
  gpio_drive_cap_t drive;
  uint8_t divider;
  SignalDriveMode signals;
};

// Classic ESP32, pinned HUB75 library: 80 MHz / (divider * 2).
// Divider 4 is its normal 10 MHz output; divider 8 gives a 5 MHz trial.
const TimingProfile timingProfiles[] = {
  {'A', false, 4, GPIO_DRIVE_CAP_2, 4, SIGNALS_DEFAULT},
  {'B', false, 4, GPIO_DRIVE_CAP_2, 8, SIGNALS_DEFAULT},
  {'C', true,  4, GPIO_DRIVE_CAP_2, 4, SIGNALS_DEFAULT},
  {'D', true,  4, GPIO_DRIVE_CAP_2, 8, SIGNALS_DEFAULT},
  {'E', false, 2, GPIO_DRIVE_CAP_2, 4, SIGNALS_DEFAULT},
  {'F', false, 2, GPIO_DRIVE_CAP_2, 8, SIGNALS_DEFAULT},
  {'G', true,  2, GPIO_DRIVE_CAP_2, 4, SIGNALS_DEFAULT},
  {'H', true,  2, GPIO_DRIVE_CAP_2, 8, SIGNALS_DEFAULT},
  {'I', false, 4, GPIO_DRIVE_CAP_1, 8, SIGNALS_DEFAULT},
  {'J', false, 4, GPIO_DRIVE_CAP_3, 8, SIGNALS_DEFAULT},
  {'K', false, 4, GPIO_DRIVE_CAP_0, 8, SIGNALS_DEFAULT},
  {'L', false, 4, GPIO_DRIVE_CAP_1, 8, CONTROL_SOFT},
  {'M', false, 4, GPIO_DRIVE_CAP_1, 8, ALL_SOFT},
  {'N', false, 4, GPIO_DRIVE_CAP_0, 10, SIGNALS_DEFAULT},
  {'O', true,  4, GPIO_DRIVE_CAP_0, 8, SIGNALS_DEFAULT},
  {'P', true,  4, GPIO_DRIVE_CAP_0, 10, SIGNALS_DEFAULT},
  {'Q', false, 4, GPIO_DRIVE_CAP_0, 20, SIGNALS_DEFAULT},
  {'R', false, 4, GPIO_DRIVE_CAP_0, 40, SIGNALS_DEFAULT},
  {'S', false, 4, GPIO_DRIVE_CAP_0, 80, SIGNALS_DEFAULT},
};
constexpr size_t TIMING_PROFILE_COUNT = sizeof(timingProfiles) / sizeof(timingProfiles[0]);
const size_t focusedProfiles[] = {13, 16, 17, 18}; // N, Q, R, S.
constexpr size_t FOCUSED_PROFILE_COUNT = sizeof(focusedProfiles) / sizeof(focusedProfiles[0]);
const TimingProfile STARTUP_TIMING = {'N', false, 4, GPIO_DRIVE_CAP_0, 10, SIGNALS_DEFAULT};
constexpr uint32_t TIMING_TEST_INTERVAL_MS = 12000;
TimingProfile timingBeforeSweep = STARTUP_TIMING;
bool timingSweepActive = false, timingSweepPaused = false;
bool focusedSweep = false;
size_t timingSweepPosition = 0;
size_t timingProfileIndex = 0;
uint32_t timingProfileStarted = 0;

void applyClockDrive(gpio_drive_cap_t drive) {
  const gpio_num_t clockPin = static_cast<gpio_num_t>(panel->getCfg().gpio.clk);
  if (gpio_set_drive_capability(clockPin, drive) != ESP_OK) {
    Serial.println("Could not set panel clock drive strength.");
    return;
  }
  clockDrive = drive;
  Serial.println("Panel clock drive level: " + String(int(drive)));
}

void applySignalDrive(SignalDriveMode mode) {
  const auto& gpio = panel->getCfg().gpio;
  const int8_t pins[] = {gpio.r1, gpio.g1, gpio.b1, gpio.r2, gpio.g2, gpio.b2,
                         gpio.a, gpio.b, gpio.c, gpio.d, gpio.e, gpio.lat, gpio.oe};
  bool success = true;
  for (const int8_t pin : pins) {
    if (pin < 0) continue;
    const bool soft = mode == ALL_SOFT ||
                      (mode == CONTROL_SOFT && (pin == gpio.lat || pin == gpio.oe));
    success &= gpio_set_drive_capability(static_cast<gpio_num_t>(pin),
                                        soft ? GPIO_DRIVE_CAP_1 : GPIO_DRIVE_CAP_3) == ESP_OK;
  }
  if (success) signalDriveMode = mode;
  else Serial.println("Could not set panel signal drive strength.");
}

String jsonString(const String& value) {
  String out = "\"";
  for (size_t i = 0; i < value.length(); ++i) {
    unsigned char c = value[i];
    if (c == '\"' || c == '\\') { out += '\\'; out += char(c); }
    else if (c < 32) { char esc[7]; snprintf(esc, sizeof(esc), "\\u%04x", c); out += esc; }
    else out += char(c);
  }
  return out + "\"";
}

void sendJSON(int code, const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", body);
}

bool isControlRequest() {
  // Custom header prevents ordinary cross-site forms from changing the display.
  // This is not user authentication: other clients on the LAN can control it.
  return server.header("X-Matrix-Control") == "1";
}

bool requireControlRequest() {
  if (isControlRequest()) return true;
  sendJSON(403, "{\"error\":\"Missing X-Matrix-Control header\"}");
  return false;
}

void showMessage(const char* text) {
  if (receivedContent) return;
  panel->clearScreen();
  panel->setTextSize(1);
  panel->setTextColor(panel->color565(70, 180, 220));
  panel->setCursor(2, 12);
  panel->print(text);
  panel->flipDMABuffer();
}

void showHardwareColorTest() {
  // Drive RGB888 directly to isolate panel/wiring faults from browser encoding.
  for (int16_t y = 0; y < 32; ++y) {
    for (int16_t x = 0; x < 64; ++x) {
      const bool white = y >= 24;
      panel->drawPixelRGB888(x, y,
                            white || x < 21 ? 255 : 0,
                            white || (x >= 21 && x < 42) ? 255 : 0,
                            white || x >= 42 ? 255 : 0);
    }
  }
  panel->flipDMABuffer();
  receivedContent = true;
  Serial.println("Hardware RGB test: RED | GREEN | BLUE, with a WHITE strip on the bottom 8 rows.");
  Serial.println("G1=GPIO26 B1=GPIO27 G2=GPIO12 B2=GPIO13; brightness=" + String(brightness));
}

void showHardwareWhiteTest() {
  panel->fillScreenRGB888(255, 255, 255);
  panel->flipDMABuffer();
  receivedContent = true;
  Serial.println("Hardware white test: all 64x32 pixels WHITE; brightness=" + String(brightness));
}

void showHardwareTextTest() {
  panel->clearScreen();
  panel->setTextWrap(false);
  panel->setTextSize(1);
  panel->setTextColor(panel->color565(255, 255, 255));
  panel->setCursor(2, 12);
  panel->print("Hello");
  panel->flipDMABuffer();
  receivedContent = true;
  Serial.println("Hardware text test: static WHITE Hello on BLACK; brightness=" + String(brightness));
}

void applyTimingProfile(const TimingProfile& profile) {
  // Temporarily blank both buffers while changing the clock routing and divider.
  panel->setBrightness8(0);
  const int clockSignal = ESP32_I2S_DEVICE == I2S_NUM_0 ? I2S0O_WS_OUT_IDX : I2S1O_WS_OUT_IDX;
  gpio_matrix_out(panel->getCfg().gpio.clk, clockSignal, profile.phase, false);
  getDev()->clkm_conf.clkm_div_num = profile.divider;
  runtimeClockPhase = profile.phase;
  panel->setLatBlanking(profile.blanking);
  applySignalDrive(profile.signals);
  applyClockDrive(profile.drive);
}

void showTimingProfile() {
  const TimingProfile& profile = timingProfiles[timingProfileIndex];
  applyTimingProfile(profile);
  panel->clearScreen();
  panel->setTextWrap(false);
  panel->setTextSize(1);
  panel->setTextColor(panel->color565(255, 255, 255));
  panel->setCursor(2, 0);
  panel->print("TEST ");
  panel->print(profile.id);
  panel->setCursor(2, 12);
  panel->print("Hello");
  panel->setCursor(2, 24);
  panel->print("Hello");
  panel->drawFastVLine(48, 12, 20, panel->color565(255, 255, 255));
  panel->flipDMABuffer();
  panel->setBrightness8(brightness);
  receivedContent = true;
  timingProfileStarted = millis();
  // The DMA chain stays unchanged during the sweep, so scan refresh scales
  // directly with clock frequency. This is an estimate, not a measurement.
  const float estimatedRefresh = float(panel->calculated_refresh_rate) *
                                 STARTUP_TIMING.divider / profile.divider;
  Serial.printf("TIMING TEST %c: clock=%.2f MHz phase=%u blanking=%u drive=%u signals=%u brightness=%u estimated_refresh=%.1f Hz\n",
                profile.id, 40.0 / profile.divider, unsigned(profile.phase),
                unsigned(profile.blanking), unsigned(profile.drive), unsigned(profile.signals),
                unsigned(brightness), double(estimatedRefresh));
}

void startTimingSweep(bool focused = false) {
  if (!timingSweepActive) {
    timingBeforeSweep = {'-', runtimeClockPhase, panel->getCfg().latch_blanking,
                         clockDrive, uint8_t(getDev()->clkm_conf.clkm_div_num), signalDriveMode};
  }
  timingSweepActive = true;
  timingSweepPaused = false;
  focusedSweep = focused;
  timingSweepPosition = 0;
  timingProfileIndex = focused ? focusedProfiles[0] : 0;
  Serial.println(focused
                 ? "Slow-clock sweep: N,Q,R,S, 12 seconds each. P=pause/resume N=next X=stop."
                 : "Timing sweep: tests A-S, 12 seconds each, repeating. P=pause/resume N=next X=stop.");
  showTimingProfile();
}

void stopTimingSweep(bool showText = false) {
  if (!timingSweepActive) return;
  timingSweepActive = false;
  timingSweepPaused = false;
  applyTimingProfile(timingBeforeSweep);
  if (showText) showHardwareTextTest();
  panel->setBrightness8(brightness);
  Serial.println("Timing sweep stopped; previous timing restored.");
}

void nextTimingProfile() {
  timingSweepPosition = (timingSweepPosition + 1) %
                        (focusedSweep ? FOCUSED_PROFILE_COUNT : TIMING_PROFILE_COUNT);
  timingProfileIndex = focusedSweep ? focusedProfiles[timingSweepPosition] : timingSweepPosition;
  showTimingProfile();
}

void updateTimingSweep() {
  if (timingSweepActive && !timingSweepPaused &&
      uint32_t(millis() - timingProfileStarted) >= TIMING_TEST_INTERVAL_MS) {
    nextTimingProfile();
  }
}

void startSetupAP() {
  if (apRunning) return;
  WiFi.mode(WIFI_AP_STA);
  apRunning = WiFi.softAP(apName.c_str(), apPassword.c_str());
  if (!apRunning) { Serial.println("Setup Wi-Fi failed to start. Reset the ESP32."); return; }
  Serial.println("\n--- Wi-Fi setup ---");
  Serial.println("Network: " + apName);
  Serial.println("Password: " + apPassword);
  Serial.println("Open http://" + WiFi.softAPIP().toString());
  showMessage("SETUP");
}

void beginHomeWiFi() {
  if (savedSSID.isEmpty()) { connecting = false; startSetupAP(); return; }
  WiFi.disconnect();
  WiFi.setAutoReconnect(true);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  connectStarted = millis();
  connecting = true;
  Serial.println("Connecting to " + savedSSID + " (2.4 GHz)...");
}

void updateWiFi() {
  if (changeWiFiPending && int32_t(millis() - changeWiFiAt) >= 0) {
    changeWiFiPending = false;
    startSetupAP(); // Retain a recovery path while trying new credentials.
    beginHomeWiFi();
  }
  bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !wasConnected) {
    connecting = false;
    if (mdnsRunning) MDNS.end();
    mdnsRunning = MDNS.begin(hostname.c_str());
    if (mdnsRunning) MDNS.addService("http", "tcp", 80);
    Serial.println("\nOpen http://" + WiFi.localIP().toString());
    Serial.println("Also try http://" + hostname + ".local");
    showMessage("READY");
  } else if (!connected && wasConnected) {
    if (mdnsRunning) { MDNS.end(); mdnsRunning = false; }
    connecting = true;
    connectStarted = millis();
  }
  if (!connected && connecting && millis() - connectStarted > 20000) {
    startSetupAP();
    connecting = false;
  }
  wasConnected = connected;
}

void statusRoute() {
  const bool connected = WiFi.status() == WL_CONNECTED;
  String out = "{\"width\":64,\"height\":32,\"connected\":";
  out += connected ? "true" : "false";
  out += ",\"ip\":" + jsonString(connected ? WiFi.localIP().toString() : "");
  out += ",\"hostname\":" + jsonString(hostname + ".local");
  out += ",\"ssid\":" + jsonString(savedSSID);
  out += ",\"ap\":" + jsonString(apRunning ? apName : "");
  out += ",\"brightness\":" + String(brightness);
  out += ",\"frames\":" + String(framesReceived);
  out += ",\"freeHeap\":" + String(ESP.getFreeHeap()) + "}";
  sendJSON(200, out);
}

void uploadFrame() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    ++uploadFiles;
    if (uploadFiles == 1) incomingFrame.start();
    if (uploadFiles != 1 || upload.name != "frame" || !isControlRequest()) incomingFrame.reject();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    incomingFrame.append(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    incomingFrame.finish();
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    incomingFrame.reject();
    uploadFiles = 0;
  }
}

void finishFrame() {
  const bool valid = uploadFiles == 1 && incomingFrame.valid();
  uploadFiles = 0;
  if (!requireControlRequest()) return;
  if (!valid) {
    sendJSON(400, "{\"error\":\"Upload one frame file of exactly 4096 RGB565-LE bytes\"}");
    return;
  }
  stopTimingSweep();
  for (size_t y = 0; y < MatrixProtocol::HEIGHT; ++y)
    for (size_t x = 0; x < MatrixProtocol::WIDTH; ++x)
      panel->drawPixel(x, y, incomingFrame.pixel(y * MatrixProtocol::WIDTH + x));
  panel->flipDMABuffer();
  receivedContent = true;
  ++framesReceived;
  sendJSON(200, "{\"ok\":true,\"frames\":" + String(framesReceived) + "}");
  incomingFrame.reject();
}

void brightnessRoute() {
  if (!requireControlRequest()) return;
  String value = server.arg("value");
  bool valid = value.length() >= 1 && value.length() <= 3;
  for (size_t i = 0; i < value.length(); ++i) valid &= value[i] >= '0' && value[i] <= '9';
  int number = value.toInt();
  if (!valid || number > 255) { sendJSON(400, "{\"error\":\"Brightness must be 0 to 255\"}"); return; }
  stopTimingSweep();
  brightness = uint8_t(number);
  panel->setBrightness8(brightness);
  sendJSON(200, "{\"ok\":true}");
}

void wifiRoute() {
  if (!requireControlRequest()) return;
  String ssid = server.arg("ssid"), password = server.arg("password");
  if (ssid.isEmpty() || ssid.length() > 32 || password.length() > 63 || password.length() < 8) {
    sendJSON(400, "{\"error\":\"Use a 1-32 byte network name and an 8-63 character Wi-Fi password\"}");
    return;
  }
  // Preferences survives power loss; passwords are never included in API responses.
  const bool stored = settings.putString("ssid", ssid) > 0 &&
                      settings.putString("password", password) > 0;
  if (!stored) { sendJSON(500, "{\"error\":\"Could not save Wi-Fi settings\"}"); return; }
  savedSSID = ssid;
  savedPassword = password;
  changeWiFiPending = true;
  changeWiFiAt = millis() + 500;
  sendJSON(200, "{\"ok\":true,\"message\":\"Saved; connecting now. Watch the connection status.\"}");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  HUB75_I2S_CFG config(64, 32, 1);
  config.gpio.r1 = 25;
  config.gpio.g1 = 26;
  config.gpio.b1 = 27;
  config.gpio.r2 = 14;
  config.gpio.g2 = 12;
  config.gpio.b2 = 13;
  config.gpio.a = 23;
  config.gpio.b = 19;
  config.gpio.c = 5;
  config.gpio.d = 17;
  config.gpio.e = -1;
  config.gpio.lat = 4;
  config.gpio.oe = 15;
  config.gpio.clk = 16;
  config.driver = HUB75_I2S_CFG::FM6124; // FM6124H appeared in the supplied panel photo.
  // Try the opposite sampling edge for this FM6124 panel's colour/flicker issues.
  config.clkphase = false;
  // Match DMA refresh calculations to the selected startup output. This version's
  // classic ESP32 bus driver needs the actual divider override after begin().
  config.i2sspeed = static_cast<HUB75_I2S_CFG::clk_speed>(40000000 / STARTUP_TIMING.divider);
  config.latch_blanking = 4; // Longer output blanking to reduce ghost pixels around text.
  config.double_buff = true;
  config.setPixelColorDepthBits(6);
  panel = new MatrixPanel_I2S_DMA(config);
  if (!panel->begin()) {
    Serial.println("Panel initialization failed. Check DMA memory and library installation.");
    while (true) delay(1000);
  }
  // Profile N was best in the latest visual comparison. Apply it every boot.
  applyTimingProfile(STARTUP_TIMING);
  panel->setBrightness8(brightness);
  Serial.println("Startup profile N: 4 MHz, clkphase=false, latch blanking=4, clock drive=0.");
  Serial.println("Calculated panel refresh: " + String(panel->calculated_refresh_rate) + " Hz.");
  showMessage("STARTING");
  if (!settings.begin("matrix-ui", false)) {
    Serial.println("Could not open settings storage.");
    while (true) delay(1000);
  }
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06lx", (unsigned long)(ESP.getEfuseMac() & 0xFFFFFF));
  hostname = "matrix-" + String(suffix);
  apName = "Matrix-" + String(suffix);
  apPassword = settings.getString("apPassword", "");
  if (apPassword.length() < 8) {
    char generated[13];
    snprintf(generated, sizeof(generated), "%08lx%04lx", (unsigned long)esp_random(),
             (unsigned long)(esp_random() & 0xFFFF));
    apPassword = generated;
    if (!settings.putString("apPassword", apPassword)) {
      Serial.println("Could not persist setup password.");
      while (true) delay(1000);
    }
  }
  savedSSID = settings.getString("ssid", "");
  savedPassword = settings.getString("password", "");
  WiFi.setHostname(hostname.c_str()); // Must precede WiFi.mode().
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  beginHomeWiFi();

  const char* headers[] = {"X-Matrix-Control"};
  server.collectHeaders(headers, 1);
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("X-Content-Type-Options", "nosniff");
    server.sendHeader("X-Frame-Options", "DENY");
    server.send_P(200, "text/html; charset=utf-8", WEB_UI);
  });
  server.on("/api/status", HTTP_GET, statusRoute);
  server.on("/api/frame", HTTP_POST, finishFrame, uploadFrame);
  server.on("/api/brightness", HTTP_POST, brightnessRoute);
  server.on("/api/wifi", HTTP_POST, wifiRoute);
  server.onNotFound([]() { sendJSON(404, "{\"error\":\"Not found\"}"); });
  server.begin();
  Serial.println("Matrix web server started.");
  Serial.println("Send T for RGB, W for full-panel white, H for white Hello, or C to toggle clock drive.");
  Serial.println("Send F for slow-clock tests N,Q,R,S or S for all A-S; P=pause/resume N=next X=stop.");
}

void loop() {
  if (Serial.available()) {
    const char command = Serial.read();
    if (command == 'T') { stopTimingSweep(); showHardwareColorTest(); }
    else if (command == 'W') { stopTimingSweep(); showHardwareWhiteTest(); }
    else if (command == 'H') { stopTimingSweep(); showHardwareTextTest(); }
    else if (command == 'C') {
      stopTimingSweep();
      applyClockDrive(clockDrive == STARTUP_TIMING.drive ? GPIO_DRIVE_CAP_3 : STARTUP_TIMING.drive);
    }
    else if (command == 'S') startTimingSweep();
    else if (command == 'F') startTimingSweep(true);
    else if (command == 'P' && timingSweepActive) {
      timingSweepPaused = !timingSweepPaused;
      timingProfileStarted = millis();
      Serial.printf("Timing sweep %s on test %c.\n", timingSweepPaused ? "paused" : "resumed",
                    timingProfiles[timingProfileIndex].id);
    }
    else if (command == 'N' && timingSweepActive) nextTimingProfile();
    else if (command == 'X') stopTimingSweep(true);
  }
  server.handleClient();
  updateWiFi();
  updateTimingSweep();
  delay(1);
}
