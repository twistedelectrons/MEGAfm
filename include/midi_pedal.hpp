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
		    : pedal_down{0}, pedal_downLast{0}, note_on_callback(note_on_callback),
		      note_off_callback(note_off_callback) {}

		// Public MIDI channels and callbacks are 1-based; storage is 0-based.
		void set_pedal(uint8_t channel, bool pedal) {
			if (channel < 1 || channel > 16)
				return;
			const uint8_t slot = channel - 1;
			if (pedal_downLast[slot] != pedal) {
				pedal_downLast[slot] = pedal;
				this->pedal_down[slot] = pedal;

				// when pedal goes down, we want to transfer the notes that are held to sustained
				if (pedal) {
					for (int i = 0; i < 128; i++) {
						sustainedNotes[slot].set(i, heldNotes[slot].get(i));
					}
				} else {

					// when pedal goes up, we want to kill the notes that are sustained
					for (int i = 0; i < 128; i++) {
						if (!heldNotes[slot].get(i) && sustainedNotes[slot].get(i)) {
							//*(only if not held by fingers)
							note_off_callback(channel, i);
						}
					}

					// clear sustained notes
					for (int i = 0; i < 128; i++) {
						sustainedNotes[slot].set(i, 0);
					}
				}
			}
		}

		void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
			if (channel < 1 || channel > 16 || note > 127)
				return;
			const uint8_t slot = channel - 1;
			if (velocity == 0) {
				note_off(channel, note);
				return;
			}

			// is the note already singing? Retrigger
			if (sustainedNotes[slot].get(note)) {

				note_off_callback(channel, note);
			}

			// trigger the note
			note_on_callback(channel, note, velocity);

			// add note to held array
			heldNotes[slot].set(note, true);

			if (pedal_down[slot]) {
				// pedal is down so we want to sustain the note
				sustainedNotes[slot].set(note, true);
			}
		}

		void note_off(uint8_t channel, uint8_t note) {
			if (channel < 1 || channel > 16 || note > 127)
				return;
			const uint8_t slot = channel - 1;

			// remove note from held array
			heldNotes[slot].set(note, 0);

			if (!pedal_down[slot]) {
				note_off_callback(channel, note);
			}
		}

		// All Notes Off releases held keys while respecting the sustain pedal.
		void all_notes_off(uint8_t channel) {
			if (channel < 1 || channel > 16)
				return;
			for (uint8_t note = 0; note < 128; ++note) {
				if (heldNotes[channel - 1].get(note))
					note_off(channel, note);
			}
		}

	private:
		BooleanArray128 heldNotes[16];
		BooleanArray128 sustainedNotes[16];
		bool pedal_down[16];
		bool pedal_downLast[16];
		note_on_callback_t note_on_callback;
		note_off_callback_t note_off_callback;
};
