#pragma once
#include <stdint.h>

typedef void (*note_on_callback_t)(uint8_t, uint8_t, uint8_t);
typedef void (*note_off_callback_t)(uint8_t, uint8_t);

/** Acts very similar to a bool[8], but differs from it in two things:
 * Firstly, it stores the bools more efficiently.
 * Secondly, you have to use my_array.get(4) and .set(4, true) instead of myarray[4].
 */
struct BooleanArray8 {
		BooleanArray8() : value(0) {} // initialize value to 0 in constructor
		void set(uint8_t index, bool newValue) { bitWrite(value, index, newValue); }
		bool get(uint8_t index) { return value & (1 << index); }

	private:
		uint8_t value = 0;
};

/** Acts very similar to a bool[128], but differs from it in two things:
 * Firstly, it stores the bools more efficiently.
 * Secondly, you have to use my_array.get(42) and .set(42, true) instead of myarray[42].
 */
struct BooleanArray128 {
		BooleanArray128()
		    : subarray{} {} // initialize subarray to empty array of BooleanArray8 with value initialized to 0

		void set(uint8_t index, bool value) {
			uint8_t subarray_index = index / 8;
			uint8_t bit_in_subarray = index % 8;
			subarray[subarray_index].set(bit_in_subarray, value);
		}
		bool get(uint8_t index) {
			uint8_t subarray_index = index / 8;
			uint8_t bit_in_subarray = index % 8;
			return subarray[subarray_index].get(bit_in_subarray);
		}

	private:
		BooleanArray8 subarray[16]; // 16 * 8 = 128.
};

class MidiPedalAdapter {
	public:
		MidiPedalAdapter(note_on_callback_t note_on_callback, note_off_callback_t note_off_callback)
		    : pedal_down{0}, note_on_callback(note_on_callback), note_off_callback(note_off_callback) {}

		void set_pedal(uint8_t channel, bool pedal) {

			this->pedal_down[channel] = pedal;

			// when pedal goes down, we want to transfer the notes that are held to sustained
			if (pedal) {
				for (int i = 0; i < 128; i++) {
					sustainedNotes[channel].set(i, heldNotes[channel].get(i));
				}
			} else {

				// when pedal goes up, we want to kill the notes that are sustained
				for (int i = 0; i < 128; i++) {
					if (!heldNotes[channel].get(i) && sustainedNotes[channel].get(i)) {
						//*(only if not held by fingers)
						note_off_callback(channel, i);
					}
				}

				// clear sustained notes
				for (int i = 0; i < 128; i++) {
					sustainedNotes[channel].set(i, 0);
				}
			}
		}

		void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
			if (velocity == 0) {
				note_off(channel, note);
				return;
			}

			// is the note already singing? Retrigger
			if (sustainedNotes[channel].get(note)) {

				note_off_callback(channel, note);
			}

			// trigger the note
			note_on_callback(channel, note, velocity);

			// add note to held array
			heldNotes[channel].set(note, true);

			if (pedal_down[channel]) {
				// pedal is down so we want to sustain the note
				sustainedNotes[channel].set(note, true);
			}
		}

		void note_off(uint8_t channel, uint8_t note) {

			// remove note from held array
			heldNotes[channel].set(note, 0);

			if (!pedal_down[channel]) {
				note_off_callback(channel, note);
			}
		}

	private:
		BooleanArray128 heldNotes[16];
		BooleanArray128 sustainedNotes[16];
		bool pedal_down[16];

		note_on_callback_t note_on_callback;
		note_off_callback_t note_off_callback;
};
