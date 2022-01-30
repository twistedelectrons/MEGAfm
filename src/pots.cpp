#include <Arduino.h>
#include <EEPROM.h>
#include "megafm.h"
#include "leds.h"
#include "pickup.h"
#include "FM.h"
#include "midi.h"
#include "pitchEngine.h"
#include "lfo.h"
#include "loop.h"

void movedPot(byte number, byte data, bool isMidi) {
  if (secPast) {
    //comes in as 8bit
    if ((!seqRec) && (!sendReceive)) {
      if (setupMode) {

        ledSet(10, 0);
        ledSet(11, 0);
        ledSet(12, 0);

        switch (number) {

          case 6:
            setupChanged = true;
            if (data > 127) {
              arpClockEnable = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              arpClockEnable = 0;
              digit(0, 0);
              digit(1, 12);
            }
            break;//arp rate
          case 15:
            setupChanged = true;
            if (data > 127) {
              lfoClockEnable[0] = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              lfoClockEnable[0] = 0;
              digit(0, 0);
              digit(1, 12);
            }
            break;//lfo 1 rate
          case 10:
            setupChanged = true;
            if (data > 127) {
              lfoClockEnable[1] = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              lfoClockEnable[1] = 0;
              digit(0, 0);
              digit(1, 12);
            }
            break;//lfo 2 rate
          case 14:
            setupChanged = true;
            if (data > 127) {
              lfoClockEnable[2] = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              lfoClockEnable[2] = 0;
              digit(0, 0);
              digit(1, 12);
            }
            break;//lfo 3 rate
          case 48:
            setupChanged = true;
            if (data > 127) {
              vibratoClockEnable = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              ledSet(9, 0);
              vibratoClockEnable = 0;
              digit(0, 0);
              digit(1, 12);
            }
            break;//vib rate

          case 12:
            setupChanged = true;
            if (data > 127) {
              lfoVel = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              lfoVel = 0;
              digit(0, 0);
              digit(1, 12);
            }
            if (lfoVel != lastLfoSetting[0]) {
              lastLfoSetting[0] = !lastLfoSetting[0];
              digit(0, 13);
              digit(1, 18);
              delay(500);
              if (data > 127) {
                digit(0, 0);
                digit(1, 19);
              } else {
                digit(0, 0);
                digit(1, 12);
              }
            }
            break;//lfo 1 depth

          case 9:
            setupChanged = true;
            if (data > 127) {
              lfoMod = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              lfoMod = 0;
              digit(0, 0);
              digit(1, 12);
            }
            if (lfoMod != lastLfoSetting[1]) {
              lastLfoSetting[1] = !lastLfoSetting[1];
              digit(0, 10);
              digit(1, 1);
              delay(500);
              if (data > 127) {
                digit(0, 0);
                digit(1, 19);
              } else {
                digit(0, 0);
                digit(1, 12);
              }
            }
            break;//lfo 2 depth

          case 2:
            setupChanged = true;
            if (data > 127) {
              lfoAt = 1;
              digit(0, 0);
              digit(1, 19);
            } else {
              lfoAt = 0;
              digit(0, 0);
              digit(1, 12);
            }
            if (lfoAt != lastLfoSetting[2]) {
              lastLfoSetting[2] = !lastLfoSetting[2];
              digit(0, 17);
              digit(1, 26);
              delay(500);
              if (data > 127) {
                digit(0, 0);
                digit(1, 19);
              } else {
                digit(0, 0);
                digit(1, 12);
              }
            }
            break;//lfo 3 depth

          case 28:
            setupChanged = true;
            if (data < 128) {
              fatMode = 0;
              digit(0, 1);
              digit(1, 5);
            } else {
              fatMode = 1;
              digit(0, 1);
              digit(1, 27);
            }
            break;//fat

          case 1:
            setupChanged = true;
            if (data > 127) {
              ignoreVolume = 0;
              digit(0, 0);
              digit(1, 19);
            } else {
              ignoreVolume = 1;
              digit(0, 0);
              digit(1, 12);
            }
            break;//volume preset


          case 13:
            byte brightness = data >> 4;
            if (brightness > 15)brightness = 10;//default;
            mydisplay.setIntensity(0, brightness); // 15 = brightest
            EEPROM.write(3965, brightness);
            break;

        }

      } else {

        // not setup mode

        if ((pickupMode) && (pickup[number])) {
          //param hasn't been picked up yet, tel user if its too high or low
          doPickup(number, data);

        } else {

          switch (number) {

            //OP1
            case 18:
              fmBase[0] = data;
              updateFMifNecessary(0);
              if (voiceMode == 2) { ledNumber(data >> 2); } else { ledNumber(-3 + (data >> 5)); }
              if (!isMidi) {
                targetPot = 0;
                sendCC(number, data >> 1);
              }
              dontShow[0] = isMidi;
              break;//detune
            case 27:
              fmBase[1] = data;
              updateFMifNecessary(1);
              if (!isMidi) {
                targetPot = 1;
                sendCC(number, data >> 1);
              }
              dontShow[1] = isMidi;
              break;//multiple
            case 19:
              fmBase[2] = data;
              updateFMifNecessary(2);
              if (!isMidi) {
                targetPot = 2;
                sendCC(number, data >> 1);
              }
              dontShow[2] = isMidi;
              break;//op level
            case 29:
              fmBase[4] = data;
              updateFMifNecessary(4);
              if (!isMidi) {
                targetPot = 4;
                sendCC(number, data >> 1);
              }
              dontShow[4] = isMidi;
              break;//attack
            case 21:
              fmBase[5] = data;
              updateFMifNecessary(5);
              if (!isMidi) {
                targetPot = 5;
                sendCC(number, data >> 1);
              }
              dontShow[5] = isMidi;
              break;//decay1
            case 25:
              fmBase[7] = data;
              updateFMifNecessary(7);
              if (!isMidi) {
                targetPot = 7;
                sendCC(number, data >> 1);
              }
              dontShow[7] = isMidi;
              break;//sustain
            case 17:
              fmBase[6] = data;
              updateFMifNecessary(9);
              if (!isMidi) {
                targetPot = 6;
                sendCC(number, data >> 1);
              }
              dontShow[6] = isMidi;
              break;//sustain rate
            case 30:
              fmBase[8] = data;
              updateFMifNecessary(8);
              if (!isMidi) {
                targetPot = 8;
                sendCC(number, data >> 1);
              }
              dontShow[8] = isMidi;
              break;//release


              //OP2
            case 31:
              fmBase[18] = data;
              updateFMifNecessary(18);
              dontShow[18] = isMidi;
              if (voiceMode == 2) { ledNumber(data >> 2); } else { ledNumber(-3 + (data >> 5)); }
              if (!isMidi) {
                targetPot = 18;
                sendCC(number, data >> 1);
              }
              break;//detune
            case 32:
              fmBase[19] = data;
              updateFMifNecessary(19);
              dontShow[19] = isMidi;
              if (!isMidi) {
                targetPot = 19;
                sendCC(number, data >> 1);
              }
              break;//multiple
            case 40:
              fmBase[20] = data;
              updateFMifNecessary(20);
              dontShow[20] = isMidi;
              if (!isMidi) {
                targetPot = 20;
                sendCC(number, data >> 1);
              }
              break;//op level
            case 36:
              fmBase[22] = data;
              updateFMifNecessary(22);
              dontShow[22] = isMidi;
              if (!isMidi) {
                targetPot = 22;
                sendCC(number, data >> 1);
              }
              break;//attack WAS 59
            case 44:
              fmBase[23] = data;
              updateFMifNecessary(23);
              dontShow[23] = isMidi;
              if (!isMidi) {
                targetPot = 23;
                sendCC(number, data >> 1);
              }
              break;//decay1 WAS 50
            case 42:
              fmBase[25] = data;
              updateFMifNecessary(25);
              dontShow[25] = isMidi;
              if (!isMidi) {
                targetPot = 25;
                sendCC(number, data >> 1);
              }
              break;//sustain WAS 60
            case 34:
              fmBase[24] = data;
              updateFMifNecessary(24);
              dontShow[24] = isMidi;
              if (!isMidi) {
                targetPot = 24;
                sendCC(number, data >> 1);
              }
              break;//sustain rate WAS 55
            case 11:
              fmBase[26] = data;
              updateFMifNecessary(26);
              dontShow[26] = isMidi;
              if (!isMidi) {
                targetPot = 26;
                sendCC(number, data >> 1);
              }
              break;//release WAS 52


              //OP3
            case 20:
              fmBase[9] = data;
              updateFMifNecessary(9);
              dontShow[9] = isMidi;
              if (voiceMode == 2) { ledNumber(data >> 2); } else { ledNumber(-3 + (data >> 5)); }
              if (!isMidi) {
                targetPot = 9;
                sendCC(number, data >> 1);
              }
              break;//detune
            case 24:
              fmBase[10] = data;
              updateFMifNecessary(10);
              dontShow[10] = isMidi;
              if (!isMidi) {
                targetPot = 10;
                sendCC(number, data >> 1);
              }
              break;//multiple
            case 16:
              fmBase[11] = data;
              updateFMifNecessary(11);
              dontShow[11] = isMidi;
              if (!isMidi) {
                targetPot = 11;
                sendCC(number, data >> 1);
              }
              break;//op level
            case 8:
              fmBase[13] = data;
              updateFMifNecessary(13);
              dontShow[13] = isMidi;
              if (!isMidi) {
                targetPot = 13;
                sendCC(49, data >> 1);
              }
              break;//attack
            case 0:
              fmBase[14] = data;
              updateFMifNecessary(14);
              dontShow[14] = isMidi;
              if (!isMidi) {
                targetPot = 14;
                sendCC(50, data >> 1);
              }
              break;//decay1
            case 7:
              fmBase[16] = data;
              updateFMifNecessary(16);
              dontShow[16] = isMidi;
              if (!isMidi) {
                targetPot = 16;
                sendCC(51, data >> 1);
              }
              break;//sustain
            case 45:
              fmBase[15] = data;
              updateFMifNecessary(15);
              dontShow[15] = isMidi;
              if (!isMidi) {
                targetPot = 15;
                sendCC(number, data >> 1);
              }
              break;//sustain rate
            case 37:
              fmBase[17] = data;
              updateFMifNecessary(17);
              dontShow[17] = isMidi;
              if (!isMidi) {
                targetPot = 17;
                sendCC(number, data >> 1);
              }
              break;//release



              //OP4
            case 47:
              fmBase[27] = data;
              updateFMifNecessary(27);
              dontShow[27] = isMidi;
              if (voiceMode == 2) { ledNumber(data >> 2); } else { ledNumber(-3 + (data >> 5)); }
              if (!isMidi) {
                targetPot = 27;
                sendCC(number, data >> 1);
              }
              break;//detune
            case 39:
              fmBase[28] = data;
              updateFMifNecessary(28);
              dontShow[28] = isMidi;
              if (!isMidi) {
                targetPot = 28;
                sendCC(number, data >> 1);
              }
              break;//multiple
            case 38:
              fmBase[29] = data;
              updateFMifNecessary(29);
              dontShow[29] = isMidi;
              if (!isMidi) {
                targetPot = 29;
                sendCC(number, data >> 1);
              }
              break;//op level
            case 46:
              fmBase[31] = data;
              updateFMifNecessary(31);
              dontShow[31] = isMidi;
              if (!isMidi) {
                targetPot = 31;
                sendCC(number, data >> 1);
              }
              break;//attack
            case 33:
              fmBase[32] = data;
              updateFMifNecessary(32);
              dontShow[32] = isMidi;
              if (!isMidi) {
                targetPot = 32;
                sendCC(number, data >> 1);
              }
              break;//decay1
            case 41:
              fmBase[34] = data;
              updateFMifNecessary(34);
              dontShow[34] = isMidi;
              if (!isMidi) {
                targetPot = 34;
                sendCC(number, data >> 1);
              }
              break;//sustain
            case 43:
              fmBase[33] = data;
              updateFMifNecessary(33);
              dontShow[33] = isMidi;
              if (!isMidi) {
                targetPot = 33;
                sendCC(number, data >> 1);
              }
              break;//sustain rate
            case 35:
              fmBase[35] = data;
              updateFMifNecessary(35);
              dontShow[35] = isMidi;
              if (!isMidi) {
                targetPot = 35;
                sendCC(number, data >> 1);
              }
              break;//release


            case 1:
              setupCounter = 0;//prevent entering setup
              if (voiceHeld) {
                fine = data;
                updateFine();

                if (fine > 127) { ledNumber(map(fine, 128, 255, 0, 32)); }
                else if (fine < 128) { ledNumber(map(fine, 128, 0, 0, 32)); }

              } else {

                vol = 128 - (data >> 1);
                if (lastVol != vol) {
                  changeVol = true;
                  lastVol = vol;
                  ledNumber(data >> 2);
                  volumeCounter = 20;
                  if (!isMidi) { sendCC(7, data >> 1); }
                }

              }
              break;//volume

            case 4:
              if (isMidi) { data -= 1; }
              fmBase[42] = data;
              updateFMifNecessary(42);
              dontShow[42] = isMidi;
              if (!isMidi) {
                targetPot = 42;
                sendCC(number, (1 + (data >> 5)));
              }
              break;//algo

            case 3:
              fmBase[43] = data;
              updateFMifNecessary(43);
              dontShow[43] = isMidi;
              if (!isMidi) {
                targetPot = 43;
                sendCC(number, (data >> 1));
              }
              break;//feedback

            case 28:
              setupCounter = 0;//prevent entering setup
              if (voiceHeld) {
                glide = data >> 4;
                updateGlideIncrements();
                fineChanged = true;
                ledNumber(data >> 2);
              } else {
                fmBase[50] = data;
                updateFMifNecessary(50);
                dontShow[50] = isMidi;
                ledNumber(data >> 2);
                if (!isMidi) {
                  targetPot = 50;
                  sendCC(number, data >> 1);
                }
              }
              break;//fat 1-127

            case 15:
              fmBase[36] = data;
              updateFMifNecessary(36);
              if ((lfoVel) && (!data)) {
                digit(0, 13);
                digit(1, 18);
              } else {
                dontShow[36] = isMidi;
                selectedLfo = 0;
                if ((lfoClockEnable[0]) && (sync)) { ledNumber(arpRateDisplay[data >> 5]); }
                else {
                  ledNumber(data >> 2);
                }
                if (!isMidi) {
                  targetPot = 36;
                  sendCC(number, data >> 1);
                }
              }
              break;//lfo 1 rate
            case 12:
              fmBase[37] = data;
              updateFMifNecessary(37);
              dontShow[37] = isMidi;
              selectedLfo = 0;
              if (!isMidi) {
                targetPot = 37;
                sendCC(number, data >> 1);
              }
              break;//lfo 1 depth


            case 10:
              fmBase[38] = data;
              updateFMifNecessary(38);
              if ((lfoMod) && (!data)) {
                digit(0, 10);
                digit(1, 1);
              } else {
                dontShow[38] = isMidi;
                selectedLfo = 1;
                if ((lfoClockEnable[1]) && (sync)) { ledNumber(arpRateDisplay[data >> 5]); }
                else {
                  ledNumber(data >> 2);
                }
                if (!isMidi) {
                  targetPot = 38;
                  sendCC(number, data >> 1);
                }
              }
              break;//lfo 2 rate
            case 9:
              fmBase[39] = data;
              updateFMifNecessary(39);
              dontShow[39] = isMidi;
              selectedLfo = 1;
              if (!isMidi) {
                targetPot = 39;
                sendCC(number, data >> 1);
              }
              break;//lfo 2 depth

            case 14:
              fmBase[40] = data;
              updateFMifNecessary(40);
              if ((lfoAt) && (data < 1)) {
                digit(0, 17);
                digit(1, 26);
              } else {
                dontShow[40] = isMidi;
                selectedLfo = 2;
                if ((lfoClockEnable[2]) && (sync)) { ledNumber(arpRateDisplay[data >> 5]); }
                else {
                  ledNumber(data >> 2);
                }
                if (!isMidi) {
                  targetPot = 40;
                  sendCC(number, data >> 1);
                }
              }
              break;//lfo 3 rate
            case 2:
              fmBase[41] = data;
              updateFMifNecessary(41);
              dontShow[41] = isMidi;
              selectedLfo = 2;
              if (!isMidi) {
                targetPot = 41;
                sendCC(number, data >> 1);
              }
              break;//lfo 3 depth


            case 6:
              fmBase[46] = data;
              updateFMifNecessary(46);
              dontShow[46] = isMidi;
              if (sync) {
                ledNumber(arpRateDisplay[data >> 5]);
                arpMidiSpeedPending = data >> 5;
              } else { ledNumber(data >> 2); }
              if (!isMidi) {
                targetPot = 46;
                sendCC(number, data >> 1);
              }
              break;///arp rate
            case 5:
              if (isMidi) { data = data << 6; }
              fmBase[47] = data;
              updateFMifNecessary(47);
              dontShow[47] = isMidi;
              if (!isMidi) {
                targetPot = 47;
                sendCC(number, data >> 1);
              }
              break;//arp range

            case 48:
              fmBase[48] = data;
              updateFMifNecessary(48);
              dontShow[48] = isMidi;
              if ((vibratoClockEnable) && (sync)) { ledNumber(data >> 5); } else { ledNumber(data >> 3); }
              if (!isMidi) {
                targetPot = 48;
                sendCC(number, data >> 1);
              }
              break;//vibrato rate WAS 7
            case 13:
              fmBase[49] = data;
              updateFMifNecessary(49);
              dontShow[49] = isMidi;
              if (!isMidi) {
                targetPot = 49;
                sendCC(number, data >> 1);
              }
              break;//vibrato depth

          }
        }



        //update display
        if (targetPotLast != targetPot) {
          targetPotLast = targetPot;
          showLink();
          applyLfo();
        }
        if (selectedLfoLast != selectedLfo) {
          selectedLfoLast = selectedLfo;
          showLfo();
        }//show lfo because we moved to another

      }
    }
  }
}
