#ifndef MEGAFM_CMAKE_MIDI_H
#define MEGAFM_CMAKE_MIDI_H

void handleAftertouch(byte channel, byte val);
void handleClock();
void handleBendy(byte channel, int bend);
void handleStop();
void handleStart();
void handleProgramChange(byte channel, byte program);
// void handleNoteOn(byte channel, byte note, byte velocity);
// void handleNoteOff(byte channel, byte note, byte velocity);
void handleControlChange(byte channel, byte number, byte val);
void pedalUp();
void pedalDown();
void dumpPresetAsSysEx();
void dumpArpAsSysEx();
void resyncArpLfo();
void sendControlChange(byte number, byte value, byte channel);
void sendNoteOff(byte note, byte velocity, byte channel);
void sendTool(byte note, byte velocity);
void sendNoteOn(byte note, byte velocity, byte channel);
void midiRead();
void resetMidiReadStatus();

#endif // MEGAFM_CMAKE_MIDI_H
