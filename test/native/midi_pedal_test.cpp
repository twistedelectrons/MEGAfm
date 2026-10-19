#include <cassert>
#include <cstdint>
#include <vector>
#define bitWrite(v, b, x) ((x) ? ((v) |= (1UL << (b))) : ((v) &= ~(1UL << (b))))
#include "../../include/midi_pedal.hpp"
struct Event {
		int channel, note, velocity;
};
static std::vector<Event> events;
static void on(uint8_t c, uint8_t n, uint8_t v) { events.push_back({c, n, v}); }
static void off(uint8_t c, uint8_t n) { events.push_back({c, n, 0}); }
int main() {
	MidiPedalAdapter adapter(on, off);
	// Ardour's pedal-off/all-notes-off broadcast, followed by a new note.
	for (int c = 1; c <= 16; ++c) {
		adapter.note_on(c, 52, 100);
		adapter.set_pedal(c, false);
		adapter.all_notes_off(c);
		assert(events.back().channel == c && events.back().velocity == 0);
	}
	adapter.note_on(1, 52, 67);
	assert(events.back().channel == 1 && events.back().velocity == 67);
	adapter.note_off(1, 52);
	// Sustain on channel 16 must preserve callback channel numbering and
	// hold notes through All Notes Off, without affecting channel 1.
	adapter.set_pedal(16, true);
	adapter.note_on(16, 127, 100);
	auto count = events.size();
	adapter.all_notes_off(16);
	assert(events.size() == count);
	adapter.note_on(1, 0, 100);
	adapter.note_off(1, 0);
	assert(events.back().channel == 1 && events.back().velocity == 0);
	adapter.set_pedal(16, false);
	assert(events.back().channel == 16 && events.back().note == 127 && events.back().velocity == 0);
	count = events.size();
	adapter.set_pedal(16, false);
	adapter.all_notes_off(16);
	assert(events.size() == count);
	// All MPE sustain slots, including channel 16, are released.
	for (int c = 1; c <= 16; ++c) {
		adapter.set_pedal(c, true);
		adapter.note_on(c, 60, 100);
		adapter.note_off(c, 60);
	}
	count = events.size();
	for (int c = 1; c <= 16; ++c)
		adapter.set_pedal(c, false);
	assert(events.size() == count + 16);
	// Invalid and transposed-out-of-range input cannot touch storage.
	count = events.size();
	for (uint8_t c : {0, 17, 255}) {
		adapter.set_pedal(c, true);
		adapter.note_on(c, 60, 1);
		adapter.note_off(c, 60);
		adapter.all_notes_off(c);
	}
	for (uint8_t n : {128, 255}) {
		adapter.note_on(1, n, 1);
		adapter.note_off(1, n);
	}
	assert(events.size() == count);
}
