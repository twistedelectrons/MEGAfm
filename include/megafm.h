//
// Created by Matt Montag on 1/22/22.
//
#ifndef MEGAFM_CMAKE_MEGAFM_R2_4_H
#define MEGAFM_CMAKE_MEGAFM_R2_4_H

#include <LedControl.h>
#include "YM2612.h"
#include "constants.h"

/* Map  pins to YM2612 pins */
#define YM_A0   20
#define YM_A1   23
#define YM_CS   10
#define YM_CS2  14
#define YM_WR   11
#define YM_IC   30


extern bool secPast;
extern bool pickupMode;
extern bool pickup[49];
extern bool ignoreVolume;
extern int bendyCounter;
extern bool arpNotes[128];
extern byte notePriority;
extern byte lastNotey[40];
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


extern bool voiceSlots[12];
extern byte noteOfVoice[12];
extern byte muxChannel;

extern bool lfoVel;//velocity affects lfo1
extern bool lfoMod;// mod affect lfo2
extern bool lfoAt;//at affect lfo3

extern float mpeBend[12];

extern byte bendUp;
extern byte bendDown;
extern int bendRoot;

extern float destiFreq[12];
extern float freq[12];
extern float freqTotal[12];//freq + vib *bend
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
extern byte fine;
extern byte presetCounts;
extern bool presetTargetMode;
extern byte presetTargetModeChanged;
extern int targetPresetModeTimer;
extern bool targetPresetFlasher;
extern int scrollDelay, scrollCounter;
extern byte noiseTableLength[3];

extern byte mem[3950];//buffer for sysex preset dumps

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
extern byte absoluteClockCounter;

extern YM2612 ym;
extern int heldKeys;
extern byte lastNote, dotTimer;
extern int arpCounter, arpStep, preset;
extern float lfoStepF[3];
extern bool arpModeHeld;
extern byte inputChannel;
extern bool changedChannel;
extern bool pedalOff[12];
extern bool pedal;
extern bool emptyStack;
extern bool fatMode;
extern int arpButtCounter;
extern float finey;
extern int ledNumberTimeOut;

extern LedControl mydisplay;

extern int presetLast;
extern int secCounter;

extern int vibIndex, vibIndexLast;
extern int arpDivider;

extern byte voiceMode;
extern bool sync;
extern bool ab;
extern int potLast[64];

extern int lfoDepth[3];
extern bool buttLast[19];
extern int algoLast;

extern byte fmBase[51], fmBaseLast[51], fmBaseLastNumber[51];
extern int fmData[51], fmDataLast[51];
extern bool linked[3][51];
extern bool dontShow[50];
extern byte lfoRandom[3][32];
extern byte randomIndex[3];

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
extern int at, atDest, atLast, atGlideCounter;
extern bool lfoNewRand[3];
extern int lfoCounter[3], lfoSpeed[3];
extern bool retrig[3];
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

#endif //MEGAFM_CMAKE_MEGAFM_R2_4_H
