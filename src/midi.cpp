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
#include "nrpn.h"
#include "sysex.h"
#include "pots.h"
#include "buttons.h"
#include "midi_pedal.hpp"
#include "FM.h"
#include "setters.h"

static byte voiceSlot;
static bool ch3Alt;
static float vibIndexF;

static int arpClockCounter;
static byte syncLfoCounter;

static bool resetNextArpRecStep = false;

void resyncArpLfo() {
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

void handleAftertouch(byte channel, byte val) {
	leftDot();
	if (mpe) {
		if (channel > 1 && channel < 14) {
			channel -= 2;
			polyPressure[channel] = val << 1;
			lastMpeVoice = channel;
		}
	} else {

		if (channel == inputChannel) {
			if (lfoAt) {
				// MONO AT IS SET TO OVERIDE LFO3
				for (int i = 0; i < 12; i++) {
					polyPressure[i] = val << 1;
				}
			}
		}
	}
}

void handlePolyAT(byte channel, byte note, byte val) {

	leftDot();
	if (channel == inputChannel && lfoAt) {

		for (int i = 0; i < 12; i++) {
			if (notey[i] == note - 10) {
				polyPressure[i] = val << 1;
				lastMpeVoice = i;
				latestChannel = i;
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
				vibIndexF += kLfoClockRates[map(fmData[48], 0, 255, 0, 12)];
				if (vibIndexF > 255) {
					vibIndexF -= 256;
				}
				vibIndex = int(vibIndexF);
			}
		}

		masterClockCounter++;
		if (masterClockCounter >= 48) {
			leftDot();
			masterClockCounter = 0;

			resyncArpLfo();
		}

		////////////////////////////////////
		////////        ARP          ///////
		///////////////////////////////////

		if ((arpClockEnable) && (arpMode) && (arpMode != 7)) {
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

		if (channel > 1 && channel < 14) {

			// channel number comes in at 2-13 (set end ch. of MPE controler to 13 (master ch. to 1))
			channel -= 2; // offset to 0-11

			bendy = 0;

			if (bend != 0) {
				if (bend < 0) {
					mpeBend[channel] = bend;
					mpeBend[channel] /= 8192;
					mpeBend[channel] *= 48;
				} else if (bend > 0) {
					mpeBend[channel] = bend;
					mpeBend[channel] /= 8191;
					mpeBend[channel] *= 48;
				}
			}

			setNote(channel, notey[channel]);
		}
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
	resyncArpLfo();
	masterClockCounter = 0;
	seqStep = 0;
	arpClockCounter = 0;
	sync = true;

	for (int i = 0; i < 3; i++) {
		if (lfoClockEnable[i]) {
			lfoStepF[i] = lfoStep[i] = 0;
		}
	}
}

void handleContinue() { handleStart(); }

void handleProgramChange(byte channel, byte program) {
	if ((program < 100) && (channel == inputChannel)) {

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
		if (mpe) {
			note += 3;
			if (channel > 1 && channel < 14) {
				channel -= 2;

				ym.noteOff(channel);
				notey[channel] = note;
				setNote(channel, notey[channel]);
				ym.noteOn(channel);

				if (lfoVel) {

					for (int i = 0; i < 12; i++)
						if (i == channel)
							polyVel[i] = velocity << 1;
				}
				latestChannel = channel;
			}
		} else {

			if (channel == inputChannel) {

				note += 3;
				rootNote = note;

				if (velocity > 0) {

					// Retrigger LFOs
					for (int i = 0; i < 3; i++) {
						if ((retrig[i]) || ((!retrig[i]) && (!looping[i]) && (heldKeys == 0))) {
							lfoStep[i] = 0;
						}
					}

					// Reset Arpeggiator
					if (arpMode == kArpSequence1) {
						arpStep = seqStep = 0;
						arpIndex = 0;
					} else if (arpMode == kArpSequence2) {
						arpCounter = 1023;
					} // next manual arp step

					int nVoices, ymfChannelsPerVoice;
					switch (voiceMode) {
						case kVoicingPoly12:
						case kVoicingWide6:
						case kVoicingWide4:
						case kVoicingWide3:

							if (arpMode) {
								// ARP

								heldKeys++;

								if (heldKeys == 1) {
									clearNotes();
									heldKeys = 1;
								}

								if ((heldKeys == 1) && (!sync)) {
									arpCounter = 1023;
								} // only retrigger arp on first key or if arp is stopped

								addNote(note);

							} else {
								// NO ARP

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
								latestChannel = voiceSlot;

								lastNotey[heldKeys] = note;

								for (int i = 0; i < ymfChannelsPerVoice; i++) {
									setNote(ymfChannelsPerVoice * voiceSlot + i, noteOfVoice[voiceSlot]);
									ym.noteOff(ymfChannelsPerVoice * voiceSlot + i);
									ym.noteOn(ymfChannelsPerVoice * voiceSlot + i);
								}

								if (heldKeys < 127)
									heldKeys++;
							}
							if (lfoVel && velocity) {
								// set velocity amount to channel
								for (int ch = 0; ch < 12; ch++) {
									if ((notey[ch] == note) || arpMode) {
										polyVel[ch] = velocity << 1;
									}
								}
							}
							break;

						case kVoicingDualCh3:
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							// dual CH3
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							if (arpMode) {
								// ARP

								heldKeys++;

								if (heldKeys == 1) {
									clearNotes();
									heldKeys = 1;
								}

								if ((heldKeys == 1) && (!sync)) {
									arpCounter = 1023;
								} // only retrigger arp on first key or if arp is stopped

								addNote(note);

							} else {
								// NO ARP

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
							}
							if (lfoVel && velocity) {
								// set velocity amount to channel
								for (int ch = 0; ch < 12; ch++) {
									if ((notey[ch] == note - 10) || arpMode) {
										polyVel[ch] = velocity << 1;
									}
								}
							}
							latestChannel = 0;
							break;

						case kVoicingUnison:
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							// unison
							////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
							///////   ////
							if (arpMode) {
								// ARP

								heldKeys++;

								if (heldKeys == 1) {
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
									if ((lfoVel && velocity) || arpMode)
										polyVel[i] = velocity << 1;
									ym.noteOff(i);
									setNote(i, note);
									ym.noteOn(i);
								}
								latestChannel = 0;
							}

							break;
						default:
							break;
					}

					if (seqRec) {
						if (resetNextArpRecStep) {
							seqLength = 0;
							resetNextArpRecStep = false;
						}
						if (!seqLength)
							rootNote1 = note;
						seq[seqLength] = note - rootNote1 + 127;
						if (seq[seqLength] == 255) {
							seq[seqLength]--;
						}

						sendNRPN(NRPN_ARP_SET_STEP, (seqLength << 8) | seq[seqLength]);

						seqLength++;
						if (seqLength > 15) {
							ledNumber(1);
							resetNextArpRecStep = true;
						} else
							ledNumber(seqLength + 1);

						for (int i = 0; i < 12; i++) {
							ym.noteOff(i);
							setNote(i, note);
							ym.noteOn(i);
						}
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

			if (channel > 1 && channel < 14)
				ym.noteOff(channel - 2);
		} else {

			if (channel == inputChannel) {

				note += 3;

				int nVoices, ymfChannelsPerVoice;
				switch (voiceMode) {
					case kVoicingPoly12:
					case kVoicingWide6:
					case kVoicingWide4:
					case kVoicingWide3:

						if (arpMode) {
							// ARP

							heldKeys--;
							removeNote(note);

						} else {
							// no arp
							nVoices = nVoicesForMode(voiceMode);
							ymfChannelsPerVoice = 12 / nVoices;

							heldKeys--;
							// scan through the noteOfVoices and kill the voice associated to it
							for (int v = 0; v < nVoices; v++) {
								if ((voiceSlots[v]) && (noteOfVoice[v] == note)) {
									voiceSlots[v] = 0;
									for (int i = 0; i < ymfChannelsPerVoice; i++) {
										ym.noteOff(ymfChannelsPerVoice * v + i);
									}
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

						} else {
							// NO ARP

							heldKeys--;
							removeNote(note);

							// LAST KEY UP?
							if (heldKeys < 1) {

								heldKeys = 0;
								clearNotes();

								for (int i = 0; i < 12; i++) {
									ym.noteOff(i);
								}

							} else {
								// NOT LAST KEY, CHANGE NOTE

								switch (notePriority) {
									case NOTE_PRIORITY_LOWEST:
										note = getLow();
										break; // LOWEST
									case NOTE_PRIORITY_HIGHEST:
										note = getHigh();
										break; // HIGHEST
									case NOTE_PRIORITY_LAST:
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
				if (heldKeys < 1) {
					heldKeys = 0;

					clearNotes();
					for (int i = 0; i < 12; i++) {
						ym.noteOff(i);
					}
				}
			}
		}
	}
}

byte lastData1, lastData2;

void handleControlChange(byte channel, byte number, byte val) {
	byte temp;

	if (number == 74) {

		if (mpe && channel > 1 && channel < 14) {
			channel -= 2;
			// this is mpe pressure.
			polyPressure[channel] = val << 1;
			lastMpeVoice = channel;
		}
	} else {
		if (toolMode && channel == 16) {
			switch (number) {
				case 1:
					if (val) {
						thru = 0;
					} else {
						thru = 1;
					}
					setThru();
					break;
				case 2:
					if (val) {
						ignoreVolume = 1;
					} else {
						ignoreVolume = 0;
					}
					setIgnoreVolume();
					break;
				case 3:
					if (val) {
						lfoClockEnable[0] = true;
					} else {
						lfoClockEnable[0] = false;
					}
					setLFO1Clock();
					break;
				case 4:
					if (val) {
						lfoClockEnable[1] = true;
					} else {
						lfoClockEnable[1] = false;
					}
					setLFO2Clock();
					break;
				case 5:
					if (val) {
						lfoClockEnable[2] = true;
					} else {
						lfoClockEnable[2] = false;
					}
					setLFO3Clock();
					break;
				case 6:
					if (val) {
						vibratoClockEnable = true;
					} else {
						vibratoClockEnable = false;
					}
					setVibratoClock();
					break;
				case 7:
					if (val) {
						arpClockEnable = true;
					} else {
						arpClockEnable = false;
					}
					setArpClock();
					break;
				case 18:
					mydisplay.setIntensity(0, constrain(val, 1, 15));
					EEPROM.update(3965, constrain(val, 0, 15));
					break;
				case 19:
					if (val) {
						fatMode = true;
					} else {
						fatMode = false;
					}
					setFatMode();
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
					setPickupMode();
					break;
				case 9:
					if (val) {
						mpe = true;
					} else {
						mpe = false;
					}
					setMPEMode();
					break;
				case 10:
					if (val) {
						lfoVel = true;
					} else {
						lfoVel = false;
					}
					setLFO1Vel();
					break;
				case 11:
					if (val) {
						lfoMod = true;
					} else {
						lfoMod = false;
					}
					setLFO2Mod();
					break;
				case 12:
					if (val) {
						lfoAt = true;
					} else {
						lfoAt = false;
					}
					setLFO3Aftertouch();
					break;
				case 13:
					if (val) {
						stereoCh3 = true;
					} else {
						stereoCh3 = false;
					}
					setStereoCh3();
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
					if (val) {
						newWide = true;
					} else {
						newWide = false;
					}
					EEPROM.update(3970, newWide);
					break;
			}
		}

		if (channel == 16 && lastData1 == 19 && lastData2 == 82 && number == 19 && val == 82) {
			// transmit all the settings to tool. Tool expects noteOff messages on CH16 (yeah I couldn't get webMidi
			// to parse sysex... )

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

			if (number == 64) {
				if (mpe) {
					for (int ch = 0; ch < 16; ch++)
						pedal_adapter.set_pedal(ch, val >> 6);
				} else {
					pedal_adapter.set_pedal(channel, val >> 6);
				}
			}

			else if (channel == inputChannel) {
				if (number == 0) {
					if (val < 6) {
						if (bank != val) {
							bank = val;
							handleProgramChange(inputChannel, preset);
						}
					}
				} else if (number == 1) { // Modulation
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
				} else if (number == 99) {
					// NRPN BEGIN ////////////////////////////////////
					nrpn_msg = val;
					nrpn_state = 1;
				} else if ((number == 98) && (nrpn_state == 1)) {
					nrpn_msg = (nrpn_msg << 7) | val;
					nrpn_state = 2;
				} else if ((number == 6) && (nrpn_state == 2)) {
					nrpn_data = val;
					nrpn_state = 3;
				} else if ((number == 38) && (nrpn_state == 3)) {
					nrpn_data = (nrpn_data << 7) | val;
					handleNRPN(nrpn_msg, nrpn_data);
					// Handle NRPN with 14 bit message
					nrpn_msg = 0;
					nrpn_data = 0;
					nrpn_state = 0;
					// NRPN DONE //////////////////////////////////////
				} else {
					byte pot = number;
					// Rewrite CC -> POT for some
					if (number == 50)
						pot = 0;
					else if (number == 51)
						pot = 7;
					else if (number == 49)
						pot = 8;
					else if (number == 7)
						pot = 1;

					movedPot(pot, val << 1, 1);
				}
			}
		}
		lastData1 = number;
		lastData2 = val;
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

void resetMidiReadStatus() {
	mStatus = 0;
	mData = 0;
}

void midiRead() {
	while (Serial.available()) {
		byte input = Serial.read();
		if (thru) {
			Serial.write(input);
		}

		if (input > 127) {
			// Status
			if ((mStatus == 8) && (input == 247)) { // input == F7
				// In SysEx and receivd Sysex end; handle data and end sysex mode
				sysExAppendByte(247);
				handleIncomingSysEx();
			} else if ((mStatus == 8) && ((input != 248) || (input != 250) || (input != 251) || (input != 252))) {
				// In SysEx but received a non-F7 status byte that is not clock, start, continue or stop; this is an
				// error
				sysExExitStatus(SYSEX_STATUS_BYTE_ERROR);
			}
			switch (input) {
				case 248:
					handleClock();
					break; // clock
				case 250:
					handleStart();
					break; // start
				case 251:
					handleContinue();
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
				case 160 ... 175:
					mChannel = input - 159;
					mStatus = 7;
					mData = 255;
					break; // Poly AfterTouch
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
				case 240:  // F0
					// SysEx start
					mStatus = 8;
					mData = 0;
					sysExReset();
					sysExAppendByte(240);
					break;
				// case 247=F7, end of sysex, is handeled above
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
						handleControlChange(mChannel, mData, input);
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
					case 7:
						handlePolyAT(mChannel, mData, input);
						mData = 255;
						break;
					case 8:
						sysExAppendByte(input);
						mData = 0;
						break;
					default:
						break;
				}
			}
		}
	}
}
