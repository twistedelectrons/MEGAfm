#include <Arduino.h>
#include "megafm.h"
#include "voice.h"
#include "midi.h"
#include "pitchEngine.h"
#include "arp.h"

int orderIndx;
byte noteOrder[128];

void addNote(byte note) {
  emptyStack = false;
  arpNotes[note] = 1;


//sort lastNotePriority
  if (notePriority == 2) {
    orderIndx = (orderIndx + 1) % 20;
    noteOrder[orderIndx] = note;
  }

}

void removeNote(byte note) {
  arpNotes[note] = 0;

  emptyStack = true;
  for (int i = 0; i < 128; i++) {
    if (arpNotes[i]) emptyStack = false;
  }
}

void resetVoices() {

  clearStack();

  for (int i = 0; i < 12; i++) {
    handleBendy(i, 0);
    voiceSlots[i] = noteOfVoice[i] = 0;
    ym.noteOff(i);

    setNote(i, 0);
    skipGlide(i);

  }

  heldKeys = 0;
  clearNotes();
  clearKeys();
  for (int i = 0; i < 128; i++) { noteOrder[i] = 0; }
  for (int i = 0; i < 40; i++) { lastNotey[i] = 0; }
}

void killVoice(byte note) {
  //scan through the noteOfVoices and kill the voice associated to it
  for (int i = 0; i < 12; i++) {
    if (noteOfVoice[i] == note) {
      noteOfVoice[i] = 0;
      voiceSlots[i] = 0;
      ym.noteOff(i);
      i = 99;//abort loop
    }
  }
}

byte noteToVoice(byte note) {
  for (int i = 0; i < 12; i++) {
    if (noteOfVoice[i] == note) {
      i = 99;//abort loop
      return (i);
    }
  }
}

void clearKeys() {
  for (int i = 0; i < 128; i++) {
    // keyOn[i] = 0; // unused?
    // noteToChannel[i] = 0; // unused?
  }
}

void clearStack() {
  for (int i = 0; i < 128; i++) {
    // stack[i] = 0; // unused?
  }

  arpIndex = 0;
}

byte getLast() {
  int8_t noteIndx;

  for (int i = 0; i < 128; i++) {
    noteIndx = noteOrder[mod(orderIndx - i, 128)];
    if (arpNotes[noteIndx]) {
      return (noteIndx);
    }
  }
}

int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

byte getHigh() {
  int topNote = 0;
  bool noteActive = false;

  for (int i = 0; i < 128; i++) {
    if (arpNotes[i]) {
      topNote = i;
    }
  }

  return (topNote);
}

byte getLow() {
  int bottomNote = 0;

  for (int i = 127; i >= 0; i--) {
    if (arpNotes[i]) {
      bottomNote = i;
    }
  }

  return (bottomNote);
}
