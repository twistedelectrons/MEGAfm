#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"
#include "buttons.h"
#include "leds.h"
#include "preset.h"
#include "dump.h"
#include "lfo.h"
#include "midi.h"
#include "nrpn.h"
#include "voice.h"
#include "FM.h"

const byte presetChords[16][6] = {
    {0, 4, 7, 10, 14, 17}, // C Major 7
    {0, 3, 7, 10, 14, 17}, // C Minor 7
    {0, 4, 7, 11, 14, 17}, // C Dominant 7
    {0, 4, 7, 11, 14, 18}, // C Major 7(#11)
    {0, 4, 7, 10, 13, 17}, // C6
    {0, 4, 7, 9, 14, 17},  // C Minor Major 7
    {0, 2, 4, 7, 9, 11},   // Cm6
    {0, 3, 6, 10, 14, 17}, // C Minor 7(b5)
    {0, 5, 7, 10, 14, 17}, // C9
    {0, 4, 7, 10, 14, 19}, // C Major 9
    {0, 4, 7, 10, 14, 15}, // Cadd9
    {0, 4, 7, 11, 14, 21}, // C13
    {0, 2, 4, 7, 9, 10},   // Cmadd9
    {0, 4, 7, 11, 15, 18}, // C7(#9)
    {0, 4, 7, 10, 14, 21}, // C6/9
    {0, 3, 7, 9, 14, 17}   // Cm(Maj7)/C Minor Major 7
};

static bool resetFunction = false;

static void finishSetup() {
	byte temp;
	ledSet(LED_VOICE_MODE_POLY12, 0);
	ledSet(LED_LFO1_LINK, 0);
	ledSet(LED_LFO2_LINK, 0);
	ledSet(LED_LFO3_LINK, 0);

	if (arpMode) {
		ledSet(LED_ARP_MODE, 1);
	} else {
		ledSet(LED_ARP_MODE, 0);
	}

	EEPROM.write(3967, notePriority);

	temp = EEPROM.read(3950);
	bitWrite(temp, 0, !thru);
	bitWrite(temp, 1, ignoreVolume);
	bitWrite(temp, 2, bitRead(noiseTableLength[0] - 2, 0));
	bitWrite(temp, 3, bitRead(noiseTableLength[0] - 2, 1));
	bitWrite(temp, 4, bitRead(noiseTableLength[1] - 2, 0));
	bitWrite(temp, 5, bitRead(noiseTableLength[1] - 2, 1));
	bitWrite(temp, 6, bitRead(noiseTableLength[2] - 2, 0));
	bitWrite(temp, 7, bitRead(noiseTableLength[2] - 2, 1));
	EEPROM.update(3950, temp);

	bitWrite(temp, 0, fatSpreadMode);
	EEPROM.write(3968, fatSpreadMode);

	temp = 0;
	bitWrite(temp, 0, lfoClockEnable[0]);
	bitWrite(temp, 1, lfoClockEnable[1]);
	bitWrite(temp, 2, lfoClockEnable[2]);
	bitWrite(temp, 3, vibratoClockEnable);
	bitWrite(temp, 4, arpClockEnable);
	bitWrite(temp, 5, fatMode);
	EEPROM.update(3953, temp);

	if (pickupMode) {
		EEPROM.update(3954, 1);
	} else {
		EEPROM.update(3954, 0);
	}

	temp = 0;
	bitWrite(temp, 0, showMidiFeedback);
	EEPROM.update(3955, temp);

	EEPROM.update(3961, lfoVel);
	EEPROM.update(3962, lfoMod);
	EEPROM.update(3963, lfoAt);

	showVoiceMode(voiceMode);

	digit(0, 21);
	digit(1, 21);
}

void showArpMode() {
	switch (arpMode) {
		case kArpOff:
			digit(0, 0);
			digit(1, 12);
			break; // off
		case kArpUp:
			digit(0, 13);
			digit(1, 14);
			break; // up
		case kArpDown:
			digit(0, 15);
			digit(1, 19);
			break; // dn
		case kArpUpDown:
			digit(0, 13);
			digit(1, 15);
			break; // ud
		case kArpRandom1:
			digit(0, 16);
			digit(1, 1);
			break; // r1
		case kArpRandom2:
			digit(0, 16);
			digit(1, 2);
			break; // r2
		case kArpSequence1:
			digit(0, 5);
			digit(1, 1);
			break; // s1
		case kArpSequence2:
			digit(0, 5);
			digit(1, 2);
			break; // s2
	}
	if (arpMode > 0) {
		ledSet(LED_ARP_MODE, 1);
	} else {
		ledSet(LED_ARP_MODE, 0);
	}
	lastNumber = -1;
	showPresetNumberTimeout = 12000;
}

static void setNoiseTableLength(byte selectedLfo, byte length) {
	if (length == noiseTableLength[selectedLfo]) {
		return;
	}

	noiseTableLength[selectedLfo] = length;
	if (noiseTableLength[selectedLfo] > 5) {
		noiseTableLength[selectedLfo] = 2;
	}

	byte temp = EEPROM.read(3950);
	bitWrite(temp, 0, !thru);
	bitWrite(temp, 1, ignoreVolume);
	bitWrite(temp, 2, bitRead(noiseTableLength[0] - 2, 0));
	bitWrite(temp, 3, bitRead(noiseTableLength[0] - 2, 1));
	bitWrite(temp, 4, bitRead(noiseTableLength[1] - 2, 0));
	bitWrite(temp, 5, bitRead(noiseTableLength[1] - 2, 1));
	bitWrite(temp, 6, bitRead(noiseTableLength[2] - 2, 0));
	bitWrite(temp, 7, bitRead(noiseTableLength[2] - 2, 1));
	EEPROM.update(3950, temp);
	fillRandomLfo(selectedLfo);
}

void showLfoWaveform(byte selectedLfo) {
	switch (lfoShape[selectedLfo]) {
		case kSquare:
			if (invertedSquare[selectedLfo]) {
				// S-
				digit(0, 5);
				digit(1, 20);
			} else {
				// S
				digit(0, 5);
				digit(1, 21);
			}
			break;
		case kTriangle:
			// tr (iangle)
			digit(0, 26);
			digit(1, 16);
			break;
		case kSaw:
			if (invertedSaw[selectedLfo]) {
				// Ra (mp)
				digit(0, 16);
				digit(1, 17);
			} else {
				// Sa (w)
				digit(0, 5);
				digit(1, 17);
			}
			break;
		case kRandom:
			if (noiseTableLength[selectedLfo] == 2) {
				// blank
				digit(0, 21);
				digit(1, 21);
			} else {
				// 8, 16, 32
				ledNumber(1 << noiseTableLength[selectedLfo]);
			}
			break;
	}

	lastNumber = -1;
	showPresetNumberTimeout = 12000;
}

void showNotePriority() {
	switch (notePriority) {
		case 0:
			digit(0, 11);
			digit(1, 27);
			delay(800);
			break; // LO
		case 1:
			digit(0, 23);
			digit(1, 1);
			delay(800);
			break; // HI
		case 2:
			digit(0, 11);
			digit(1, 17);
			delay(800);
			break; // LA
	}
}

void buttChanged(Button number, bool value) {
	if (millis() > 1000) {
		if (setupMode) {
			if (!value && millis() > 2000) {
				switch (number) {
					case kButtonChainLfo1:
						thru = !thru;
						ledSet(LED_LFO1_LINK, thru);
						digit(0, 26);
						if (thru) {
							digit(1, 1);
						} else {
							digit(1, 0);
						}
						delay(800);
						sendNRPN(NRPN_SET_MIDI_THRU, thru);
						break; // chain1

					case kButtonChainLfo2:
						pickupMode = !pickupMode;
						ledSet(LED_LFO2_LINK, pickupMode);
						if (pickupMode) {
							digit(0, 14);
							digit(1, 14);
							delay(800);
						} else {
							digit(0, 19);
							digit(1, 27);
							delay(400);
							digit(0, 14);
							digit(1, 14);
							delay(400);
						}
						sendNRPN(NRPN_SET_PICKUP_MODE, pickupMode);
						break; // chain 2

					case kButtonRetrig:
						notePriority++;
						if (notePriority > 2)
							notePriority = 0;
						showNotePriority();
						sendNRPN(NRPN_NOTE_PRIORITY, notePriority);
						break; // retrig

					case kButtonChainLfo3:
						stereoCh3 = !stereoCh3;
						ledSet(LED_LFO3_LINK, stereoCh3);
						EEPROM.update(3966, stereoCh3);
						if (stereoCh3) {
							digit(0, 5);
							digit(1, 3);
							delay(800);
						} else {
							digit(0, 15);
							digit(1, 3);
							delay(800);
						}
						sendNRPN(NRPN_SET_STEREO_CH3, stereoCh3);
						break; // chain 3

					case kButtonArpMode:
						mpe = !mpe;
						setupChanged = true;
						if (mpe) {
							digit(0, 17);
							digit(1, 10);
							delay(800);
							EEPROM.write(3958, 48);
							EEPROM.write(3959, 48);
						} else {
							digit(0, 1);
							digit(1, 10);
							delay(800);
							EEPROM.write(3958, bendDown);
							EEPROM.write(3959, bendUp);
						}
						EEPROM.write(3960, mpe);
						ledSet(LED_ARP_MODE, !mpe);
						sendNRPN(NRPN_SET_MPE_MODE, mpe);
						break; // MPE mode

					case kButtonNoise:
						fatSpreadMode = !fatSpreadMode;
						ledSet(LED_RANDOM, fatSpreadMode);
						sendNRPN(NRPN_SET_FAT_SPREAD, fatSpreadMode);
						break; // NOISE

					case kButtonVoiceMode:
						//////////    //////////    //////////    //////////    //////////    //////////
						//////////    //////////   QUITTING SETUP, SAVING VALUES     //////////    //////////
						//////////    //////////    //////////    //////////    //////////    //////////
						setupMode = false; // setupExit=true;
						justQuitSetup = true;
						// save setup values here

						finishSetup();
						break;
					default:
						break;
				}
			}
			// END SETUP BUTTONS
		} else if (sendReceive) {
			if (!value) {
				// Pressed
				switch (number) {
					case kButtonSquare:

						if (bank == 0) {
							ab = !ab;
						}
						bank = 0;
						ledSet(LED_SQUARE + bank, 1);
						flashCounter2 = 0;
						showSendReceive();

						break; // square
					case kButtonTriangle:
						if (bank == 1) {
							ab = !ab;
						}
						bank = 1;
						ledSet(LED_SQUARE + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // triangle
					case kButtonSaw:
						if (bank == 2) {
							ab = !ab;
						}
						bank = 2;
						ledSet(LED_SQUARE + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // saw
					case kButtonNoise:
						if (bank == 3) {
							ab = !ab;
						}
						bank = 3;
						ledSet(LED_SQUARE + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // noise
					case kButtonRetrig:
						if (bank == 4) {
							ab = !ab;
						}
						bank = 4;
						ledSet(LED_SQUARE + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // retrig
					case kButtonLoop:
						if (bank == 5) {
							ab = !ab;
						}
						bank = 5;
						ledSet(LED_SQUARE + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // loop
					case kButtonPresetDown:
						if ((millis() > 2000) && (sendReceive == 2))
							sendDump();
						break;
					case kButtonPresetUp:
						if ((millis() > 2000) && (sendReceive == 1))
							recieveDump();
						break;
					default:
						break;
				}
			}
		} else {
			if (!value) {
				// Pressed
				switch (number) {
					case kButtonArpMode:
						changedChannel = false;
						arpModeHeld = true;
						arpButtCounter = 0;
						break; // arp mode

					case kButtonVoiceMode:
						movedFineKnob = false;
						justQuitSetup = false;
						fineChanged = false;
						voiceHeld = true;
						setupCounter = 16000;
						break; // voice mode

					case kButtonPresetUp:
						if (!seqRec) {
							if (voiceHeld) {
								if (octOffset < 3) {
									octOffset++;
								}
								fineChanged = true; // prevents voice mode change
								setupCounter = 0;   // prevents entering setup
								ledNumber(octOffset);
								sendNRPN(NRPN_SET_OCT_OFFSET, octOffset);
							} else {
								if (arpModeHeld) {
									changedChannel = true;
									if (inputChannel < 16) {
										inputChannel++;
										EEPROM.write(3951, inputChannel);
									}
									ledNumber(inputChannel);
								} else {
									saved = false;
									pressedUp = true;
									scrollDelay = 3000;
									scrollCounter = 0;
									if (pressedDown) {
										if (!presetTargetMode) {
											presetCounts = 40;
											presetTargetMode = true;
											presetTargetModeChanged = 2;
										}
									}
								}
								clearLfoLeds();
								bankCounter = 20;
							}
						}
						break; // preset up

					case kButtonPresetDown:
						if (!seqRec) {
							if (voiceHeld) {
								if (octOffset > 0) {
									octOffset--;
								}
								ledNumber(octOffset);
								fineChanged = true; // prevents voice mode change
								setupCounter = 0;   // prevents entering setup
								sendNRPN(NRPN_SET_OCT_OFFSET, octOffset);
							} else {
								if (arpModeHeld) {
									changedChannel = true;
									if (inputChannel > 1) {
										inputChannel--;
										EEPROM.write(3951, inputChannel);
									}
									ledNumber(inputChannel);
								} else {
									saved = false;
									pressedDown = true;
									scrollDelay = 3000;
									scrollCounter = 0;
									if (pressedUp) {
										if (!presetTargetMode) {
											presetCounts = 40;
											presetTargetMode = true;
											presetTargetModeChanged = 2;
										}
									}
								}
								clearLfoLeds();
								bankCounter = 20;
							}
						}
						break; // preset down

					case kButtonPresetReset:
						shuffled = false;
						resetHeld = true;
						shuffleTimer = 4000;
						break; // reset

					case kButtonArpRec:
						noRecAction = true;
						recHeld = true;
						break; // arp rec

					case kButtonChainLfo1:
						chainPressed = 1;
						linkCounter = 3;
						selectedLfo = 0;
						cleared = false;
						break; // chain1
					case kButtonChainLfo2:
						chainPressed = 2;
						linkCounter = 3;
						selectedLfo = 1;
						cleared = false;
						break; // chain2
					case kButtonChainLfo3:
						chainPressed = 3;
						linkCounter = 3;
						selectedLfo = 2;
						cleared = false;
						break; // chain3

					case kButtonSquare:
						if (bankCounter) {
							if (presetTargetMode) {
								presetCounts = 40;
							}
							bank = 0;
							ledSet(LED_SQUARE + bank, 1);
							flashCounter2 = 0;
							flasher = false;
							EEPROM.write(3964, bank);
							clearLfoLeds();
							if (!presetTargetMode) {
								loadPreset();
								resetFunction = 0;
							}
							bankCounter = 20;
						} else {
							if (!showSSEGCounter) {
								if (lfoShape[selectedLfo] == kSquare) {
									invertedSquare[selectedLfo] = !invertedSquare[selectedLfo];
								} else {
									lfoShape[selectedLfo] = kSquare;
									invertedSquare[selectedLfo] = false;
									showLfo();
								}
								sendNRPN(NRPN_LFO_SHAPE + selectedLfo, invertedSquare[selectedLfo]);
								showLfoWaveform(selectedLfo);
							}
						}
						break; // square

					case kButtonTriangle:
						if (bankCounter) {
							if (presetTargetMode) {
								presetCounts = 40;
							}
							bank = 1;
							ledSet(LED_SQUARE + bank, 1);
							flashCounter2 = 0;
							flasher = false;
							EEPROM.write(3964, bank);
							clearLfoLeds();
							if (!presetTargetMode) {
								loadPreset();
								resetFunction = 0;
							}
							bankCounter = 20;
						} else {
							if (!showSSEGCounter) {
								lfoShape[selectedLfo] = kTriangle;
								showLfo();
								sendNRPN(NRPN_LFO_SHAPE + selectedLfo, 2);
								showLfoWaveform(selectedLfo);
							}

							else {
								setSSEG(lastOperator, 0, 1); // operator bitIndex value
							}
						}
						break; // triangle

					case kButtonSaw:
						if (bankCounter) {
							if (presetTargetMode) {
								presetCounts = 40;
							}
							bank = 2;
							ledSet(LED_SQUARE + bank, 1);
							flashCounter2 = 0;
							flasher = false;
							EEPROM.write(3964, bank);
							clearLfoLeds();
							if (!presetTargetMode) {
								loadPreset();
								resetFunction = 0;
							}
							bankCounter = 20;
						} else {
							if (!showSSEGCounter) {
								if (lfoShape[selectedLfo] == kSaw) {
									invertedSaw[selectedLfo] = !invertedSaw[selectedLfo];
								} else {
									lfoShape[selectedLfo] = kSaw;
									invertedSaw[selectedLfo] = false;
									showLfo();
								}
								sendNRPN(NRPN_LFO_SHAPE + selectedLfo, 3 + invertedSaw[selectedLfo]);
								showLfoWaveform(selectedLfo);
							} else {
								setSSEG(lastOperator, 0, 0); // operator bitIndex value}
							}
						}
						break; // saw

					case kButtonNoise:
						if (bankCounter) {
							if (presetTargetMode) {
								presetCounts = 40;
							}
							bank = 3;
							ledSet(LED_SQUARE + bank, 1);
							flashCounter2 = 0;
							flasher = false;
							EEPROM.write(3964, bank);
							clearLfoLeds();
							if (!presetTargetMode) {
								loadPreset();
								resetFunction = 0;
							}
							bankCounter = 20;
						} else {
							if (!showSSEGCounter) {
								// Change LFOLENGTH
								if (lfoShape[selectedLfo] == kRandom) {
									setNoiseTableLength(selectedLfo, noiseTableLength[selectedLfo] + 1);
								} else {
									lfoShape[selectedLfo] = kRandom;
									setNoiseTableLength(selectedLfo, 2);
									showLfo();
								}
								sendNRPN(NRPN_LFO_SHAPE + selectedLfo, noiseTableLength[selectedLfo] + 3);

								showLfoWaveform(selectedLfo);
							}
						}
						break; // noise

					case kButtonRetrig:
						if (bankCounter) {
							if (presetTargetMode) {
								presetCounts = 40;
							}
							bank = 4;
							ledSet(LED_SQUARE + bank, 1);
							flashCounter2 = 0;
							flasher = false;
							EEPROM.write(3964, bank);
							clearLfoLeds();
							if (!presetTargetMode) {
								loadPreset();
								resetFunction = 0;
							}
							bankCounter = 20;
						} else {
							if (!showSSEGCounter) {
								retrig[selectedLfo] = !retrig[selectedLfo];
								showLfo();
								sendNRPN(NRPN_LFO_RETRIG + selectedLfo, retrig[selectedLfo]);
							}
						}
						break; // retrig
					case kButtonLoop:
						loopChanged = false;
						loopHeld = true;
						break; // loop
					default:
						break;
				}
			} else {
				// Released
				switch (number) {
					case kButtonVoiceMode:
						movedFineKnob = false;
						voiceHeld = false;
						if (recHeld) {
							noRecAction = false;
							chord = !chord;
							if (chord) {

								bool noNotes = true;
								digit(0, 10);
								digit(1, 23);
								for (int i = 128; i > 0; i--) {
									chordNotes[i] = heldNotes[i]; // copy held to chord buffer
									if (heldNotes[i]) {
										chordRoot = i; // root is lowest note of the held notes
										noNotes = false;
									}
								}

								if (noNotes) {
									chordRoot = 36;
									for (int i = 0; i < 6; i++) {

										if (presetChordNumber % 2 == 0) { // alternate between 3 and 6 note chords
											if (i < 3) {
												chordNotes[36 + presetChords[presetChordNumber][i]] = 1;
											}
										} else {
											chordNotes[36 + presetChords[presetChordNumber][i]] = 1;
										}
									}
								}

							} else {
								presetChordNumber++;
								if (presetChordNumber > 15) {
									presetChordNumber = 0;
								}
								for (int i = 128; i > 0; i--) {
									chordNotes[i] = heldNotes[i] = 0;
								}
								heldKeys = 0;
								for (int i = 0; i < 12; i++) {
									ym.noteOff(i);
								}
								digit(0, 27);
								digit(1, 12);
							} // copy the held notes to the chord buffer

						} else {
							if (!justQuitSetup) {
								if (!fineChanged) {

									if (!mpe) {
										//                             Poly12         Wide6          dualCh3 unison
										//                             Wide4          Wide3

										VoiceMode nextVoiceMode[6] = {kVoicingWide6,  kVoicingWide4, kVoicingUnison,
										                              kVoicingPoly12, kVoicingWide3, kVoicingDualCh3};

										if (voiceMode < kVoiceModeCount) {
											voiceMode = nextVoiceMode[voiceMode];
										} else {
											voiceMode = kVoicingPoly12;
										}
										if (!newWide && (voiceMode == kVoicingWide4 || voiceMode == kVoicingWide3)) {
											voiceMode = kVoicingDualCh3;
										}
										showVoiceMode(voiceMode);
										sendNRPN(NRPN_SET_VOICE_MODE, voiceMode);

										// Reset notes after a voiceChange
										resetVoices();

									} else {
										digit(0, 17);
										digit(1, 10);
									}
								}
							}
						}

						break; // voice mode

					case kButtonPresetUp:
						pressedUp = false;
						if (seqRec) {
							if (seqLength < 16) {
								seq[seqLength] = 255;
								sendNRPN(NRPN_ARP_SET_STEP, (seqLength << 8) | 255);
								seqLength++;
								ledNumber(seqLength + 1);
							}
						} else {
							if (!voiceHeld) {

								if (!saved) {
									if (presetTargetMode) {
										if (presetTargetModeChanged) {
											presetTargetModeChanged--;
										} else {
											presetCounts = 40;
											if (!scrollCounter) {
												preset++;
												if (preset > 99) {
													preset = 0;
												}
											}
										}
									} else {
										if (!scrollCounter) {
											preset++;
											if (preset > 99) {
												preset = 0;
											}
										}
										loadPreset();
										resetFunction = 0;
										EEPROM.write(3952, preset);
									}
								}
							}
						}
						break; // preset up

					case kButtonPresetDown:
						pressedDown = false;
						if (seqRec) {
							if (seqLength > 0) {
								seqLength--;
								ledNumber(seqLength + 1);
							}
						} else {
							if (!voiceHeld) {
								if (!saved) {
									if (presetTargetMode) {
										if (presetTargetModeChanged) {
											presetTargetModeChanged--;
										} else {
											presetCounts = 40;
											if (!scrollCounter) {
												preset--;
												if (preset < 0) {
													preset = 99;
												}
											}
										}
									} else {
										if (!scrollCounter) {
											preset--;
											if (preset < 0) {
												preset = 99;
											}
										}
										loadPreset();
										resetFunction = 0;
										EEPROM.write(3952, preset);
									}
								}
							}
						}
						break; // preset down

					case kButtonPresetReset:
						if (presetTargetMode) {
							presetTargetMode = false;
						} else {
							if (!shuffled) {
								if (!resetFunction) {
									loadZero();
									dumpPresetAsSysEx();
									digit(0, 14);
									digit(1, 0);
								} else {
									panel();
								}
								resetFunction = !resetFunction;
							}
						}
						resetHeld = false;

						break; // reset

					case kButtonArpMode:
						arpModeHeld = false;

						if (!mpe) {
							if (!changedChannel) {

								if ((arpMode == kArpOff) && (arpJustWentOff)) {
									arpJustWentOff = false;
								} else if ((arpMode == kArpOff) && (!arpJustWentOff)) {

									resetVoices();

									arpMode = arpModeLast;
									sendNRPN(NRPN_ARP_MODE, arpMode);
									showArpMode();
									ledSet(LED_ARP_MODE, 1);
								} else if (arpMode > 0) {

									arpMode++;
									if (arpMode >= kArpModeCount) {
										arpMode = kArpUp;
										resyncArp = true;
									}
									arpModeLast = arpMode;
									sendNRPN(NRPN_ARP_MODE, arpMode);
									showArpMode();
								}
							}
						}

						break; // arp mode
					case kButtonArpRec:
						if (noRecAction) {
							if (mpe) {
								// no seq in mpe mode
							} else {

								if (presetTargetMode) {
									presetTargetMode = false;
									savePreset();
								} else {
									if (!seqRec) {
										seqLength = 0;
										seqRec = true;
										displayFreeze = 0;
										arpMode = kArpSequence1;
										sendNRPN(NRPN_ARP_MODE, arpMode);
										ledSet(LED_ARP_MODE, 1);
										digit(0, 5);
										digit(1, 18);
									} else {
										seqRec = false;
										ledSet(LED_SEQ_REC, 0);
									}
								}
							}
						}
						recHeld = false;
						break;
					case kButtonChainLfo1:
						chainPressed = 0;
						if ((targetPot != 36) && (targetPot != 37)) { // prevent lfo linking to itself
							if (!cleared) {
								linked[0][targetPot] = !linked[0][targetPot];
								showLink();
								sendNRPN(NRPN_LFO_LINK, 2 * targetPot + linked[0][targetPot]);
							}
						}
						break; // chain1

					case kButtonChainLfo2:
						chainPressed = 0;
						if ((targetPot != 38) && (targetPot != 39)) { // prevent lfo linking to itself
							if (!cleared) {
								linked[1][targetPot] = !linked[1][targetPot];
								showLink();
								sendNRPN(NRPN_LFO_LINK + 1, 2 * targetPot + linked[1][targetPot]);
							}
						}
						break; // chain2

					case kButtonChainLfo3:
						chainPressed = 0;
						if ((targetPot != 40) && (targetPot != 41)) { // prevent lfo linking to itself
							if (!cleared) {
								linked[2][targetPot] = !linked[2][targetPot];
								showLink();
								sendNRPN(NRPN_LFO_LINK + 2, 2 * targetPot + linked[2][targetPot]);
							}
						}
						break; // chain3
					case kButtonLoop:
						loopHeld = false;
						if (!loopChanged) {
							if (bankCounter) {
								if (presetTargetMode) {
									presetCounts = 40;
								}
								bank = 5;
								ledSet(LED_SQUARE + bank, 1);
								flashCounter2 = 0;
								flasher = false;
								EEPROM.write(3964, bank);
								clearLfoLeds();
								if (!presetTargetMode) {
									loadPreset();
									resetFunction = 0;
								}
								bankCounter = 20;
							} else {
								if (!showSSEGCounter) {

									looping[selectedLfo] = !looping[selectedLfo];
									showLfo();
									sendNRPN(NRPN_LFO_LOOPING + selectedLfo, looping[selectedLfo]);
								} else {
									setSSEG(lastOperator, 1,
									        !bitRead(SSEG[lastOperator], 1)); // flip the SSEG enable bit
								}
							}
						}
						break; // loop

					case kButtonSquare:   // square
					case kButtonTriangle: // triangle
					case kButtonSaw:      // saw
					case kButtonNoise:    // noise
					case kButtonRetrig:   // retrig

					default:
						break;
				}
			}
		}
	}
}
