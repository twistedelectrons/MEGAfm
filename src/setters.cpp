#include <Arduino.h>
#include <EEPROM.h>
#include "constants.h"
#include "megafm.h"
#include "FM.h"
#include "buttons.h"
#include "leds.h"

void setThru() {
	byte temp = EEPROM.read(3950);
	bitWrite(temp, 0, !thru);
	EEPROM.update(3950, temp);
}

void setLFO1Clock() {
	byte temp = EEPROM.read(3953);
	bitWrite(temp, 0, lfoClockEnable[0]);
	EEPROM.update(3953, temp);
}

void setLFO2Clock() {
	byte temp = EEPROM.read(3953);
	bitWrite(temp, 1, lfoClockEnable[1]);
	EEPROM.update(3953, temp);
}

void setLFO3Clock() {
	byte temp = EEPROM.read(3953);
	bitWrite(temp, 2, lfoClockEnable[2]);
	EEPROM.update(3953, temp);
}

void setVibratoClock() {
	byte temp = EEPROM.read(3953);
	bitWrite(temp, 3, vibratoClockEnable);
	EEPROM.update(3953, temp);
}

void setArpClock() {
	byte temp = EEPROM.read(3953);
	bitWrite(temp, 4, arpClockEnable);
	EEPROM.update(3953, temp);
}

void setArpStep(byte step, byte value) {
	if (step > 15)
		// Ignore errors
		return;
	if (step > 0)
		// step[0] == 127 always = the note played
		seq[step] = value;
	if (step >= seqLength) {
		for (int i = seqLength; i < step; i++) {
			// Fill previous value if we extend the sequence
			seq[i] = seq[seqLength - 1];
		}
		seqLength = step + 1;
		digit(0, 17); // A(rp)
		digit(1, 17); // A(ppend)
	} else {
		digit(0, 17); // A(rp)
		digit(1, 16); // r(eplace)
	}
	lastNumber = -1;
	showPresetNumberTimeout = 12000;
}

void setFatSpreadMode() {
	byte temp = EEPROM.read(3968);
	bitWrite(temp, 0, fatSpreadMode);
	EEPROM.write(3968, fatSpreadMode);
}

void setLFO1Vel() { EEPROM.update(3961, lfoVel); }

void setLFO2Mod() { EEPROM.update(3962, lfoMod); }

void setLFO3Aftertouch() { EEPROM.update(3963, lfoAt); }

void setIgnoreVolume() {
	byte temp = EEPROM.read(3950);
	bitWrite(temp, 1, ignoreVolume);
	EEPROM.update(3950, temp);
}

void setMPEMode() { EEPROM.update(3960, mpe); }

void setPickupMode() { EEPROM.update(3954, pickupMode); }

void setStereoCh3() { EEPROM.update(3966, stereoCh3); }

void setFatMode() {
	byte temp = EEPROM.read(3953);
	bitWrite(temp, 5, fatMode);
	EEPROM.update(3953, temp);
}

void setNotePriority() { EEPROM.write(3967, notePriority); }

void setBrightness(byte brightness) {
	mydisplay.setIntensity(0, brightness); // 15 = brightest
	EEPROM.write(3965, brightness);
}

void setOperatorEnvelopeMode(byte op, kEnvelopeMode mode) {
	switch (mode) {
		case kEnvelopeOff:
			if (bitRead(SSEG[op], 1))
				// turn off
				setSSEG(op, 1, 0);
			break;
		case kEnvelopeOnce:
			if (!bitRead(SSEG[op], 1))
				// turn on, set mode
				setSSEG(op, 1, 1);
			setSSEG(op, 0, 0);
			break;
		case kEnvelopPingPong:
			if (!bitRead(SSEG[op], 1))
				// turn on, set mode
				setSSEG(op, 1, 1);
			setSSEG(op, 0, 1);
			break;
	}
}

kEnvelopeMode getOperatorEnvelopeMode(byte op) {
	if (bitRead(SSEG[op], 1)) {
		if (bitRead(SSEG[op], 0)) {
			return kEnvelopPingPong;
		} else {
			return kEnvelopeOnce;
		}
	}
	return kEnvelopeOff;
}

void setLFOShape(byte lfo, byte value) {
	switch (value) {
		case 0:
			// square
			invertedSquare[lfo] = false;
			lfoShape[lfo] = kSquare;
			break;
		case 1:
			// inverted square
			invertedSquare[lfo] = true;
			lfoShape[lfo] = kSquare;
			break;
		case 2:
			// triangle
			lfoShape[lfo] = kTriangle;
			break;
		case 3:
			// saw
			invertedSaw[lfo] = false;
			lfoShape[lfo] = kSaw;
			break;
		case 4:
			// inverted saw
			invertedSaw[lfo] = true;
			lfoShape[lfo] = kSaw;
			break;
		case 5:
			// random
			lfoShape[lfo] = kRandom;
			noiseTableLength[lfo] = 2;
			break;
		case 6:
			lfoShape[lfo] = kRandom;
			noiseTableLength[lfo] = 3;
			break;
		case 7:
			lfoShape[lfo] = kRandom;
			noiseTableLength[lfo] = 4;
			break;
		case 8:
			lfoShape[lfo] = kRandom;
			noiseTableLength[lfo] = 5;
			break;
	}
	showLfoWaveform(lfo);
}
