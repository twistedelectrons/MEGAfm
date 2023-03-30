#include "megafm.h"
#include "lfo.h"
#include "leds.h"

static int lfoCalculated[3];
static bool notRandomed[3];

void applyLfo() {
	// calculate the 3 lfos
	for (int number = 0; number < 3; number++) {
		int lfoModifier = lfo[number] - 128;
		lfoModifier *= 2; // set to +/- 256 range
		float depth = lfoDepth[number];
		depth /= 255;
		lfoModifier = lfoModifier * depth;
		lfoCalculated[number] = lfoModifier;
	}

	// APPLY
	for (int i = 0; i < 51; i++) {
		// IF the number is linked to either lfo grab base
		if ((linked[0][i]) || (linked[1][i]) || (linked[2][i])) {
			fmData[i] = fmBase[i];

			// add lfo accordingly
			if (linked[0][i]) {
				fmData[i] += lfoCalculated[0];
			}
			if (linked[1][i]) {
				fmData[i] += lfoCalculated[1];
			}
			if (linked[2][i]) {
				fmData[i] += lfoCalculated[2];
			}

		} else {
			fmData[i] = fmBase[i];
		}
		fmData[i] = constrain(fmData[i], 0, 255);
	}
}

void lfoBlink() {
	// called when lfo resets to blink the LED
	if ((!bankCounter) && (!showSSEGCounter)) {
		ledSet(16 + lfoShape[selectedLfo], 0);
		if (lfoLedCounter < 15) {
			lfoLedCounter = 15;
			ledSet(16 + lfoShape[selectedLfo], 0);
		}
	}
}

void lfoTick() {
	for (int i = 0; i < 3; i++) {
		if ((i == 0) && (lfoVel) && (fmBase[36] == 0)) {
		} //
		else if ((i == 1) && (lfoMod) && (fmBase[38] == 0)) {
		} //
		else if ((i == 2) && (lfoAt) && (fmBase[40] == 0)) {
		} //
		else {
			switch (lfoShape[i]) {
				case 0:
					if (invertedSquare[i]) {
						if (lfoStep[i] < 127) {
							lfo[i] = 0;
						} else {
							lfo[i] = 255;
						}
					} else {
						if (lfoStep[i] < 127) {
							lfo[i] = 255;
						} else {
							lfo[i] = 0;
						}
					}
					break; // square

				case 1:
					if (lfoStep[i] < 128) {
						lfo[i] = lfoStep[i] << 1;
					} else {
						lfo[i] = 255 - (lfoStep[i] << 1);
					}
					lfo[i] = lfo[i];
					break; // triangle

				case 2:
					if (!invertedSaw[i]) {
						lfo[i] = lfoStep[i];
					} else {
						lfo[i] = 255 - lfoStep[i];
					}
					break; // saw

				case 3:
					if ((sync) && (lfoClockEnable[i])) {
						if (lfoNewRand[i]) {
							lfo[i] = getRandom(i);
							lfoNewRand[i] = false;
						}
					} else {
						if (notRandomed[i]) {
							if (selectedLfo == i) {
								lfoBlink();
							}
							notRandomed[i] = false;
							lfo[i] = getRandom(i);
						}
					}
					break; // noise
			}
		}

		bool apply = false;

		for (int i = 0; i < 3; i++) {
			if (lfoLast[i] != lfo[i]) {
				lfoLast[i] = lfo[i];
				apply = true;
			}
		}

		if (apply)
			applyLfo();
	}
}

void lfoAdvance() {
	// LFO
	for (int i = 0; i < 3; i++) {
		if ((i == 0) && (lfoVel) && (fmBase[36] == 0)) {
		} //
		else if ((i == 1) && (lfoMod) && (fmBase[38] == 0)) {
		} //
		else if ((i == 2) && (lfoAt) && (fmBase[40] == 0)) {
		} //
		else {
			if ((lfoClockEnable[i]) && (sync)) {
			} else {
				lfoCounter[i] += lfoSpeed[i];
			}

			while (lfoCounter[i] >= 512) {
				lfoCounter[i] -= 512;
				if (lfoStep[i] < 255) {
					lfoStep[i]++;
				} else if (lfoStep[i] == 255) {
					if (looping[i]) {
						if (i == selectedLfo) {
							lfoBlink();
						}
						notRandomed[i] = true;
						lfoStep[i] = 0;
					} else {
						lfoStep[i] = 255;
					}
				}
			}
		}
	}
}

void clearLinks() {
	cleared = true;
	for (int i = 0; i < 51; i++) {
		linked[chainPressed - 1][i] = 0;
	}
	digit(0, 10);
	digit(1, 11);
	showLink();
}

byte lastSelectedlfo;

byte getRandom(byte number) {
	if ((looping[number]) && (noiseTableLength[number] != 2)) {
		randomIndex[number]++;
		if (randomIndex[number] > 1 << noiseTableLength[number]) {
			randomIndex[number] = 0;
		}
		return (lfoRandom[number][randomIndex[number]]);
	} else {
		return (random(255));
	}
}

void fillRandomLfo(byte number) {
	for (int i = 0; i < 1 << noiseTableLength[number]; i++) {
		lfoRandom[number][i] = random(255);
	}
}

void lfoLedOn() { ledSet(16 + lfoShape[selectedLfo], 1); }

void fillAllLfoTables() {
	for (int i = 0; i < 32; i++) {
		lfoRandom[0][i] = random(255);
		lfoRandom[1][i] = random(255);
		lfoRandom[2][i] = random(255);
	}
}
