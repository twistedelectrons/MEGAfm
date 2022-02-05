#include "megafm.h"
#include "midi.h"
#include "midiEngine.h"

byte mStatus;
byte mData;
byte mChannel;
int midiNoteOffset = -12;

void sendMegaChip(byte number, byte value) {
  if (kMegaChip) {
    if ((lastSentMega[0] == number) && (lastSentMega[1] == value)) {
      //don't repeat send!
    } else {
      lastSentMega[0] = number;
      lastSentMega[1] = value;

      Serial.write(123);
      Serial.write(number);
      Serial.write(value);
    }
  }
}

void sendControlChange(byte number, byte value, byte channel) {
  if (!thru) {
    lastSentCC[0] = number;
    lastSentCC[1] = value;

    if (!kMegaChip) {
      Serial.write(175 + channel);
      Serial.write(number);
      Serial.write(value);
    }
  }
}

void sendNoteOff(byte note, byte velocity, byte channel) {
  if (!thru) {
    if (!kMegaChip) {
      Serial.write(127 + channel);
      Serial.write(note);
      Serial.write(1);
    }
  }
}

void sendNoteOn(byte note, byte velocity, byte channel) {
  if (!thru) {
    if (!kMegaChip) {
      Serial.write(143 + channel);
      Serial.write(note);
      Serial.write(1);
    }
  }
}

void midiRead() {
  while (Serial.available()) {
    byte input = Serial.read();
    if (thru) {
      if (!kMegaChip) {
        Serial.write(input);
      }
    }

    if (input > 127) {
      // Status
      switch (input) {
        //@formatter:off
        case 248: handleClock(); break; //clock
        case 250: handleStart(); break; //start
        case 251:                break; //continue
        case 252: handleStop();  break; //stop

        case 128 ... 143: mChannel=input-127; mStatus=2; mData=255; break; //noteOff
        case 144 ... 159: mChannel=input-143; mStatus=1; mData=255; break; //noteOn
        case 176 ... 191: mChannel=input-175; mStatus=3; mData=255; break; //CC
        case 192 ... 207: mChannel=input-191; mStatus=6; mData=0;   break; //program Change
        case 208 ... 223: mChannel=input-207; mStatus=5; mData=0;   break; //Aftertouch
        case 224 ... 239: mChannel=input-223; mStatus=4; mData=255; break; //Pitch Bend

        default: mStatus=0; mData=255; break;
        //@formatter:on
      }
    } else {
      // Data
      if (mData == 255) { mData = input; } //data byte 1
      else {
        //data byte 2
        switch (mStatus) {
          //@formatter:off
          case 1:
            if(input) {
              handleNoteOn(mChannel,mData+midiNoteOffset,input);
            } else {
              handleNoteOff(mChannel,mData+midiNoteOffset,input);
            }
            mData=255;
            break;//noteOn
          case 2: handleNoteOff(mChannel,mData+midiNoteOffset,input); mData=255; break;
          case 3: HandleControlChange(mChannel,mData,input);          mData=255; break;
          case 4: handleBendy(mChannel,(input<<7)+mData-8192);        mData=255; break;
          case 5: handleAftertouch(mChannel,input);                   mData=255; break;
          case 6: handleProgramChange(mChannel,input);                mData=255; break;
          default: break;
          //@formatter:on
        }
      }
    }
  }
}
