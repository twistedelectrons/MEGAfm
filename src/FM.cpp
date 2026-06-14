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

	// scale and offset finer tuning
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
			op(2);
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
			op(1);
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
			lfoClockSpeedPending[0] = map(data, 0, 255, 0, 12);
			break;
		case 38:
			lfoClockSpeedPending[1] = map(data, 0, 255, 0, 12);
			break;
		case 40:
			lfoClockSpeedPending[2] = map(data, 0, 255, 0, 12);
			break;

		case 42:
			if (algoLast != data) {
				algoLast = data;
				ym.setAlgorithm(data >> fmShifts[number]);
				showAlgo(data >> fmShifts[number]);
			}
			break; // algo
		case 43:
			ym.setFeedback(data >> fmShifts[number]);
			break; // feedback
		case 50:
			setFat(data);
			break; // fat
	}
}

void fmMpe(byte channel, byte number, byte data) {
	switch (number) {
		// OP1
		case 0:
			op(0);
			ym.setDetune(channel, detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 1:
			op(0);
			ym.setMultiply(channel, data >> fmShifts[number]);
			break; // multiple
		case 2:
			op(0);
			ym.setTotalLevel(channel, 127 - (data >> fmShifts[number]));
			break; // op level
		case 3:
			op(0);
			ym.setRateScaling(channel, data >> fmShifts[number]);
			break;
		case 4:
			op(0);
			ym.setAttackRate(channel, 31 - (data >> fmShifts[number]));
			break; // a
		case 5:
			op(0);
			ym.setDecayRate(channel, 31 - (data >> fmShifts[number]));
			break; // d
		case 6:
			op(0);
			ym.setSustainRate(channel, 31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 7:
			op(0);
			ym.setSustainLevel(channel, 15 - (data >> fmShifts[number]));
			break; // s
		case 8:
			op(0);
			ym.setReleaseRate(channel, 15 - (data >> fmShifts[number]));
			break; // r
		// OP2
		case 9:
			op(1);
			ym.setDetune(channel, detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 10:
			op(1);
			ym.setMultiply(channel, data >> fmShifts[number]);
			break; // multiple
		case 11:
			op(1);
			ym.setTotalLevel(channel, 127 - (data >> fmShifts[number]));
			break; // op level
		case 12:
			op(2);
			ym.setRateScaling(channel, (data) >> fmShifts[number]);
			break;
		case 13:
			op(1);
			ym.setAttackRate(channel, 31 - (data >> fmShifts[number]));
			break; // a
		case 14:
			op(1);
			ym.setDecayRate(channel, 31 - (data >> fmShifts[number]));
			break; // d
		case 15:
			op(1);
			ym.setSustainRate(channel, 31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 16:
			op(1);
			ym.setSustainLevel(channel, 15 - (data >> fmShifts[number]));
			break; // s
		case 17:
			op(1);
			ym.setReleaseRate(channel, 15 - (data >> fmShifts[number]));
			break; // r
		// OP3
		case 18:
			op(2);
			ym.setDetune(channel, detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 19:
			op(2);
			ym.setMultiply(channel, data >> fmShifts[number]);
			break; // multiple
		case 20:
			op(2);
			ym.setTotalLevel(channel, 127 - (data >> fmShifts[number]));
			break; // op level
		case 21:
			op(1);
			ym.setRateScaling(channel, data >> fmShifts[number]);
			break;
		case 22:
			op(2);
			ym.setAttackRate(channel, 31 - (data >> fmShifts[number]));
			break; // a
		case 23:
			op(2);
			ym.setDecayRate(channel, 31 - (data >> fmShifts[number]));
			break; // d
		case 24:
			op(2);
			ym.setSustainRate(channel, 31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 25:
			op(2);
			ym.setSustainLevel(channel, 15 - (data >> fmShifts[number]));
			break; // s
		case 26:
			op(2);
			ym.setReleaseRate(channel, 15 - (data >> fmShifts[number]));
			break; // r
		// OP4
		case 27:
			op(3);
			ym.setDetune(channel, detuneFix[data] >> fmShifts[number]);
			break; // detune
		case 28:
			op(3);
			ym.setMultiply(channel, data >> fmShifts[number]);
			break; // multiple
		case 29:
			op(3);
			ym.setTotalLevel(channel, 127 - (data >> fmShifts[number]));
			break; // op level
		case 30:
			op(3);
			ym.setRateScaling(channel, data >> fmShifts[number]);
			break;
		case 31:
			op(3);
			ym.setAttackRate(channel, 31 - (data >> fmShifts[number]));
			break; // a
		case 32:
			op(3);
			ym.setDecayRate(channel, 31 - (data >> fmShifts[number]));
			break; // d
		case 33:
			op(3);
			ym.setSustainRate(channel, 31 - (data >> fmShifts[number]));
			break; // sustain rate
		case 34:
			op(3);
			ym.setSustainLevel(channel, 15 - (data >> fmShifts[number]));
			break; // s
		case 35:
			op(3);
			ym.setReleaseRate(channel, 15 - (data >> fmShifts[number]));
			break; // r
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
