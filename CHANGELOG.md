# CHANGELOG

[Friends don't let friends dump git logs into changelogs.](https://keepachangelog.com/en/1.1.0/).

4.1 (Updated May 23, 2024)

Arpeggiator updates
- Now available in all voice modes.
- Previously limited to unison mode, the arpeggiator now works in all voice modes and supports overlapping notes.
- Holding a single key makes the arpeggiator cycle through all voices in round-robin style.

Improved arpeggiator timing
- More clock division options when syncing the arpeggiator to MIDI clock for finer timing control.
- Faster synchronization with the master clock for tighter timing.

Polyphonic LFO enhancements
- Polyphonic LFO1 (velocity): Modulates operator parameters per key based on velocity.
- Example: Assign an FM modulator level to LFO1 so note timbre changes with playing intensity.
- Activation: Enter setup mode by holding the voice mode button for a few seconds, then turn LFO1 depth past 50%.
- Polyphonic LFO3 (poly aftertouch): Modulates operator parameters per key based on aftertouch pressure.
- Example: Assign an operator multiplier to LFO3 to raise pitch with increased pressure.
- Activation: Enter setup mode by holding the voice mode button for a few seconds, then turn LFO3 depth past 50%.

3.1
- Fix display issue where some faders didn't always update the LED screen
- Add Octave transposing per preset
- New wide modes optional (activated via tool)
- Preset number displayed after 2 seconds of inactivity
- Fix arp sequencer bug (not storing if length>15 steps)

3.0
- Add Support for webMIDI tool
- Fix THX chord generation, actually hit the chord.
- Add option to keep the old chord
- Add Wide4 and Wide3 voicing modes, always use staggering.
- AuthoChord feature
- looping envelopes
- Consider only released voices for voice stealing.
