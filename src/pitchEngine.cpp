#include "megafm.h"
#include "pitchEngine.h"

float innerNoteToFreq(float note) {
	static float baseFreq = 440.f / 32.f;
	return baseFreq * float(pow(2, ((note - 9) / 12)));
}

float noteToFrequencyFloat(float note) {
	if (note <= 0) {
		note = 1;
	}
	note += 5;

	return innerNoteToFreq(note + bendy);
}

float noteToFrequency(int note) { // FIXME delete if possible?
	if (note <= 0) {
		note = 1;
	}
	note += 5;

	return innerNoteToFreq(note + bendy);
}

float noteToFrequencyMpe(int note, int channel) {
	if (note <= 0) {
		note = 1;
	}
	note += 5;

	if (bendy == 666666) {
		static float freq[] = {
		    261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f, 466.16f, 493.88f,
		};
		static float multiplier[] = {
		    0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f,
		};

		return (freq[note % 12] * multiplier[note / 12]);
	} else {
		return innerNoteToFreq(note + mpeBend[channel]);
	}
}

/*
float destiFreq[12];
float freq[12];
float freqTotal[12];//freq + vib *bend
float freqLast[12];
*/

void updatePitch() {

	if (voiceMode == kVoicingDualCh3) {

		ch3x[0] = fmData[27];
		ch3x[1] = fmData[9];
		ch3x[2] = fmData[0];
		ch3x[3] = fmData[18];

		for (int i = 0; i < 4; i++) {
			ch3x[i] /= 127; // convert to 0-1
			ch3x[i] += .2;
		}

		// ONLY VOICE 3

		byte i = 2; // voice 3 chip 1
		// glide
		if (freq[i] < destiFreq[i]) {
			freq[i] += glideIncrement[i];
			if (freq[i] >= destiFreq[i]) {
				freq[i] = destiFreq[i];
			}
		} else if (freq[i] > destiFreq[i]) {
			freq[i] -= glideIncrement[i];
			if (freq[i] <= destiFreq[i]) {
				freq[i] = destiFreq[i];
			}
		}

		if ((fmData[48]) && (fmData[49])) {
			freqTotal[i] = (freq[i] + vibPitch) * finey;
		} else {
			freqTotal[i] = (freq[i]) * finey;
		} // apply vib and bend

		ym.setFrequency3(0, i, freqTotal[i] * ch3x[0]); // offset operator1 by fmData[0](detune1)
		ym.setFrequency3(1, i, freqTotal[i] * ch3x[1]); // offset operator2 by fmData[9](detune2)
		ym.setFrequency3(2, i, freqTotal[i] * ch3x[2]); // offset operator3 by fmData[18](detune3)
		ym.setFrequency3(3, i, freqTotal[i] * ch3x[3]); // offset operator4 by fmData[27](detune4)

		i = 8; // voice 3 chip2
		// glide
		if (freq[i] < destiFreq[i]) {
			freq[i] += glideIncrement[i];
			if (freq[i] >= destiFreq[i]) {
				freq[i] = destiFreq[i];
			}
		} else if (freq[i] > destiFreq[i]) {
			freq[i] -= glideIncrement[i];
			if (freq[i] <= destiFreq[i]) {
				freq[i] = destiFreq[i];
			}
		}

		if ((fmData[48]) && (fmData[49])) {
			freqTotal[i] = (freq[i] + vibPitch) * finey;
		} else {
			freqTotal[i] = (freq[i]) * finey;
		} // apply vib and bend

		ym.setFrequency3(0, i, freqTotal[i] * ch3x[0]); // offset operator1 by fmData[0](detune1)
		ym.setFrequency3(1, i, freqTotal[i] * ch3x[1]); // offset operator2 by fmData[9](detune2)
		ym.setFrequency3(2, i, freqTotal[i] * ch3x[2]); // offset operator3 by fmData[18](detune3)
		ym.setFrequency3(3, i, freqTotal[i] * ch3x[3]); // offset operator4 by fmData[27](detune4)

	} else {

		// ALL VOICES
		for (byte i = 0; i < 12; i++) {

			// glide
			if (freq[i] < destiFreq[i]) {
				freq[i] += glideIncrement[i];
				if (freq[i] >= destiFreq[i]) {
					freq[i] = destiFreq[i];
				}
			} else if (freq[i] > destiFreq[i]) {
				freq[i] -= glideIncrement[i];
				if (freq[i] <= destiFreq[i]) {
					freq[i] = destiFreq[i];
				}
			}

			// Apply vibrato and tuning
			if (fmData[48] && fmData[49]) {
				freqTotal[i] = (freq[i] + vibPitch) * finey;
			} else {
				freqTotal[i] = freq[i] * finey;
			}

			// Send
			if (freqTotal[i] != freqLast[i]) {
				freqLast[i] = freqTotal[i];
				ym.setFrequencySingle(i, freqTotal[i]);
			}
		}
	}
}

void setNote(uint8_t channel, uint8_t note) {
	notey[channel] = note;

	float detune;
	if (fatSpreadMode == FAT_SPREAD_MODE_1DOWN2UP) {
		detune = (channel % 2 == 0) ? fat : -fat;
	} else {
		detune = (channel < 6) ? -fat : fat;
	}

	switch (voiceMode) {
		case kVoicingWide4:
			if (channel % 3 == 0)
				detune = 0;
			break;
		case kVoicingWide3:
			if (channel % 4 >= 2)
				detune /= 2.f;
			break;
		case kVoicingWide6:
		case kVoicingPoly12:
		case kVoicingDualCh3:
		case kVoicingUnison:
			break;
	}

	if (mpe) {
		destiFreq[channel] = (noteToFrequencyMpe(notey[channel], channel));
		freq[channel] = destiFreq[channel];
	} else {
		if (fat < .005) {
			destiFreq[channel] = (noteToFrequency(notey[channel]));
		} else {
			if (voiceMode == kVoicingUnison) {
				// Here is where the unison spread "THX" chord is implemented.
				// FIXME: why the 0.01?
				// FIXME: bug-to-bug compat mode?
				const int thx_chord[] = {-20, -17, -12, -8, -5, 0, 0, 4, 7, 12, 16, 19};
				destiFreq[channel] = noteToFrequencyFloat(notey[channel] + thx_chord[channel] * (fat - 0.01f));
			} else {
				if (fatMode == FAT_MODE_OCTAVE) {
					if (detune >= 0)
						destiFreq[channel] = noteToFrequency(notey[channel]) * (1 + detune);
					else
						destiFreq[channel] = noteToFrequency(notey[channel]) / (1 - detune);
				} else {
					if (detune >= 0)
						destiFreq[channel] =
						    noteToFrequency(notey[channel]) +
						    (((noteToFrequency(notey[channel] + 1) - noteToFrequency(notey[channel]))) * detune);
					else
						destiFreq[channel] =
						    noteToFrequency(notey[channel]) -
						    (((noteToFrequency(notey[channel]) - noteToFrequency(notey[channel] - 1))) * (-detune));
				}
			}
		}
		glideIncrement[channel] = calculateIncrement(freq[channel], destiFreq[channel]);
	}
}

void skipGlide(uint8_t channel) { freq[channel] = destiFreq[channel]; }

void updateGlideIncrements() {
	for (int i = 0; i < 12; i++) {
		if (mpe) {
			freq[i] = destiFreq[i];
		} else {
			glideIncrement[i] = calculateIncrement(freq[i], destiFreq[i]);
		}
	}
}

float calculateIncrement(float present, float future) {
	if (present < future) {
		return ((future - present) / (glide << 4));
	} else if (present > future) {
		return ((present - future) / (glide << 4));
	} else {
		return 0;
	}
}

void setFat(int number) {
	// TODO(montag): missing 'else'? This will have no effect since 'fat' gets assigned again below.
	if (mpe) {
		fat = 0;
	}
	if (fatLast != number) {
		fatLast = number;
		if (number < 64) {
			fat = number;
			fat /= 512;
		} else {
			fat = float(map(number, 64, 255, 64, 512));
			fat /= 512;
		}

		for (int i = 0; i < 12; i++) {
			setNote(i, notey[i]); // update destiFreqs
		}
	}
}
