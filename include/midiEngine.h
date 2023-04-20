#ifndef MEGAFM_CMAKE_MIDIENGINE_H
#define MEGAFM_CMAKE_MIDIENGINE_H

#include <Arduino.h>

void sendControlChange(byte number, byte value, byte channel);
void sendNoteOff(byte note, byte velocity, byte channel);
void sendTool(byte note, byte velocity);
void sendNoteOn(byte note, byte velocity, byte channel);
void midiRead();

#endif // MEGAFM_CMAKE_MIDIENGINE_H
