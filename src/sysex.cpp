#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"

#include "midi.h"
#include "nrpn.h"
#include "leds.h"
#include "setters.h"
#include "sysex.h"

int sysExDataIndex = 0;
bool _processingSysex = false;

bool processingSysex() { return _processingSysex; }

static void sysexWriteNRPN(int msg, int val) {
	Serial.write(msg >> 7);   // parameter MSB
	Serial.write(msg & 0x7F); // parameter LSB
	Serial.write(val >> 7);   // data MSB
	Serial.write(val & 0x7F); // data LSB
}

void sysExReset() { sysExDataIndex = 0; }

void sysExAppendByte(byte b) {
	if (sysExDataIndex < MAX_SYSEX_DATA_LENGTH) {
		sysexBuffer[sysExDataIndex++] = b;
	} else {
		sysExExitStatus(SYSEX_STATUS_LENGTH_ERROR);
	}
}

void sysExExitStatus(byte error) {
	resetMidiReadStatus();
	sysExDataIndex = 0;
	digit(0, 5);     // S(ysEx)
	digit(1, error); // error code, 0=ok, 1=byte error, 2=header mismatch, 3=length error, 4=dump length error
	lastNumber = -1;
	showPresetNumberTimeout = 12000;
	_processingSysex = false;
}

void handleIncomingSysEx() {
	// Handle the received SysEx data in sysExData array with length sysExDataIndex

	// Need at least seven bytes:
	// 240 0 33 68 [91|93] [arpLen] [2*arpLen bytes for note offsets 0-255] [4*N bytes for 0<=N NRPN messages] 24

	// Ensure we do not send midi or nrpn feedback on changes we make while processing the sysex message
	// All exit paths from this function go through sysExExitStatus which sets processingSysex back to false.
	_processingSysex = true;

	if (sysExDataIndex < 7) {
		sysExExitStatus(SYSEX_STATUS_LENGTH_ERROR);
		digit(0, sysExDataIndex);
		return;
	}

	// check if it's a SysEx message for us ( - we do the same as the firmware and take the
	// default from hex2sys = \x00\x21\x44 = 00 33 68)
	if (sysexBuffer[1] != 0 || sysexBuffer[2] != 33 || sysexBuffer[3] != 68) {
		// not for us, ignore the rest of the message
		sysExExitStatus(SYSEX_STATUS_HEADER_MISMATCH);
		return;
	}

	byte arpLen = 0;

	if (sysexBuffer[4] == 91) {
		arpLen = sysexBuffer[5];

		if (arpLen > 16 || sysExDataIndex < 7 + arpLen * 2 || (sysExDataIndex - 7 - 2 * arpLen) % 4 != 0) {
			sysExExitStatus(SYSEX_STATUS_DUMP_LENGTH_ERROR);
			return;
		}

		seqLength = arpLen;
		for (int i = 0; i < seqLength; i++) {
			seq[i] = (sysexBuffer[6 + i * 2] << 7) | sysexBuffer[6 + i * 2 + 1];
		}

		for (int i = 6 + arpLen * 2; i < sysExDataIndex - 1; i += 4) {
			// Decode NRPN from byte MSB i and LSB i+1
			int nrpn = (sysexBuffer[i] << 7) | sysexBuffer[i + 1];
			// Decode value from byte MSB i+3 and LSB i+2
			int value = (sysexBuffer[i + 2] << 7) | sysexBuffer[i + 3];
			handleNRPN(nrpn, value);
		}
		sysExExitStatus(SYSEX_STATUS_OK);
	} else if (sysexBuffer[4] == 93) {
		// Arp dump: payload is 1 byte length, followed by that many notes (2 bytes each MSB/LSB, 255=rest)
		if (sysExDataIndex < 7) {
			sysExExitStatus(SYSEX_STATUS_DUMP_LENGTH_ERROR);
			return;
		}
		arpLen = sysexBuffer[5];
		if (arpLen > 16 || sysExDataIndex != 7 + arpLen * 2) {
			sysExExitStatus(SYSEX_STATUS_DUMP_LENGTH_ERROR);
			return;
		}
		seqLength = arpLen;
		for (int i = 0; i < arpLen; i++) {
			seq[i] = (sysexBuffer[6 + i * 2] << 7) | sysexBuffer[6 + i * 2 + 1];
		}
		sysExExitStatus(SYSEX_STATUS_OK);
	} else {
		// Unknown command, ignore
		sysExExitStatus(SYSEX_STATUS_UNKNOWN_COMMAND);
	}
}

void dumpArpAsSysEx() {
	// SysEx header: F0 00 21 44 [command=94 = arp dump]
	Serial.write(0xF0);
	Serial.write(0);
	Serial.write(33);
	Serial.write(68);
	Serial.write(94);

	// Payload: length (1 byte) + note data (2*length bytes)
	Serial.write(seqLength);
	for (int i = 0; i < seqLength; i++) {
		Serial.write(seq[i] >> 7);
		Serial.write(seq[i] & 0x7F);
	}

	// SysEx end
	Serial.write(0xF7);
}

void dumpPresetAsSysEx() {
	// Total NRPN count breakdown:
	//   15  LFO settings (shape/looping/retrig/clock/vel-mod-at)
	//   13  Knob parameters (fine, glide, LFO rates/depths, fat, volume, feedback, algo, notePriority)
	//   10  Global settings (brightness, thru, pickup, stereoCh3, mpe, fatSpread, ignoreVol, fatMode, voiceMode,
	//   octOffset)
	//    4  Arp (mode, clock, rate, range)
	//    3  Vibrato (clock, rate, depth)
	//  135  LFO links (3 LFOs × 45 pots, skipping unused slots 3,12,21,30,44,45)
	//   40  Operators (4 ops × 10 params each)
	// ----
	//  220  total

	// SysEx header: F0 00 21 44 [command=92 = preset dump]
	Serial.write(0xF0);
	Serial.write(0);
	Serial.write(33);
	Serial.write(68);
	Serial.write(92);

	// First the arp data

	// Payload: length (1 byte) + note data (2*length bytes)
	Serial.write(seqLength);
	for (int i = 0; i < seqLength; i++) {
		Serial.write(seq[i] >> 7);
		Serial.write(seq[i] & 0x7F);
	}

	// Then the NRPN data for all the other settings

	// LFO shapes (100-102)
	for (int i = 0; i < 3; i++) {
		byte shape = lfoShape[i];
		byte val;
		if (shape == kSquare)
			val = invertedSquare[i] ? 1 : 0;
		else if (shape == kTriangle)
			val = 2;
		else if (shape == kSaw)
			val = 3 + (invertedSaw[i] ? 1 : 0);
		else // kRandom
			val = 5 + (noiseTableLength[i] - 2);
		sysexWriteNRPN(NRPN_LFO_SHAPE + i, val);
	}

	// LFO looping (103-105), retrig (106-108), MIDI sync (109-111)
	// MIDI sync must go first, so the host knows how to interpret the rate
	for (int i = 0; i < 3; i++)
		sysexWriteNRPN(NRPN_LFO_CLOCK_SYNC + i, lfoClockEnable[i]);
	for (int i = 0; i < 3; i++)
		sysexWriteNRPN(NRPN_LFO_LOOPING + i, looping[i]);
	for (int i = 0; i < 3; i++)
		sysexWriteNRPN(NRPN_LFO_RETRIG + i, retrig[i]);

	// LFO vel/mod/at (112-114)
	sysexWriteNRPN(NRPN_LFO_VEL, lfoVel);
	sysexWriteNRPN(NRPN_LFO_MOD, lfoMod);
	sysexWriteNRPN(NRPN_LFO_AT, lfoAt);

	// Fine tune (220) and Glide (221)
	// fine is 0-255; movedPot(KNOB_VOLUME, fine) with voiceHeld restores it directly.
	// glide is 0-15; movedPot(KNOB_FAT, glide<<4) with voiceHeld restores it (glide = data>>4).
	sysexWriteNRPN(NRPN_FINE_TUNE, fine);
	sysexWriteNRPN(NRPN_GLIDE, glide << 4);

	// LFO rates and depths
	sysexWriteNRPN(NRPN_LFO1_RATE, fmBase[36]);  // LFO1 rate
	sysexWriteNRPN(NRPN_LFO2_RATE, fmBase[38]);  // LFO2 rate
	sysexWriteNRPN(NRPN_LFO3_RATE, fmBase[40]);  // LFO3 rate
	sysexWriteNRPN(NRPN_LFO1_DEPTH, fmBase[37]); // LFO1 depth
	sysexWriteNRPN(NRPN_LFO2_DEPTH, fmBase[39]); // LFO2 depth
	sysexWriteNRPN(NRPN_LFO3_DEPTH, fmBase[41]); // LFO3 depth

	// Fat, Volume, Feedback, Algorithm
	// volume: movedPot(KNOB_VOLUME, data) stores vol = 128-(data>>1), so data = (128-vol)<<1
	sysexWriteNRPN(NRPN_FAT, fmBase[50]);
	sysexWriteNRPN(NRPN_VOLUME, (128 - vol) << 1);
	sysexWriteNRPN(NRPN_FEEDBACK, fmBase[43]);
	sysexWriteNRPN(NRPN_ALGORITHM, fmBase[42]);

	// Note priority
	sysexWriteNRPN(NRPN_NOTE_PRIORITY, notePriority);

	// Global settings
	sysexWriteNRPN(NRPN_SET_BRIGHTNESS, EEPROM.read(3965));  // brightness (0-15)
	sysexWriteNRPN(NRPN_SET_MIDI_THRU, thru);                // MIDI thru
	sysexWriteNRPN(NRPN_SHOW_FEEDBACK, showMidiFeedback);    // MIDI feedback
	sysexWriteNRPN(NRPN_SET_PICKUP_MODE, EEPROM.read(3954)); // pickup mode
	sysexWriteNRPN(NRPN_SET_STEREO_CH3, stereoCh3);          // stereo ch3
	sysexWriteNRPN(NRPN_SET_MPE_MODE, mpe);                  // MPE mode
	sysexWriteNRPN(NRPN_SET_FAT_SPREAD, fatSpreadMode);      // fat spread mode
	sysexWriteNRPN(NRPN_SET_IGNORE_VOL, ignoreVolume);       // ignore preset volume
	sysexWriteNRPN(NRPN_SET_FAT_MODE, fatMode);              // fat mode (0=octave, 1=semitone)
	sysexWriteNRPN(NRPN_SET_VOICE_MODE, (byte)voiceMode);    // voice mode (0-5)
	sysexWriteNRPN(NRPN_SET_OCT_OFFSET, octOffset);          // octave offset (0-3)

	// Arp; clock first to let the host know how to interpret the rate
	sysexWriteNRPN(NRPN_ARP_CLOCK_SYNC, arpClockEnable); // arp MIDI clock sync
	sysexWriteNRPN(NRPN_ARP_MODE, arpMode);              // arp mode (0-7)
	sysexWriteNRPN(NRPN_ARP_RATE, fmBase[46]);           // arp rate
	sysexWriteNRPN(NRPN_ARP_RANGE, fmBase[47]);          // arp range

	// Vibrato, clock first to let the host know how to interpret the rate
	sysexWriteNRPN(NRPN_VIB_CLOCK_SYNC, vibratoClockEnable); // vibrato MIDI clock sync
	sysexWriteNRPN(NRPN_VIB_RATE, fmBase[48]);               // vibrato rate
	sysexWriteNRPN(NRPN_VIB_DEPTH, fmBase[49]);              // vibrato depth

	// LFO links (1000-1002): one NRPN per (lfo, targetPot), value = (targetPot<<1)|linked
	for (int i = 0; i < 3; i++) {
		for (int targetPot = 0; targetPot < 51; targetPot++) {
			// Exclude operator rate scaling (3/12/21/30) and unused pot/fmBase 44, 45
			if ((targetPot != 3) && (targetPot != 12) && (targetPot != 21) && (targetPot != 30) && (targetPot != 44) &&
			    (targetPot != 45))
				sysexWriteNRPN(NRPN_LFO_LINK + i, (targetPot << 1) | linked[i][targetPot]);
		}
	}

	// Operator 1 (2000-2009)
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_DETUNE, fmBase[0]);                    // detune
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_MULT, fmBase[1]);                      // multiple
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_LEVEL, fmBase[2]);                     // level
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_ATTACK, fmBase[4]);                    // attack
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_DECAY, fmBase[5]);                     // decay
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_SUSTAIN_LVL, fmBase[7]);               // sustain
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_SUSTAIN_RATE, fmBase[6]);              // sustain rate
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_RELEASE, fmBase[8]);                   // release
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_ENV_MODE, getOperatorEnvelopeMode(0)); // envelope mode (0-2)
	sysexWriteNRPN(NRPN_OP1_BASE + NRPN_OP_RATE_SCALE, fmBase[3]);                // rate scaling (raw: 0,64,128,192)

	// Operator 2 (3000-3009)
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_DETUNE, fmBase[18]);                   // detune
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_MULT, fmBase[19]);                     // multiple
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_LEVEL, fmBase[20]);                    // level
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_ATTACK, fmBase[22]);                   // attack
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_DECAY, fmBase[23]);                    // decay
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_SUSTAIN_LVL, fmBase[25]);              // sustain
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_SUSTAIN_RATE, fmBase[24]);             // sustain rate
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_RELEASE, fmBase[26]);                  // release
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_ENV_MODE, getOperatorEnvelopeMode(1)); // envelope mode (0-2)
	sysexWriteNRPN(NRPN_OP2_BASE + NRPN_OP_RATE_SCALE, fmBase[12]);               // rate scaling (raw: 0,64,128,192)

	// Operator 3 (4000-4009)
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_DETUNE, fmBase[9]);                    // detune
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_MULT, fmBase[10]);                     // multiple
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_LEVEL, fmBase[11]);                    // level
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_ATTACK, fmBase[13]);                   // attack
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_DECAY, fmBase[14]);                    // decay
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_SUSTAIN_LVL, fmBase[16]);              // sustain
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_SUSTAIN_RATE, fmBase[15]);             // sustain rate
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_RELEASE, fmBase[17]);                  // release
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_ENV_MODE, getOperatorEnvelopeMode(2)); // envelope mode (0-2)
	sysexWriteNRPN(NRPN_OP3_BASE + NRPN_OP_RATE_SCALE, fmBase[21]);               // rate scaling (raw: 0,64,128,192)

	// Operator 4 (5000-5009)
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_DETUNE, fmBase[27]);                   // detune
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_MULT, fmBase[28]);                     // multiple
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_LEVEL, fmBase[29]);                    // level
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_ATTACK, fmBase[31]);                   // attack
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_DECAY, fmBase[32]);                    // decay
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_SUSTAIN_LVL, fmBase[34]);              // sustain
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_SUSTAIN_RATE, fmBase[33]);             // sustain rate
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_RELEASE, fmBase[35]);                  // release
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_ENV_MODE, getOperatorEnvelopeMode(3)); // envelope mode (0-2)
	sysexWriteNRPN(NRPN_OP4_BASE + NRPN_OP_RATE_SCALE, fmBase[30]);               // rate scaling (raw: 0,64,128,192)

	Serial.write(0xF7); // SysEx end

	digit(0, 5); // S(ysEx)
	digit(1, 0); // O(ut)
	lastNumber = -1;
	showPresetNumberTimeout = 12000;
}
