//
// Created by Matt Montag on 1/22/22.
//
#ifndef MEGAFM_CMAKE_MEGAFM_R2_4_H
#define MEGAFM_CMAKE_MEGAFM_R2_4_H

#include <LedControl.h>
#include "YM2612.h"
#include "constants.h"

/* Map  pins to YM2612 pins */
/* RD can be tied to 3.3 V for now */
#define YM_A0 20
#define YM_A1 23
#define YM_CS 10
#define YM_CS2 14
#define YM_WR 11
#define YM_IC 30
extern bool loopChanged;
extern bool loopHeld;
extern byte v4Preset; // detect with a magic bit in the preset if it's already been saved within 4.x  or disable arp in
                      // non unison mode
extern byte presetChordNumber;
extern bool newWide; // enable new wide modes
extern bool chord;
extern bool noRecAction;
extern bool recHeld;
extern bool showLfoFlag;
extern int showSSEGCounter;
extern int showPresetNumberTimeout; // we show the preset number when this expires (after moving a knob or fader);
extern bool timeToShowPresetNumber; // set true when timeout expires
extern byte SSEG[4];
extern byte lastOperator;
extern bool secPast;
extern byte lastSentCC[2];
extern byte lastSentMega[2];
extern byte lastSentYm[2];
extern int displayFreeze; // when >0 show only preset number after preset change
extern bool fatSpreadMode;
constexpr bool FAT_SPREAD_MODE_1DOWN2UP = false; // chip 1 down, chip 2 up
constexpr bool FAT_SPREAD_MODE_MIXED = true;     // both chips go up and downa
extern int pickupFrame;
extern int pickupFrameUpTimer;
extern bool pickupFrameUp;
extern bool showPickupAnimation;
extern bool pickupIsFader;
extern bool pickupAnimationNewFrame;
extern bool pickupMode;
extern bool pickup[49];
extern bool ignoreVolume;
extern int bendyCounter;
extern bool arpNotes[128];
extern byte notePriority;
extern byte lastNotey[128];
extern bool resyncArp;
extern bool stereoCh3;
extern byte bank;
extern int lfoLedCounter;
extern byte seq[16];
extern byte seqStep;
extern byte seqLength;
extern bool seqRec;
extern bool flasher;
extern int flashCounter, flashCounter2, bankCounter;
extern bool test;
extern byte sendReceive;
extern byte rootNote, rootNote1;
extern bool thru;
extern int setupCounter;
extern bool justQuitSetup;
extern bool invertedSquare[3];
extern bool invertedSaw[3];
extern bool toolMode;
extern bool voiceSlots[12];
extern byte noteOfVoice[12];
extern byte muxChannel;

extern bool lfoVel; // velocity affects lfo1
extern bool lfoMod; // mod affect lfo2
extern bool lfoAt;  // at affect lfo3

extern float mpeBend[12];

extern byte bendUp;
extern byte bendDown;
extern int bendRoot;

extern float destiFreq[12];
extern float freq[12];
extern float freqTotal[12]; // freq + vib *bend
extern float freqLast[12];
extern float glideIncrement[12];
extern int volumeCounter;
extern bool changeVol;
extern bool dotOn;
extern bool arpClockEnable;

extern bool lfoClockEnable[3];
extern bool vibratoClockEnable;
extern byte glide;
extern byte lastVol, vol;
extern bool arpJustWentOff;
extern bool setupMode;
extern bool fineChanged;

extern byte arpMidiSpeedPending, arpMidiSpeed;
extern int fine;
extern byte presetCounts;
extern bool presetTargetMode;
extern byte presetTargetModeChanged;
extern int targetPresetModeTimer;
extern bool targetPresetFlasher;
extern int scrollDelay, scrollCounter;
extern byte noiseTableLength[3];
extern byte finePot;

extern int finerFine;      // more precise tuning
extern bool movedFineKnob; // track if we adjusted tune to override glide knob for finer tuning

extern byte mem[3950];

extern int notey[12];
extern byte arpMode, arpModeLast;
extern byte version;
extern bool looping[3];
extern int arpIndex;
extern bool resetHeld;
extern bool shuffled;
extern bool voiceHeld;
extern int lastNumber;
extern byte lfoClockSpeed[3];
extern byte lfoClockSpeedPending[3];
extern byte lfoClockSpeedPendingLast[3];
extern int masterClockCounter;
extern bool newFat;
extern YM2612 ym;
extern byte latestChannel; // keep track of latest voice for global velocity/aftertouch modulation
extern int heldKeys;
extern byte lastNote, dotTimer;
extern int arpCounter, arpStep, preset;
extern float lfoStepF[3];
extern bool arpModeHeld;
extern byte inputChannel;
extern bool changedChannel;
extern bool pedalOff[12];
extern bool emptyStack;
extern bool chordNotes[128];
extern bool heldNotes[128];
extern byte chordRoot;
extern bool fatMode;
constexpr bool FAT_MODE_SEMITONE = false;
constexpr bool FAT_MODE_OCTAVE = true;

extern int arpButtCounter;
extern float finey;
extern int ledNumberTimeOut;

extern LedControl mydisplay;

extern int presetLast;
extern int secCounter;

extern int vibIndex, vibIndexLast;
extern int arpDivider;

extern VoiceMode voiceMode;
extern bool sync;
extern bool ab;
extern int potLast[64];
extern int lastMpeVoice;
extern int pressureCounter;
extern int lfoDepth[3];
extern bool buttLast[19];
extern byte keyPressure[128];
extern int algoLast;
extern int polyPressure[12];
extern int polyVel[12];
extern byte fmBase[51], fmBaseLast[51], fmBaseLastNumber[51];
extern int fmData[51], fmDataLast[51];
extern bool linked[3][51];
extern byte lfoRandom[3][32];
extern byte valPlusPressureLast[12][36];
extern byte valPlusVelLast[12][36];
extern byte randomIndex[3];
extern byte octOffset; // offset the preset by 0-3 octaves
extern bool pressedUp, pressedDown;
extern bool saved;
extern int presetFlasher;
extern byte updatePitchCounter;
extern int shuffleCounter2;
extern bool shuffleAlternator;
extern int shuffleCounter;
extern byte linkCounter;
extern byte selectedLfo, selectedLfoLast;
extern bool cleared;
extern byte chainPressed;
extern byte targetPot, targetPotLast;
extern byte masterChannelOut;

extern float ch3x[4];
extern byte lfoShape[3];
extern byte lfo[3], lfoLast[3];
extern int lfoStep[3];
extern int lfoStepLast[3];
extern bool lfoNewRand[3];
extern int lfoCounter[3], lfoSpeed[3];
extern bool retrig[3];
extern byte robin; // used for polyphonic ar voice ordering (round robin)
extern int fatLast;
extern float fat;
extern float bendy;
extern bool setupChanged;
extern bool lastLfoSetting[3];
extern int vibPitch;

extern byte presetTp;
extern int vibCounter;
extern int shuffleTimer;
extern byte seed;

extern byte potClock;
extern bool mpe;

void enterSetup();

#endif // MEGAFM_CMAKE_MEGAFM_R2_4_H
