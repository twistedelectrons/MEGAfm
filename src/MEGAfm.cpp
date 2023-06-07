/*
cd /Users/a/Documents/bootloaderT cd /Users/a/Documents/bootloaderT&&cp -f /private/var/hex/MEGAfm.ino.hex
/Users/a/Documents/bootloaderT&&python tools/hex2sysex/hex2sysex.py --syx -o firmware.syx MEGAfm.ino.hex
*/

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <TimerOne.h>

#include "megafm.h"
#include "lfo.h"
#include "mux.h"
#include "FM.h"
#include "leds.h"
#include "isr.h"
#include "preset.h"

// check that we didn't do a rec+voicing before latching arp rec mode
bool newWide; // enable new wide modes
bool noRecAction;
bool recHeld;
bool chordNotes[128];
bool heldNotes[128];
byte chordRoot;
byte presetChordNumber;
// Whether one second has elapsed since device boot.
// Used in: loop.cpp, pots.cpp
bool secPast = false;
byte lastSentCC[2];
byte lastSentMega[2];
byte lastSentYm[2];

// when true we play chords (captured with rec+voicing)
bool chord;
// 0=chip1 down chip2 up 1=both chips go up and down (mixed)
bool fatSpreadMode;
// animate the pickup funciton with dots that move up or down
int pickupFrame;
bool pickupIsFader;
bool pickupFrameUp;
int pickupFrameUpTimer;
bool showPickupAnimation;
bool pickupAnimationNewFrame;
// Whether the knobs/sliders should use 'pickup' behavior
// Used in: buttons.cpp, megafm.cpp, pots.cpp
bool pickupMode = true;
// When pickup mode is active we set this high when the preset value has been reached (picked up) after a preset change.
// Used in: pickup.cpp, pots.cpp, preset.cpp
bool pickup[49];
// Similar to pickup? Ignore volume knob immediately after loading a preset
// Used in: buttons.cpp, megafm.cpp, pots.cpp, preset.cpp
bool ignoreVolume;
// Some kind of defer countdown for pitch bend
// Used in: loop.cpp, midi.cpp
int bendyCounter;
// True for each MIDI note included in arpeggiation
// Used in: arp.cpp, voice.cpp
bool arpNotes[128];
// Note priority: lowest (0), highest (1), or last (2).
// Used in: buttons.cpp, megafm.cpp, midi.cpp, voice.cpp
byte notePriority = 2;
// Track the last note for glide.
// Distance to go from last note to future.
// Used in: midi.cpp, voice.cpp
byte lastNotey[128];
/**
 * When the arp needs to be resynced; MIDI clock changes or arp mode change
 * Used in: buttons.cpp, midi.cpp
 */
bool resyncArp = false;
bool stereoCh3;
/**
 * We have 100 presets arranged in 6 banks. Change the bank by pressing
 * an LFO waveform during the preset loading processes (blinking display).
 * FYI presets 0-50 are in the internal EEPROM, presets 50-99 of bank 1
 * and all presets of banks 2-6 are in the external SPI EEPROM.
 */
byte bank;
int lfoLedCounter;
byte seq[16];
byte seqStep;
byte seqLength;
bool seqRec;
bool flasher;
int flashCounter, flashCounter2, bankCounter;
bool toolMode;
/**
 * When set to true, megaFM boots in test mode (hold reset sat startup
 * to enter test mode after a factory reset) : (plays some chords at
 * various volumes and fires a note on every channel in a loop). This
 * is used for me to check that the chips and volume control circuits
 * are ok.
 */
bool test;
/**
 * This mode is used for sending and receiving preset sysex dumps.
 * Hold preset up or down at startup to boot in send or receive modes.
 */
byte sendReceive;
byte rootNote, rootNote1;
bool thru;
int setupCounter;
bool justQuitSetup;
bool invertedSquare[3];
bool invertedSaw[3];
bool showLfoFlag;
int showSSEGCounter; // used to temporarily show the SSEG settings of the last tweaked operator
byte lastOperator;   // keep track of the last operator we tweaked to enable SSGEG
byte muxChannel;
bool voiceSlots[12];
byte noteOfVoice[12];
byte SSEG[4]; // bit 0=shape (0=triangle 1=square) bit 1=enabled
// Velocity affects lfo1
bool lfoVel;
// Mod affects lfo2
bool lfoMod;
// Aftertouch affects lfo3
bool lfoAt;
float mpeBend[12];
byte bendUp = 12;
byte bendDown = 12;
int bendRoot = -1;
float destiFreq[12];
float freq[12];
// freqTotal = freq + vib * bend
float freqTotal[12];
float freqLast[12];
float glideIncrement[12];
int volumeCounter;
bool changeVol;
bool dotOn;
bool arpClockEnable;
bool lfoClockEnable[3];
bool vibratoClockEnable;
byte glide = 0;
byte lastVol, vol;
bool arpJustWentOff;
bool setupMode;
bool fineChanged;
byte arpMidiSpeedPending, arpMidiSpeed;
byte fine;
byte presetCounts;
bool presetTargetMode;
byte presetTargetModeChanged;
int targetPresetModeTimer;
bool targetPresetFlasher;
int scrollDelay, scrollCounter;
byte noiseTableLength[3];
// Buffer for sysex preset dumps.
byte mem[3950];
int notey[12];
byte arpMode, arpModeLast;
bool looping[3];
int arpIndex;
bool resetHeld;
bool shuffled;
bool voiceHeld;
int lastNumber = 255;
byte lfoClockSpeed[3];
// To keep the lfo beat in sync with the MIDI master clock,
// this is used to store a new lfo rate until the incoming MIDI clock counter resets,
// whereas changing immediately would make it go out of sync.
byte lfoClockSpeedPending[3];
byte lfoClockSpeedPendingLast[3];
byte absoluteClockCounter;
YM2612 ym;
int heldKeys;
byte lastNote, dotTimer;
int arpCounter, arpStep, preset;
float lfoStepF[3];
bool arpModeHeld;
byte inputChannel = 1;
bool changedChannel;
bool newFat;
/**
 * Whether to turn off the voice slot when the pedal is lifted.
 */
bool pedalOff[12];
bool pedal;
/**
 * Used to track if any notes are stored in the arpeggiator stack/array (true=empty).
 * Notes are added or removed from the stack when MIDI notes on/off are parsed.
 */
bool emptyStack;
bool fatMode; // false == 1 semitone; true == 1 octave
int arpButtCounter;

/**
 * Used to adjust the global tuning of MEGAfm (when voice mode button is held
 * and volume knob is turned) by multiplying it by a range of 1 to 2 (1 octave)
 */
float finey = 1;
int ledNumberTimeOut;
LedControl mydisplay = LedControl(13, 29, 12, 1);
int presetLast;
int secCounter;
int vibIndex, vibIndexLast;
int arpDivider;
VoiceMode voiceMode = kVoicingPoly12;
bool sync;
/**
 * Banks of 100 presets are sent and received in 2 halves (A=presets 0-49. B=presets 50-99).
 * I'm unsure why I chose to do this, from memory the serial USART had trouble buffering
 * more than 50 presets at a time.
 */
bool ab;
int potLast[64];
int lfoDepth[3];
bool buttLast[19];
int algoLast;
/**
 * These are all the parameters before they get modulated by LFO
 * (they are loaded by the preset and changed by MIDI CC or knob/fader movements).
 * Basically it's all the stuff we can modulate to change things.
 */
byte fmBase[51], fmBaseLast[51], fmBaseLastNumber[51];
/**
 * These are the same values as fmBase but after LFO modulation,
 * before being shifted and sent to the FM chips.
 */
int fmData[51], fmDataLast[51];
bool linked[3][51];
byte octOffset;
byte lfoRandom[3][32];
int displayFreeze;
int showPresetNumberTimeout; // we show the preset number when this expires (after moving a knob or fader);
bool timeToShowPresetNumber; // set true when timeout expires
byte randomIndex[3];
bool pressedUp, pressedDown;
bool saved;
int presetFlasher;
byte updatePitchCounter;
int shuffleCounter2;
bool shuffleAlternator;
int shuffleCounter;
byte linkCounter;
byte selectedLfo, selectedLfoLast;
bool cleared;
/** Which LFO chain button is being pressed. 1, 2, or 3; 0 = none. */
byte chainPressed;
byte targetPot, targetPotLast;
byte masterChannelOut = 1;
float ch3x[4];
byte lfoShape[3];
byte lfo[3], lfoLast[3];
int lfoStep[3];
int lfoStepLast[3];
/**
 * incoming MIDI aftertouch (the at can be assigned to one of the LFO)
 */
int at;
/**
 * These are aftertouch values. I interpolate between last (current) and
 * dest (next) values with some gliding so the aftertouch response isn't too choppy.
 * FYI aftertouch can override one of the LFOs.
 */
int atDest, atLast, atGlideCounter;
bool lfoNewRand[3];
int lfoCounter[3], lfoSpeed[3];
bool retrig[3];
int fatLast;
float fat;
float bendy = 0;
bool setupChanged;
bool lastLfoSetting[3];
int vibPitch;
byte presetTp;
int vibCounter;
int shuffleTimer;
byte seed;
byte potClock;
bool mpe;

void enterSetup() {
	digit(0, 5);
	digit(1, 18);
	setupMode = true;
	ledSet(13, thru);
	ledSet(14, pickupMode);
	ledSet(19, fatSpreadMode);
}

void setup() {
	lastSentCC[0] = 255;

	fillAllLfoTables();

	Wire.begin();

	potClock = 19;

	// dPot
	digitalWrite(13, LOW);
	digitalWrite(29, LOW);
	digitalWrite(21, LOW);

	// dPot
	pinMode(potClock, OUTPUT); // clock
	pinMode(18, OUTPUT);       // data
	pinMode(21, OUTPUT);       // latch

	arpModeLast = 6;

	for (int i = 0; i < 16; i++) {
		readMux();
	}

	ym.setup(30, 10, 11, 11, 20, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23);

	setupFM();

	pinMode(A1, INPUT);
	digitalWrite(A1, HIGH);

	pinMode(22, INPUT);
	digitalWrite(22, HIGH); // pc6 retrig

	mydisplay.shutdown(0, false); // turns on display

	byte brightness = EEPROM.read(3965);
	if (brightness > 15)
		brightness = 10;                   // default;
	mydisplay.setIntensity(0, brightness); // 15 = brightest

	// show version
	digit(0, kVersion0);
	digit(1, kVersion1);
	// show dot
	mydisplay.setLed(0, 7, 6, 1);
	delay(500);
	mydisplay.setLed(0, 7, 6, 0);

	Timer1.initialize(150);      //
	Timer1.attachInterrupt(isr); // attach the service routine here

	inputChannel = EEPROM.read(3951);
	if ((inputChannel > 16) || (inputChannel < 1)) {
		inputChannel = 1;
		EEPROM.write(3951, inputChannel);
	}

	// check if enter setup mode
	mux(15);
	if (!digitalRead(A1)) {
		enterSetup();
	}

	// mux(5);//preset up

	// MIDI port at 31250 baud
	Serial.begin(31250);
	// midiSetup();

	mux(13);
	if ((!digitalRead(A1)) ||
	    ((EEPROM.read(0) == 255) && (EEPROM.read(1) == 255) && (EEPROM.read(2) == 255) && (EEPROM.read(3) == 255))) {
		// test mode
		digit(0, 16);
		digit(1, 18);

		for (int i = 0; i < 4000; i++) {
			EEPROM.update(i, kFactoryPresets[i]);
		}
		clearSSEG(0);
		EEPROM.update(3970, 0);
		newWide = false;
		EEPROM.update(3969, 82);

		loadPreset();
		eWrite(69, 69);
		if (eRead(69) != 69) {

			while (1) {

				digit(0, 18);
				digit(1, 18);
				delay(200);
				digit(0, 21);
				digit(1, 21);
				delay(200);
			}
		}
		EEPROM.write(3964, 0); // bank 0
		test = true;
		bank = 0;
		preset = 0;
	}

	// first boot into 3.X? clear the SSEG and Offsets so presets aren't crazy (yet)!
	if (EEPROM.read(3969) != 82) {
		clearSSEG(1);
		EEPROM.write(3969, 82);
	}

	for (int i = 0; i < 50; i++) {
		ledSet(i, test);
	}

	for (int i = 0; i < 16; i++) {
		readMux();
	}

	bendUp = EEPROM.read(3959);
	if ((bendUp > 48) || (!bendUp)) {
		bendUp = 48;
	}
	bendDown = EEPROM.read(3958);
	if ((bendDown > 48) || (!bendDown)) {
		bendDown = 48;
	}

	if (EEPROM.read(3960) == 1) {
		mpe = 1;
	} else {
		mpe = 0;
	}

	mux(14);
	if (!digitalRead(A1)) {
		sendReceive = 2; // preset down = send midi dump
		bank = 0;
		showSendReceive();
	}
	mux(5);
	if (!digitalRead(A1)) {
		sendReceive = 1; // preset up = get midi dump
		bank = 0;
		showSendReceive();
	}

	// LOAD SETTINGS

	// 3950 = bit 0 thru
	// 3950 = bit 1 ignore preset volume
	// 3950 = bit 2-7 noisetableLength

	// 3951 = midi channel
	// 3952 = last preset

	// 3953 bit0  =  midi clock lfo1
	// 3953 bit1    midi clock lfo2
	// 3953 bit2 =  midi clock lfo3
	// 3953 bit3 =  midi clock vibra
	// 3953 bit4 =  midi clock  arp
	// 3953 bit5 =  fatMode
	// 3953 bit6 = new fat tuning

	// 3954 = pickup mode (0=on)
	// 3958 = bendDown
	// 3959 = bendUp
	// 3960 = mpe mode
	// 3961 = vel >lfo1 depth
	// 3962 = mod >lfo2 depth
	// 3963 = at >lfo3 depth
	// 3964 lastbank
	// 3965 brightness.
	// 3966 = bit 0 stereoCh3
	// 3967 = note priority 0=low 1=high 2=last
	// 3968 = bit 0 fatSpreadMode

	// 3969 = magic value 82 says we are already on FW 3.0
	// 3970 = enable new wide modes
	// otherwise set all SSEG to off
	byte input = EEPROM.read(3968);
	fatSpreadMode = bitRead(input, 0);

	notePriority = EEPROM.read(3967);
	if (notePriority > 2)
		notePriority = 0;

	input = EEPROM.read(3966);
	stereoCh3 = bitRead(input, 0);

	if (EEPROM.read(3970)) {
		newWide = true;
	} else {
		newWide = false;
	}

	input = EEPROM.read(3950);
	noiseTableLength[0] = 0;
	bitWrite(noiseTableLength[0], 0, bitRead(input, 2));
	bitWrite(noiseTableLength[0], 1, bitRead(input, 3));
	noiseTableLength[0] += 2;

	noiseTableLength[1] = 0;
	bitWrite(noiseTableLength[1], 0, bitRead(input, 4));
	bitWrite(noiseTableLength[1], 1, bitRead(input, 5));
	noiseTableLength[1] += 2;

	noiseTableLength[2] = 0;
	bitWrite(noiseTableLength[2], 0, bitRead(input, 6));
	bitWrite(noiseTableLength[2], 1, bitRead(input, 7));
	noiseTableLength[2] += 2;

	thru = !bitRead(input, 0);

	if (EEPROM.read(3961)) {
		lfoVel = 1;
	} else {
		lfoVel = 0;
	}
	if (EEPROM.read(3962)) {
		lfoMod = 1;
	} else {
		lfoMod = 0;
	}
	if (EEPROM.read(3963)) {
		lfoAt = 1;
	} else {
		lfoAt = 0;
	}

	if (EEPROM.read(3954)) {
		pickupMode = true;
	} else {
		pickupMode = false;
	}

	lfoClockEnable[0] = bitRead(EEPROM.read(3953), 0);
	lfoClockEnable[1] = bitRead(EEPROM.read(3953), 1);
	lfoClockEnable[2] = bitRead(EEPROM.read(3953), 2);
	vibratoClockEnable = bitRead(EEPROM.read(3953), 3);
	arpClockEnable = bitRead(EEPROM.read(3953), 4);
	fatMode = bitRead(EEPROM.read(3953), 5);
	newFat = bitRead(EEPROM.read(3953), 6);

	ignoreVolume = bitRead(EEPROM.read(3950), 1);

	bank = EEPROM.read(3964);
	if (bank > 5)
		bank = 0;
	preset = EEPROM.read(3952);
	if (preset > 99) {
		preset = 0;
	}

	if (!sendReceive) {
		loadPreset();
		loadPreset();
	}
}
