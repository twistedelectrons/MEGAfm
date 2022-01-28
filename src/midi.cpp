#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"
#include "leds.h"
#include "arp.h"
#include "lfo.h"
#include "pitchEngine.h"
#include "preset.h"
#include "voice.h"
#include "midi.h"
#include "midiEngine.h"
#include "pots.h"

byte voiceSlot;
bool ch3Alt;
const float vibIncrements[8] = {5.312, 7.968, 10.625, 14.166, 15.937, 31.875, 42.5, 63.75};
float vibIndexF;

byte lastCC[60];

byte freeVoice = 99;

int arpClockCounter;
byte syncLfoCounter;

bool clockAlternator;
int heldKeysMinus = 0;
bool arpClearFlag = false;

void HandleAt(byte channel, byte val) {
  leftDot();
  if (channel == inputChannel) {

    if (lfoAt) {
      if (fmBase[40]) {
        fmBase[41] = val << 1;
        fmBaseLast[41] = fmBase[41] - 1;
        ledNumberTimeOut = 20;
      } else {
        atDest = val << 1;
        ledNumberTimeOut = 20;
      }
    }
  }

}

void handleClock() {
  if (sync) {

////////////////////////////////////

////////      VIBRATO        ///////

///////////////////////////////////
    if (vibratoClockEnable) {
      if (fmData[48]) {

        vibIndexF += vibIncrements[fmData[48] >> 5];
        if (vibIndexF > 255) { vibIndexF -= 256; }
        vibIndex = int(vibIndexF);

      }
    }

    absoluteClockCounter++;
    if (absoluteClockCounter >= 48) {
      leftDot();
      absoluteClockCounter = 0;

      if (arpClockEnable) {
        if (arpMidiSpeed != arpMidiSpeedPending || resyncArp) {
          arpMidiSpeed = arpMidiSpeedPending;
          arpClockCounter = 0;
          resyncArp = false;
        }
      }

      for (int i = 0; i < 3; i++) {
        if (lfoClockEnable[i]) {
          if (lfoClockSpeedPending[i]) {
            lfoClockSpeed[i] = lfoClockSpeedPending[i] - 1;
            lfoClockSpeedPending[i] = 0;
            lfoStepF[i] = 0;
          }
        }
      }

    }


////////////////////////////////////

////////        ARP          ///////

///////////////////////////////////

    if ((arpClockEnable) && (arpMode) && (voiceMode == 3) && (arpMode != 7)) {
      arpClockCounter++;
      if ((arpClockCounter >= midiArpTicks[arpMidiSpeed])) {
        arpClockCounter = 0;
        if (!emptyStack) { arpFire(); }
      }
    }



    //if Any LFOClock Enable

    if ((lfoClockEnable[0]) || (lfoClockEnable[1]) || (lfoClockEnable[2])) {

      //activate the LFO
      lfoTick();

      syncLfoCounter++;
      if (syncLfoCounter == 24) {
        syncLfoCounter = 0;
      }

      for (int i = 0; i < 3; i++) {
        if (lfoClockEnable[i]) {
          lfoStepF[i] += lfoClockRates[lfoClockSpeed[i]];
          if (lfoStepF[i] >= 255) {
            if (looping[i]) {
              if (selectedLfo == i) { lfoBlink(); }
              lfoStepF[i] = 0;
              lfoNewRand[i] = 1;
            } else { lfoStepF[i] = 255; }
          }
          lfoStep[i] = int(lfoStepF[i]);
        }
      }
    }
  }
}

void handleBendy(byte channel, int bend) {
  leftDot();

  if (mpe) {
    if (channel > 12)channel = 12;
    bendy = 0;
    if (bend != 0) {
      if (bend < 0) {
        mpeBend[channel - 1] = bend;
        mpeBend[channel - 1] /= 8192;
        mpeBend[channel - 1] *= bendDown;
      } else if (bend > 0) {
        mpeBend[channel - 1] = bend;
        mpeBend[channel - 1] /= 8191;
        mpeBend[channel - 1] *= bendUp;
      }
    }
    setNote(channel - 1, notey[channel - 1]);

  } else {
    if (channel == inputChannel) {
      //-8192 to 8191
      bendy = 0;
      if (bend != 0) {
        if (bend < 0) {
          bendy = bend;
          bendy /= 8192;
          bendy *= bendDown;
        } else if (bend > 0) {
          bendy = bend;
          bendy /= 8191;
          bendy *= bendUp;
        }
      }

      bendyCounter = 4;
    }
  }
}

void handleStop() {

  sync = false;
}

void handleStart() {

  if (vibratoClockEnable)vibIndex = 0;

  absoluteClockCounter = 0;
  seqStep = 0;
  arpClockCounter = 0;
  sync = true;
}

void HandlePc(byte channel, byte program) {

  if ((program < 99) && (channel == inputChannel)) {

    preset = program;
    loadPreset();

  }
}

void handleNoteOn(byte channel, byte note, byte velocity) {

  byte distanceFromNewNote;

  if (setupMode) {

    if (bendRoot == -1) {
      bendRoot = note;

      digit(0, 10);
      digit(1, 23);
      delay(500);
      lastNumber = -1;
      ledNumber(inputChannel);
      delay(750);

      if (channel != inputChannel) {
        inputChannel = channel;
        EEPROM.write(3951, inputChannel);
      }

    } else {
      if (note > bendRoot) {
        bendUp = note - bendRoot;
        if (bendUp > 48) { bendUp = 48; }
        EEPROM.write(3959, byte(bendUp));

        digit(0, 13);
        digit(1, 14);
        delay(500);
        lastNumber = -1;
        ledNumber(bendUp);
        delay(750);
      } else if (note < bendRoot) {
        bendDown = bendRoot - note;
        if (bendDown > 48) { bendDown = 48; }
        EEPROM.write(3958, byte(bendDown));

        digit(0, 15);
        digit(1, 19);
        delay(500);
        lastNumber = -1;
        ledNumber(bendDown);
        delay(750);
      }
    }

  } else {

    if (lfoVel) {
      if (fmBase[36]) {
        fmBase[37] = velocity << 1;
        fmBaseLast[37] = fmBase[37] - 1;
        dontShow[37] = 1;
      } else {
        lfo[0] = velocity << 1;
        applyLfo();
      }
    }

    if (mpe) {

      note += 3;

      if (channel > 12)channel = 12;

      ym.noteOff(channel - 1);
      notey[channel - 1] = note;
      setNote(channel - 1, notey[channel - 1]);
      ym.noteOn(channel - 1);
    } else {

      if (channel == inputChannel) {

        note += 3;
        rootNote = note;

        if (velocity) {

          for (int i = 0; i < 3; i++) {
            if ((retrig[i]) || ((!retrig[i]) && (!looping[i]) && (heldKeys == 0))) { lfoStep[i] = 0; }
          }
//reset ARP
          if (arpMode == 6) {
            arpStep = seqStep = 0;
            arpIndex = 0;
          } else if (arpMode == 7) { arpCounter = 1023; }//next manual arp step

          switch (voiceMode) {
////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
            case 0:// poly12
              ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
              voiceSlots[voiceSlot] = 1;
//if gliding jump to last pitch associated to keycounter 
              if (glide) {
                setNote(voiceSlot, lastNotey[heldKeys]);
                skipGlide(voiceSlot);
              }

              noteOfVoice[voiceSlot] = note;
              lastNotey[heldKeys] = note;
              setNote(voiceSlot, noteOfVoice[voiceSlot]);
              ym.noteOff(voiceSlot);
              ym.noteOn(voiceSlot);

              voiceSlot++;
              if (voiceSlot > 11)voiceSlot = 0;

              if (heldKeys < 30)heldKeys++;

              break;

////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
            case 1:// wide6
              ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////

              voiceSlots[voiceSlot] = 1;
//if gliding jump to last pitch associated to keycounter 
              if (glide) {
                setNote(voiceSlot, lastNotey[heldKeys]);
                skipGlide(voiceSlot);
                setNote(voiceSlot + 6, lastNotey[heldKeys]);
                skipGlide(voiceSlot + 6);
              }
              noteOfVoice[voiceSlot] = note;
              lastNotey[heldKeys] = note;
              setNote(voiceSlot, noteOfVoice[voiceSlot]);
              setNote(voiceSlot + 6, noteOfVoice[voiceSlot]);
              ym.noteOff(voiceSlot);
              ym.noteOff(voiceSlot + 6);
              ym.noteOn(voiceSlot);
              ym.noteOn(voiceSlot + 6);

              voiceSlot++;
              if (voiceSlot > 5)voiceSlot = 0;

              if (heldKeys < 30)heldKeys++;

              break;

              ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
            case 2:
// dual CH3
              ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////

              if (stereoCh3) {
//fire at the same time

//                noteToChannel[note] = 4; unused?
                ym.noteOff(4);
                setNote(4, note);
                ym.noteOn(4);//CHIP1
                setNote(2, note);

//                noteToChannel[note] = 5; unused?
                ym.noteOff(5);
                setNote(5, note);
                ym.noteOn(5);//CHIP2
                setNote(8, note);
              } else {

                ch3Alt = !ch3Alt;

                if (ch3Alt) {

//                  noteToChannel[note] = 4; // unused?
                  ym.noteOff(4);
                  setNote(4, note);
                  ym.noteOn(4);//CHIP1
                  setNote(2, note);

                } else {

//                  noteToChannel[note] = 5; // unused?
                  ym.noteOff(5);
                  setNote(5, note);
                  ym.noteOn(5);//CHIP2
                  setNote(8, note);
                }

              }

              break;

              ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
            case 3:// unison
              ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
              if ((arpMode) && (fmData[46])) {
                //ARP

                heldKeys++;

                if (((pedal) && (arpClearFlag)) || (heldKeys == 1)) {
                  arpClearFlag = false;
                  clearNotes();
                  heldKeys = 1;
                }

                if ((heldKeys == 1) && (!pedal) &&
                    (!sync)) { arpCounter = 1023; }//only retrigger arp on first key or if arp is stopped

                addNote(note);

              } else {
//NO ARP

                heldKeys++;

                lastNote = note;
                addNote(note);

                for (int i = 0; i < 12; i++) {
                  ym.noteOff(i);
                  setNote(i, note);
                  ym.noteOn(i);
                }
              }

              if (seqRec) {
                if (!seqLength)rootNote1 = note;
                seq[seqLength] = note - rootNote1 + 127;
                if (seq[seqLength] == 255) { seq[seqLength]--; }
                if (seqLength < 16) {
                  seqLength++;
                  ledNumber(seqLength);
                }

                for (int i = 0; i < 12; i++) {
                  ym.noteOff(i);
                  setNote(i, note);
                  ym.noteOn(i);
                }

              }

              break;

          }

        } else { handleNoteOff(channel, note, velocity); }
        leftDot();
      }
    }
  }
}

///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /
///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /   ///// /// // /

void handleNoteOff(byte channel, byte note, byte velocity) {

  if (setupMode) {

    if (note == bendRoot) { bendRoot = -1; }
  } else {

    if (mpe) {

      note += 3;

      if (channel > 12)channel = 12;
      ym.noteOff(channel - 1);
    } else {

      if (channel == inputChannel) {

        note += 3;

        switch (voiceMode) {
////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
          case 0:// poly12
            ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
            heldKeys--;
            if (heldKeys < 0)heldKeys = 0;

            if (pedal) {
              //find out which channel the note is and add to pedalOff
              for (int i = 0; i < 12; i++) {
                if (noteOfVoice[i] == note) {
                  pedalOff[i] = 1;//abort loop
                }
              }

            } else {

//scan through the noteOfVoices and kill the voice associated to it
              for (int i = 0; i < 12; i++) {
                if ((voiceSlots[i]) && (noteOfVoice[i] == note)) {
                  voiceSlots[i] = 0;
                  ym.noteOff(i);

                }
              }
            }

            break;


////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
          case 1:// wide6
            ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////   ////
            heldKeys--;
            if (heldKeys < 0)heldKeys = 0;

            if (pedal) {
              //find out which channel the note is and add to pedalOff
              for (int i = 0; i < 6; i++) {
                if (noteOfVoice[i] == note) {
                  pedalOff[i] = 1;
                  pedalOff[i + 6] = 1;//abort loop
                }
              }

            } else {

//scan through the noteOfVoices and kill the voice associated to it
              for (int i = 0; i < 6; i++) {
                if ((voiceSlots[i]) && (noteOfVoice[i] == note)) {
                  voiceSlots[i] = 0;
                  ym.noteOff(i);
                  voiceSlots[i + 6] = 0;
                  ym.noteOff(i + 6);

                }
              }
            }

            break;

          case 2://dual CH3

            if (stereoCh3) {
//fire at the same time

              ym.noteOff(4);//CHIP1
              ym.noteOff(5);//CHIP2

            } else {

              ch3Alt = !ch3Alt;

              if (ch3Alt) {

                ym.noteOff(4);//CHIP1

              } else {

                ym.noteOff(5);//CHIP2
              }

            }

            break;

          case 3:// unison
            if (arpMode) {

// ARP
              if (pedal) {
//here we must prepare to erase arp stack
                arpClearFlag = true;
                heldKeysMinus++;
              } else {
                heldKeys--;
                removeNote(note);

                if (heldKeys < 1) {
                  heldKeys = 0;

                  if (!pedal) { for (int i = 0; i < 12; i++) { ym.noteOff(i); }}
                  else { for (int i = 0; i < 12; i++) { pedalOff[i] = 1; }}

                }
              }
            } else {

// NO ARP

              heldKeys--;
              removeNote(note);


//LAST KEY UP?
              if (heldKeys < 1) {

                heldKeys = 0;
                clearNotes();

                if (!pedal) { for (int i = 0; i < 12; i++) { ym.noteOff(i); }}
                else { for (int i = 0; i < 12; i++) { pedalOff[i] = 1; }}

              } else {
                //NOT LAST KEY, CHANGE NOTE

                switch (notePriority) {
                  case 0:
                    note = getLow();
                    break;//LOWEST
                  case 1:
                    note = getHigh();
                    break;//HIGHEST
                  case 2:
                    note = getLast();
                    break;//LAST
                }
                rootNote = note;
                for (int i = 0; i < 12; i++) { setNote(i, note); }

              }
            }
            break;

            leftDot();

        }

      }
    }
  }
}

void sendCC(byte number, int value) {

  if (lastCC[number] != value) {
    lastCC[number] = value;
    rightDot();
    sendControlChange(number, value, masterChannelOut);
  }
}

void sendMidiButt(byte number, int value) {
  rightDot();
  sendCC(number, value);
}

void HandleControlChange(byte channel, byte number, byte val) {
  if((lastSentCC[0]==number)&&(lastSentCC[1]==val)){
    //ignore same CC and DATA as sent to avoid feedback
  } else {
    leftDot();
    if (channel == inputChannel) {
      if (number == 0) {
        if (val < 5) {
          if (bank != val) {
            bank = val;
            HandlePc(inputChannel, preset - 1);
          }
        }
      } else if (number == 50) { movedPot(0, val << 1, 1); }
      else if (number == 42) { movedPot(42, val << 5, 1); }
      else if (number == 51) { movedPot(7, val << 1, 1); }
      else if (number == 49) { movedPot(8, val << 1, 1); }

      else if (number == 1) {

        if (lfoMod) {
          if (fmBase[38]) {
            fmBase[39] = val << 1;
            fmBaseLast[39] = fmBase[39] - 1;
            ledNumberTimeOut = 20;
          } else {
            lfo[1] = val << 1;
            applyLfo();
          }
        }
      } else if (number == 7) {
        if (allCC) { movedPot(1, val << 1, 1); }
      } else if (number == 64) {
        if (val > 63) { pedalDown(); } else { pedalUp(); }
      } else {
        if (allCC) { movedPot(number, val << 1, 1); }
        else {
          if ((number != 19) && (number != 40) && (number != 16) && (number != 38))movedPot(number, val << 1, 1);
        }
      }
    }
  }
}

void midiOut(byte note) {
  rightDot();
// midiB.sendNoteOff(lastNote+1,127,masterChannelOut);
// midiB.sendNoteOn(note+1,velocityLast,masterChannelOut);
  lastNote = note;
}

void pedalUp() {
  if (voiceMode == 3) {
    if (heldKeysMinus) {
      heldKeys -= heldKeysMinus;
      if (heldKeys < 1) {
        heldKeys = 0;
        for (int i = 0; i < 12; i++) { ym.noteOff(i); }

      }
      heldKeysMinus = 0;
    }

    if (!heldKeys)clearNotes();
  } else {

    for (int i = 0; i < 12; i++) {
      if (pedalOff[i]) {

        voiceSlots[i] = 0;
        noteOfVoice[i] = 0;

        switch (voiceMode) {

          case 0:

            ym.noteOff(i);
            pedalOff[i] = 0;
            if (heldKeys) { heldKeys--; }
            break;//poly

          case 1:
          case 2:
          case 3:

            ym.noteOff(i);
            pedalOff[i] = 0;
            if (heldKeys) { heldKeys--; }
            break;
        }
      }

    }
  }
  pedal = false;
}

void pedalDown() {
  pedal = true;
}

void dumpPreset() {
  for (int number = 0; number < 58; number++) {
    switch (number) {
      //@formatter:off
      //OP1
      case 18: sendCC(number, fmBase[0] >> 1);          break; //detune
      case 27: sendCC(number, fmBase[1] >> 1);          break; //multiple
      case 19: sendCC(number, fmBase[2] >> 1);          break; //op level
      case 29: sendCC(number, fmBase[4] >> 1);          break; //attack
      case 21: sendCC(number, fmBase[5] >> 1);          break; //decay1
      case 25: sendCC(number, fmBase[7] >> 1);          break; //sustain
      case 17: sendCC(number, fmBase[6] >> 1);          break; //sustain rate
      case 30: sendCC(number, fmBase[8] >> 1);          break; //release
      //OP2
      case 31: sendCC(number, fmBase[18] >> 1);         break; //detune
      case 32: sendCC(number, fmBase[19] >> 1);         break; //multiple
      case 40: sendCC(number, fmBase[20] >> 1);         break; //op level
      case 36: sendCC(number, fmBase[22] >> 1);         break; //attack
      case 44: sendCC(number, fmBase[23] >> 1);         break; //decay1
      case 42: sendCC(number, fmBase[25] >> 1);         break; //sustain
      case 34: sendCC(number, fmBase[24] >> 1);         break; //sustain rate
      case 11: sendCC(number, fmBase[26] >> 1);         break; //release
      //OP3
      case 20: sendCC(number, fmBase[9] >> 1);          break; //detune
      case 24: sendCC(number, fmBase[10] >> 1);         break; //multiple
      case 16: sendCC(number, fmBase[11] >> 1);         break; //op level
      case 8:  sendCC(49, fmBase[13] >> 1);             break; //attack
      case 0:  sendCC(50, fmBase[14] >> 1);             break; //decay1
      case 7:  sendCC(51, fmBase[16] >> 1);             break; //sustain
      case 45: sendCC(number, fmBase[15] >> 1);         break; //sustain rate
      case 37: sendCC(number, fmBase[17] >> 1);         break; //release
      //OP4
      case 47: sendCC(number, fmBase[27] >> 1);         break; //detune
      case 39: sendCC(number, fmBase[28] >> 1);         break; //multiple
      case 38: sendCC(number, fmBase[29] >> 1);         break; //op level
      case 46: sendCC(number, fmBase[31] >> 1);         break; //attack
      case 33: sendCC(number, fmBase[32] >> 1);         break; //decay1
      case 41: sendCC(number, fmBase[34] >> 1);         break; //sustain
      case 43: sendCC(number, fmBase[33] >> 1);         break; //sustain rate
      case 35: sendCC(number, fmBase[35] >> 1);         break; //release

      case 1:  sendCC(7, vol >> 1);                     break; //volume //SEND FINE!!!!!!!!!!!!
      case 4:  sendCC(number, (1 + (fmBase[42] >> 5))); break; //algo
      case 3:  sendCC(number, fmBase[43] >> 1);         break; //feedback
      case 28: sendCC(number, fmBase[50] >> 1);         break; //fat 1-127 // SEND GLIDE!!!!!!!!!!!!
      case 15: sendCC(number, fmBase[36] >> 1);         break; //lfo 1 rate
      case 12: sendCC(number, fmBase[37] >> 1);         break; //lfo 1 depth
      case 10: sendCC(number, fmBase[38] >> 1);         break; //lfo 2 rate
      case 9:  sendCC(number, fmBase[39] >> 1);         break; //lfo 2 depth
      case 14: sendCC(number, fmBase[40] >> 1);         break; //lfo 3 rate
      case 2:  sendCC(number, fmBase[41] >> 1);         break; //lfo 3 depth
      case 6:  sendCC(number, fmBase[46] >> 1);         break; ///arp rate
      case 5:  sendCC(number, fmBase[47] >> 1);         break; //arp range
      case 48: sendCC(number, fmBase[48] >> 1);         break; //vibrato rate WAS 7
      case 13: sendCC(number, fmBase[49] >> 1);         break; //vibrato depth
      //@formatter:on
    }
  }
}
