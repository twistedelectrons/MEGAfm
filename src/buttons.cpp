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

int ledTest = 0;
bool resetFunction = false;

void buttChanged(byte number, bool value) {
  if (millis() > 1000) {
    if (setupMode) {
      if ((!value) && (millis() > 2000)) {
        switch (number) {
          case 4:
            thru = !thru;
            ledSet(13, thru);

            digit(0, 26);
            if (thru) { digit(1, 1); } else { digit(1, 0); }
            delay(800);

            break;//chain1

          case 9:
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

            break;//chain 2


          case 18:
            notePriority++;
            if (notePriority > 2)notePriority = 0;
            switch (notePriority) {

              case 0:
                digit(0, 11);
                digit(1, 27);
                delay(800);
                break;//LO
              case 1:
                digit(0, 23);
                digit(1, 1);
                delay(800);
                break;//HI
              case 2:
                digit(0, 11);
                digit(1, 17);
                delay(800);
                break;//LA

            }

            break;//retrig


          case 8:

            stereoCh3 = !stereoCh3;
            ledSet(15, stereoCh3);

            if (stereoCh3) {
              digit(0, 5);
              digit(1, 3);
              delay(800);
            } else {
              digit(0, 15);
              digit(1, 3);
              delay(800);
            }

            break;//chain 3

          case 7:
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
            break;//MPE mode

          case 15:
            setupMode = false;//setupExit=true;
            justQuitSetup = true;
//save setup values here

            ledSet(9, 0);
            ledSet(13, 0);
            ledSet(14, 0);
            ledSet(15, 0);

            if (arpMode) { ledSet(23, 1); } else { ledSet(23, 0); }

            EEPROM.write(3967, notePriority);

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

            temp = 0;
            bitWrite(temp, 0, lfoClockEnable[0]);
            bitWrite(temp, 1, lfoClockEnable[1]);
            bitWrite(temp, 2, lfoClockEnable[2]);
            bitWrite(temp, 3, vibratoClockEnable);
            bitWrite(temp, 4, arpClockEnable);
            bitWrite(temp, 5, fatMode);
            EEPROM.update(3953, temp);

            if (pickupMode) { EEPROM.update(3954, 1); } else { EEPROM.update(3954, 0); }

            EEPROM.update(3961, lfoVel);
            EEPROM.update(3962, lfoMod);
            EEPROM.update(3963, lfoAt);

            storeInvert();

            showVoiceMode();

            digit(0, 21);
            digit(1, 21);

            break;

        }
      }
    } else if (sendReceive) {

      //sendReceive


      if (!value) {
        //Pressed
        switch (number) {

          case 0:
            if (bank == 0) { ab = !ab; }
            bank = 0;
            ledSet(16 + bank, 1);
            flashCounter2 = 0;
            showSendReceive();
            break;//square
          case 1:
            if (bank == 1) { ab = !ab; }
            bank = 1;
            ledSet(16 + bank, 1);
            flashCounter2 = 0;
            showSendReceive();
            break;//triangle
          case 6:
            if (bank == 2) { ab = !ab; }
            bank = 2;
            ledSet(16 + bank, 1);
            flashCounter2 = 0;
            showSendReceive();
            break;//saw
          case 10:
            if (bank == 3) { ab = !ab; }
            bank = 3;
            ledSet(16 + bank, 1);
            flashCounter2 = 0;
            showSendReceive();
            break;//noise
          case 18:
            if (bank == 4) { ab = !ab; }
            bank = 4;
            ledSet(16 + bank, 1);
            flashCounter2 = 0;
            showSendReceive();
            break;//retrig
          case 11:
            if (bank == 5) { ab = !ab; }
            bank = 5;
            ledSet(16 + bank, 1);
            flashCounter2 = 0;
            showSendReceive();
            break;//loop


          case 14:
            if ((millis() > 2000) && (sendReceive == 2))sendDump();
            break;
          case 5:
            if ((millis() > 2000) && (sendReceive == 1))recieveDump();
            break;

        }
      }

    } else {

      if (!value) {
        //Pressed
        switch (number) {

          case 7:
            changedChannel = false;
            arpModeHeld = true;
            arpButtCounter = 0;
            break;//arp mode

          case 15:
            justQuitSetup = false;
            fineChanged = false;
            voiceHeld = true;
            setupCounter = 16000;
            break;//voice mode

          case 5:
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
            break;//preset up


          case 14:
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
            break;//preset down

          case 13:
            shuffled = false;
            resetHeld = true;
            shuffleTimer = 4000;
            break;//reset


          case 3:

            if (presetTargetMode) {
              presetTargetMode = false;
              savePreset();
            } else {
              if (!seqRec) {
                seqLength = 0;
                if (voiceMode != 3) {
                  voiceMode = 3;
                  showVoiceMode();
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
            break;//arp rec

          case 4:
            chainPressed = 1;
            linkCounter = 3;
            selectedLfo = 0;
            cleared = false;
            break;//chain1
          case 9:
            chainPressed = 2;
            linkCounter = 3;
            selectedLfo = 1;
            cleared = false;
            break;//chain2
          case 8:
            chainPressed = 3;
            linkCounter = 3;
            selectedLfo = 2;
            cleared = false;
            break;//chain3



          case 0:
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
              if (lfoShape[selectedLfo] == 0) {
                invertedSquare[selectedLfo] = !invertedSquare[selectedLfo];
                storeInvert();
              } else {
                lfoShape[selectedLfo] = 0;
                lfoLedOn();
                showLfo();
              }
            }
            break;//square

          case 1:
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
              lfoShape[selectedLfo] = 1;
              lfoLedOn();
              showLfo();
            }
            break;//triangle

          case 6:
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
              if (lfoShape[selectedLfo] == 2) {
                invertedSaw[selectedLfo] = !invertedSaw[selectedLfo];
                storeInvert();
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
            }
            break;//saw


          case 10:
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

              //Change LFOLENGTH
              if (lfoShape[selectedLfo] == 3) {
                noiseTableLength[selectedLfo]++;
                if (noiseTableLength[selectedLfo] > 5) { noiseTableLength[selectedLfo] = 2; }

                if (noiseTableLength[selectedLfo] == 2) {
                  digit(0, 21);
                  digit(1, 21);
                } else { ledNumber(1 << noiseTableLength[selectedLfo]); }

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
            break;//noise
          case 18:
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
              retrig[selectedLfo] = !retrig[selectedLfo];
              showLfo();
              sendCC(57, retrig[selectedLfo]);
            }
            break;//retrig
          case 11:
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
              looping[selectedLfo] = !looping[selectedLfo];
              showLfo();
              sendCC(58, looping[selectedLfo]);
            }
            break;//loop




        }
      } else {

        //Released
        switch (number) {

          case 15:
            voiceHeld = false;
            if (!justQuitSetup) {
              if (!fineChanged) {

                if (!mpe) {
                  voiceMode++;
                  if (voiceMode > 3) { voiceMode = 0; }
                  showVoiceMode();
                  sendCC(51, voiceMode);

//Reset notes after a voiceChange
                  resetVoices();

                } else {
                  digit(0, 17);
                  digit(1, 10);
                }

              }
            }

            break;//voice mode


          case 5:
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
                  if (presetTargetModeChanged) { presetTargetModeChanged--; }
                  else {
                    presetCounts = 40;
                    if (!scrollCounter) {
                      preset++;
                      if (preset > 99) { preset = 0; }
                    }
                  }
                } else {
                  if (!scrollCounter) {
                    preset++;
                    if (preset > 99) { preset = 0; }
                  }
                  loadPreset();
                  resetFunction = 0;
                  EEPROM.write(3952, preset);
                }
              }
            }
            break;//preset up

          case 14:
            pressedDown = false;
            if (seqRec) {
              if (seqLength > 1) {
                seqLength--;
                ledNumber(seqLength);
              }
            } else {
              if (!saved) {
                if (presetTargetMode) {
                  if (presetTargetModeChanged) { presetTargetModeChanged--; }
                  else {
                    presetCounts = 40;
                    if (!scrollCounter) {
                      preset--;
                      if (preset < 0) { preset = 99; }
                    }
                  }
                } else {
                  if (!scrollCounter) {
                    preset--;
                    if (preset < 0) { preset = 99; }
                  }
                  loadPreset();
                  resetFunction = 0;
                  EEPROM.write(3952, preset);
                }
              }
            }
            break;//preset down


          case 13:
            if (!shuffled) {
              if (!resetFunction) {
                loadZero();
                digit(0, 14);
                digit(1, 0);
              } else { panel(); }
              resetFunction = !resetFunction;
            }
            resetHeld = false;
            break;//reset

          case 7:
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
                      break;//up
                    case 2:
                      digit(0, 15);
                      digit(1, 19);
                      break;//dn
                    case 3:
                      digit(0, 13);
                      digit(1, 15);
                      break;//ud
                    case 4:
                      digit(1, 1);
                      digit(0, 16);
                      break;//r1
                    case 5:
                      digit(1, 2);
                      digit(0, 16);
                      break;//r2
                    case 6:
                      digit(0, 5);
                      digit(1, 18);
                      break;//se
                  }
                } else if (arpMode > 0) {

                  arpMode++;
                  if (arpMode > 7) {
                    arpMode = 1;
                    resyncArp = true;
                  }
                  arpModeLast = arpMode;

                  if ((arpMode) && (voiceMode != 3) && (!mpe)) {
                    voiceMode = 3;
                    showVoiceMode();
                    sendCC(51, voiceMode);
                  }

                  switch (arpMode) {
                    case 1:
                      digit(0, 13);
                      digit(1, 14);
                      break;//up
                    case 2:
                      digit(0, 15);
                      digit(1, 19);
                      break;//dn
                    case 3:
                      digit(0, 13);
                      digit(1, 15);
                      break;//ud
                    case 4:
                      digit(1, 1);
                      digit(0, 16);
                      break;//r1
                    case 5:
                      digit(1, 2);
                      digit(0, 16);
                      break;//r2
                    case 6:
                      digit(0, 5);
                      digit(1, 1);
                      break;//s1
                    case 7:
                      digit(0, 5);
                      digit(1, 2);
                      break;//s2
                  }
                }
              }
            }

            break;//arp mode
          case 3://arp rec


          case 4:
            chainPressed = 0;
            if ((targetPot != 36) && (targetPot != 37)) {//prevent lfo linking to itsef
              if (!cleared) {
                linked[0][targetPot] = !linked[0][targetPot];
                showLink();
              }
            }
            break;//chain1

          case 9:
            chainPressed = 0;
            if ((targetPot != 38) && (targetPot != 39)) {//prevent lfo linking to itsef
              if (!cleared) {
                linked[1][targetPot] = !linked[1][targetPot];
                showLink();
              }
            }
            break;//chain2

          case 8:
            chainPressed = 0;
            if ((targetPot != 40) && (targetPot != 41)) {//prevent lfo linking to itsef
              if (!cleared) {
                linked[2][targetPot] = !linked[2][targetPot];
                showLink();
              }
            }
            break;//chain3

          case 0: //square
          case 1: //triangle
          case 6: //saw
          case 10: //noise
          case 18: //retrig
          case 11: //loop
          default:
            break;
        }
      }
    }
  }
}
