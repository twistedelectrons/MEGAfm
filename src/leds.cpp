/*
  0
5   1
  6
4   2
  3   7




  1-a1
  2-a2
  3-a3
  4-a4
  5-a5
  6-a6
  7-a7
  8-a8

  9-poly12
  10-wide6
  11-dual ch3
  12-unison


  13-chain1
  14-chain2
  15-chain3

  16-square
  17-tri?
  18-saw1
  19-noise

  20-retrig
  21-loop

  22-rec

  23-am1
  24-am2
  25-am3
  26-am4

  27-fm1
  28-fm2
  29-fm3
  30-fm4

*/
#include <Arduino.h>
#include "leds.h"
#include "megafm.h"
#include "voice.h"

void showNumber(byte movedPotOrFader, byte value) {

	int theNumber = -1;

	switch (movedPotOrFader) {

		case 2:
		case 11:
		case 20:
		case 29:
		case 37:
		case 39:
		case 41:
		case 49:
			theNumber = value >> 2;
			break; //>>2

		case 4:
		case 5:
		case 6:
		case 13:
		case 14:
		case 15:
		case 22:
		case 23:
		case 24:
		case 31:
		case 32:
		case 33:
			theNumber = value >> 3;
			break; //>>3

		case 7:
		case 35:
		case 8:
		case 16:
		case 17:
		case 25:
		case 26:
		case 34:
			theNumber = value >> 4;
			break; //>>4

		case 1:
		case 10:
		case 19:
		case 28:
			theNumber = value >> 4;
			if (!theNumber) {
				theNumber = 666;
			}

			break; // mult

		case 43:
			theNumber = value >> 5;
			break; //>>5

		case 42:
			theNumber = value >> 5;
			theNumber++;
			break; //>>5 +1

		case 47:
			theNumber = value >> 6;
			theNumber++;
			break; //>>6 +1
	}
	if (theNumber != -1)
		ledNumber(theNumber);
}

void updateLedNumber() {

	if (setupMode) {
		if (!setupChanged) {
			digit(0, 5);
			digit(1, 18);
		}
	}
}

void clearLfoLeds() { // hide the lfo leds
	ledSet(16, 0);
	ledSet(17, 0);
	ledSet(18, 0);
	ledSet(19, 0);
	ledSet(20, 0);
	ledSet(21, 0);
}

void showLfo() {
	if ((mpe || lfoAt) && selectedLfo == 2) {
		switch (map(map(polyPressure[lastMpeVoice], 0, 255, 0, fmData[41]), 0, 255, 0, 7)) {

			case 0:
				ledSet(16, 0);
				ledSet(17, 0);
				ledSet(18, 0);
				ledSet(19, 0);
				ledSet(20, 0);
				ledSet(21, 0);
				break;

			case 1:
				ledSet(16, 1);
				ledSet(17, 0);
				ledSet(18, 0);
				ledSet(19, 0);
				ledSet(20, 0);
				ledSet(21, 0);
				break;

			case 2:
				ledSet(16, 1);
				ledSet(17, 1);
				ledSet(18, 0);
				ledSet(19, 0);
				ledSet(20, 0);
				ledSet(21, 0);
				break;

			case 3:
				ledSet(16, 1);
				ledSet(17, 1);
				ledSet(18, 1);
				ledSet(19, 0);
				ledSet(20, 0);
				ledSet(21, 0);
				break;

			case 4:
				ledSet(16, 1);
				ledSet(17, 1);
				ledSet(18, 1);
				ledSet(19, 1);
				ledSet(20, 0);
				ledSet(21, 0);
				break;

			case 5:
				ledSet(16, 1);
				ledSet(17, 1);
				ledSet(18, 1);
				ledSet(19, 1);
				ledSet(20, 1);
				ledSet(21, 0);
				break;

			case 6:
			case 7:
				ledSet(16, 1);
				ledSet(17, 1);
				ledSet(18, 1);
				ledSet(19, 1);
				ledSet(20, 1);
				ledSet(21, 1);
				break;
		}
	} else {
		if (!bankCounter) {
			ledSet(16, 0);
			ledSet(17, 0);
			ledSet(18, 0);
			ledSet(19, 0);
			ledSet(16 + lfoShape[selectedLfo], 1);

			ledSet(20, retrig[selectedLfo]);
			ledSet(21, looping[selectedLfo]);
		}
	}
}

void showSSEG() {
	ledSet(16, 0);
	ledSet(19, 0);
	ledSet(21, 0);
	ledSet(17, bitRead(SSEG[lastOperator], 0));  // triangle
	ledSet(18, !bitRead(SSEG[lastOperator], 0)); // saw
	ledSet(20, 0);
	ledSet(21, bitRead(SSEG[lastOperator], 1)); // SSEG enabled or not?
	showSSEGCounter = 12000;                    // show the regular LFO leds when this expires
}

void showLink() {
	ledSet(13, linked[0][targetPot]);
	ledSet(14, linked[1][targetPot]);
	ledSet(15, linked[2][targetPot]);
}

void showVoiceMode(VoiceMode voiceMode) {

	if (mpe) {

		ledSet(9, 1);
		ledSet(10, 0);
		ledSet(11, 0);
		ledSet(12, 0);

	} else {
		// FIXME clean this up, too!
		if (voiceMode == kVoicingDualCh3) {
			ym.setChan3Mode(1); // FIXME clean this up!
		} else {
			ym.setChan3Mode(0);
		}
		//              kVoicingPoly12, kVoicingWide6, kVoicingDualCh3, kVoicingUnison, kVoicingWide4, kVoicingWide3
		uint8_t leds[] = {0x1, 0x2, 0x4, 0x8, 0xF, 0xE};

		ledSet(9, leds[voiceMode] & 1);
		ledSet(10, leds[voiceMode] & 2);
		ledSet(11, leds[voiceMode] & 4);
		ledSet(12, leds[voiceMode] & 8);

		resetVoices(); // What does this do here?! FIXME clean this up!
	}
}

void rightDot() {

	mydisplay.setLed(0, 7, 7, 1);

	dotTimer = 50;
}

void leftDot() {

	mydisplay.setLed(0, 7, 6, 1);
	dotTimer = 50;
}

void showSendReceive() {

	showPresetNumberTimeout = 0;

	ledSet(16, 0);
	ledSet(17, 0);
	ledSet(18, 0);
	ledSet(19, 0);
	ledSet(20, 0);
	ledSet(21, 0);

	if (sendReceive == 1) {
		digit(0, 16);
	} else {
		digit(0, 5);
	}
	if (ab) {
		digit(1, 25);
	} else {
		digit(1, 17);
	}
}

int lastValue;
void ledNumber(int value) {

	if (!displayFreeze && !sendReceive) {
		if (value == 666) { //.5
			digit(0, 21);
			digit(1, 5);
			mydisplay.setLed(0, 7, 6, 1);
			lastNumber = value;
		} else {

			dotTimer = 10;

			if (value < 0) {
				if (value > -10) {
					digit(0, 20); // minus
					digit(1, -value);
					lastNumber = value;
				}
			} else {
				if (value > 99)
					value = 99;

				if (lastNumber != value) {
					lastNumber = value;
					digit(0, value / 10);
					digit(1, value - ((value / 10) * 10));
				}
			}
		}
	}
	showPresetNumberTimeout = 12000;
	lastValue = value;
}

void ledNumberForced(int value) {

	if (value > 99)
		value = 99;
	digit(0, value / 10);
	digit(1, value - ((value / 10) * 10));
}

/*
  0
5   1
  6
4   2
  3   7

  */

void showPickup() {
	pickupAnimationNewFrame = false;
	if (pickupIsFader) {
		switch (pickupFrame) {
			case 0:
				digit(0, 29);
				digit(1, 29);
				break;
			case 1:
				digit(0, 20);
				digit(1, 20);
				break;
			case 2:
				digit(0, 28);
				digit(1, 28);
				break;
			case 3 ... 8:

				digit(0, 21);
				digit(1, 21);
				break;
		}
	} else {
		switch (pickupFrame) {
			case 3:
				digit(0, 30);
				digit(1, 21);
				break;
			case 2:
				digit(0, 1);
				digit(1, 21);
				break;
			case 1:
				digit(0, 21);
				digit(1, 30);
				break;
			case 0:
				digit(0, 21);
				digit(1, 1);
				break;
			case 4 ... 8:
				digit(0, 21);
				digit(1, 21);
				break;
		}
	}
}

void digit(byte channel, byte number) {
	switch (number) {

		case 30: // left bar
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);
			break;

		case 29: // down arrow
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);
			break;

		case 28: // up arrow
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 1);
			break;

		case 27: // o
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 26: // t
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 25: // b
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 24: // all lit (plus dots)
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			mydisplay.setLed(0, 7, 7, 1);
			mydisplay.setLed(0, 7, 6, 1);

			break;

		case 22: // random
			mydisplay.setLed(0, 0, 6 + channel, random(2));
			mydisplay.setLed(0, 5, 6 + channel, random(2));
			mydisplay.setLed(0, 1, 6 + channel, random(2));
			mydisplay.setLed(0, 6, 6 + channel, random(2));
			mydisplay.setLed(0, 4, 6 + channel, random(2));
			mydisplay.setLed(0, 2, 6 + channel, random(2));
			mydisplay.setLed(0, 3, 6 + channel, random(2));
			break;

		case 21: // blank
			lastNumber = -1;
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);
			break;

		case 20: //-
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);
			break;

		case 23: // h
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);
			break;

		case 19: // n
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);
			break;

		case 18: // E
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 1);
			break;
		case 17: // A
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;

		case 16: // r
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;

		case 15: // d
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 14: // P
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;

		case 13: // U
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 12: // F
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;
		case 11: // L
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 10: // C
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 9:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;

		case 8:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 7:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;
		case 6:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 5:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 4:
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;

		case 3:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 2:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 1);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 1:
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;

		case 0:
			mydisplay.setLed(0, 0, 6 + channel, 1);
			mydisplay.setLed(0, 5, 6 + channel, 1);
			mydisplay.setLed(0, 1, 6 + channel, 1);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 1);
			mydisplay.setLed(0, 2, 6 + channel, 1);
			mydisplay.setLed(0, 3, 6 + channel, 1);

			break;

		case 99: // BLANK
			mydisplay.setLed(0, 0, 6 + channel, 0);
			mydisplay.setLed(0, 5, 6 + channel, 0);
			mydisplay.setLed(0, 1, 6 + channel, 0);
			mydisplay.setLed(0, 6, 6 + channel, 0);
			mydisplay.setLed(0, 4, 6 + channel, 0);
			mydisplay.setLed(0, 2, 6 + channel, 0);
			mydisplay.setLed(0, 3, 6 + channel, 0);

			break;
	}
}

void showAlgo(byte number) {

	for (int i = 1; i < 9; i++)
		ledSet(i, 0);
	ledSet(number + 1, 1);
}

void ledSet(byte number, bool value) {

	switch (number) {
		case 1:
			mydisplay.setLed(0, 0, 1, value);
			break;
		case 2:
			mydisplay.setLed(0, 1, 1, value);
			break;
		case 3:
			mydisplay.setLed(0, 2, 1, value);
			break;
		case 4:
			mydisplay.setLed(0, 3, 1, value);
			break;

		case 5:
			mydisplay.setLed(0, 4, 1, value);
			break;
		case 6:
			mydisplay.setLed(0, 5, 1, value);
			break;
		case 7:
			mydisplay.setLed(0, 6, 1, value);
			break;
		case 8:
			mydisplay.setLed(0, 7, 1, value);
			break;

		case 9:
			mydisplay.setLed(0, 0, 2, value);
			break;
		case 10:
			mydisplay.setLed(0, 1, 2, value);
			break;
		case 11:
			mydisplay.setLed(0, 2, 2, value);
			break;
		case 12:
			mydisplay.setLed(0, 3, 2, value);
			break;

		case 13:
			mydisplay.setLed(0, 4, 2, value);
			break;
		case 14:
			mydisplay.setLed(0, 5, 2, value);
			break;
		case 15:
			mydisplay.setLed(0, 6, 2, value);
			break;

		case 16:
			mydisplay.setLed(0, 7, 2, value);
			break;
		case 17:
			mydisplay.setLed(0, 0, 3, value);
			break;

		case 18:
			mydisplay.setLed(0, 1, 3, value);
			break;
		case 19:
			mydisplay.setLed(0, 2, 3, value);
			break;

		case 20:
			mydisplay.setLed(0, 3, 3, value);
			break;
		case 21:
			mydisplay.setLed(0, 4, 3, value);
			break;

		case 22:
			mydisplay.setLed(0, 5, 3, value);
			break;
		case 23:
			mydisplay.setLed(0, 6, 3, value);
			break;
		case 24:
			mydisplay.setLed(0, 7, 3, value);
			break;
		case 25:
			mydisplay.setLed(0, 0, 4, value);
			break;
		case 26:
			mydisplay.setLed(0, 1, 4, value);
			break;

		case 27:
			mydisplay.setLed(0, 2, 4, value);
			break;
		case 28:
			mydisplay.setLed(0, 3, 4, value);
			break;
		case 29:
			mydisplay.setLed(0, 4, 4, value);
			break;

		case 30:
			mydisplay.setLed(0, 5, 4, value);
			break;
		case 31:
			mydisplay.setLed(0, 6, 4, value);
			break;
	}
}
