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
	if (heldKeys && arpMode && fmData[46]) {

		if (!sync || (sync && !arpClockEnable)) {
			arpCounter += fmData[46];
		}
	}
}

static int arpRootNote;

void arpFire() {

	if (heldKeys) {
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

		switch (voiceMode) {

			case kVoicingPoly12:
				robin = constrain(robin, 0, 11);
				robin++;
				if (robin > 11) {
					robin = 0;
				}
				if (robin == 0) {
					ym.noteOff(11);

				} else {
					ym.noteOff(robin - 1);
				}
				setNote(robin, lastNote);
				ym.noteOn(robin);

				break;

			case kVoicingDualCh3:
			case kVoicingWide3:
			case kVoicingWide4:
			case kVoicingWide6:

				robin = constrain(robin, 0, 5);

				robin++;
				if (robin > 5) {
					robin = 0;
				}
				if (robin < 1) {
					ym.noteOff(5);
					ym.noteOff(11);
				} else {
					ym.noteOff(robin - 1);
					ym.noteOff(robin + 5);
				}
				setNote(robin, lastNote);
				ym.noteOn(robin);
				setNote(robin + 6, lastNote);
				ym.noteOn(robin + 6);

				break;

			case kVoicingUnison:
				for (int i = 0; i < 12; i++) {
					ym.noteOff(i);
					if (lastNote) {
						setNote(i, lastNote);
						ym.noteOn(i);
					}
				}
				break;
		}
	}
	if (!heldKeys) {
		for (int i = 0; i < 12; i++) {
			ym.noteOff(i);
		}
	}
}
