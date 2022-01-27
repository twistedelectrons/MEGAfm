//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_MIDIENGINE_H
#define MEGAFM_CMAKE_MIDIENGINE_H

void sendControlChange(byte number,byte value,byte channel);
void sendNoteOff(byte note,byte velocity,byte channel);
void sendNoteOn(byte note,byte velocity,byte channel);
void midiRead();

#endif //MEGAFM_CMAKE_MIDIENGINE_H
