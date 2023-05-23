#include <TimerOne.h>
#include "megafm.h"
#include "isr.h"
#include "arp.h"
#include "lfo.h"
#include "leds.h"

void stopTimer() {

	Timer1.detachInterrupt(); //
	Timer1.stop();            // attach the service routine here
}

void startTimer() {

	Timer1.initialize(150);      //
	Timer1.attachInterrupt(isr); // attach the service routine here
}

void isr() {

	// animate pickup
	if (pickupMode) {

		pickupFrameUpTimer++;
		if (pickupFrameUpTimer > 400) {
			pickupFrameUpTimer = 0;
			if (!pickupFrameUp) {
				pickupFrame++;
				if (pickupFrame > 8)
					pickupFrame = 0;
			} else {
				pickupFrame--;
				if (pickupFrame < 0)
					pickupFrame = 8;
			}
			pickupAnimationNewFrame = true;
		}
	}

	if ((lfoAt) && (!fmBase[40]) && (atDest != at)) {
		atGlideCounter++;
		if (atGlideCounter > 5) {
			atGlideCounter = 0;
			if (at > atDest) {
				at--;
			} else if (at < atDest) {
				at++;
			}
		}
	}

	if ((voiceHeld) && (setupCounter)) {
		setupCounter--;
		if (!setupCounter) {
			enterSetup();
		}
	}

	if (showPresetNumberTimeout) {
		showPresetNumberTimeout--;
		if (!showPresetNumberTimeout)
			timeToShowPresetNumber = true;
	}
	if (updatePitchCounter < 20)
		updatePitchCounter++;

	if (displayFreeze)
		displayFreeze--;

	if (showSSEGCounter) {
		showSSEGCounter--;
		if (!showSSEGCounter) {
			showLfoFlag = true;
		}
	}
	if (ledNumberTimeOut)
		ledNumberTimeOut--;

	if (bankCounter || sendReceive) {
		flashCounter2++;
	}

	if (arpModeHeld) {
		if (arpButtCounter < 4000) {
			arpButtCounter++;
		}
	}

	if ((pressedUp || pressedDown) && scrollDelay) {
		if (presetTargetMode)
			presetCounts = 40;

		scrollDelay--;
		if (!scrollDelay) {
			scrollDelay = 2000 - (scrollCounter * 200);
			if (scrollCounter < 8)
				scrollCounter++;
			if (pressedUp) {
				preset++;
				if (preset > 99) {
					preset = 0;
				}
			} else {
				preset--;
				if (preset < 0) {
					preset = 99;
				}
			}
		}
	}

	if (shuffleCounter) {
		shuffleCounter2++;
	}

	if (presetTargetMode) {
		targetPresetModeTimer++;
	} else {

		if (presetFlasher) {
			presetFlasher--;

			if (!setupMode) {
				if (presetFlasher == 1500) {
					lastNumber = 200;
					ledNumber(preset);
				}
				if (presetFlasher == 750) {
					digit(0, 99);
					digit(1, 99);
				}
				if (!presetFlasher) {
					lastNumber = 200;
					ledNumber(preset);
				}
			}
		}
	}

	if ((resetHeld) && (shuffleTimer)) {
		shuffleTimer--;
	}

	if (seqRec)
		flashCounter++;

	if ((arpMode) && (!seqRec) && (arpMode != 7)) {

		arpDivider++;
		if (arpDivider > 30) {
			arpDivider = 0;
			arpTick();
		}
	}

	// vibrato
	if ((!vibratoClockEnable) || (!sync)) {

		// free
		if (fmData[48]) {
			vibCounter += fmData[48];

			if (vibCounter >= 511) {
				vibCounter -= 511;
				vibIndex++;
				if (vibIndex > 255)
					vibIndex = 0;
			}
		}
	}

	lfoAdvance();

	if (dotTimer) {
		dotOn = true;
		dotTimer--;
	}

	secCounter++;
	if (secCounter > 3000) {
		secCounter = 0;
		everySec();
	}
}

void everySec() {

	if (chainPressed && linkCounter) {
		linkCounter--;
		if (!linkCounter) {
			clearLinks();
		}
	}
}
