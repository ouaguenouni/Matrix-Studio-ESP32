#include "BluetoothSpeaker.h"
#include "CryAudio.h"
#include "cry_assets.h"
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_gap_bt_api.h>
#include <esp_a2dp_api.h>
#include <esp32-hal-bt.h>
#include <WiFi.h>
#include <math.h>
#include <esp_heap_caps.h>

extern void sendJSON(int code, const String& body);
extern String jsonString(const String& value);
extern bool requireControlRequest();
extern const uint8_t cryData[] asm("_binary_assets_cries_cries_bin_start");

namespace {
enum class Command { None, Scan, Connect, Disconnect };
enum class MediaState { Idle, Checking, Starting, Running, Suspending };
enum class AudioKind { Tone, Cry };
struct Device { uint8_t address[6]; char name[64]; int rssi; };
portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
WebServer* web = nullptr;
Device devices[16] = {}, target = {};
size_t deviceCount = 0;
bool enabled = false, ready = false, scanning = false, desiredScan = false;
bool connectPending = false, connected = false, disconnecting = false;
bool failed = false, audioRequested = false;
MediaState mediaState = MediaState::Idle;
AudioKind audioKind = AudioKind::Tone;
uint16_t cryId = 0;
uint32_t audioVersion = 0;
uint32_t startedAt = 0, connectAt = 0, audioAt = 0, disconnectAt = 0;
uint8_t testVolume = 25;
Command pending = Command::None;
char errorMessage[128] = {};
constexpr uint32_t SAMPLE_RATE = 44100, TONE_SAMPLES = SAMPLE_RATE * 2;

void setError(const char* message) {
  portENTER_CRITICAL(&lock);
  snprintf(errorMessage, sizeof(errorMessage), "%s", message);
  audioRequested = false; ++audioVersion;
  portEXIT_CRITICAL(&lock);
  Serial.printf("Bluetooth speaker: %s\n", message);
}
String addressString(const uint8_t* address) {
  char out[18];
  snprintf(out, sizeof(out), "%02X:%02X:%02X:%02X:%02X:%02X",
           address[0], address[1], address[2], address[3], address[4], address[5]);
  return out;
}
void recordDevice(esp_bt_gap_cb_param_t* param) {
  Device found = {}; found.rssi = -127;
  memcpy(found.address, param->disc_res.bda, 6);
  uint32_t cod = 0;
  for (int i = 0; i < param->disc_res.num_prop; ++i) {
    auto& prop = param->disc_res.prop[i];
    if (prop.type == ESP_BT_GAP_DEV_PROP_COD) memcpy(&cod, prop.val, sizeof(cod));
    else if (prop.type == ESP_BT_GAP_DEV_PROP_RSSI) found.rssi = *static_cast<int8_t*>(prop.val);
    else if (prop.type == ESP_BT_GAP_DEV_PROP_BDNAME) {
      const size_t length = std::min(size_t(prop.len), sizeof(found.name) - 1);
      memcpy(found.name, prop.val, length); found.name[length] = 0;
    } else if (prop.type == ESP_BT_GAP_DEV_PROP_EIR) {
      uint8_t length = 0;
      uint8_t* name = esp_bt_gap_resolve_eir_data(static_cast<uint8_t*>(prop.val), ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &length);
      if (!name) name = esp_bt_gap_resolve_eir_data(static_cast<uint8_t*>(prop.val), ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &length);
      if (name) {
        const size_t count = std::min(size_t(length), sizeof(found.name) - 1);
        memcpy(found.name, name, count); found.name[count] = 0;
      }
    }
  }
  const bool audio = esp_bt_gap_is_valid_cod(cod) &&
    (esp_bt_gap_get_cod_srvc(cod) & (ESP_BT_COD_SRVC_AUDIO | ESP_BT_COD_SRVC_RENDERING));
  // Some speakers omit class-of-device; retain named JBL results too.
  if (!audio && !strstr(found.name, "JBL")) return;
  portENTER_CRITICAL(&lock);
  size_t index = 0;
  while (index < deviceCount && memcmp(devices[index].address, found.address, 6)) ++index;
  if (index < 16) {
    if (!found.name[0] && index < deviceCount) memcpy(found.name, devices[index].name, sizeof(found.name));
    devices[index] = found;
    if (index == deviceCount) ++deviceCount;
  }
  portEXIT_CRITICAL(&lock);
}
bool isTarget(const uint8_t* address) {
  portENTER_CRITICAL(&lock);
  const bool match = memcmp(target.address, address, 6) == 0;
  portEXIT_CRITICAL(&lock);
  return match;
}
void gapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {
  if (event == ESP_BT_GAP_DISC_RES_EVT) recordDevice(param);
  else if (event == ESP_BT_GAP_DISC_STATE_CHANGED_EVT) {
    portENTER_CRITICAL(&lock);
    scanning = param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED;
    portEXIT_CRITICAL(&lock);
  } else if (event == ESP_BT_GAP_CFM_REQ_EVT) {
    esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, isTarget(param->cfm_req.bda));
  } else if (event == ESP_BT_GAP_PIN_REQ_EVT) {
    esp_bt_pin_code_t pin = {'0','0','0','0'};
    esp_bt_gap_pin_reply(param->pin_req.bda, isTarget(param->pin_req.bda), 4, pin);
  } else if (event == ESP_BT_GAP_AUTH_CMPL_EVT && param->auth_cmpl.stat != ESP_BT_STATUS_SUCCESS) {
    setError("Pairing failed. Press the JBL Bluetooth button and try again.");
  }
}
void a2dpCallback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param) {
  if (event == ESP_A2D_PROF_STATE_EVT) {
    portENTER_CRITICAL(&lock);
    ready = param->a2d_prof_stat.init_state == ESP_A2D_INIT_SUCCESS;
    portEXIT_CRITICAL(&lock);
  } else if (event == ESP_A2D_CONNECTION_STATE_EVT) {
    const auto state = param->conn_stat.state;
    portENTER_CRITICAL(&lock);
    connected = state == ESP_A2D_CONNECTION_STATE_CONNECTED;
    if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTING) {
      if (!disconnecting) disconnectAt = millis();
      disconnecting = true;
    }
    if (connected) { connectPending = false; disconnecting = false; errorMessage[0] = 0; }
    if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
      if (connectPending && !disconnecting)
        snprintf(errorMessage, sizeof(errorMessage), "Could not connect. Put the JBL in pairing mode and scan again.");
      connectPending = false; disconnecting = false;
      audioRequested = false; mediaState = MediaState::Idle; ++audioVersion;
    }
    portEXIT_CRITICAL(&lock);
    Serial.printf("Bluetooth speaker connection state: %d\n", int(state));
  } else if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT) {
    Serial.printf("Bluetooth audio command %d: status %d\n", int(param->media_ctrl_stat.cmd), int(param->media_ctrl_stat.status));
    if (param->media_ctrl_stat.status != ESP_A2D_MEDIA_CTRL_ACK_SUCCESS) {
      portENTER_CRITICAL(&lock); mediaState = MediaState::Idle; portEXIT_CRITICAL(&lock);
      setError("The speaker could not play the sound. Reconnect and try again."); return;
    }
    if (param->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY) {
      portENTER_CRITICAL(&lock);
      const bool requested = audioRequested;
      mediaState = requested ? MediaState::Starting : MediaState::Idle;
      portEXIT_CRITICAL(&lock);
      if (requested && esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START) != ESP_OK) {
        portENTER_CRITICAL(&lock); mediaState = MediaState::Idle; portEXIT_CRITICAL(&lock);
        setError("Could not start speaker audio. Reconnect and try again.");
      }
    } else if (param->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_START) {
      portENTER_CRITICAL(&lock);
      mediaState = MediaState::Running;
      portEXIT_CRITICAL(&lock);
    } else if (param->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_SUSPEND) {
      portENTER_CRITICAL(&lock); mediaState = MediaState::Idle; portEXIT_CRITICAL(&lock);
    }
  }
}
int32_t audioData(uint8_t* data, int32_t length) {
  if (!data || length <= 0) return 0;
  memset(data, 0, length);
  portENTER_CRITICAL(&lock);
  const bool running = audioRequested && mediaState == MediaState::Running;
  const uint32_t version = audioVersion;
  const AudioKind kind = audioKind;
  const uint16_t species = cryId;
  const uint8_t volume = testVolume;
  portEXIT_CRITICAL(&lock);
  if (!running) return length;
  // Only this callback owns the decoder. Requests replace it at the next PCM
  // buffer boundary; callback processing never holds the cross-core spinlock.
  static uint32_t playingVersion = 0, toneLeft = 0, drainLeft = 0;
  static MatrixAudio::CryPlayer player;
  if (playingVersion != version) {
    playingVersion = version;
    // Keep feeding silence briefly after the clip so A2DP can encode and
    // transmit its queued tail before SUSPEND closes the audio stream.
    drainLeft = SAMPLE_RATE / 3;
    if (kind == AudioKind::Cry) {
      const CryAsset& asset = CRY_ASSETS[species - 1];
      player.reset(cryData + asset.offset, asset.size, asset.samples, CRY_SAMPLE_RATE);
    } else toneLeft = TONE_SAMPLES;
  }
  for (int32_t i = 0; i < length / 4; ++i) {
    int16_t sample = 0;
    if (kind == AudioKind::Cry) {
      if (!player.finished()) sample = int32_t(player.next()) * volume / 100;
      else if (drainLeft) --drainLeft;
    }
    else if (toneLeft) {
      const uint32_t position = TONE_SAMPLES - toneLeft--;
      const float fade = std::min(1.0f, std::min(float(position), float(TONE_SAMPLES - position)) / 882.0f);
      sample = int16_t(sinf(2.0f * float(M_PI) * 440.0f * float(position) / SAMPLE_RATE) * 4000.0f * volume / 100.0f * fade);
    } else if (drainLeft) --drainLeft;
    // Stereo, signed 16-bit little endian; no I2S peripheral or LED pins used.
    data[i*4] = data[i*4+2] = uint16_t(sample) & 255;
    data[i*4+1] = data[i*4+3] = uint16_t(sample) >> 8;
  }
  if (!drainLeft && ((kind == AudioKind::Cry && player.finished()) || (kind == AudioKind::Tone && !toneLeft))) {
    portENTER_CRITICAL(&lock);
    if (version == audioVersion) audioRequested = false;
    portEXIT_CRITICAL(&lock);
  }
  return length;
}
bool requestAudio(AudioKind kind, uint16_t species, bool replace) {
  portENTER_CRITICAL(&lock);
  const bool available = connected && !disconnecting &&
    (replace || (!audioRequested && mediaState == MediaState::Idle));
  if (available) {
    audioKind = kind; cryId = species; audioRequested = true;
    audioAt = millis(); ++audioVersion; errorMessage[0] = 0;
  }
  portEXIT_CRITICAL(&lock);
  return available;
}
bool initializeBluetooth() {
  if (enabled) return true;
  if (failed) return false;
  // IDF's Wi-Fi/BT coexistence requires modem sleep; WIFI_PS_NONE aborts
  // inside controller_enable(). Keep low-latency Wi-Fi until audio is used.
  if (!WiFi.setSleep(true)) {
    desiredScan = false;
    setError("Could not enable Wi-Fi/Bluetooth coexistence. Try scanning again.");
    return false;
  }
  // Classic audio only; release BLE's unused memory before enabling the controller.
  esp_bt_controller_config_t config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  config.mode = ESP_BT_MODE_CLASSIC_BT;
  config.bt_max_acl_conn = 1; // One audio speaker, rather than unused extra links.
  Serial.printf("Starting Bluetooth controller; free heap: %u bytes.\n", ESP.getFreeHeap());
  esp_err_t result = esp_bt_controller_init(&config);
  Serial.printf("Bluetooth controller init: %s\n", esp_err_to_name(result));
  if (result == ESP_OK) result = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  Serial.printf("Bluetooth controller enable: %s\n", esp_err_to_name(result));
  if (result == ESP_OK) result = esp_bluedroid_init();
  Serial.printf("Bluetooth host init: %s\n", esp_err_to_name(result));
  if (result == ESP_OK) result = esp_bluedroid_enable();
  Serial.printf("Bluetooth host enable: %s\n", esp_err_to_name(result));
  if (result == ESP_OK) result = esp_bt_dev_set_device_name("Matrix Studio Audio");
  if (result == ESP_OK) result = esp_bt_gap_register_callback(gapCallback);
  if (result == ESP_OK) result = esp_a2d_register_callback(a2dpCallback);
  if (result == ESP_OK) result = esp_a2d_source_register_data_callback(audioData);
  if (result == ESP_OK) result = esp_a2d_source_init();
  if (result != ESP_OK) {
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED) esp_bluedroid_disable();
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_INITIALIZED) esp_bluedroid_deinit();
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED) esp_bt_controller_disable();
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) esp_bt_controller_deinit();
    failed = true; desiredScan = false;
    char message[128]; snprintf(message, sizeof(message), "Bluetooth could not start (%s). Restart the ESP32 and try again.", esp_err_to_name(result));
    setError(message); return false;
  }
  esp_bt_io_cap_t capability = ESP_BT_IO_CAP_NONE;
  esp_bt_gap_set_security_param(ESP_BT_SP_IOCAP_MODE, &capability, sizeof(capability));
  esp_bt_pin_code_t pin = {'0','0','0','0'};
  esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, pin);
  esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
  enabled = true; startedAt = millis();
  Serial.printf("Bluetooth audio started; free heap: %u bytes.\n", ESP.getFreeHeap());
  return true;
}
void statusRoute() {
  Device snapshot[16], peer; size_t count; bool isReady, isScanning, isConnected, isConnecting, isDisconnecting, isTesting, isPlaying;
  uint16_t playingCry;
  uint8_t volume; char message[128];
  portENTER_CRITICAL(&lock);
  memcpy(snapshot, devices, sizeof(devices)); peer = target; count = deviceCount;
  isReady = ready; isScanning = scanning; isConnected = connected; isConnecting = connectPending;
  isDisconnecting = disconnecting; volume = testVolume;
  isPlaying = audioRequested || mediaState != MediaState::Idle;
  isTesting = isPlaying && audioKind == AudioKind::Tone;
  playingCry = isPlaying && audioKind == AudioKind::Cry ? cryId : 0;
  memcpy(message, errorMessage, sizeof(message));
  portEXIT_CRITICAL(&lock);
  const char* state = failed ? "error" : !enabled && pending == Command::None ? "off" :
    !isReady ? "initializing" : isDisconnecting ? "disconnecting" : isConnected ? "connected" :
    isConnecting || pending == Command::Connect ? "connecting" : isScanning || desiredScan || pending == Command::Scan ? "scanning" : "idle";
  String out = "{\"state\":" + jsonString(state) + ",\"connected\":" + (isConnected ? "true" : "false");
  out += ",\"testing\":" + String(isTesting ? "true" : "false") + ",\"volume\":" + String(volume);
  out += ",\"playing\":" + String(isPlaying ? "true" : "false") + ",\"cryId\":" + String(playingCry);
  out += ",\"name\":" + jsonString(peer.name) + ",\"address\":" + jsonString(addressString(peer.address));
  out += ",\"message\":" + jsonString(message) + ",\"devices\":[";
  for (size_t i = 0; i < count; ++i) {
    if (i) out += ',';
    out += "{\"name\":" + jsonString(snapshot[i].name[0] ? snapshot[i].name : "Bluetooth speaker");
    out += ",\"address\":" + jsonString(addressString(snapshot[i].address)) + ",\"rssi\":" + String(snapshot[i].rssi) + "}";
  }
  sendJSON(200, out + "]}");
}
bool busyOperation() {
  portENTER_CRITICAL(&lock); const bool busy = connected || connectPending || disconnecting; portEXIT_CRITICAL(&lock);
  return busy || pending != Command::None;
}
} // namespace

bool playPokemonCry(uint16_t species) {
  if (species < 1 || species > sizeof(CRY_ASSETS) / sizeof(CRY_ASSETS[0])) return false;
  return requestAudio(AudioKind::Cry, species, true);
}

void prepareSpeakerMemory() {
  // Recover unused BLE RAM before the panel allocates its DMA buffers. Bluetooth
  // Classic remains available; its controller is initialized only by Scan.
  // Referencing the Arduino HAL also links its btInUse() hook, so initArduino()
  // retains Classic Bluetooth memory instead of releasing all BT RAM at boot.
  if (!btStarted()) esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
}

void registerSpeakerRoutes(WebServer& server) {
  web = &server;
  server.on("/api/speaker", HTTP_GET, statusRoute);
  server.on("/api/speaker/scan", HTTP_POST, []() {
    if (!requireControlRequest()) return;
    if (busyOperation()) { sendJSON(409, "{\"error\":\"Disconnect the current speaker before scanning.\"}"); return; }
    if (failed) { sendJSON(503, "{\"error\":\"Restart the ESP32 to retry Bluetooth initialization.\"}"); return; }
    portENTER_CRITICAL(&lock); deviceCount = 0; errorMessage[0] = 0; portEXIT_CRITICAL(&lock);
    pending = Command::Scan;
    sendJSON(202, "{\"ok\":true,\"message\":\"Scanning for speakers. Keep the JBL in pairing mode.\"}");
  });
  server.on("/api/speaker/connect", HTTP_POST, []() {
    if (!requireControlRequest()) return;
    if (busyOperation()) { sendJSON(409, "{\"error\":\"A speaker connection is already active.\"}"); return; }
    const String address = web->arg("address"); bool found = false;
    // Select only a device actually found by this ESP32, using its address.
    Device snapshot[16]; size_t count;
    portENTER_CRITICAL(&lock); memcpy(snapshot, devices, sizeof(devices)); count = deviceCount; portEXIT_CRITICAL(&lock);
    for (size_t i = 0; i < count; ++i) if (address.equalsIgnoreCase(addressString(snapshot[i].address))) {
      portENTER_CRITICAL(&lock); target = snapshot[i]; errorMessage[0] = 0; portEXIT_CRITICAL(&lock);
      found = true; break;
    }
    if (!found) { sendJSON(400, "{\"error\":\"Choose a speaker from the scan results.\"}"); return; }
    pending = Command::Connect;
    sendJSON(202, "{\"ok\":true,\"message\":\"Connecting to the selected speaker…\"}");
  });
  server.on("/api/speaker/disconnect", HTTP_POST, []() {
    if (!requireControlRequest()) return;
    pending = Command::Disconnect;
    sendJSON(202, "{\"ok\":true,\"message\":\"Stopping speaker connection…\"}");
  });
  server.on("/api/speaker/test", HTTP_POST, []() {
    if (!requireControlRequest()) return;
    const bool available = requestAudio(AudioKind::Tone, 0, false);
    if (!available) { sendJSON(409, "{\"error\":\"Connect a speaker and wait for the current test to finish.\"}"); return; }
    sendJSON(202, "{\"ok\":true,\"message\":\"Playing a short test sound.\"}");
  });
  server.on("/api/speaker/cry/stop", HTTP_POST, []() {
    if (!requireControlRequest()) return;
    portENTER_CRITICAL(&lock);
    if (audioKind == AudioKind::Cry) { audioRequested = false; ++audioVersion; }
    portEXIT_CRITICAL(&lock);
    sendJSON(200, "{\"ok\":true}");
  });
  server.on("/api/speaker/volume", HTTP_POST, []() {
    if (!requireControlRequest()) return;
    const String value = web->arg("value");
    bool valid = value.length() > 0 && value.length() <= 3;
    for (size_t i = 0; i < value.length(); ++i) valid &= isDigit(value[i]);
    const int volume = value.toInt();
    if (!valid || volume > 100) { sendJSON(400, "{\"error\":\"Speaker audio volume must be 0–100.\"}"); return; }
    portENTER_CRITICAL(&lock); testVolume = volume; portEXIT_CRITICAL(&lock);
    sendJSON(200, "{\"ok\":true}");
  });
}

void updateBluetoothSpeaker() {
  static uint32_t memoryLogAt = 0;
  if (enabled && millis() - memoryLogAt >= 10000) {
    memoryLogAt = millis();
    Serial.printf("Audio heap: free=%u, largest=%u, internal=%u\n", ESP.getFreeHeap(),
                  heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  }
  if (pending == Command::Scan) {
    pending = Command::None; desiredScan = true;
    initializeBluetooth();
  } else if (pending == Command::Connect) {
    pending = Command::None; desiredScan = false;
    portENTER_CRITICAL(&lock); connectPending = true; portEXIT_CRITICAL(&lock);
    connectAt = millis();
    if (scanning) esp_bt_gap_cancel_discovery();
  } else if (pending == Command::Disconnect) {
    pending = Command::None; desiredScan = false;
    portENTER_CRITICAL(&lock);
    const bool wasActive = connected || connectPending;
    connectPending = false; disconnecting = wasActive; audioRequested = false; ++audioVersion;
    portEXIT_CRITICAL(&lock);
    if (scanning) esp_bt_gap_cancel_discovery();
    if (wasActive) {
      disconnectAt = millis();
      if (esp_a2d_source_disconnect(target.address) != ESP_OK) {
        portENTER_CRITICAL(&lock); disconnecting = false; portEXIT_CRITICAL(&lock);
      }
    }
  }
  if (!enabled) return;
  portENTER_CRITICAL(&lock);
  const bool isReady = ready, isScanning = scanning, isConnected = connected;
  const bool isConnecting = connectPending, isDisconnecting = disconnecting;
  const MediaState stage = mediaState;
  const bool requested = audioRequested;
  const bool audioExpired = requested && millis() - audioAt > 15000;
  portEXIT_CRITICAL(&lock);
  if (!isReady) {
    if (millis() - startedAt > 10000 && !failed) { failed = true; desiredScan = false; setError("Bluetooth audio initialization timed out. Restart the ESP32."); }
    return;
  }
  if (desiredScan && !isScanning) {
    desiredScan = false;
    const esp_err_t result = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
    if (result != ESP_OK) setError("Could not scan for speakers. Try again.");
  }
  if (isConnecting && !isConnected && !isScanning && connectAt) {
    const esp_err_t result = esp_a2d_source_connect(target.address);
    connectAt = 0;
    if (result != ESP_OK) {
      portENTER_CRITICAL(&lock); connectPending = false; portEXIT_CRITICAL(&lock);
      setError("Could not start the speaker connection. Scan again.");
    } else startedAt = millis();
  }
  if (isConnecting && !connectAt && millis() - startedAt > 30000) {
    portENTER_CRITICAL(&lock); connectPending = false; portEXIT_CRITICAL(&lock);
    esp_a2d_source_disconnect(target.address);
    setError("Connection timed out. Press the JBL Bluetooth button and try again.");
  }
  if (isDisconnecting && millis() - disconnectAt > 10000) {
    portENTER_CRITICAL(&lock); disconnecting = false; portEXIT_CRITICAL(&lock);
    setError("The speaker did not confirm disconnection. Restart the ESP32 if needed.");
  }
  if (audioExpired) setError("Speaker audio timed out. Reconnect the speaker and try again.");
  if (stage == MediaState::Idle && requested && !audioExpired) {
    portENTER_CRITICAL(&lock); mediaState = MediaState::Checking; portEXIT_CRITICAL(&lock);
    if (esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY) != ESP_OK) {
      portENTER_CRITICAL(&lock); mediaState = MediaState::Idle; portEXIT_CRITICAL(&lock);
      setError("Could not prepare speaker audio. Reconnect and try again.");
    }
  } else if (stage == MediaState::Running && (!requested || audioExpired)) {
    portENTER_CRITICAL(&lock); mediaState = MediaState::Suspending; portEXIT_CRITICAL(&lock);
    if (esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_SUSPEND) != ESP_OK) {
      portENTER_CRITICAL(&lock); mediaState = MediaState::Idle; portEXIT_CRITICAL(&lock);
      setError("Could not stop speaker audio. Reconnect and try again.");
    } else Serial.println("Bluetooth sound completed or stopped.");
  }
}
