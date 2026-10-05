#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>
class MatrixPanel_I2S_DMA;
void giftBegin(Preferences& settings);
bool giftConfigure(JsonObjectConst request,String& error);
void giftStatus(JsonObject out,uint32_t now);
void giftTick(uint32_t now,const tm* local,bool canCelebrate);
bool giftRender(MatrixPanel_I2S_DMA* panel,uint8_t palette,uint32_t now);
int giftMode(uint32_t now,bool youtube);
void giftManual();
void giftSurprise(uint32_t now);
uint8_t giftBrightness(uint8_t requested,uint32_t now,const tm* local);

void giftRenderArt(MatrixPanel_I2S_DMA* panel,uint8_t palette,uint32_t now);
