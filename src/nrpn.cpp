#include <Arduino.h>
#include "constants.h"
#include "megafm.h"
#include "pots.h"
#include "buttons.h"
#include "voice.h"
#include "leds.h"
#include "midi.h"
#include "nrpn.h"
#include "setters.h"
#include "preset.h"
#include "sysex.h"

int nrpn_msg = 0;
int nrpn_data = 0;
byte nrpn_state = 0;

// Compact index for the 88 distinct NRPN message numbers used by this firmware:
//   100-114 → 0-14, 200-209 → 15-24, 220-232 → 25-37, 300-303 → 38-41,
//   500-502 → 42-44, 1000-1002 → 45-47, 2000-2009 → 48-57, 3000-3009 → 58-67,
//   4000-4009 → 68-77, 5000-5009 → 78-87
static int16_t lastNRPN[88];

static int nrpnIndex(int msg) {
	if (msg >= NRPN_LFO_SHAPE && msg <= NRPN_LFO_AT)
		return msg - NRPN_LFO_SHAPE;
	if (msg >= NRPN_SET_BRIGHTNESS && msg <= NRPN_SET_OCT_OFFSET)
		return msg - 185;
	if (msg >= NRPN_FINE_TUNE && msg <= NRPN_NOTE_PRIORITY)
		return msg - 195;
	if (msg >= NRPN_ARP_MODE && msg <= NRPN_ARP_RANGE)
		return msg - 262;
	if (msg >= NRPN_VIB_CLOCK_SYNC && msg <= NRPN_VIB_DEPTH)
		return msg - 458;
	if (msg >= NRPN_LFO_LINK && msg <= NRPN_LFO_LINK + 2)
		return msg - 955;
	if (msg >= NRPN_OP1_BASE && msg <= NRPN_OP1_BASE + NRPN_OP_RATE_SCALE)
		return msg - 1952;
	if (msg >= NRPN_OP2_BASE && msg <= NRPN_OP2_BASE + NRPN_OP_RATE_SCALE)
		return msg - 2942;
	if (msg >= NRPN_OP3_BASE && msg <= NRPN_OP3_BASE + NRPN_OP_RATE_SCALE)
		return msg - 3932;
	if (msg >= NRPN_OP4_BASE && msg <= NRPN_OP4_BASE + NRPN_OP_RATE_SCALE)
		return msg - 4922;
	return -1;
}

void initLastNRPN() {
	for (int i = 0; i < 88; i++) {
		lastNRPN[i] = -1;
	}
}

void sendNRPN(int msg, int value) {
	if (processingSysex())
		return;
	int idx = nrpnIndex(msg);
	if (idx >= 0 && lastNRPN[idx] == value)
		return;
	if (idx >= 0)
		lastNRPN[idx] = value;

	sendControlChange(99, msg >> 7, masterChannelOut);     // NRPN MSB
	sendControlChange(98, msg & 0x7F, masterChannelOut);   // NRPN LSB
	sendControlChange(6, value >> 7, masterChannelOut);    // Data Entry MSB
	sendControlChange(38, value & 0x7F, masterChannelOut); // Data Entry LSB
}

void handleNRPN(int msg, int int_val) {
	// NRPN values are sent as 14 bit values, but we only mostly only use the lower 8 bits
	byte byte_val = (byte)(int_val & 0xFF);
	bool bool_val = (int_val > 0);

	// (temporarily) disable pickup mode
	bool oldPickupMode = pickupMode;
	pickupMode = false;

	// Update the deduplication cache with the received value so that a subsequent
	// sendNRPN() with the same value is suppressed — prevents echoing a received
	// message back to the sender (e.g. DAW sends NRPN → firmware applies it →
	// dumpPreset() would re-send the same value without this guard).
	int idx = nrpnIndex(msg);
	if (idx >= 0)
		lastNRPN[idx] = int_val;

	if (msg == NRPN_DUMP_CURRENT_SETTINGS) {
		dumpPresetAsSysEx();
	} else if (msg == NRPN_CHANGE_PROGRAM) {
		if ((int_val >= 0) && (int_val < 600)) {
			bank = int_val / 100;
			preset = int_val % 100;
			loadPreset();
		}
	} else if ((msg >= NRPN_LFO_SHAPE) && (msg <= NRPN_LFO_AT)) {
		if ((msg >= NRPN_LFO_SHAPE) && (msg <= NRPN_LFO_SHAPE + 2)) {
			// Shape LFO1: 100, LFO2: 101, LFO3: 102
			selectedLfo = (byte)(msg - NRPN_LFO_SHAPE);
			setLFOShape(selectedLfo, byte_val);
		} else if ((msg >= NRPN_LFO_LOOPING) && (msg <= NRPN_LFO_LOOPING + 2)) {
			// Looping LFO1: 103, LFO2: 104, LFO3: 105
			selectedLfo = (byte)(msg - NRPN_LFO_LOOPING);
			looping[selectedLfo] = bool_val;
		} else if ((msg >= NRPN_LFO_RETRIG) && (msg <= NRPN_LFO_RETRIG + 2)) {
			// Retrig LFO1: 106, LFO2: 107, LFO3: 108
			selectedLfo = (byte)(msg - NRPN_LFO_RETRIG);
			retrig[selectedLfo] = bool_val;
		} else if ((msg >= NRPN_LFO_CLOCK_SYNC) && (msg <= NRPN_LFO_CLOCK_SYNC + 2)) {
			// MIDI Sync LFO1: 109, LFO2: 110, LFO3: 111
			selectedLfo = (byte)(msg - NRPN_LFO_CLOCK_SYNC);
			lfoClockEnable[selectedLfo] = bool_val;
			switch (selectedLfo) {
				case 0:
					setLFO1Clock();
					break;
				case 1:
					setLFO2Clock();
					break;
				case 2:
					setLFO3Clock();
					break;
			}
			showOnOff(bool_val);
		} else if ((msg >= NRPN_LFO_VEL) && (msg <= NRPN_LFO_AT)) {
			selectedLfo = (byte)(msg - NRPN_LFO_VEL);
			if (msg == NRPN_LFO_VEL) {
				lfoVel = bool_val;
				setLFO1Vel();
			} else if (msg == NRPN_LFO_MOD) {
				lfoMod = bool_val;
				setLFO2Mod();
			} else if (msg == NRPN_LFO_AT) {
				lfoAt = bool_val;
				setLFO3Aftertouch();
			}
			showOnOff(bool_val);
			lastLfoSetting[selectedLfo] = bool_val;
		}
		lfoLedOn();
		showLfo();
	} else if (msg == NRPN_SET_BRIGHTNESS) {
		// Set brightness (0-15)
		if (byte_val < 16)
			setBrightness(byte_val);
	} else if (msg == NRPN_SET_MIDI_THRU) {
		// Set MIDI thru (0 = off, >0 = on)
		thru = bool_val;
		setThru();
		showOnOff(thru);
	} else if (msg == NRPN_SET_PICKUP_MODE) {
		// Set Pickup Mode (0 = off, >0 = on)
		pickupMode = bool_val;
		oldPickupMode = pickupMode;
		setPickupMode();
		showOnOff(pickupMode);
	} else if (msg == NRPN_SET_STEREO_CH3) {
		// Set Stereo Channel 3 Mode (0 = off, >0 = on)
		stereoCh3 = bool_val;
		setStereoCh3();
		showOnOff(stereoCh3);
	} else if (msg == NRPN_SET_MPE_MODE) {
		// Set MPE Mode (0 = off, >0 = on)
		mpe = bool_val;
		setMPEMode();
		showOnOff(mpe);
	} else if (msg == NRPN_SET_FAT_SPREAD) {
		// Set Fat Spread Mode (0 = off, >0 = on)
		fatSpreadMode = bool_val;
		setFatSpreadMode();
		showOnOff(fatSpreadMode);
	} else if (msg == NRPN_SET_IGNORE_VOL) {
		// Set Ignore Preset Volume (0 = off, >0 = on)
		ignoreVolume = bool_val;
		setIgnoreVolume();
		showOnOff(ignoreVolume);
	} else if (msg == NRPN_SET_FAT_MODE) {
		// Set Fat Mode (0 = semitone, >0 = octave)
		digit(0, 1);
		if (bool_val) {
			fatMode = FAT_MODE_OCTAVE;
			digit(1, 27); // o(ctave)
		} else {
			fatMode = FAT_MODE_SEMITONE;
			digit(1, 5); // S(emitone)
		}
		setFatMode();
		lastNumber = -1;
		showPresetNumberTimeout = 12000;
	} else if (msg == NRPN_SET_VOICE_MODE) {
		// Set Voice Mode (0-5 = (Poly12, Wide6, DualCh3, Unison, Wide4, Wide3)
		if (byte_val <= 5) {
			if (!mpe) {
				voiceMode = VoiceMode(byte_val);
				showVoiceMode(voiceMode);
				resetVoices();
			}
		}
	} else if (msg == NRPN_SET_OCT_OFFSET) {
		// Set octave offset (0-3)
		if (byte_val < 4) {
			octOffset = byte_val;
			ledNumber(octOffset);
		}
	} else if (msg == NRPN_FINE_TUNE) {
		// Set Tune (0-255)
		// fine = byte_val;
		// updateFine();
		// if (fine > 127) {
		// 	ledNumber(map(fine, 128, 255, 0, 32));
		// } else if (fine < 128) {
		// 	ledNumber(map(fine, 128, 0, 0, 32));
		// }
		voiceHeld = true;
		movedPot(KNOB_VOLUME, byte_val, 1);
		voiceHeld = false;
		movedFineKnob = false;
	} else if (msg == NRPN_GLIDE) {
		voiceHeld = true;
		movedPot(KNOB_FAT, byte_val, 1);
		voiceHeld = false;
	} else if (msg == NRPN_LFO1_RATE) {
		// LFO 1 Rate
		movedPot(KNOB_LFO1_RATE, byte_val, 1);
	} else if (msg == NRPN_LFO2_RATE) {
		// LFO 2 Rate
		movedPot(KNOB_LFO2_RATE, byte_val, 1);
	} else if (msg == NRPN_LFO3_RATE) {
		// LFO 3 Rate
		movedPot(KNOB_LFO3_RATE, byte_val, 1);
	} else if (msg == NRPN_LFO1_DEPTH) {
		// LFO 1 Depth
		movedPot(KNOB_LFO1_DEPTH, byte_val, 1);
	} else if (msg == NRPN_LFO2_DEPTH) {
		// LFO 2 Depth
		movedPot(KNOB_LFO2_DEPTH, byte_val, 1);
	} else if (msg == NRPN_LFO3_DEPTH) {
		// LFO 3 Depth
		movedPot(KNOB_LFO3_DEPTH, byte_val, 1);
	} else if (msg == NRPN_FAT) {
		// Fat
		movedPot(KNOB_FAT, byte_val, 1);
	} else if (msg == NRPN_VOLUME) {
		// Volume
		movedPot(KNOB_VOLUME, byte_val, 1);
	} else if (msg == NRPN_FEEDBACK) {
		// Feedback
		movedPot(KNOB_FEEDBACK, byte_val, 1);
	} else if (msg == NRPN_ALGORITHM) {
		// Algorithm
		movedPot(KNOB_ALGO, byte_val, 1);
	} else if (msg == NRPN_NOTE_PRIORITY) {
		notePriority = byte_val;
		setNotePriority();
	} else if (msg == NRPN_ARP_MODE) {
		if (byte_val < 8) {
			arpMode = byte_val;
			showArpMode();
			resetVoices();
		}
	} else if (msg == NRPN_ARP_CLOCK_SYNC) {
		arpClockEnable = bool_val;
		setArpClock();
		showOnOff(arpClockEnable);
	} else if (msg == NRPN_ARP_RATE) {
		// Arp Rate
		movedPot(KNOB_ARP_RATE, byte_val, 1);
	} else if (msg == NRPN_ARP_RANGE) {
		// Arp Range
		movedPot(KNOB_ARP_RANGE, byte_val, 1);
	} else if (msg == NRPN_ARP_SET_STEP) {
		setArpStep(int_val >> 8, int_val & 0xFF);
	} else if (msg == NRPN_VIB_CLOCK_SYNC) {
		vibratoClockEnable = bool_val;
		setVibratoClock();
		showOnOff(vibratoClockEnable);
	} else if (msg == NRPN_VIB_RATE) {
		// Vibrato Rate
		movedPot(KNOB_VIB_RATE, byte_val, 1);
	} else if (msg == NRPN_VIB_DEPTH) {
		// Vibrato Depth
		movedPot(KNOB_VIB_DEPTH, byte_val, 1);
	} else if ((msg >= NRPN_LFO_LINK) && (msg <= NRPN_LFO_LINK + 2)) {
		// Link LFO to target
		// msg 1000 = LFO1, 1001 = LFO2, 1002 = LFO3
		//
		// bit 0 = linked if 1 else unlinked
		// bit 1-7 = target pot (0-50)
		//
		// Example:
		// val is even (0, 2, 4, ...): not linked
		// val is odd (1, 3, 5, ...): linked
		// val is 0 or 1: target pot 0
		// val is 2 or 3: target pot 1
		//
		// So to link to pot 10, val should be 21 (10*2 + 1) and to unlink it val should be 20 (10*2 + 0)12
		byte lfo = msg - NRPN_LFO_LINK;
		bool isLinked = byte_val & 1;
		targetPot = byte_val >> 1;
		if (targetPot < 51) {
			linked[lfo][targetPot] = isLinked;
			showLink();
		}
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_DETUNE) {
		// Operator 1 Detune
		movedPot(FADER_DETUNE_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_MULT) {
		// Operator 1 Multiplier
		movedPot(FADER_MULT_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_LEVEL) {
		// Operator 1 Level
		movedPot(FADER_LEVEL_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_ATTACK) {
		// Operator 1 Attack
		movedPot(FADER_ATTACK_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_DECAY) {
		// Operator 1 Decay Rate
		movedPot(FADER_DECAY_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_SUSTAIN_LVL) {
		// Operator 1 Sustain Level
		movedPot(FADER_SUSTAIN_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_SUSTAIN_RATE) {
		// Operator 1 Sustain Rate
		movedPot(FADER_SUSTAIN_RATE_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_RELEASE) {
		// Operator 1 Release Rate
		movedPot(FADER_RELEASE_1, byte_val, 1);
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_ENV_MODE) {
		// Set envelope mode for operator 1, 0-2 = (off, forward, ping pong)
		if (byte_val < 3) {
			setOperatorEnvelopeMode(0, kEnvelopeMode(byte_val));
		}
	} else if (msg == NRPN_OP1_BASE + NRPN_OP_RATE_SCALE) {
		// Set rate scaling for operators 1 (0-3)
		// if (byte_val < 4) {
		// 	updateFMifNecessary(3);
		// 	fmBase[3] = byte_val << 6; // 0-3 becomes 0-192 (4 steps: 0, 64, 128, 192)
		// 	ledNumber(byte_val);
		// }
		loopHeld = true;
		movedPot(FADER_DETUNE_1, byte_val, 1);
		loopHeld = false;
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_DETUNE) {
		// Operator 2 Detune
		movedPot(FADER_DETUNE_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_MULT) {
		// Operator 2 Multiplier
		movedPot(FADER_MULT_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_LEVEL) {
		// Operator 2 Level
		movedPot(FADER_LEVEL_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_ATTACK) {
		// Operator 2 Attack
		movedPot(FADER_ATTACK_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_DECAY) {
		// Operator 2 Decay Rate
		movedPot(FADER_DECAY_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_SUSTAIN_LVL) {
		// Operator 2 Sustain Level
		movedPot(FADER_SUSTAIN_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_SUSTAIN_RATE) {
		// Operator 2 Sustain Rate
		movedPot(FADER_SUSTAIN_RATE_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_RELEASE) {
		// Operator 2 Release Rate
		movedPot(FADER_RELEASE_2, byte_val, 1);
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_ENV_MODE) {
		// Set envelope mode for operator 2, 0-2 = (off, forward, ping pong)
		if (byte_val < 3) {
			setOperatorEnvelopeMode(1, kEnvelopeMode(byte_val));
		}
	} else if (msg == NRPN_OP2_BASE + NRPN_OP_RATE_SCALE) {
		// Set rate scaling for operators 2 (0-3)
		// if (byte_val < 4) {
		// 	updateFMifNecessary(12);
		// 	fmBase[12] = byte_val << 6; // 0-3 becomes 0-192 (4 steps: 0, 64, 128, 192)
		// 	ledNumber(byte_val);
		// }
		loopHeld = true;
		movedPot(FADER_DETUNE_2, byte_val, 1);
		loopHeld = false;
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_DETUNE) {
		// Operator 3 Detune
		movedPot(FADER_DETUNE_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_MULT) {
		// Operator 3 Multiplier
		movedPot(FADER_MULT_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_LEVEL) {
		// Operator 3 Level
		movedPot(FADER_LEVEL_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_ATTACK) {
		// Operator 3 Attack
		movedPot(FADER_ATTACK_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_DECAY) {
		// Operator 3 Decay Rate
		movedPot(FADER_DECAY_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_SUSTAIN_LVL) {
		// Operator 3 Sustain Level
		movedPot(FADER_SUSTAIN_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_SUSTAIN_RATE) {
		// Operator 3 Sustain Rate
		movedPot(FADER_SUSTAIN_RATE_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_RELEASE) {
		// Operator 3 Release Rate
		movedPot(FADER_RELEASE_3, byte_val, 1);
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_ENV_MODE) {
		// Set envelope mode for operator 3, 0-2 = (off, forward, ping pong)
		if (byte_val < 3) {
			setOperatorEnvelopeMode(2, kEnvelopeMode(byte_val));
		}
	} else if (msg == NRPN_OP3_BASE + NRPN_OP_RATE_SCALE) {
		// Set rate scaling for operators 3 (0-3)
		// if (byte_val < 4) {
		// 	updateFMifNecessary(21);
		// 	fmBase[21] = byte_val << 6; // 0-3 becomes 0-192 (4 steps: 0, 64, 128, 192)
		// 	ledNumber(byte_val);
		// }
		loopHeld = true;
		movedPot(FADER_DETUNE_3, byte_val, 1);
		loopHeld = false;
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_DETUNE) {
		// Operator 4 Detune
		movedPot(FADER_DETUNE_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_MULT) {
		// Operator 4 Multiplier
		movedPot(FADER_MULT_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_LEVEL) {
		// Operator 4 Level
		movedPot(FADER_LEVEL_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_ATTACK) {
		// Operator 4 Attack
		movedPot(FADER_ATTACK_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_DECAY) {
		// Operator 4 Decay Rate
		movedPot(FADER_DECAY_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_SUSTAIN_LVL) {
		// Operator 4 Sustain Level
		movedPot(FADER_SUSTAIN_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_SUSTAIN_RATE) {
		// Operator 4 Sustain Rate
		movedPot(FADER_SUSTAIN_RATE_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_RELEASE) {
		// Operator 4 Release Rate
		movedPot(FADER_RELEASE_4, byte_val, 1);
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_ENV_MODE) {
		// Set envelope mode for operator 4, 0-2 = (off, forward, ping pong)
		if (byte_val < 3) {
			setOperatorEnvelopeMode(3, kEnvelopeMode(byte_val));
		}
	} else if (msg == NRPN_OP4_BASE + NRPN_OP_RATE_SCALE) {
		// Set rate scaling for operators 4 (0-3)
		loopHeld = true;
		movedPot(FADER_DETUNE_4, byte_val, 1);
		loopHeld = false;
	}

	// reset pickup mode to previous state after handling the NRPN
	pickupMode = oldPickupMode;
}
