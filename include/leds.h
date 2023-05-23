#ifndef MEGAFM_CMAKE_LEDS_H
#define MEGAFM_CMAKE_LEDS_H

#include "constants.h"

void updateLedNumber();
void clearLfoLeds();
void showLfo();
void showSSEG();
void showLink();
void showVoiceMode(VoiceMode voiceMode);
void rightDot();
void leftDot();
void showSendReceive();
void ledNumber(int value);
void ledNumberForced(int value);
void showPickup();
void digit(byte channel, byte number);
void showAlgo(byte number);
void ledSet(byte number, bool value);
void showNumber(byte movedPotOrFader, byte value);
#endif // MEGAFM_CMAKE_LEDS_H
