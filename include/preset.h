#ifndef MEGAFM_CMAKE_PRESET_H
#define MEGAFM_CMAKE_PRESET_H

void loadZero();
void panel();
void setIndex();
void loadPreset();
void savePreset();
void shuffle();
byte getByte();
void store(byte input);
void eWrite(uint16_t theMemoryAddress, uint8_t u8Byte);
uint8_t eRead(uint16_t theMemoryAddress);
void clearSSEG(bool allBanks);
#endif // MEGAFM_CMAKE_PRESET_H
