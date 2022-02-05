#ifndef MEGAFM_CMAKE_VOICE_H
#define MEGAFM_CMAKE_VOICE_H

void addNote(byte note);
void removeNote(byte note);
void resetVoices();
byte getLast();
int mod(int a, int b) ;
byte getHigh();
byte getLow();

#endif //MEGAFM_CMAKE_VOICE_H
