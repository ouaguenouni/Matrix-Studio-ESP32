#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
class MatrixPanel_I2S_DMA;
struct LiveRequest {
  String mode, theme, target, palette, layout, motion, city, key, refresh;
  bool locate=false;
};
void liveBegin(Preferences& store,MatrixPanel_I2S_DMA* panel);
void liveTick(bool diagnostic=false);
void liveSetBrightness(uint8_t value);
void liveSurprise();
bool liveGiftConfigure(JsonObjectConst request,String& error);
void liveHold();
void liveNextMode();
bool liveConfigure(const LiveRequest& request,String& error);
String liveStatusJson();
