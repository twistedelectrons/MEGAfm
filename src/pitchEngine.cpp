#include "megafm.h"
#include "pitchEngine.h"

float noteToFrequency(int note) {
  if (note <= 0)note = 1;
  note += 5;

  float tempF1 = note + bendy;
  float tempF2 = (440 / 32) * (pow(2, ((tempF1 - 9) / 12)));

  return (tempF2);
}

float noteToFrequencyMpe(int note, int channel) {
  if (note <= 0)note = 1;
  note += 5;

  if (bendy == 666666) {

    static float freq[] = {261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f,
                           466.16f, 493.88f,};
    static float multiplier[] = {0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f,};

    return (freq[note % 12] * multiplier[note / 12]);
  } else {
    float tempF1 = note + mpeBend[channel];
    float tempF2 = (440 / 32) * (pow(2, ((tempF1 - 9) / 12)));

    return (tempF2);
  }
}

/*
float destiFreq[12];
float freq[12];
float freqTotal[12];//freq + vib *bend
float freqLast[12];
*/

void updatePitch() {

  if (voiceMode == 2) {

    ch3x[0] = fmData[27];
    ch3x[1] = fmData[9];
    ch3x[2] = fmData[0];
    ch3x[3] = fmData[18];

    for (int i = 0; i < 4; i++) {
      ch3x[i] /= 127;//convert to 0-1
      ch3x[i] += .2;
    }

    //ONLY VOICE 3

    byte i = 2;//voice 3 chip 1
    //glide
    if (freq[i] < destiFreq[i]) {
      freq[i] += glideIncrement[i];
      if (freq[i] >= destiFreq[i]) { freq[i] = destiFreq[i]; }
    } else if (freq[i] > destiFreq[i]) {
      freq[i] -= glideIncrement[i];
      if (freq[i] <= destiFreq[i]) { freq[i] = destiFreq[i]; }
    }

    if ((fmData[48]) && (fmData[49])) { freqTotal[i] = (freq[i] + vibPitch) * finey; }
    else {
      freqTotal[i] = (freq[i]) * finey;
    }//apply vib and bend

    ym.setFrequency3(0, i, freqTotal[i] * ch3x[0]);  //offset operator1 by fmData[0](detune1)
    ym.setFrequency3(1, i, freqTotal[i] * ch3x[1]);  //offset operator2 by fmData[9](detune2)
    ym.setFrequency3(2, i, freqTotal[i] * ch3x[2]);  //offset operator3 by fmData[18](detune3)
    ym.setFrequency3(3, i, freqTotal[i] * ch3x[3]);  //offset operator4 by fmData[27](detune4)

    i = 8;//voice 3 chip2
    //glide
    if (freq[i] < destiFreq[i]) {
      freq[i] += glideIncrement[i];
      if (freq[i] >= destiFreq[i]) { freq[i] = destiFreq[i]; }
    } else if (freq[i] > destiFreq[i]) {
      freq[i] -= glideIncrement[i];
      if (freq[i] <= destiFreq[i]) { freq[i] = destiFreq[i]; }
    }

    if ((fmData[48]) && (fmData[49])) { freqTotal[i] = (freq[i] + vibPitch) * finey; }
    else {
      freqTotal[i] = (freq[i]) * finey;
    }//apply vib and bend

    ym.setFrequency3(0, i, freqTotal[i] * ch3x[0]);  //offset operator1 by fmData[0](detune1)
    ym.setFrequency3(1, i, freqTotal[i] * ch3x[1]);  //offset operator2 by fmData[9](detune2)
    ym.setFrequency3(2, i, freqTotal[i] * ch3x[2]);  //offset operator3 by fmData[18](detune3)
    ym.setFrequency3(3, i, freqTotal[i] * ch3x[3]);  //offset operator4 by fmData[27](detune4)

  } else {

    //ALL VOICES
    for (byte i = 0; i < 12; i++) {

      //glide
      if (freq[i] < destiFreq[i]) {
        freq[i] += glideIncrement[i];
        if (freq[i] >= destiFreq[i]) { freq[i] = destiFreq[i]; }
      } else if (freq[i] > destiFreq[i]) {
        freq[i] -= glideIncrement[i];
        if (freq[i] <= destiFreq[i]) { freq[i] = destiFreq[i]; }
      }

      if ((fmData[48]) && (fmData[49])) { freqTotal[i] = (freq[i] + vibPitch) * finey; }
      else {
        freqTotal[i] = (freq[i]) * finey;
      }//apply vib and bend


      if (freqTotal[i] != freqLast[i]) {
        freqLast[i] = freqTotal[i];
        ym.setFrequencySingle(i, freqTotal[i]);
      }//send

    }
  }
}

void setNote(uint8_t channel, uint8_t note) {
  if (mpe) {
    notey[channel] = note;
    destiFreq[channel] = (noteToFrequencyMpe(notey[channel], channel));
    freq[channel] = destiFreq[channel];
  } else {
    notey[channel] = note;

    if (fat < .005) { destiFreq[channel] = (noteToFrequency(notey[channel])); }
    else {
      if (voiceMode == 3) {
        switch (channel) {
          case 0:
            destiFreq[channel] = noteToFrequency(notey[channel]);
            break;
          case 2:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) +
                                   (noteToFrequency(notey[channel] + 12)) * (fat - .01)));
            break;
          case 4:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) +
                                   (noteToFrequency(notey[channel] + 4)) * (fat - .01)));
            break;
          case 6:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) +
                                   (noteToFrequency(notey[channel] + 7)) * (fat - .01)));
            break;
          case 8:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) +
                                   (noteToFrequency(notey[channel] + 16)) * (fat - .01)));
            break;
          case 10:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) +
                                   (noteToFrequency(notey[channel] + 19)) * (fat - .01)));
            break;

          case 1:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) -
                                   (noteToFrequency(notey[channel]) / 2) * (fat - .01)));
            break;
          case 3:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) -
                                   (noteToFrequency(notey[channel] + 12) / 2) * (fat - .01)));
            break;
          case 5:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) -
                                   (noteToFrequency(notey[channel] + 4) / 2) * (fat - .01)));
            break;
          case 7:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) -
                                   (noteToFrequency(notey[channel] + 7) / 2) * (fat - .01)));
            break;
          case 9:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) -
                                   (noteToFrequency(notey[channel] + 16) / 2) * (fat - .01)));
            break;
          case 11:
            destiFreq[channel] = ((noteToFrequency(notey[channel]) -
                                   (noteToFrequency(notey[channel] + 19) / 2) * (fat - .01)));
            break;
        }
      } else if (voiceMode == 0) {
        if ((fatMode) && (!fatSpreadMode)) {
          //chip1 down chip2 up
          switch (channel) {
            default:
              destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + fat);
              break;

            case 1:
            case 3:
            case 5:
            case 7:
            case 9:
            case 11:
              destiFreq[channel] = noteToFrequency(notey[channel]) / (1 + fat);
              break;
          }
        } else if ((fatMode) && (fatSpreadMode)) {
          //both up and down (mixed)
          switch (channel) {

            default:
              destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + fat);
              break;

            case 0 ... 5:
              destiFreq[channel] = noteToFrequency(notey[channel]) / (1 + fat);
              break;

          }
        } else {

          if (!fatSpreadMode) {
            switch (channel) {
              //1 semi
              default:
                destiFreq[channel] = noteToFrequency(notey[channel]) +
                                     (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) *
                                      (fat - .005));
                break;
              case 1:
              case 3:
              case 5:
              case 7:
              case 9:
              case 11:
                destiFreq[channel] = noteToFrequency(notey[channel]) -
                                     (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) *
                                      (fat - .005));
                break;
            }
          } else {
            switch (channel) {
              //1 semi
              default:
                destiFreq[channel] = noteToFrequency(notey[channel]) +
                                     (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) *
                                      (fat - .005));
                break;
              case 0 ... 5:
                destiFreq[channel] = noteToFrequency(notey[channel]) -
                                     (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) *
                                      (fat - .005));
                break;
            }
          }
        }
      } else if (voiceMode == 1) {
        if ((fatMode) && (!fatSpreadMode)) {
          switch (channel) {
            default:
              destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + fat);
              break;
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
              destiFreq[channel] = noteToFrequency(notey[channel]) / (1 + fat);
              break;
          }
        } else if ((fatMode) && (fatSpreadMode)) {
          switch (channel) {
            default:
              destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + fat);
              break;
            case 0:
            case 2:
            case 4:
            case 6:
            case 8:
            case 10:
              destiFreq[channel] = noteToFrequency(notey[channel]) / (1 + fat);
              break;
          }
        } else {
          if (!fatSpreadMode) {
            switch (channel) {
              //1 semi
              default:
                destiFreq[channel] = noteToFrequency(notey[channel]) +
                                     (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) *
                                      (fat - .005));
                break;
              case 6:
              case 7:
              case 8:
              case 9:
              case 10:
              case 11:
                destiFreq[channel] = noteToFrequency(notey[channel]) -
                                     (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) *
                                      (fat - .005));
                break;
            }
          } else if (fatSpreadMode) {
            switch (channel) {
              //1 semi
              default:
                destiFreq[channel] = noteToFrequency(notey[channel]) +
                                     (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) *
                                      (fat - .005));
                break;
              case 0:
              case 2:
              case 4:
              case 6:
              case 8:
              case 10:
                destiFreq[channel] = noteToFrequency(notey[channel]) -
                                     (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) *
                                      (fat - .005));
                break;
            }
          }
        }
      } else {
        //voicemides 2 and 3
        if ((fatMode) && (!fatSpreadMode)) {
          switch (channel) {

            default:
              destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + fat);
              break;

            case 1:
            case 3:
            case 5:
            case 7:
            case 9:
            case 11:
              destiFreq[channel] = noteToFrequency(notey[channel]) / (1 + fat);
              break;

          }
        } else if ((fatMode) && (fatSpreadMode)) {
          switch (channel) {

            default:
              destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + fat);
              break;

            case 0 ... 5:
              destiFreq[channel] = noteToFrequency(notey[channel]) / (1 + fat);
              break;

          }
        } else {
          if (fatSpreadMode) {
            switch (channel) {
              //1 semi
              default:
                destiFreq[channel] = noteToFrequency(notey[channel]) +
                                     (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) *
                                      (fat - .005));
                break;
              case 1:
              case 3:
              case 5:
              case 7:
              case 9:
              case 11:
                destiFreq[channel] = noteToFrequency(notey[channel]) -
                                     (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) *
                                      (fat - .005));
                break;
            }
          } else if (!fatSpreadMode) {
            switch (channel) {
              //1 semi
              default:
                destiFreq[channel] = noteToFrequency(notey[channel]) +
                                     (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) *
                                      (fat - .005));
                break;
              case 0 ... 5:
                destiFreq[channel] = noteToFrequency(notey[channel]) -
                                     (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) *
                                      (fat - .005));
                break;
            }
          }
        }
      }
    }

    glideIncrement[channel] = calculateIncrement(freq[channel], destiFreq[channel]);
  }
}

void skipGlide(uint8_t channel) {
  freq[channel] = destiFreq[channel];
}

void updateGlideIncrements() {
  for (int i = 0; i < 12; i++) {
    if (mpe) { freq[i] = destiFreq[i]; }
    else {
      glideIncrement[i] = calculateIncrement(freq[i], destiFreq[i]);
    }
  }
}

float calculateIncrement(float present, float future) {
  if (present < future) {
    return ((future - present) / (glide << 4));
  } else if (present > future) { return ((present - future) / (glide << 4)); } else { return (0); }
}

void setFat(int number) {
  if (mpe)fat = 0;
  if (fatLast != number) {
    fatLast = number;
    if (number < 64) {
      fat = number;
      fat /= 512;
    } else {
      fat = map(number, 64, 255, 64, 512);
      fat /= 512;
    }

    for (int i = 0; i < 12; i++) {
      setNote(i, notey[i]);//update destiFreqs
    }
  }
}
