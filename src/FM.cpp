#include <Arduino.h>
#include "FM.h"
#include "megafm.h"
#include "leds.h"        // showAlgo()
#include "pitchEngine.h" // setFat()

void setSSEG(byte op, bool index, bool value) {
	// operator bitIndex value
	bitWrite(SSEG[op], index, value);
	showSSEG();
	updateSSEG(op);
	showSSEGCounter = 24000; // show SSEG for longer because we're messing with it
}

void updateSSEG(byte op) {

	ym.selectOperator(0, 0);
	ym.selectOperator(1, 0);
	ym.selectOperator(2, 0);
	ym.selectOperator(3, 0);
	ym.selectOperator(op, 1);
	byte temp = 0;
	bitWrite(temp, 3, bitRead(SSEG[op], 1));  // set enable
	bitWrite(temp, 1, !bitRead(SSEG[op], 0)); // set waveform
	bitWrite(temp, 2, !bitRead(SSEG[op], 0)); // set waveform
	ym.setSSG_EG(temp);
	ym.setRateScaling(3);
}

void updateFine() {
	fineChanged = true;
	finey = fine;
	finey /= 255;
	finey += 1;
}

void updateFMifNecessary(byte number) {
	// if not assigned to LFO, update the engine
	if ((!linked[0][number]) && (!linked[1][number]) && (!linked[2][number])) {
		fmData[number] = fmBase[number];
	}
}

void fmResetValues() {
	for (int i = 0; i < 51; i++) {

		fmDataLast[i] = -1;
		updateFMifNecessary(i);
	}
}

void fmUpdate() {

	for (int i = 0; i < 51; i++) {

		if (fmDataLast[i] != fmData[i]) {
			fmDataLast[i] = fmData[i];
			fm(i, fmData[i]);
		}
	}
}

void fm(byte number, byte data) {
	switch (number) {
		// OP1
		case 0:
			op(0);
			ym.setDetune(detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 1:
			op(0);
			ym.setMultiply(data >> fmShifts[number]);
			break; // multiple
		case 2:
			op(0);
			ym.setTotalLevel(127 - (data >> fmShifts[number]));
			break; // op level
		case 3:
			op(0);
			ym.setRateScaling(data >> fmShifts[number]);
			break;
		case 4:
			op(0);
			ym.setAttackRate(31 - (data >> fmShifts[number]));
			break; // a
		case 5:
			op(0);
			ym.setDecayRate(31 - (data >> fmShifts[number]));
			break; // d
		case 6:
			op(0);
			ym.setSustainRate(31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 7:
			op(0);
			ym.setSustainLevel(15 - (data >> fmShifts[number]));
			break; // s
		case 8:
			op(0);
			ym.setReleaseRate(15 - (data >> fmShifts[number]));
			break; // r
		// OP2
		case 9:
			op(1);
			ym.setDetune(detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 10:
			op(1);
			ym.setMultiply(data >> fmShifts[number]);
			break; // multiple
		case 11:
			op(1);
			ym.setTotalLevel(127 - (data >> fmShifts[number]));
			break; // op level
		case 12:
			op(1);
			ym.setRateScaling((data) >> fmShifts[number]);
			break;
		case 13:
			op(1);
			ym.setAttackRate(31 - (data >> fmShifts[number]));
			break; // a
		case 14:
			op(1);
			ym.setDecayRate(31 - (data >> fmShifts[number]));
			break; // d
		case 15:
			op(1);
			ym.setSustainRate(31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 16:
			op(1);
			ym.setSustainLevel(15 - (data >> fmShifts[number]));
			break; // s
		case 17:
			op(1);
			ym.setReleaseRate(15 - (data >> fmShifts[number]));
			break; // r
		// OP3
		case 18:
			op(2);
			ym.setDetune(detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 19:
			op(2);
			ym.setMultiply(data >> fmShifts[number]);
			break; // multiple
		case 20:
			op(2);
			ym.setTotalLevel(127 - (data >> fmShifts[number]));
			break; // op level
		case 21:
			op(2);
			ym.setRateScaling(data >> fmShifts[number]);
			break;
		case 22:
			op(2);
			ym.setAttackRate(31 - (data >> fmShifts[number]));
			break; // a
		case 23:
			op(2);
			ym.setDecayRate(31 - (data >> fmShifts[number]));
			break; // d
		case 24:
			op(2);
			ym.setSustainRate(31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 25:
			op(2);
			ym.setSustainLevel(15 - (data >> fmShifts[number]));
			break; // s
		case 26:
			op(2);
			ym.setReleaseRate(15 - (data >> fmShifts[number]));
			break; // r
		// OP4
		case 27:
			op(3);
			ym.setDetune(detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 28:
			op(3);
			ym.setMultiply(data >> fmShifts[number]);
			break; // multiple
		case 29:
			op(3);
			ym.setTotalLevel(127 - (data >> fmShifts[number]));
			break; // op level
		case 30:
			op(3);
			ym.setRateScaling(data >> fmShifts[number]);
			break;
		case 31:
			op(3);
			ym.setAttackRate(31 - (data >> fmShifts[number]));
			break; // a
		case 32:
			op(3);
			ym.setDecayRate(31 - (data >> fmShifts[number]));
			break; // d
		case 33:
			op(3);
			ym.setSustainRate(31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 34:
			op(3);
			ym.setSustainLevel(15 - (data >> fmShifts[number]));
			break; // s
		case 35:
			op(3);
			ym.setReleaseRate(15 - (data >> fmShifts[number]));
			break; // r

		case 36:
			lfoClockSpeedPending[0] = map(data, 0, 255, 0, 11);
			break;
		case 38:
			lfoClockSpeedPending[1] = map(data, 0, 255, 0, 11);
			break;
		case 40:
			lfoClockSpeedPending[2] = map(data, 0, 255, 0, 11);
			break;

		case 42:
			if (algoLast != data) {
				algoLast = data;
				ym.setAlgorithm(data >> 5);
				showAlgo(data >> 5);
			}
			break; // algo
		case 43:
			ym.setFeedback(data >> 5);
			break; // feedback
		case 50:
			setFat(data);
			break; // fat
	}
}

void op(byte number) {

	ym.selectOperator(0, 0);
	ym.selectOperator(1, 0);
	ym.selectOperator(2, 0);
	ym.selectOperator(3, 0);
	ym.selectOperator(number, 1);
}

#define mask(s) (~(~0 << s))

void setupFM() {

	pinMode(14, OUTPUT); // CS2

	DDRD |= _BV(7); //  CLOCK
	DDRB = 255;     // data port

	// Set Timer 2 CTC mode with no prescaling.  OC2A toggles on compare match
	//
	// WGM22:0 = 010: CTC Mode, toggle OC
	// WGM2 bits 1 and 0 are in TCCR2A,
	// WGM2 bit 2 is in TCCR2B
	// COM2A0 sets OC2A (arduino pin 11 on Uno or Duemilanove) to toggle on compare match
	//
	TCCR2A = ((1 << WGM21) | (1 << COM2A0));

	// Set Timer 2  No prescaling  (i.e. prescale division = 1)
	//
	// CS22:0 = 001: Use CPU clock with no prescaling
	// CS2 bits 2:0 are all in TCCR2B
	TCCR2B = (1 << CS20);

	// Make sure Compare-match register A interrupt for timer2 is disabled
	TIMSK2 = 0;
	// This value determines the output frequency
	OCR2A = 0;

	pinMode(YM_A0, OUTPUT);
	pinMode(YM_A1, OUTPUT);
	pinMode(YM_CS, OUTPUT);
	pinMode(YM_WR, OUTPUT);
	pinMode(YM_IC, OUTPUT);

	digitalWrite(YM_A1, 0);
	digitalWrite(YM_CS, 1);
	digitalWrite(YM_WR, 1);
	digitalWrite(YM_IC, 1);

	/* Reset YM2612 */
	digitalWrite(YM_IC, 0);
	delay(10);
	digitalWrite(YM_IC, 1);
	delay(10);
}

/* transfer one byte to data lines */
/* This code can be optimized later */
// TODO: unused
// void WriteYMData(byte data) { PORTB = data; }
//
// float noteToFrequency(uint8_t note) {
//  note += 5;
//  static float freq[] = {261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f,
//                         466.16f, 493.88f,};
//  static float multiplier[] = {0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f,};
//
//  /*
//    static float note2freq[] = {
//   8.18f,     8.66f,     9.18f,     9.72f,    10.30f,    10.91f,    11.56f,    12.25f,    12.98f,    13.75f,    14.57f,
//   15.43f, 16.35f,    17.32f,    18.35f,    19.45f,    20.60f,    21.83f,    23.12f,    24.50f,    25.96f,    27.50f,
//   29.14f,    30.87f, 32.70f,    34.65f,    36.71f,    38.89f,    41.20f,    43.65f,    46.25f,    49.00f,    51.91f,
//   55.00f,    58.27f,    61.74f, 65.41f,    69.30f,    73.42f,    77.78f,    82.41f,    87.31f,    92.50f,    98.00f,
//   103.83f,   110.00f,   116.54f,   123.47f, 130.81f,   138.59f,   146.83f,   155.56f,   164.81f,   174.61f, 185.00f,
//   196.00f,   207.65f,   220.00f,   233.08f,   246.94f, 261.63f,   277.18f,   293.66f,   311.13f,   329.63f, 349.23f,
//   369.99f,   392.00f,   415.30f,   440.00f,   466.16f,   493.88f, 523.25f,   554.37f,   587.33f,   622.25f, 659.26f,
//   698.46f,   739.99f,   783.99f,   830.61f,   880.00f,   932.33f,   987.77f, 1046.50f,  1108.73f,  1174.66f,
//   1244.51f,  1318.51f,  1396.91f,  1479.98f,  1567.98f,  1661.22f,  1760.00f,  1864.66f,  1975.53f, 2093.00f,
//   2217.46f,  2349.32f,  2489.02f,  2637.02f,  2793.83f,  2959.96f,  3135.96f,  3322.44f,  3520.00f,  3729.31f,
//   3951.07f, 4186.01f,  4434.92f,  4698.64f,  4978.03f,  5274.04f,  5587.65f,  5919.91f,  6271.93f,  6644.88f,
//   7040.00f,  7458.62f,  7902.13f, 8372.02f,  8869.84f,  9397.27f,  9956.06f, 10548.08f, 11175.30f, 11839.82f,
//   12543.85f
//   };
//   */
//  return (freq[note % 12] * multiplier[note / 12]) * 1.015;
//}
//
// void setFrequency(byte chan, float frequency) {
//  int block = 2;
//  uint16_t freq;
//  while (frequency >= 2048) {
//    frequency /= 2;
//    block++;
//  }
//  freq = (uint16_t) frequency;
//}
