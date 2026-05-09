#include <Arduino.h>
#include <constants.h>
#include "YM2612.h"

void YM2612::setup(uint8_t ic_pin, uint8_t cs_pin, uint8_t wr_pin, uint8_t rd_pin, uint8_t a0_pin, uint8_t a1_pin,
                   uint8_t mc_pin, uint8_t data0_pin, uint8_t data1_pin, uint8_t data2_pin, uint8_t data3_pin,
                   uint8_t data4_pin, uint8_t data5_pin, uint8_t data6_pin, uint8_t data7_pin) {
	this->ic_pin = ic_pin;
	this->cs_pin = cs_pin;
	this->wr_pin = wr_pin;
	this->rd_pin = rd_pin;
	this->a0_pin = a0_pin;
	this->a1_pin = a1_pin;
	this->mc_pin = mc_pin;
	this->data0_pin = data0_pin;
	this->data1_pin = data1_pin;
	this->data2_pin = data2_pin;
	this->data3_pin = data3_pin;
	this->data4_pin = data4_pin;
	this->data5_pin = data5_pin;
	this->data6_pin = data6_pin;
	this->data7_pin = data7_pin;

	for (int i = 0; i < 6; i++) {
		selected_channel = i + 1;
		setStereo(3);
	}
	setDefaults();
}

void YM2612::setDefaults() {
	selected_channel = 1;
	operators[0] = true;
	operators[1] = true;
	operators[2] = true;
	operators[3] = true;

	for (int i = 0; i < 6; i++) {
		keyOff(i);
	}
	// channel params
	setFeedback(0);
	setAlgorithm(7);
	setStereo(3);
	setAMS(0);
	setFMS(7);
	// operator params
	setAmplitudeModulation(1);
	setAttackRate(31);
	setDecayRate(31);
	setSustainRate(0);
	setReleaseRate(8);
	setTotalLevel(0);
	setSustainLevel(0);
	setMultiply(2);
	setDetune(3);
}

void YM2612::sendData(uint8_t data) {
	if (chip) {
		PORTD &= ~_BV(chip); // digitalWrite(cs_pin, LOW);
	} else {
		PORTD &= ~_BV(2); // digitalWrite(cs_pin, LOW);
		PORTD &= ~_BV(6); // digitalWrite(cs_pin, LOW);
	}
	PORTB = data;

	delayMicroseconds(5); // it was 1 before
	PORTD &= ~_BV(3);     // digitalWrite(wr_pin, LOW);
	delayMicroseconds(5);
	PORTD |= _BV(3); // digitalWrite(wr_pin, HIGH);

	if (chip) {
		PORTD |= _BV(chip); // digitalWrite(cs_pin, HIGH);
	} else {
		PORTD |= _BV(2); // digitalWrite(cs_pin, HIGH);
		PORTD |= _BV(6); // digitalWrite(cs_pin, HIGH);
	}
}

void YM2612::setRegister(uint8_t part, uint8_t reg, uint8_t data) {
	if (part) {
		PORTC |= _BV(7); // digitalWrite(a1_pin, HIGH);
	} else {
		PORTC &= ~_BV(7); // digitalWrite(a1_pin, LOW);
	}
	PORTC &= ~_BV(4); // digitalWrite(a0_pin, LOW);
	sendData(reg);
	PORTC |= _BV(4); // digitalWrite(a0_pin, HIGH);
	sendData(data);
}

void YM2612::setMasterParameter(int reg_offset, int val_size, int val_shift, int val) {
	uint8_t* p = (uint8_t*)(&master) + reg_offset;

	*(p) &= ~(mask(val_size) << val_shift);        // clean
	*(p) |= ((mask(val_size) & val) << val_shift); // write
	setRegister(0, YM_MASTER_ADDR + reg_offset, *(p));
}

void YM2612::setChannelParameter(int reg_offset, int val_size, int val_shift, int val) {
	for (int i = 0; i < 6; i++)
		setChannelParameter(i, reg_offset, val_size, val_shift, val);
}

void YM2612::setChannelParameter(int chan, int reg_offset, int val_size, int val_shift, int val) {
	int channel_part = chan / 3;
	uint8_t* p = (uint8_t*)(&channels[channel_part]) + reg_offset;
	int channel_offset = chan % 3; // which of the 12 registers

	*(p + channel_offset) &= ~(mask(val_size) << val_shift);        // clean
	*(p + channel_offset) |= ((mask(val_size) & val) << val_shift); // write
	setRegister(channel_part, YM_CHN_ADDR + reg_offset + channel_offset, *(p + channel_offset));
}

void YM2612::setOperatorParameter(int reg_offset, int val_size, int val_shift, int val) {
	for (int i = 0; i < 6; i++)
		setOperatorParameter(i, reg_offset, val_size, val_shift, val);
}

void YM2612::setOperatorParameter(int chan, int reg_offset, int val_size, int val_shift, int val) {
	for (int i = 0; i < 4; i++)
		if (operators[i])
			setOperatorParameter(chan, i, reg_offset, val_size, val_shift, val);
}

void YM2612::setOperatorParameter(int chan, int oper, int reg_offset, int val_size, int val_shift, int val) {
	val = constrain(val, 0, 255);
	int channel_part = chan / 3;
	uint8_t* p = (uint8_t*)(&channels[channel_part]) + reg_offset;
	int op_offset = chan % 3 + oper * 4;                       // which of the 12 registers
	*(p + op_offset) &= ~(mask(val_size) << val_shift);        // clean
	*(p + op_offset) |= ((mask(val_size) & val) << val_shift); // write
	setRegister(channel_part, YM_CHN_ADDR + reg_offset + op_offset, *(p + op_offset));
}

void YM2612::noteOff(byte channel) {
	channel = stagger[channel];

	keyOff(channel);
}

void YM2612::noteOn(byte channel) {
	channel = stagger[channel];
	keyOn(channel);
}

void YM2612::setFrequency(uint8_t chan, float frequency) {

	chan = stagger[chan];

	int block = 2;
	uint16_t freq;
	if (frequency <= 22)
		frequency = 22;

	while (frequency >= 2048) {
		frequency /= 2;
		block++;
	}
	freq = (uint16_t)frequency;

	if (chan > 5) {
		chip = 6;
		chan -= 6;
	} else {
		chip = 2;
		frequency++;
	}

	setRegister(chan / 3, 0xA4 + (chan % 3), ((freq >> 8) & mask(3)) | ((block & mask(3)) << 3)); // Set frequency
	setRegister(chan / 3, 0xA0 + (chan % 3), freq);

	chip = 0;
}

void YM2612::setFrequency3(byte op, uint8_t chan, float frequency) {
	if (frequency <= 0)
		frequency = 1;
	int block = 2;
	uint16_t freq;
	while (frequency >= 2048) {
		frequency /= 2;
		block++;
	}
	freq = (uint16_t)frequency;

	if (chan > 5) {
		chip = 6;
		chan -= 6;
	} else {
		chip = 2;
	}

	/*
	If Channel 3 is in special mode:
	Operator 1's frequency is in A2 and A6
	Operator 2's frequency is in A8 and AC
	Operator 3's frequency is in A9 and AD
	Operator 4's frequency is in AA and AE
	*/

	switch (op) {
		case 0:
			setRegister(chan / 3, 0xA6, ((freq >> 8) & mask(3)) | ((block & mask(3)) << 3)); // Set frequency
			setRegister(chan / 3, 0xA2, freq);
			break;

		case 1:
			setRegister(chan / 3, 0xAC, ((freq >> 8) & mask(3)) | ((block & mask(3)) << 3)); // Set frequency
			setRegister(chan / 3, 0xA8, freq);
			break;

		case 2:
			setRegister(chan / 3, 0xAD, ((freq >> 8) & mask(3)) | ((block & mask(3)) << 3)); // Set frequency
			setRegister(chan / 3, 0xA9, freq);
			break;

		case 3:
			setRegister(chan / 3, 0xAE, ((freq >> 8) & mask(3)) | ((block & mask(3)) << 3)); // Set frequency
			setRegister(chan / 3, 0xAA, freq);
			break;
	}

	chip = 0;
}

void YM2612::setFrequencySingle(uint8_t chan, float frequency) { setFrequency(chan, frequency); }

void YM2612::keyOn(uint8_t chan) {
	chan = constrain(chan, 0, 11);
	if (chan > 5) {
		chip = 6;
		chan -= 6;
	} else {
		chip = 2;
	}
	setMasterParameter(YM_MA_OP_CHAN, 0xF0 + ((chan / 3) * 4 + chan % 3));
	chip = 0;
}

void YM2612::keyOff(uint8_t chan) {
	if (chan > 5) {
		chip = 6;
		chan -= 6;
	} else {
		chip = 2;
	}
	setMasterParameter(YM_MA_OP_CHAN, 0x00 + ((chan / 3) * 4 + chan % 3));
	chip = 0;
}

// void YM2612::update() {}
