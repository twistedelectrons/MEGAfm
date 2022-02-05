#include <Arduino.h>
#include "megafm.h"
#include "loop.h"
#include "lfo.h"
#include "leds.h"
#include "midi.h"
#include "volume.h"
#include "pitchEngine.h"
#include "mux.h"
#include "voice.h"
#include "preset.h"
#include "arp.h"
#include "midiEngine.h"
#include "FM.h"

byte testCounts;

void loop() {

  if (!secPast) { if (millis() > 1000) { secPast = 1; }}

  if (at != atLast) {
    atLast = at;
    if ((lfoAt) && (!fmBase[40])) {
      lfo[2] = at;
      applyLfo();
    }

  }

  if (lfoLedCounter > 0) {
    lfoLedCounter--;
    if (lfoLedCounter < 1) {
      ledSet(16 + lfoShape[selectedLfo], 1);
    }
  }

  if (test) {
    glide = 0;
    for (int i = 0; i < 50; i++) {
      ledSet(i, test);
    }
    digit(0, 24);
    digit(1, 24);

    mpe = 1;
    digitalPotWrite(0, 110);
    digitalPotWrite(B00010000, 110);
    handleNoteOn(1, 36, 1);
    handleNoteOn(7, 36 + 12, 1);
    updatePitch();
    handleNoteOn(2, 48, 1);
    handleNoteOn(8, 48 + 12, 1);
    updatePitch();
    handleNoteOn(3, 60, 1);
    handleNoteOn(9, 60 + 12, 1);
    updatePitch();
    delay(500);
    handleNoteOff(1, 36, 1);
    handleNoteOff(7, 36 + 12, 1);
    updatePitch();
    handleNoteOff(2, 48 + 4, 1);
    handleNoteOff(8, 48 - 8 + 12, 1);
    updatePitch();
    handleNoteOff(3, 60, 1);
    handleNoteOff(9, 60 + 12, 1);
    updatePitch();
    delay(200);
    digitalPotWrite(0, 10);
    digitalPotWrite(B00010000, 10);
    handleNoteOn(1, 36, 1);
    handleNoteOn(7, 36 + 12, 1);
    handleNoteOn(2, 48 + 3, 1);
    handleNoteOn(8, 48 - 8 + 12, 1);
    handleNoteOn(3, 60, 1);
    handleNoteOn(9, 60 + 12, 1);
    delay(500);
    handleNoteOff(1, 36, 1);
    handleNoteOff(7, 36 + 12, 1);
    handleNoteOff(2, 48, 1);
    handleNoteOff(8, 48 + 12, 1);
    handleNoteOff(3, 60, 1);
    handleNoteOff(9, 60 + 12, 1);
    delay(200);
    digitalPotWrite(0, 60);
    digitalPotWrite(B00010000, 60);
    handleNoteOn(1, 36, 1);
    handleNoteOn(7, 36 + 12, 1);
    handleNoteOn(2, 48 + 7, 1);
    handleNoteOn(8, 48 + 12 + 4, 1);
    handleNoteOn(3, 60, 1);
    handleNoteOn(9, 60 + 12, 1);
    delay(600);
    handleNoteOff(1, 36, 1);
    handleNoteOff(7, 36 + 12, 1);
    handleNoteOff(2, 48 + 7, 1);
    handleNoteOff(8, 48 + 12 + 4, 1);
    handleNoteOff(3, 60, 1);
    handleNoteOff(9, 60 + 12, 1);

    handleNoteOn(1, 36, 1);
    delay(60);
    handleNoteOff(1, 36, 1);
    delay(06);
    updatePitch();
    handleNoteOn(2, 38, 1);
    delay(60);
    handleNoteOff(2, 38, 1);
    delay(60);
    updatePitch();
    handleNoteOn(3, 40, 1);
    delay(60);
    handleNoteOff(3, 40, 1);
    delay(60);
    updatePitch();
    handleNoteOn(4, 42, 1);
    delay(60);
    handleNoteOff(4, 42, 1);
    delay(60);
    updatePitch();
    handleNoteOn(5, 44, 1);
    delay(60);
    handleNoteOff(5, 44, 1);
    delay(60);
    updatePitch();
    handleNoteOn(6, 46, 1);
    delay(60);
    handleNoteOff(6, 46, 1);
    delay(60);
    updatePitch();
    handleNoteOn(7, 44, 1);
    delay(60);
    handleNoteOff(7, 44, 1);
    delay(60);
    updatePitch();
    handleNoteOn(8, 42, 1);
    delay(60);
    handleNoteOff(8, 42, 1);
    delay(60);
    updatePitch();
    handleNoteOn(9, 40, 1);
    delay(60);
    handleNoteOff(9, 40, 1);
    delay(60);
    updatePitch();
    handleNoteOn(10, 38, 1);
    delay(60);
    handleNoteOff(10, 38, 1);
    delay(60);
    updatePitch();
    handleNoteOn(11, 36, 1);
    delay(60);
    handleNoteOff(11, 36, 1);
    delay(60);
    updatePitch();
    handleNoteOn(12, 34, 1);
    delay(60);
    handleNoteOff(12, 34, 1);
    delay(60);
    updatePitch();
    testCounts++;
    mux(13);
    if (!digitalRead(A1)) {
      if (testCounts > 4)test = false;
      mpe = 0;
    }

  }
  if (sendReceive) {

    readMux();

    // SEND RECEIVE
    if (flashCounter2 > 800) {
      flasher = !flasher;
      clearLfoLeds();
      ledSet(16 + bank, flasher);
      flashCounter2 = 0;
    }

  } else {

    if (bendyCounter) {

      bendyCounter--;
      if (!bendyCounter) {
        for (int i = 0; i < 12; i++) { setNote(i, notey[i]); }
      }
    }
    if (volumeCounter) {
      volumeCounter--;
      if ((!volumeCounter) && (changeVol)) {
        changeVol = false;
        digitalPotWrite(0, vol - 1);
        digitalPotWrite(B00010000, vol - 1);
      }
    }

    if (vibIndex != vibIndexLast) {
      vibIndexLast = vibIndex;
      vibPitch = kSineTable[vibIndex] - 127;
      float vibPitchDepth = fmData[49] >> 3;
      vibPitchDepth /= 48;
      vibPitch = vibPitch * vibPitchDepth;

    }

    updateLedNumber();

    if (arpButtCounter > 3999) {
      arpButtCounter = 0;
      arpMode = 0;
      resetVoices();

      ledSet(23, 0);
      digit(0, 21);
      digit(1, 21);
      arpJustWentOff = true;
    }

    if (shuffleCounter) {
      if (shuffleCounter2 > shuffleCounter * 25) {

        shuffleCounter2 = 0;
        shuffleCounter--;
        if (shuffleCounter) {

          shuffleAlternator = !shuffleAlternator;

          digit(0, 21 + shuffleAlternator);
          digit(1, 22 - shuffleAlternator);

        } else {
          lastNumber = -1;
          ledNumber(preset);
        }
      }
    }

    if (updatePitchCounter >= 20) {
      updatePitchCounter = 0;
      updatePitch();
    }

    if (presetTargetMode) {
      if (presetCounts > 16) {
        if (targetPresetModeTimer > 900) {
          targetPresetModeTimer = 0;
          presetCounts -= 2;
          targetPresetFlasher = !targetPresetFlasher;

          if (!setupMode) {
            if (targetPresetFlasher) {
              digit(0, 21);
              digit(1, 21);
            } else { ledNumber(preset); }
          }
        }
      } else {

        if (targetPresetModeTimer > 350) {
          targetPresetModeTimer = 0;
          presetCounts -= 1;
          if (presetCounts < 1) {
            presetTargetMode = false;
            savePreset();
          }
          targetPresetFlasher = !targetPresetFlasher;

          if (!setupMode) {
            if (targetPresetFlasher) {
              digit(0, 21);
              digit(1, 21);
            } else { ledNumber(preset); }
          }
        }
      }
    }

    if (arpCounter > 1022) {
      arpCounter = 0;
      arpFire();
    }

    seed++;
    if (!seed) { seed = random(3); }

    if ((bankCounter) && (flashCounter2 > 400)) {
      flasher = !flasher;
      clearLfoLeds();
      ledSet(16 + bank, flasher);
      flashCounter2 = 0;
      if ((!pressedUp) && (!pressedDown)) {
        bankCounter--;
        if (!bankCounter) { showLfo(); }
      }
    }

    if ((seqRec) && (flashCounter > 400)) {
      flasher = !flasher;
      ledSet(22, flasher);
      flashCounter = 0;
    }

    lfoSpeed[0] = fmData[36];
    lfoSpeed[1] = fmData[38];
    lfoSpeed[2] = fmData[40];

    lfoDepth[0] = fmData[37];
    lfoDepth[1] = fmData[39];
    lfoDepth[2] = fmData[41];

    // only fire LFO if the step changed
    if ((lfoStepLast[0] != lfoStep[0]) || (lfoStepLast[1] != lfoStep[1]) || (lfoStepLast[2] != lfoStep[2])) {
      lfoStepLast[0] = lfoStep[0];
      lfoStepLast[1] = lfoStep[1];
      lfoStepLast[2] = lfoStep[2];
      lfoTick();
    }


    //fire lfo anyway if some base values changed via CC
    bool fmBaseChanged = false;
    for (int i = 0; i < 51; i++) {
      if (fmBase[i] != fmBaseLast[i]) {
        fmBaseLast[i] = fmBase[i];
        fmBaseChanged = true;
      }
    }
    if (fmBaseChanged) { applyLfo(); }

    midiRead();

    if ((resetHeld) && (!shuffleTimer)) {
      resetHeld = false;
      shuffle();
      shuffleCounter = 20;
    }
    fmUpdate();

    if (!shuffleCounter)readMux();

    if ((!dotTimer) && (dotOn)) {
      dotOn = false;
      if (lastNumber != 666) { mydisplay.setLed(0, 7, 6, 0); }
      mydisplay.setLed(0, 7, 7, 0);
    }
  }
}
