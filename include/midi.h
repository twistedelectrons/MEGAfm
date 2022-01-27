//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_MIDI_H
#define MEGAFM_CMAKE_MIDI_H

void HandleAt(byte channel,byte val);
void handleClock();
void handleBendy(byte channel, int bend) ;
void handleStop();
void handleStart();
void HandlePc(byte channel,byte program);
void handleNoteOn(byte channel,byte note,byte velocity);
void handleNoteOff(byte channel,byte note,byte velocity);
void sendCC(byte number,int value);
void sendMidiButt(byte number,int value);
void HandleControlChange(byte channel,byte number,byte val);
void midiOut(byte note);
void pedalUp();
void pedalDown();
void dumpPreset();

#endif //MEGAFM_CMAKE_MIDI_H
