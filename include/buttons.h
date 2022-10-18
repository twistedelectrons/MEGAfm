#ifndef MEGAFM_CMAKE_BUTTONS_H
#define MEGAFM_CMAKE_BUTTONS_H

enum Button : uint8_t {
	kButtonArpMode = 7,
	kButtonVoiceMode = 15,
	kButtonChainLfo1 = 4,
	kButtonChainLfo2 = 9,
	kButtonRetrig = 18,
	kButtonChainLfo3 = 8,
	kButtonNoise = 10,
	kButtonPresetUp = 5,
	kButtonPresetDown = 14,
	kButtonPresetReset = 13,
	kButtonArpRec = 3,
	kButtonSquare = 0,
	kButtonTriangle = 1,
	kButtonSaw = 6,
	kButtonLoop = 11,
};

/**
 * Invoked when a button changes state.
 * @param number Button ID.
 * @param value New button state (false = pressed, true = released).
 */
void buttChanged(Button number, bool value);

#endif // MEGAFM_CMAKE_BUTTONS_H
