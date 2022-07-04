#include <Arduino.h>
#include "arp.h"
#include "megafm.h"
#include "pitchEngine.h"

static bool arpAlternator = false;

void clearNotes() {
	arpIndex = 0;
	arpAlternator = true;

	switch (arpMode) {

		case 2:
			arpIndex = 127;
			break;
	}

	for (int i = 0; i < 128; i++) {
		arpNotes[i] = 0;
	}

	emptyStack = true;
}

void arpReset() {

	if (arpMode == 2) {
		arpStep = fmData[47] >> 6;
		arpIndex = 127;
	} else {
		arpStep = 0;
		arpIndex = 0;
	}
	arpAlternator = false;
}

void nextArpStep() { // octave shift

	if (arpMode == 2) {
		arpStep--;
		if (arpStep < 0)
			arpStep = fmData[47] >> 6;

	} else {
		arpStep++;
		if (arpStep > fmData[47] >> 6)
			arpStep = 0;
	}
}

void arpTick() {
	if (((heldKeys) || ((pedal) && (!emptyStack))) && (arpMode) && (voiceMode == kVoicingUnison) && (fmData[46])) {

		if ((!sync) || ((sync) && (!arpClockEnable))) {
			arpCounter += fmData[46];
		}
	}
}

static int arpRootNote;

void arpFire() {
	//  byte nextNote = 0; // unused

	if ((heldKeys) || (pedal)) {
		if (retrig[0])
			lfoStep[0] = 0;
		if (retrig[1])
			lfoStep[1] = 0;
		if (retrig[2])
			lfoStep[2] = 0;

		switch (arpMode) {

			case 1: // going Up
				arpIndex++;
				while (!arpNotes[arpIndex]) {
					arpIndex++;
					if (arpIndex > 127) {
						arpIndex = -1;
						nextArpStep();
					}
				}
				lastNote = arpIndex + (arpStep * 12);
				break;

			case 2: // going down
				arpIndex--;
				while (!arpNotes[arpIndex]) {
					arpIndex--;
					if (arpIndex < 0) {
						arpIndex = 127;
						nextArpStep();
					}
				}
				lastNote = arpIndex + (arpStep * 12);
				break;

			case 3: // Up/Down

				if (arpAlternator) {
					arpIndex++;
				} else {
					arpIndex--;
				}
				constrain(arpIndex, 0, 127);
				while ((!arpNotes[arpIndex])) {

					if (arpAlternator) {
						arpIndex++;
						if (arpIndex > 127) {
							arpIndex = 127;
							arpAlternator = 0;
						}
					} else {
						arpIndex--;
						if (arpIndex < 0) {
							arpIndex = 0;
							arpAlternator = 1;
							nextArpStep();
						}
					}
				}
				lastNote = arpIndex + (arpStep * 12);

				break;

			case 4:

				for (int i = 0; i < random(10); i++) {
					arpIndex++;
					while (!arpNotes[arpIndex]) {
						arpIndex++;
						if (arpIndex > 127) {
							arpIndex = -1;
						}
					}
				}
				arpStep = random((fmData[47] >> 6) + 1);
				lastNote = arpIndex + (arpStep * 12);
				break;
			case 5:
				lastNote = random(20, 100);
				break;

			case 6:
			case 7:

				if (seq[seqStep] != 255) {
					lastNote = arpRootNote + rootNote + (seq[seqStep] - 127) + (arpStep * 12);
				}

				seqStep++;
				if (seqStep >= seqLength) {
					seqStep = 0;

					arpIndex++;
					while (!arpNotes[arpIndex]) {
						arpIndex++;
						if (arpIndex > 127) {
							arpIndex = -1;
							nextArpStep();
						}
					}
					arpRootNote = arpIndex - rootNote;
				}
				break;
		}

		for (int i = 0; i < 12; i++) {
			ym.noteOff(i);
			if (lastNote) {
				setNote(i, lastNote);
				ym.noteOn(i);
			}
		}
	}
}
