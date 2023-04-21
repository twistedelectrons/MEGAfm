#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"
#include "buttons.h"
#include "leds.h"
#include "preset.h"
#include "dump.h"
#include "lfo.h"
#include "midi.h"
#include "voice.h"
#include "FM.h"

static bool resetFunction = false;

void buttChanged(Button number, bool value) {
	if (millis() > 1000) {
		if (setupMode) {
			if (!value && millis() > 2000) {
				byte temp;

				switch (number) {
					case kButtonChainLfo1:
						thru = !thru;
						ledSet(13, thru);

						digit(0, 26);
						if (thru) {
							digit(1, 1);
						} else {
							digit(1, 0);
						}
						delay(800);

						break; // chain1

					case kButtonChainLfo2:
						pickupMode = !pickupMode;
						ledSet(14, pickupMode);

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
						break; // chain 2

					case kButtonRetrig:
						notePriority++;
						if (notePriority > 2)
							notePriority = 0;
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
						break; // retrig

					case kButtonChainLfo3:
						stereoCh3 = !stereoCh3;
						ledSet(15, stereoCh3);
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

						break; // chain 3

					case kButtonArpMode:
						setupChanged = true;
						mpe = !mpe;
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
						ledSet(23, !mpe);
						break; // MPE mode

					case kButtonNoise:
						fatSpreadMode = !fatSpreadMode;
						ledSet(19, fatSpreadMode);
						break; // NOISE

					case kButtonVoiceMode:
						//////////    //////////    //////////    //////////    //////////    //////////
						//////////    //////////   QUITTING SETUP, SAVING VALUES     //////////    //////////
						//////////    //////////    //////////    //////////    //////////    //////////
						setupMode = false; // setupExit=true;
						justQuitSetup = true;
						// save setup values here

						ledSet(9, 0);
						ledSet(13, 0);
						ledSet(14, 0);
						ledSet(15, 0);

						if (arpMode) {
							ledSet(23, 1);
						} else {
							ledSet(23, 0);
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

						EEPROM.update(3961, lfoVel);
						EEPROM.update(3962, lfoMod);
						EEPROM.update(3963, lfoAt);

						showVoiceMode(voiceMode);

						digit(0, 21);
						digit(1, 21);

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
						ledSet(16 + bank, 1);
						flashCounter2 = 0;
						showSendReceive();

						break; // square
					case kButtonTriangle:
						if (bank == 1) {
							ab = !ab;
						}
						bank = 1;
						ledSet(16 + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // triangle
					case kButtonSaw:
						if (bank == 2) {
							ab = !ab;
						}
						bank = 2;
						ledSet(16 + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // saw
					case kButtonNoise:
						if (bank == 3) {
							ab = !ab;
						}
						bank = 3;
						ledSet(16 + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // noise
					case kButtonRetrig:
						if (bank == 4) {
							ab = !ab;
						}
						bank = 4;
						ledSet(16 + bank, 1);
						flashCounter2 = 0;
						showSendReceive();
						break; // retrig
					case kButtonLoop:
						if (bank == 5) {
							ab = !ab;
						}
						bank = 5;
						ledSet(16 + bank, 1);
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
						justQuitSetup = false;
						fineChanged = false;
						voiceHeld = true;
						setupCounter = 16000;
						break; // voice mode

					case kButtonPresetUp:
						if (!seqRec) {
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
						break; // preset up

					case kButtonPresetDown:
						if (!seqRec) {
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
							bank = 0;
							ledSet(16 + bank, 1);
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
								if (lfoShape[selectedLfo] == 0) {
									invertedSquare[selectedLfo] = !invertedSquare[selectedLfo];
								} else {
									lfoShape[selectedLfo] = 0;
									lfoLedOn();
									showLfo();
								}
							}
						}
						break; // square

					case kButtonTriangle:
						if (bankCounter) {
							bank = 1;
							ledSet(16 + bank, 1);
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
								lfoShape[selectedLfo] = 1;
								lfoLedOn();
								showLfo();
							}

							else {
								setSSEG(lastOperator, 0, 1); // operator bitIndex value
							}
						}
						break; // triangle

					case kButtonSaw:
						if (bankCounter) {
							bank = 2;
							ledSet(16 + bank, 1);
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
								if (lfoShape[selectedLfo] == 2) {
									invertedSaw[selectedLfo] = !invertedSaw[selectedLfo];

									if (invertedSaw[selectedLfo]) {
										digit(0, 5);
										digit(1, 17);
									} else {
										digit(0, 16);
										digit(1, 17);
									}
								} else {
									lfoShape[selectedLfo] = 2;
									lfoLedOn();
									showLfo();
									if (invertedSaw[selectedLfo]) {
										digit(0, 5);
										digit(1, 17);
									} else {
										digit(0, 16);
										digit(1, 17);
									}
								}
							} else {
								setSSEG(lastOperator, 0, 0); // operator bitIndex value}
							}
						}
						break; // saw

					case kButtonNoise:
						if (bankCounter) {
							bank = 3;
							ledSet(16 + bank, 1);
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
								if (lfoShape[selectedLfo] == 3) {
									noiseTableLength[selectedLfo]++;
									if (noiseTableLength[selectedLfo] > 5) {
										noiseTableLength[selectedLfo] = 2;
									}

									if (noiseTableLength[selectedLfo] == 2) {
										digit(0, 21);
										digit(1, 21);
									} else {
										ledNumber(1 << noiseTableLength[selectedLfo]);
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
								}
								lfoShape[selectedLfo] = 3;
								fillRandomLfo(selectedLfo);
								showLfo();
							}
						}
						break; // noise

					case kButtonRetrig:
						if (bankCounter) {
							bank = 4;
							ledSet(16 + bank, 1);
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
								sendCC(57, retrig[selectedLfo]);
							}
						}
						break; // retrig
					case kButtonLoop:
						if (bankCounter) {
							bank = 5;
							ledSet(16 + bank, 1);
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
								sendCC(58, looping[selectedLfo]);
							} else {
								setSSEG(lastOperator, 1, !bitRead(SSEG[lastOperator], 1)); // flip the SSEG enable bit
							}
						}
						break; // loop
					default:
						break;
				}
			} else {
				// Released
				switch (number) {
					case kButtonVoiceMode:
						voiceHeld = false;
						if (recHeld) {
							noRecAction = false;
							chord = !chord;
							if (chord) {
								digit(0, 10);
								digit(1, 23);
								for (int i = 128; i > 0; i--) {
									chordNotes[i] = heldNotes[i]; // copy held to chord buffer
									if (heldNotes[i])
										chordRoot = i; // root is lowest note of the held notes
								}
							} else {
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
										showVoiceMode(voiceMode);
										sendCC(51, voiceMode);

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
								seqLength++;
								ledNumber(seqLength);
							}
						} else {
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
						break; // preset up

					case kButtonPresetDown:
						pressedDown = false;
						if (seqRec) {
							if (seqLength > 1) {
								seqLength--;
								ledNumber(seqLength);
							}
						} else {
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
						break; // preset down

					case kButtonPresetReset:
						if (!shuffled) {
							if (!resetFunction) {
								loadZero();
								digit(0, 14);
								digit(1, 0);
							} else {
								panel();
							}
							resetFunction = !resetFunction;
						}
						resetHeld = false;
						break; // reset

					case kButtonArpMode:
						arpModeHeld = false;

						if (!mpe) {
							if (!changedChannel) {

								if ((arpMode == 0) && (arpJustWentOff)) {
									arpJustWentOff = false;
								} else if ((arpMode == 0) && (!arpJustWentOff)) {

									resetVoices();

									ledSet(23, 1);
									arpMode = arpModeLast;
									switch (arpMode) {
										case 1:
											digit(0, 13);
											digit(1, 14);
											break; // up
										case 2:
											digit(0, 15);
											digit(1, 19);
											break; // dn
										case 3:
											digit(0, 13);
											digit(1, 15);
											break; // ud
										case 4:
											digit(1, 1);
											digit(0, 16);
											break; // r1
										case 5:
											digit(1, 2);
											digit(0, 16);
											break; // r2
										case 6:
											digit(0, 5);
											digit(1, 18);
											break; // se
									}
								} else if (arpMode > 0) {

									arpMode++;
									if (arpMode > 7) {
										arpMode = 1;
										resyncArp = true;
									}
									arpModeLast = arpMode;

									if ((arpMode) && (voiceMode != kVoicingUnison) && (!mpe)) {
										voiceMode = kVoicingUnison;
										showVoiceMode(voiceMode);
										sendCC(51, voiceMode);
									}

									switch (arpMode) {
										case 1:
											digit(0, 13);
											digit(1, 14);
											break; // up
										case 2:
											digit(0, 15);
											digit(1, 19);
											break; // dn
										case 3:
											digit(0, 13);
											digit(1, 15);
											break; // ud
										case 4:
											digit(1, 1);
											digit(0, 16);
											break; // r1
										case 5:
											digit(1, 2);
											digit(0, 16);
											break; // r2
										case 6:
											digit(0, 5);
											digit(1, 1);
											break; // s1
										case 7:
											digit(0, 5);
											digit(1, 2);
											break; // s2
									}
								}
							}
						}

						break; // arp mode
					case kButtonArpRec:
						if (noRecAction) {
							if (presetTargetMode) {
								presetTargetMode = false;
								savePreset();
							} else {
								if (!seqRec) {
									seqLength = 0;
									if (voiceMode != kVoicingUnison) {
										voiceMode = kVoicingUnison;
										showVoiceMode(voiceMode);
									}
									seqRec = true;
									arpMode = 6;
									ledSet(23, 1);
									digit(0, 5);
									digit(1, 18);
								} else {
									seqRec = false;
									ledSet(22, 0);
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
							}
						}
						break; // chain1

					case kButtonChainLfo2:
						chainPressed = 0;
						if ((targetPot != 38) && (targetPot != 39)) { // prevent lfo linking to itself
							if (!cleared) {
								linked[1][targetPot] = !linked[1][targetPot];
								showLink();
							}
						}
						break; // chain2

					case kButtonChainLfo3:
						chainPressed = 0;
						if ((targetPot != 40) && (targetPot != 41)) { // prevent lfo linking to itself
							if (!cleared) {
								linked[2][targetPot] = !linked[2][targetPot];
								showLink();
							}
						}
						break; // chain3

					case kButtonSquare:   // square
					case kButtonTriangle: // triangle
					case kButtonSaw:      // saw
					case kButtonNoise:    // noise
					case kButtonRetrig:   // retrig
					case kButtonLoop:     // loop
					default:
						break;
				}
			}
		}
	}
}
