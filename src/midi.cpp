#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"
#include "leds.h"
#include "arp.h"
#include "lfo.h"
#include "pitchEngine.h"
#include "preset.h"
#include "voice.h"
#include "midi.h"
#include "pots.h"
#include "midi_pedal.hpp"

static byte voiceSlot;
static bool ch3Alt;
static const float vibIncrements[8] = {5.312, 7.968, 10.625, 14.166, 15.937, 31.875, 42.5, 63.75};
static float vibIndexF;

static byte lastCC[60];

static int arpClockCounter;
static byte syncLfoCounter;

static bool arpClearFlag = false;

void handleAftertouch(byte channel, byte val) {
	leftDot();
	if (channel == inputChannel) {
		if (lfoAt) {
			if (fmBase[40]) {
				fmBase[41] = val << 1;
				fmBaseLast[41] = fmBase[41] - 1;
				ledNumberTimeOut = 20;
			} else {
				atDest = val << 1;
				ledNumberTimeOut = 20;
			}
		}
	}
}

void handleClock() {
	if (sync) {
		////////////////////////////////////
		////////      VIBRATO        ///////
		////////////////////////////////////
		if (vibratoClockEnable) {
			if (fmData[48]) {
				vibIndexF += vibIncrements[fmData[48] >> 5];
				if (vibIndexF > 255) {
					vibIndexF -= 256;
				}
				vibIndex = int(vibIndexF);
			}
		}

		absoluteClockCounter++;
		if (absoluteClockCounter >= 96) {
			leftDot();
			absoluteClockCounter = 0;

			if (arpClockEnable) {
				if (arpMidiSpeed != arpMidiSpeedPending || resyncArp) {
					arpMidiSpeed = arpMidiSpeedPending;
					arpClockCounter = 0;
					resyncArp = false;
				}
			}

			for (int i = 0; i < 3; i++) {
				if (lfoClockEnable[i]) {
					if (lfoClockSpeedPending[i] != lfoClockSpeedPendingLast[i]) {
						lfoClockSpeed[i] = lfoClockSpeedPending[i];
						lfoClockSpeedPendingLast[i] = lfoClockSpeedPending[i];
						lfoStepF[i] = 0;
					}
				}
			}
		}

		////////////////////////////////////
		////////        ARP          ///////
		///////////////////////////////////

		if ((arpClockEnable) && (arpMode) && (voiceMode == kVoicingUnison) && (arpMode != 7)) {
			arpClockCounter++;
			if ((arpClockCounter >= kMidiArpTicks[arpMidiSpeed])) {
				arpClockCounter = 0;
				if (!emptyStack) {
					arpFire();
				}
			}
		}

		// if Any LFOClock Enable
		if ((lfoClockEnable[0]) || (lfoClockEnable[1]) || (lfoClockEnable[2])) {

			// activate the LFO
			lfoTick();

			syncLfoCounter++;
			if (syncLfoCounter == 24) {
				syncLfoCounter = 0;
			}

			for (int i = 0; i < 3; i++) {
				if (lfoClockEnable[i]) {
					lfoStepF[i] += kLfoClockRates[lfoClockSpeed[i]];
					if (lfoStepF[i] >= 255) {
						if (looping[i]) {
							if (selectedLfo == i) {
								lfoBlink();
							}
							lfoStepF[i] = 0;
							lfoNewRand[i] = 1;
						} else {
							lfoStepF[i] = 255;
						}
					}
					lfoStep[i] = int(lfoStepF[i]);
				}
			}
		}
	}
}

void handleBendy(byte channel, int bend) {
	leftDot();

	if (mpe) {
		if (channel > 12)
			channel = 12;

		bendy = 0;

		if (bend != 0) {
			if (bend < 0) {
				mpeBend[channel - 1] = bend;
				mpeBend[channel - 1] /= 8192;
				mpeBend[channel - 1] *= bendDown;
			} else if (bend > 0) {
				mpeBend[channel - 1] = bend;
				mpeBend[channel - 1] /= 8191;
				mpeBend[channel - 1] *= bendUp;
			}
		}

		setNote(channel - 1, notey[channel - 1]);
	} else {
		if (channel == inputChannel) {
			//-8192 to 8191
			bendy = 0;
			if (bend != 0) {
				if (bend < 0) {
					bendy = bend;
					bendy /= 8192;
					bendy *= bendDown;
				} else if (bend > 0) {
					bendy = bend;
					bendy /= 8191;
					bendy *= bendUp;
				}
			}

			bendyCounter = 4;
		}
	}
}

void handleStop() { sync = false; }

void handleStart() {
	if (vibratoClockEnable)
		vibIndex = 0;

	absoluteClockCounter = 0;
	seqStep = 0;
	arpClockCounter = 0;
	sync = true;

	for (int i = 0; i < 3; i++) {
		if (lfoClockEnable[i]) {
			lfoStepF[i] = lfoStep[i] = 0;
		}
	}
}

void handleProgramChange(byte channel, byte program) {
	if ((program < 99) && (channel == inputChannel)) {

		preset = program;
		loadPreset();
	}
}

/// Try to find the next free voice slot. If there is none, returns the next voice slot, independent of whether it's
/// free or not. Considers only the first `nSlots` slots for search.
static byte findNextFreeVoiceSlot(byte prevSlot, bool voiceSlots[], byte nSlots) {
	for (byte i = 0; i < nSlots; i++) {
		byte slot = (prevSlot + 1 + i) % nSlots;
		if (voiceSlots[slot] == false) {
			return slot;
		}
	}

	return (prevSlot + 1) % nSlots;
}

int nVoicesForMode(VoiceMode voiceMode) {
	switch (voiceMode) {
		case kVoicingPoly12:
			return 12;
		case kVoicingWide6:
			return 6;
		case kVoicingWide4:
			return 4;
		case kVoicingWide3:
			return 3;
		case kVoicingUnison:
			return 1;
		case kVoicingDualCh3:
			return 1;
		default:
			return 1; // should not happen
	}
}

static void handleNoteOn(byte channel, byte note, byte velocity);
static void handleNoteOff(byte channel, byte note);

static MidiPedalAdapter pedal_adapter(handleNoteOn, handleNoteOff);

static void handleNoteOn(byte channel, byte note, byte velocity) {
	// byte distanceFromNewNote; // unused
	if (channel == inputChannel)
		heldNotes[note] = true;

	if (setupMode) {

		if (bendRoot == -1) {
			bendRoot = note;

			digit(0, 10);
			digit(1, 23);
			delay(500);
			lastNumber = -1;
			ledNumber(inputChannel);
			delay(750);

			if (channel != inputChannel) {
				inputChannel = channel;
				EEPROM.write(3951, inputChannel);
			}

		} else {
			if (note > bendRoot) {
				bendUp = note - bendRoot;
				if (bendUp > 48) {
					bendUp = 48;
				}
				EEPROM.write(3959, byte(bendUp));

				digit(0, 13);
				digit(1, 14);
				delay(500);
				lastNumber = -1;
				ledNumber(bendUp);
				delay(750);
			} else if (note < bendRoot) {
				bendDown = bendRoot - note;
				if (bendDown > 48) {
					bendDown = 48;
				}
				EEPROM.write(3958, byte(bendDown));

				digit(0, 15);
				digit(1, 19);
				delay(500);
				lastNumber = -1;
				ledNumber(bendDown);
				delay(750);
			}
		}
	} else {
		if (lfoVel) {
			if (fmBase[36]) {
				fmBase[37] = velocity << 1;
				fmBaseLast[37] = fmBase[37] - 1;
			} else {
				lfo[0] = velocity << 1;
				applyLfo();
			}
		}

		if (mpe) {

			note += 3;

			if (channel > 12)
				channel = 12;

			ym.noteOff(channel - 1);
			notey[channel - 1] = note;
			setNote(channel - 1, notey[channel - 1]);
			ym.noteOn(channel - 1);
		} else {

			if (channel == inputChannel) {

				note += 3;
				rootNote = note;

				if (velocity) {

					// Retrigger LFOs
					for (int i = 0; i < 3; i++) {
						if ((retrig[i]) || ((!retrig[i]) && (!looping[i]) && (heldKeys == 0))) {
							lfoStep[i] = 0;
						}
					}

					// Reset Arpeggiator
					if (arpMode == 6) {
						arpStep = seqStep = 0;
						arpIndex = 0;
					} else if (arpMode == 7) {
						arpCounter = 1023;
					} // next manual arp step

					int nVoices, ymfChannelsPerVoice;
					switch (voiceMode) {
						case kVoicingPoly12:
						case kVoicingWide6:
						case kVoicingWide4:
						case kVoicingWide3:
							nVoices = nVoicesForMode(voiceMode);
							ymfChannelsPerVoice = 12 / nVoices;

							voiceSlot = findNextFreeVoiceSlot(voiceSlot, voiceSlots, nVoices);
							voiceSlots[voiceSlot] = 1;
							// if gliding jump to last pitch associated to keycounter
							if (glide) {
								for (int i = 0; i < ymfChannelsPerVoice; i++) {
									setNote(ymfChannelsPerVoice * voiceSlot + i, lastNotey[heldKeys]);
									skipGlide(ymfChannelsPerVoice * voiceSlot + i);
								}
							}
							noteOfVoice[voiceSlot] = note;
							lastNotey[heldKeys] = note;

							for (int i = 0; i < ymfChannelsPerVoice; i++) {
								setNote(ymfChannelsPerVoice * voiceSlot + i, noteOfVoice[voiceSlot]);
								ym.noteOff(ymfChannelsPerVoice * voiceSlot + i);
								ym.noteOn(ymfChannelsPerVoice * voiceSlot + i);
							}

							if (heldKeys < 127)
								heldKeys++;

							break;

						case kVoicingDualCh3:
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							// dual CH3
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							if (stereoCh3) {
								// fire at the same time

								//                noteToChannel[note] = 4; unused?
								ym.noteOff(4);
								setNote(4, note);
								ym.noteOn(4); // CHIP1
								setNote(2, note);

								//                noteToChannel[note] = 5; unused?
								ym.noteOff(5);
								setNote(5, note);
								ym.noteOn(5); // CHIP2
								setNote(8, note);
							} else {

								ch3Alt = !ch3Alt;

								if (ch3Alt) {

									//                  noteToChannel[note] = 4; // unused?
									ym.noteOff(4);
									setNote(4, note);
									ym.noteOn(4); // CHIP1
									setNote(2, note);

								} else {

									//                  noteToChannel[note] = 5; // unused?
									ym.noteOff(5);
									setNote(5, note);
									ym.noteOn(5); // CHIP2
									setNote(8, note);
								}
							}

							break;

						case kVoicingUnison:
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							// unison
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							if ((arpMode) && (fmData[46])) {
								// ARP

								heldKeys++;

								if (heldKeys == 1) {
									arpClearFlag = false;
									clearNotes();
									heldKeys = 1;
								}

								if ((heldKeys == 1) && (!sync)) {
									arpCounter = 1023;
								} // only retrigger arp on first key or if arp is stopped

								addNote(note);

							} else {
								// NO ARP

								heldKeys++;

								lastNote = note;
								addNote(note);

								for (int i = 0; i < 12; i++) {
									ym.noteOff(i);
									setNote(i, note);
									ym.noteOn(i);
								}
							}

							if (seqRec) {
								if (!seqLength)
									rootNote1 = note;
								seq[seqLength] = note - rootNote1 + 127;
								if (seq[seqLength] == 255) {
									seq[seqLength]--;
								}
								ledNumber(seqLength + 1);
								if (seqLength < 15) {
									seqLength++;
								}

								for (int i = 0; i < 12; i++) {
									ym.noteOff(i);
									setNote(i, note);
									ym.noteOn(i);
								}
							}

							break;
						default:
							break;
					}

				} else {
					handleNoteOff(channel, note);
				}
				leftDot();
			}
		}
	}
}

static void handleNoteOff(byte channel, byte note) {
	if (channel == inputChannel)
		heldNotes[note] = false;
	if (setupMode) {

		if (note == bendRoot) {
			bendRoot = -1;
		}
	} else {

		if (mpe) {

			note += 3;

			if (channel > 12)
				channel = 12;
			ym.noteOff(channel - 1);
		} else {

			if (channel == inputChannel) {

				note += 3;

				int nVoices, ymfChannelsPerVoice;
				switch (voiceMode) {
					case kVoicingPoly12:
					case kVoicingWide6:
					case kVoicingWide4:
					case kVoicingWide3:
						nVoices = nVoicesForMode(voiceMode);
						ymfChannelsPerVoice = 12 / nVoices;

						heldKeys--;
						if (heldKeys < 1) {
							heldKeys = 0;
						}

						// scan through the noteOfVoices and kill the voice associated to it
						for (int v = 0; v < nVoices; v++) {
							if ((voiceSlots[v]) && (noteOfVoice[v] == note)) {
								voiceSlots[v] = 0;
								for (int i = 0; i < ymfChannelsPerVoice; i++) {
									ym.noteOff(ymfChannelsPerVoice * v + i);
								}
							}
						}

						break;

					case kVoicingDualCh3: // dual CH3
						////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
						///////
						// dual CH3
						////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
						///////
						if (stereoCh3) {
							// fire at the same time

							ym.noteOff(4); // CHIP1
							ym.noteOff(5); // CHIP2

						} else {

							ch3Alt = !ch3Alt;

							if (ch3Alt) {

								ym.noteOff(4); // CHIP1

							} else {

								ym.noteOff(5); // CHIP2
							}
						}

						break;

					case kVoicingUnison: // unison
						if (arpMode) {
							// ARP

							heldKeys--;
							removeNote(note);

							if (heldKeys < 1) {
								heldKeys = 0;

								for (int i = 0; i < 12; i++) {
									ym.noteOff(i);
								}
							}

						} else {
							// NO ARP

							heldKeys--;
							removeNote(note);

							// LAST KEY UP?
							if (heldKeys < 1) {

								heldKeys = 0;
								clearNotes();

								for (int i = 0; i < 12; i++) {
									pedalOff[i] = 1;
								}

							} else {
								// NOT LAST KEY, CHANGE NOTE

								switch (notePriority) {
									case 0:
										note = getLow();
										break; // LOWEST
									case 1:
										note = getHigh();
										break; // HIGHEST
									case 2:
										note = getLast();
										break; // LAST
								}
								rootNote = note;
								for (int i = 0; i < 12; i++) {
									setNote(i, note);
								}
							}
						}
						break;

						leftDot();
				}
			}
		}
	}
}

void sendCC(byte number, int value) {

	if (lastCC[number] != value) {
		lastCC[number] = value;
		rightDot();
		sendControlChange(number, value, masterChannelOut);
	}
}

void sendMidiButt(byte number, int value) {
	rightDot();
	sendCC(number, value);
}

byte lastData1, lastData2;

void HandleControlChange(byte channel, byte number, byte val) {
	byte temp;
	if (toolMode && channel == 16) {
		switch (number) {
			case 1:
				if (val) {
					thru = 0;
				} else {
					thru = 1;
				}
				temp = EEPROM.read(3950);
				bitWrite(temp, 0, !thru);
				EEPROM.update(3950, temp);
				break;
			case 2:
				if (val) {
					ignoreVolume = 1;
				} else {
					ignoreVolume = 0;
				}
				temp = EEPROM.read(3950);
				bitWrite(temp, 1, ignoreVolume);
				EEPROM.update(3950, temp);
				break;

			case 3:
				if (val) {
					lfoClockEnable[0] = true;
				} else {
					lfoClockEnable[0] = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 0, lfoClockEnable[0]);
				EEPROM.update(3953, temp);
				break;
			case 4:
				if (val) {
					lfoClockEnable[1] = true;
				} else {
					lfoClockEnable[1] = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 1, lfoClockEnable[1]);
				EEPROM.update(3953, temp);
				break;
			case 5:
				if (val) {
					lfoClockEnable[2] = true;
				} else {
					lfoClockEnable[2] = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 2, lfoClockEnable[2]);
				EEPROM.update(3953, temp);
				break;

			case 6:
				if (val) {
					vibratoClockEnable = true;
				} else {
					vibratoClockEnable = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 3, vibratoClockEnable);
				EEPROM.update(3953, temp);
				break;
			case 7:
				if (val) {
					arpClockEnable = true;
				} else {
					arpClockEnable = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 4, arpClockEnable);
				EEPROM.update(3953, temp);
				break;

			case 19:
				if (val) {
					fatMode = true;
				} else {
					fatMode = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 5, fatMode);
				EEPROM.update(3953, temp);
				break;

			case 14:
				if (val) {
					newFat = true;
				} else {
					newFat = false;
				}
				temp = EEPROM.read(3953);
				bitWrite(temp, 6, newFat);
				EEPROM.update(3953, temp);
				break;

			case 8:
				if (val) {
					pickupMode = true;
				} else {
					pickupMode = false;
				}
				EEPROM.update(3954, pickupMode);
				break;
			case 9:
				if (val) {
					mpe = true;
				} else {
					mpe = false;
				}
				EEPROM.update(3960, mpe);
				break;
			case 10:
				if (val) {
					lfoVel = true;
				} else {
					lfoVel = false;
				}
				EEPROM.update(3961, lfoVel);
				break;
			case 11:
				if (val) {
					lfoMod = true;
				} else {
					lfoMod = false;
				}
				EEPROM.update(3962, lfoMod);
				break;
			case 12:
				if (val) {
					lfoAt = true;
				} else {
					lfoAt = false;
				}
				EEPROM.update(3963, lfoAt);
				break;
			case 13:
				if (val) {
					stereoCh3 = true;
				} else {
					stereoCh3 = false;
				}
				EEPROM.update(3966, stereoCh3);
				break;

			case 15:
				inputChannel = constrain(val, 1, 16);
				EEPROM.update(3951, inputChannel);
				break;

			case 16:
				bendUp = constrain(val, 1, 48);
				EEPROM.update(3959, bendUp);
				break;

			case 17:
				bendDown = constrain(val, 1, 48);
				EEPROM.update(3958, bendDown);
				break;

			case 20:
				newWide = constrain(val, 0, 1);
				EEPROM.update(3970, constrain(val, 0, 1));
				break;
		}
	}

	if (channel == 16 && lastData1 == 19 && lastData2 == 82 && number == 19 && val == 82) {
		// transmit all the settings to tool. Tool expects noteOff messages on CH16 (yeah I couldn't get webMidi to
		// parse sysex... )

		sendTool(0, 3); // 3 is MEGAFM
		sendTool(82, kVersion0);
		sendTool(83, kVersion1);

		sendTool(1, thru);
		sendTool(2, ignoreVolume);
		sendTool(3, lfoClockEnable[0]);
		sendTool(4, lfoClockEnable[1]);
		sendTool(5, lfoClockEnable[2]);
		sendTool(6, vibratoClockEnable);
		sendTool(7, arpClockEnable);
		sendTool(8, pickupMode);

		sendTool(9, mpe);
		sendTool(10, lfoVel);
		sendTool(11, lfoMod);
		sendTool(12, lfoAt);
		sendTool(13, stereoCh3);
		sendTool(14, newFat); // new fat tuning
		sendTool(15, inputChannel);
		sendTool(16, bendUp);
		sendTool(17, bendDown);
		sendTool(18, EEPROM.read(3965));
		sendTool(19, fatMode);
		sendTool(20, newWide);
		toolMode = true; // MEGAfm is listening to new settings (CC on CH16)
	}
	// did we receive 1982 twice on channel 16?

	if ((lastSentCC[0] == number) && (lastSentCC[1] == val)) {
		// ignore same CC and DATA as sent to avoid feedback
	} else {
		leftDot();
		if (channel == inputChannel) {
			if (number == 0) {
				if (val < 5) {
					if (bank != val) {
						bank = val;
						handleProgramChange(inputChannel, preset - 1);
					}
				}
			} else if (number == 50) {
				movedPot(0, val << 1, 1);
			} else if (number == 42) {
				movedPot(42, val << 5, 1);
			} else if (number == 51) {
				movedPot(7, val << 1, 1);
			} else if (number == 49) {
				movedPot(8, val << 1, 1);
			}

			else if (number == 1) {

				if (lfoMod) {
					if (fmBase[38]) {
						fmBase[39] = val << 1;
						fmBaseLast[39] = fmBase[39] - 1;
						ledNumberTimeOut = 20;
					} else {
						lfo[1] = val << 1;
						applyLfo();
					}
				}
			} else if (number == 7) {
				if (kAllCC) {
					movedPot(1, val << 1, 1);
				}
			} else if (number == 64) {
				pedal_adapter.set_pedal(channel, val >= 64);
				// if (val > 63) {
				//	pedalDown();
				// } else {
				//	pedalUp();
				// }
			} else {
				if (kAllCC) {
					movedPot(number, val << 1, 1);
				} else {
					if ((number != 19) && (number != 40) && (number != 16) && (number != 38))
						movedPot(number, val << 1, 1);
				}
			}
		}
	}
	lastData1 = number;
	lastData2 = val;
}

void midiOut(byte note) {
	rightDot();
	// midiB.sendNoteOff(lastNote+1,127,masterChannelOut);
	// midiB.sendNoteOn(note+1,velocityLast,masterChannelOut);
	lastNote = note;
}

void dumpPreset() {
	for (int number = 0; number < 58; number++) {
		switch (number) {
			// OP1
			case 18:
				sendCC(number, fmBase[0] >> 1);
				break; // detune
			case 27:
				sendCC(number, fmBase[1] >> 1);
				break; // multiple
			case 19:
				sendCC(number, fmBase[2] >> 1);
				break; // op level
			case 29:
				sendCC(number, fmBase[4] >> 1);
				break; // attack
			case 21:
				sendCC(number, fmBase[5] >> 1);
				break; // decay1
			case 25:
				sendCC(number, fmBase[7] >> 1);
				break; // sustain
			case 17:
				sendCC(number, fmBase[6] >> 1);
				break; // sustain rate
			case 30:
				sendCC(number, fmBase[8] >> 1);
				break; // release
			// OP2
			case 31:
				sendCC(number, fmBase[18] >> 1);
				break; // detune
			case 32:
				sendCC(number, fmBase[19] >> 1);
				break; // multiple
			case 40:
				sendCC(number, fmBase[20] >> 1);
				break; // op level
			case 36:
				sendCC(number, fmBase[22] >> 1);
				break; // attack
			case 44:
				sendCC(number, fmBase[23] >> 1);
				break; // decay1
			case 42:
				sendCC(number, fmBase[25] >> 1);
				break; // sustain
			case 34:
				sendCC(number, fmBase[24] >> 1);
				break; // sustain rate
			case 11:
				sendCC(number, fmBase[26] >> 1);
				break; // release
			// OP3
			case 20:
				sendCC(number, fmBase[9] >> 1);
				break; // detune
			case 24:
				sendCC(number, fmBase[10] >> 1);
				break; // multiple
			case 16:
				sendCC(number, fmBase[11] >> 1);
				break; // op level
			case 8:
				sendCC(49, fmBase[13] >> 1);
				break; // attack
			case 0:
				sendCC(50, fmBase[14] >> 1);
				break; // decay1
			case 7:
				sendCC(51, fmBase[16] >> 1);
				break; // sustain
			case 45:
				sendCC(number, fmBase[15] >> 1);
				break; // sustain rate
			case 37:
				sendCC(number, fmBase[17] >> 1);
				break; // release
			// OP4
			case 47:
				sendCC(number, fmBase[27] >> 1);
				break; // detune
			case 39:
				sendCC(number, fmBase[28] >> 1);
				break; // multiple
			case 38:
				sendCC(number, fmBase[29] >> 1);
				break; // op level
			case 46:
				sendCC(number, fmBase[31] >> 1);
				break; // attack
			case 33:
				sendCC(number, fmBase[32] >> 1);
				break; // decay1
			case 41:
				sendCC(number, fmBase[34] >> 1);
				break; // sustain
			case 43:
				sendCC(number, fmBase[33] >> 1);
				break; // sustain rate
			case 35:
				sendCC(number, fmBase[35] >> 1);
				break; // release

			case 1:
				sendCC(7, vol >> 1);
				break; // volume //SEND FINE!!!!!!!!!!!!
			case 4:
				sendCC(number, (1 + (fmBase[42] >> 5)));
				break; // algo
			case 3:
				sendCC(number, fmBase[43] >> 1);
				break; // feedback
			case 28:
				sendCC(number, fmBase[50] >> 1);
				break; // fat 1-127 // SEND GLIDE!!!!!!!!!!!!
			case 15:
				sendCC(number, fmBase[36] >> 1);
				break; // lfo 1 rate
			case 12:
				sendCC(number, fmBase[37] >> 1);
				break; // lfo 1 depth
			case 10:
				sendCC(number, fmBase[38] >> 1);
				break; // lfo 2 rate
			case 9:
				sendCC(number, fmBase[39] >> 1);
				break; // lfo 2 depth
			case 14:
				sendCC(number, fmBase[40] >> 1);
				break; // lfo 3 rate
			case 2:
				sendCC(number, fmBase[41] >> 1);
				break; // lfo 3 depth
			case 6:
				sendCC(number, fmBase[46] >> 1);
				break; /// arp rate
			case 5:
				sendCC(number, fmBase[47] >> 1);
				break; // arp range
			case 48:
				sendCC(number, fmBase[48] >> 1);
				break; // vibrato rate WAS 7
			case 13:
				sendCC(number, fmBase[49] >> 1);
				break; // vibrato depth
		}
	}
}
static byte mStatus;
static byte mData;
static byte mChannel;
static int midiNoteOffset = -13;

void sendControlChange(byte number, byte value, byte channel) {
	if (!thru) {
		lastSentCC[0] = number;
		lastSentCC[1] = value;
		Serial.write(175 + channel);
		Serial.write(number);
		Serial.write(value);
	}
}

void sendNoteOff(byte note, byte velocity, byte channel) {
	if (!thru) {

		Serial.write(127 + channel);
		Serial.write(note);
		Serial.write(velocity);
	}
}

void sendTool(byte note, byte velocity) {
	Serial.write(143);
	Serial.write(note);
	Serial.write(velocity);
}

void sendNoteOn(byte note, byte velocity, byte channel) {
	if (!thru) {
		Serial.write(143 + channel);
		Serial.write(note);
		Serial.write(velocity);
	}
}

void midiRead() {
	while (Serial.available()) {
		byte input = Serial.read();
		if (thru) {

			Serial.write(input);
		}

		if (input > 127) {
			// Status
			switch (input) {
				case 248:
					handleClock();
					break; // clock
				case 250:
					handleStart();
					break; // start
				case 251:
					break; // continue
				case 252:
					handleStop();
					break; // stop

				case 128 ... 143:
					mChannel = input - 127;
					mStatus = 2;
					mData = 255;
					break; // noteOff
				case 144 ... 159:
					mChannel = input - 143;
					mStatus = 1;
					mData = 255;
					break; // noteOn
				case 176 ... 191:
					mChannel = input - 175;
					mStatus = 3;
					mData = 255;
					break; // CC
				case 192 ... 207:
					mChannel = input - 191;
					mStatus = 6;
					mData = 0;
					break; // program Change
				case 208 ... 223:
					mChannel = input - 207;
					mStatus = 5;
					mData = 0;
					break; // Aftertouch
				case 224 ... 239:
					mChannel = input - 223;
					mStatus = 4;
					mData = 255;
					break; // Pitch Bend

				default:
					mStatus = 0;
					mData = 255;
					break;
			}
		} else {
			// Data
			if (mData == 255) {
				mData = input;
			} // data byte 1
			else {
				// data byte 2
				switch (mStatus) {
					case 1:
						if (input) {

							if (chord) {
								for (int i = 0; i < 128; i++) {
									if (chordNotes[i]) {
										pedal_adapter.note_on(mChannel, i + mData + midiNoteOffset - chordRoot, input);
									}
								}
							} else {
								pedal_adapter.note_on(mChannel, mData + midiNoteOffset, input);
							}
							// handleNoteOn(mChannel, mData + midiNoteOffset, input);
						} else {

							if (chord) {
								for (int i = 0; i < 128; i++) {
									if (chordNotes[i]) {
										pedal_adapter.note_off(mChannel, i + mData + midiNoteOffset - chordRoot);
									}
								}
							} else {
								pedal_adapter.note_off(mChannel, mData + midiNoteOffset);
							}
							// handleNoteOff(mChannel, mData + midiNoteOffset);
						}
						mData = 255;
						break; // noteOn
					case 2:

						if (chord) {
							for (int i = 0; i < 128; i++) {
								if (chordNotes[i]) {
									pedal_adapter.note_off(mChannel, i + mData + midiNoteOffset - chordRoot);
								}
							}
						} else {
							pedal_adapter.note_off(mChannel, mData + midiNoteOffset);
						}
						// handleNoteOff(mChannel, mData + midiNoteOffset);
						mData = 255;
						break;
					case 3:
						HandleControlChange(mChannel, mData, input);
						mData = 255;
						break;
					case 4:
						handleBendy(mChannel, (input << 7) + mData - 8192);
						mData = 255;
						break;
					case 5:
						handleAftertouch(mChannel, input);
						mData = 255;
						break;
					case 6:
						handleProgramChange(mChannel, input);
						mData = 255;
						break;
					default:
						break;
				}
			}
		}
	}
}
