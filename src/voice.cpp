#include <Arduino.h>
#include "megafm.h"
#include "voice.h"
#include "midi.h"
#include "pitchEngine.h"
#include "arp.h"

static int orderIndx;
static byte noteOrder[128];

void addNote(byte note) {
	emptyStack = false;
	arpNotes[note] = 1;

	// sort lastNotePriority
	if (notePriority == 2) {
		orderIndx = (orderIndx + 1) % 20;
		noteOrder[orderIndx] = note;
	}
}

void removeNote(byte note) {
	arpNotes[note] = 0;

	emptyStack = true;
	for (int i = 0; i < 128; i++) {
		if (arpNotes[i])
			emptyStack = false;
	}
}

void resetVoices() {
	arpIndex = 0;

	for (int i = 0; i < 12; i++) {
		handleBendy(i, 0);
		voiceSlots[i] = 0;
		noteOfVoice[i] = 0;
		ym.noteOff(i);

		setNote(i, 0);
		skipGlide(i);
	}

	heldKeys = 0;
	clearNotes();

	for (int i = 0; i < 128; i++) {
		noteOrder[i] = 0;
	}
	for (int i = 0; i < 128; i++) {
		lastNotey[i] = 0;
	}
}

byte getLast() {
	int8_t noteIndex;

	for (int i = 0; i < 128; i++) {
		noteIndex = noteOrder[mod(orderIndx - i, 128)];
		if (arpNotes[noteIndex]) {
			return noteIndex;
		}
	}
	return 0;
}

int mod(int a, int b) {
	int r = a % b;
	return r < 0 ? r + b : r;
}

byte getHigh() {
	int topNote = 0;
	// bool noteActive = false; // unused?

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
