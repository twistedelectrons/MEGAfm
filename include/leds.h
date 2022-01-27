//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_LEDS_H
#define MEGAFM_CMAKE_LEDS_H

void updateLedNumber();
void clearLfoLeds();
void showLfo();
void showLink();
void showVoiceMode();
void rightDot();
void leftDot();
void showSendReceive();
void ledNumber(int value);
void ledNumberForced(int value);
void rightArrow();
void leftArrow();
void digit(byte channel, byte number);
void showAlgo(byte number);
void ledSet(byte number, bool value);

#endif //MEGAFM_CMAKE_LEDS_H
