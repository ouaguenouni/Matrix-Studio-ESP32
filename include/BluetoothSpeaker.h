#pragma once
#include <WebServer.h>

// Bluetooth Classic A2DP source, enabled only after a browser action.
void registerSpeakerRoutes(WebServer& server);
void prepareSpeakerMemory();
void updateBluetoothSpeaker();
// Queue the species cry when its first display frame is accepted. Returns false
// if no speaker is connected; image playback remains available without audio.
bool playPokemonCry(uint16_t species);
