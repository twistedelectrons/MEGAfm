//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_VOICE_H
#define MEGAFM_CMAKE_VOICE_H

void addNote(byte note);
void removeNote(byte note);
void resetVoices();
void killVoice(byte note);
byte noteToVoice(byte note);
void clearKeys();
void clearStack();
byte getLast();
int mod(int a, int b) ;
byte getHigh();
byte getLow();

#endif //MEGAFM_CMAKE_VOICE_H
