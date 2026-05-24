#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"
#include "leds.h"
#include "pickup.h"
#include "FM.h"
#include "midi.h"
#include "nrpn.h"
#include "pitchEngine.h"
#include "lfo.h"
#include "loop.h"

void updateDisplayArpMidi(byte data, bool isMidi) {
	if (!isMidi || showMidiFeedback) {
		byte valueToShow = map(data, 0, 255, 0, 12);
		if (valueToShow > 2) {
			ledNumber(kArpRateDisplay[valueToShow]);
		} // it's less than a bar, show division
		else {
			digit(0, 25); // b
			switch (valueToShow) {
				case 0:
					digit(1, 4);
					break;
				case 1:
					digit(1, 2);
					break;
				case 2:
					digit(1, 1);
					break;
			}
		} // show number of bars (b1 or b2 or b4)
	}
}

void updateDisplayLFOWhenSynced(byte data, bool isMidi) {
	if (!isMidi || showMidiFeedback) {
		if ((lfoClockEnable[selectedLfo]) && (sync)) {
			updateDisplayArpMidi(data, isMidi);
		} else {
			ledNumber(data >> 2);
		}
	}
}

void ledNumberCond(byte number, bool isMidi) {
	if (!isMidi || showMidiFeedback) {
		ledNumber(number);
	}
}

void showNumberCond(byte movedPotOrFader, byte value, bool isMidi) {
	if (!isMidi || showMidiFeedback) {
		showNumber(movedPotOrFader, value);
	}
}

void movedPot(byte number, byte data, bool isMidi) {
	if (secPast) {

		bool isFader = false; // only display SSGEG if we move a fader
		// comes in as 8bit
		if ((!seqRec) && (!sendReceive)) {
			if (setupMode) {

				ledSet(LED_VOICE_MODE_WIDE6, 0);
				ledSet(LED_VOICE_MODE_DUALCH3, 0);
				ledSet(LED_VOICE_MODE_UNISON, 0);

				switch (number) {

					case KNOB_ARP_RATE:
						setupChanged = true;
						if (data > 127) {
							arpClockEnable = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							arpClockEnable = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (!isMidi)
							sendNRPN(NRPN_ARP_CLOCK_SYNC, arpClockEnable);
						break; // arp rate
					case KNOB_LFO1_RATE:
						setupChanged = true;
						if (data > 127) {
							lfoClockEnable[0] = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							lfoClockEnable[0] = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (!isMidi)
							sendNRPN(NRPN_LFO_CLOCK_SYNC, lfoClockEnable[0]);
						break; // lfo 1 rate
					case KNOB_LFO2_RATE:
						setupChanged = true;
						if (data > 127) {
							lfoClockEnable[1] = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							lfoClockEnable[1] = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (!isMidi)
							sendNRPN(NRPN_LFO_CLOCK_SYNC + 1, lfoClockEnable[1]);
						break; // lfo 2 rate
					case KNOB_LFO3_RATE:
						setupChanged = true;
						if (data > 127) {
							lfoClockEnable[2] = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							lfoClockEnable[2] = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (!isMidi)
							sendNRPN(NRPN_LFO_CLOCK_SYNC + 2, lfoClockEnable[2]);
						break; // lfo 3 rate
					case KNOB_VIB_RATE:
						setupChanged = true;
						if (data > 127) {
							vibratoClockEnable = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							vibratoClockEnable = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (!isMidi)
							sendNRPN(NRPN_VIB_CLOCK_SYNC, vibratoClockEnable);
						break; // vib rate

					case KNOB_LFO1_DEPTH:
						setupChanged = true;
						if (data > 127) {
							lfoVel = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							lfoVel = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (lfoVel != lastLfoSetting[0]) {
							lastLfoSetting[0] = !lastLfoSetting[0];
							digit(0, 13);
							digit(1, 18);
							delay(500);
							if (data > 127) {
								digit(0, 0);
								digit(1, 19);
							} else {
								digit(0, 0);
								digit(1, 12);
							}
						}
						if (!isMidi)
							sendNRPN(NRPN_LFO_VEL, lfoVel);
						break; // lfo 1 depth

					case KNOB_LFO2_DEPTH:
						setupChanged = true;
						if (data > 127) {
							lfoMod = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							lfoMod = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (lfoMod != lastLfoSetting[1]) {
							lastLfoSetting[1] = !lastLfoSetting[1];
							digit(0, 10);
							digit(1, 1);
							delay(500);
							if (data > 127) {
								digit(0, 0);
								digit(1, 19);
							} else {
								digit(0, 0);
								digit(1, 12);
							}
						}
						if (!isMidi)
							sendNRPN(NRPN_LFO_MOD, lfoMod);
						break; // lfo 2 depth

					case KNOB_LFO3_DEPTH:
						setupChanged = true;
						if (data > 127) {
							lfoAt = 1;
							digit(0, 0);
							digit(1, 19);
						} else {
							lfoAt = 0;
							digit(0, 0);
							digit(1, 12);
						}
						if (lfoAt != lastLfoSetting[2]) {
							lastLfoSetting[2] = !lastLfoSetting[2];
							digit(0, 17);
							digit(1, 26);
							delay(500);
							if (data > 127) {
								digit(0, 0);
								digit(1, 19);
							} else {
								digit(0, 0);
								digit(1, 12);
							}
						}
						if (!isMidi)
							sendNRPN(NRPN_LFO_AT, lfoAt);
						break; // lfo 3 depth

					case KNOB_FAT:
						setupChanged = true;
						if (data < 128) {
							fatMode = FAT_MODE_SEMITONE;
							digit(0, 1);
							digit(1, 5);
						} else {
							fatMode = FAT_MODE_OCTAVE;
							digit(0, 1);
							digit(1, 27);
						}
						if (!isMidi)
							sendNRPN(NRPN_SET_FAT_MODE, !fatMode);
						break; // fat

					case KNOB_VOLUME:
						setupChanged = true;
						if (data > 127) {
							ignoreVolume = 0;
							digit(0, 0);
							digit(1, 19);
						} else {
							ignoreVolume = 1;
							digit(0, 0);
							digit(1, 12);
						}
						if (!isMidi)
							sendNRPN(NRPN_SET_IGNORE_VOL, ignoreVolume);
						break; // volume preset

					case KNOB_VIB_DEPTH:
						byte brightness = data >> 4;
						if (brightness > 15)
							brightness = 10;                   // default;
						mydisplay.setIntensity(0, brightness); // 15 = brightest
						EEPROM.write(3965, brightness);
						if (!isMidi)
							sendNRPN(NRPN_SET_BRIGHTNESS, brightness);
						break;
				}

			} else {

				// not setup mode
				if ((pickupMode) && (!pickup[number])) {
					// param hasn't been picked up yet, tell user if its too high or low
					doPickup(number, data);
				} else {

					switch (number) {

						// OP1
						case FADER_DETUNE_1:
							showPickupAnimation = false;
							if (loopHeld) {
								loopChanged = true;
								updateFMifNecessary(3);
								fmBase[3] = data;
								ledNumberCond(data >> 6, isMidi);
								if (!isMidi) {
									sendNRPN(NRPN_OP1_BASE + NRPN_OP_RATE_SCALE, data);
								}
							} else {
								fmBase[0] = data;
								updateFMifNecessary(0);
								if (voiceMode == kVoicingDualCh3) {
									ledNumberCond(data >> 2, isMidi);
								} else {
									ledNumberCond(-3 + (data >> 5), isMidi);
								}
								if (!isMidi) {
									isFader = true;
									targetPot = 0;
									sendNRPN(NRPN_OP1_BASE + NRPN_OP_DETUNE, data);
								}
							}

							break; // detune
						case FADER_MULT_1:
							showPickupAnimation = false;
							fmBase[1] = data;
							updateFMifNecessary(1);
							showNumberCond(1, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 1;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_MULT, data);
							}
							break; // multiple
						case FADER_LEVEL_1:
							showPickupAnimation = false;
							fmBase[2] = data;
							updateFMifNecessary(2);
							showNumberCond(2, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 2;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_LEVEL, data);
							}
							break; // op level
						case FADER_ATTACK_1:
							showPickupAnimation = false;
							fmBase[4] = data;
							updateFMifNecessary(4);
							showNumberCond(4, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 4;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_ATTACK, data);
							}
							break; // attack
						case FADER_DECAY_1:
							showPickupAnimation = false;
							fmBase[5] = data;
							updateFMifNecessary(5);
							showNumberCond(5, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 5;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_DECAY, data);
							}
							break; // decay1
						case FADER_SUSTAIN_1:
							showPickupAnimation = false;
							fmBase[7] = data;
							updateFMifNecessary(7);
							showNumberCond(7, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 7;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_SUSTAIN_LVL, data);
							}
							break; // sustain
						case FADER_SUSTAIN_RATE_1:
							showPickupAnimation = false;
							fmBase[6] = data;
							updateFMifNecessary(9);
							showNumberCond(6, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 6;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_SUSTAIN_RATE, data);
							}
							break; // sustain rate
						case FADER_RELEASE_1:
							showPickupAnimation = false;
							fmBase[8] = data;
							updateFMifNecessary(8);
							showNumberCond(8, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 8;
								sendNRPN(NRPN_OP1_BASE + NRPN_OP_RELEASE, data);
							}
							break; // release

							// OP2
						case FADER_DETUNE_2:
							showPickupAnimation = false;
							if (loopHeld) {
								loopChanged = true;
								updateFMifNecessary(12);
								fmBase[12] = data;
								ledNumberCond(data >> 6, isMidi);
								if (!isMidi) {
									sendNRPN(NRPN_OP2_BASE + NRPN_OP_RATE_SCALE, data);
								}
							} else {
								fmBase[18] = data;
								updateFMifNecessary(18);
								if (voiceMode == kVoicingDualCh3) {
									ledNumberCond(data >> 2, isMidi);
								} else {
									ledNumberCond(-3 + (data >> 5), isMidi);
								}
								if (!isMidi) {
									isFader = true;
									targetPot = 18;
									sendNRPN(NRPN_OP2_BASE + NRPN_OP_DETUNE, data);
								}
							}
							break; // detune
						case FADER_MULT_2:
							showPickupAnimation = false;
							fmBase[19] = data;
							updateFMifNecessary(19);
							showNumberCond(19, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 19;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_MULT, data);
							}
							break; // multiple
						case FADER_LEVEL_2:
							showPickupAnimation = false;
							fmBase[20] = data;
							updateFMifNecessary(20);
							showNumberCond(20, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 20;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_LEVEL, data);
							}
							break; // op level
						case FADER_ATTACK_2:
							showPickupAnimation = false;
							fmBase[22] = data;
							updateFMifNecessary(22);
							showNumberCond(22, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 22;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_ATTACK, data);
							}
							break; // attack WAS 59
						case FADER_DECAY_2:
							showPickupAnimation = false;
							fmBase[23] = data;
							updateFMifNecessary(23);
							showNumberCond(23, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 23;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_DECAY, data);
							}
							break; // decay1 WAS 50
						case FADER_SUSTAIN_2:
							showPickupAnimation = false;
							fmBase[25] = data;
							updateFMifNecessary(25);
							showNumberCond(25, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 25;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_SUSTAIN_LVL, data);
							}
							break; // sustain WAS 60
						case FADER_SUSTAIN_RATE_2:
							showPickupAnimation = false;
							fmBase[24] = data;
							updateFMifNecessary(24);
							showNumberCond(24, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 24;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_SUSTAIN_RATE, data);
							}
							break; // sustain rate WAS 55
						case FADER_RELEASE_2:
							showPickupAnimation = false;
							fmBase[26] = data;
							updateFMifNecessary(26);
							showNumberCond(26, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 26;
								sendNRPN(NRPN_OP2_BASE + NRPN_OP_RELEASE, data);
							}
							break; // release WAS 52

							// OP3
						case FADER_DETUNE_3:
							showPickupAnimation = false;

							if (loopHeld) {
								loopChanged = true;
								updateFMifNecessary(21);
								fmBase[21] = data;
								ledNumberCond(data >> 6, isMidi);
								if (!isMidi) {
									sendNRPN(NRPN_OP3_BASE + NRPN_OP_RATE_SCALE, data);
								}
							} else {
								fmBase[9] = data;
								updateFMifNecessary(9);
								if (voiceMode == kVoicingDualCh3) {
									ledNumberCond(data >> 2, isMidi);
								} else {
									ledNumberCond(-3 + (data >> 5), isMidi);
								}
								if (!isMidi) {
									isFader = true;
									targetPot = 9;
									sendNRPN(NRPN_OP3_BASE + NRPN_OP_DETUNE, data);
								}
							}
							break; // detune
						case FADER_MULT_3:
							showPickupAnimation = false;
							fmBase[10] = data;
							updateFMifNecessary(10);
							showNumberCond(10, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 10;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_MULT, data);
							}
							break; // multiple
						case FADER_LEVEL_3:
							showPickupAnimation = false;
							fmBase[11] = data;
							updateFMifNecessary(11);
							showNumberCond(11, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 11;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_LEVEL, data);
							}
							break; // op level
						case FADER_ATTACK_3:
							showPickupAnimation = false;
							fmBase[13] = data;
							updateFMifNecessary(13);
							showNumberCond(13, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 13;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_ATTACK, data);
							}
							break; // attack
						case FADER_DECAY_3:
							showPickupAnimation = false;
							fmBase[14] = data;
							updateFMifNecessary(14);
							showNumberCond(14, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 14;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_DECAY, data);
							}
							break; // decay1
						case FADER_SUSTAIN_3:
							showPickupAnimation = false;
							fmBase[16] = data;
							updateFMifNecessary(16);
							showNumberCond(16, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 16;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_SUSTAIN_LVL, data);
							}
							break; // sustain
						case FADER_SUSTAIN_RATE_3:
							showPickupAnimation = false;
							fmBase[15] = data;
							updateFMifNecessary(15);
							showNumberCond(15, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 15;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_SUSTAIN_RATE, data);
							}
							break; // sustain rate
						case FADER_RELEASE_3:
							showPickupAnimation = false;
							fmBase[17] = data;
							updateFMifNecessary(17);
							showNumberCond(17, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 17;
								sendNRPN(NRPN_OP3_BASE + NRPN_OP_RELEASE, data);
							}
							break; // release

							// OP4
						case FADER_DETUNE_4:
							showPickupAnimation = false;
							if (loopHeld) {
								loopChanged = true;
								updateFMifNecessary(30);
								fmBase[30] = data;
								ledNumberCond(data >> 6, isMidi);
								if (!isMidi) {
									sendNRPN(NRPN_OP4_BASE + NRPN_OP_RATE_SCALE, data);
								}
							} else {
								fmBase[27] = data;
								updateFMifNecessary(27);
								if (voiceMode == kVoicingDualCh3) {
									ledNumberCond(data >> 2, isMidi);
								} else {
									ledNumberCond(-3 + (data >> 5), isMidi);
								}
								if (!isMidi) {
									isFader = true;
									targetPot = 27;
									sendNRPN(NRPN_OP4_BASE + NRPN_OP_DETUNE, data);
								}
							}
							break; // detune
						case FADER_MULT_4:
							showPickupAnimation = false;
							fmBase[28] = data;
							updateFMifNecessary(28);
							showNumberCond(28, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 28;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_MULT, data);
							}
							break; // multiple
						case FADER_LEVEL_4:
							showPickupAnimation = false;
							fmBase[29] = data;
							updateFMifNecessary(29);
							showNumberCond(29, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 29;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_LEVEL, data);
							}
							break; // op level
						case FADER_ATTACK_4:
							showPickupAnimation = false;
							fmBase[31] = data;
							updateFMifNecessary(31);
							showNumberCond(31, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 31;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_ATTACK, data);
							}
							break; // attack
						case FADER_DECAY_4:
							showPickupAnimation = false;
							fmBase[32] = data;
							updateFMifNecessary(32);
							showNumberCond(32, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 32;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_DECAY, data);
							}
							break; // decay1
						case FADER_SUSTAIN_4:
							showPickupAnimation = false;
							fmBase[34] = data;
							updateFMifNecessary(34);
							showNumberCond(34, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 34;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_SUSTAIN_LVL, data);
							}
							break; // sustain
						case FADER_SUSTAIN_RATE_4:
							showPickupAnimation = false;
							fmBase[33] = data;
							updateFMifNecessary(33);
							showNumberCond(33, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 33;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_SUSTAIN_RATE, data);
							}
							break; // sustain rate
						case FADER_RELEASE_4:
							showPickupAnimation = false;
							fmBase[35] = data;
							updateFMifNecessary(35);
							showNumberCond(35, data, isMidi);
							if (!isMidi) {
								isFader = true;
								targetPot = 35;
								sendNRPN(NRPN_OP4_BASE + NRPN_OP_RELEASE, data);
							}
							break; // release

						case KNOB_VOLUME:
							showPickupAnimation = false;
							setupCounter = 0; // prevent entering setup
							if (voiceHeld) {
								movedFineKnob = true;

								fine = finePot = data;
								updateFine();

								if (fine > 127) {
									ledNumberCond(map(fine, 128, 255, 0, 32), isMidi);
								} else if (fine < 128) {
									ledNumberCond(map(fine, 128, 0, 0, 32), isMidi);
								}
								if (!isMidi)
									sendNRPN(NRPN_FINE_TUNE, data);
							} else {

								vol = 128 - (data >> 1);
								if (lastVol != vol) {
									changeVol = true;
									lastVol = vol;
									ledNumberCond(data >> 2, isMidi);
									volumeCounter = 20;
									if (!isMidi) {
										sendNRPN(NRPN_VOLUME, data);
									}
								}
							}
							break; // volume

						case KNOB_ALGO:
							showPickupAnimation = false;
							fmBase[42] = data;
							updateFMifNecessary(42);
							showNumberCond(42, data, isMidi);
							if (!isMidi) {
								targetPot = 42;
								sendNRPN(NRPN_ALGORITHM, data);
							}
							break; // algo

						case KNOB_FEEDBACK:
							showPickupAnimation = false;
							fmBase[43] = data;
							updateFMifNecessary(43);
							showNumberCond(43, data, isMidi);
							if (!isMidi) {
								targetPot = 43;
								sendNRPN(NRPN_FEEDBACK, data);
							}
							break; // feedback

						case KNOB_FAT:
							showPickupAnimation = false;
							setupCounter = 0; // prevent entering setup

							if (voiceHeld && movedFineKnob) {
								fine = finePot + map(data, 0, 255, -10, 10);
								fine = constrain(fine, 0, 255);
								updateFine();

							} else {

								if (voiceHeld) {
									glide = data >> 4;
									updateGlideIncrements();
									fineChanged = true;
									ledNumberCond(data >> 4, isMidi);
									if (!isMidi)
										sendNRPN(NRPN_GLIDE, glide << 4);
								} else {
									fmBase[50] = data;
									updateFMifNecessary(50);
									ledNumberCond(data >> 2, isMidi);
									if (!isMidi) {
										targetPot = 50;
										sendNRPN(NRPN_FAT, data);
									}
								}
							}
							break; // fat 1-127

						case KNOB_LFO1_RATE:
							showPickupAnimation = false;
							fmBase[36] = data;
							if (data < 5 && lfoVel) {
								fmBase[36] = data = 0;
							}
							updateFMifNecessary(36);
							if (lfoVel) {
								digit(0, 13);
								digit(1, 18);
							} else {
								selectedLfo = 0;
								updateDisplayLFOWhenSynced(data, isMidi);
								if (!isMidi) {
									targetPot = 36;
									sendNRPN(NRPN_LFO1_RATE, data);
								}
							}
							break; // lfo 1 rate
						case KNOB_LFO1_DEPTH:
							showPickupAnimation = false;
							fmBase[37] = data;
							updateFMifNecessary(37);
							selectedLfo = 0;
							showNumberCond(37, data, isMidi);
							if (!isMidi) {
								targetPot = 37;
								sendNRPN(NRPN_LFO1_DEPTH, data);
							}
							break; // lfo 1 depth

						case KNOB_LFO2_RATE:
							showPickupAnimation = false;
							fmBase[38] = data;
							updateFMifNecessary(38);
							if ((lfoMod) && (!data)) {
								digit(0, 10);
								digit(1, 1);
							} else {
								selectedLfo = 1;
								updateDisplayLFOWhenSynced(data, isMidi);
								if (!isMidi) {
									targetPot = 38;
									sendNRPN(NRPN_LFO2_RATE, data);
								}
							}
							break; // lfo 2 rate
						case KNOB_LFO2_DEPTH:
							showPickupAnimation = false;
							fmBase[39] = data;
							updateFMifNecessary(39);
							selectedLfo = 1;
							showNumberCond(39, data, isMidi);
							if (!isMidi) {
								targetPot = 39;
								sendNRPN(NRPN_LFO2_DEPTH, data);
							}
							break; // lfo 2 depth

						case KNOB_LFO3_RATE:
							showPickupAnimation = false;
							fmBase[40] = data;
							updateFMifNecessary(40);

							if (data < 5 && lfoAt) {
								fmBase[40] = data = 0;
							}

							if (lfoAt) {
								digit(0, 17);
								digit(1, 26);
							} else {
								selectedLfo = 2;
								updateDisplayLFOWhenSynced(data, isMidi);
								if (!isMidi) {
									targetPot = 40;
									sendNRPN(NRPN_LFO3_RATE, data);
								}
							}
							break; // lfo 3 rate
						case KNOB_LFO3_DEPTH:
							showPickupAnimation = false;
							fmBase[41] = data;
							updateFMifNecessary(41);
							selectedLfo = 2;
							showNumberCond(41, data, isMidi);
							if (!isMidi) {
								targetPot = 41;
								sendNRPN(NRPN_LFO3_DEPTH, data);
							}
							break; // lfo 3 depth

						case KNOB_ARP_RATE:
							showPickupAnimation = false;
							fmBase[46] = data;
							updateFMifNecessary(46);
							if (sync) {
								updateDisplayArpMidi(data, isMidi);
								arpMidiSpeedPending = map(data, 0, 255, 0, 12);
							} else {
								ledNumberCond(data >> 2, isMidi);
							}
							if (!isMidi) {
								targetPot = 46;
								sendNRPN(NRPN_ARP_RATE, data);
							}
							break; /// arp rate
						case KNOB_ARP_RANGE:
							showPickupAnimation = false;
							fmBase[47] = data;
							updateFMifNecessary(47);
							showNumberCond(47, data, isMidi);
							if (!isMidi) {
								targetPot = 47;
								sendNRPN(NRPN_ARP_RANGE, data);
							}
							break; // arp range

						case KNOB_VIB_RATE:
							showPickupAnimation = false;
							fmBase[48] = data;
							updateFMifNecessary(48);
							if ((vibratoClockEnable) && (sync)) {
								updateDisplayArpMidi(data, isMidi);
							} else {
								ledNumberCond(data >> 3, isMidi);
							}
							if (!isMidi) {
								targetPot = 48;
								sendNRPN(NRPN_VIB_RATE, data);
							}
							break; // vibrato rate WAS 7
						case KNOB_VIB_DEPTH:
							showPickupAnimation = false;
							fmBase[49] = data;
							updateFMifNecessary(49);
							showNumberCond(49, data, isMidi);
							if (!isMidi) {
								targetPot = 49;
								sendNRPN(NRPN_VIB_DEPTH, data);
							}
							break; // vibrato depth
					}
				}

				if (!isMidi || showMidiFeedback) {
					// update display
					if (targetPotLast != targetPot) {
						targetPotLast = targetPot;
						showLink();
						applyLfo();
					}
					if (selectedLfoLast != selectedLfo) {
						selectedLfoLast = selectedLfo;
						showSSEGCounter = 0; // quit the SSEG display
						showLfo();
					} // show lfo because we moved to another

					if (isFader) {
						if (targetPot < 9) {
							lastOperator = 0;
							showSSEG();
						} else if ((targetPot > 8) && (targetPot < 18)) {
							lastOperator = 2;
							showSSEG();
						} else if ((targetPot > 17) && (targetPot < 27)) {
							lastOperator = 1;
							showSSEG();
						} else if ((targetPot > 26) && (targetPot < 36)) {
							lastOperator = 3;
							showSSEG();
						}
					}
				}
			}
		}
	}
}
