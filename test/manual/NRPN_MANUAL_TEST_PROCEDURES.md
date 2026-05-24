# MEGAfm NRPN Manual Hardware Test Procedures

## Overview

These procedures verify that NRPN implementation works correctly on actual hardware.
They complement automated tests by checking LED indicators, audio output, and MIDI monitoring.

---

## C1: Ingestion — Manual Verification

### Objective
Verify that NRPN values sent are correctly applied to hardware state.

### Setup

- MEGAfm hardware connected via USB MIDI
- Tool to send NRPN
- Ability to see LEDs/display on MEGAfm front panel
- Optional: Audio monitoring to hear parameter changes

### Procedure

#### Part A: LFO Parameters

**Test A1 – LFO Shape**

1. Send NRPN #100 (LFO1 Shape), set to value 3 (Saw)
2. **Observe MEGAfm**:
   - LED/display should show LFO1 shape changed to "Saw"
   - If LFO is being used on a parameter, sound should reflect sawtooth LFO

**Test A2 – LFO Rate**

1. Send NRPN #222 (LFO1 Rate), set to value 200
2. **Observe**:
   - If LFO is modulating a parameter, modulation speed should be very fast
   - Compare to a lower rate (send NRPN #222 value 50) → should be noticeably slower

**Test A3 – LFO Looping**

1. Send NRPN #103 (LFO1 Looping) value 0
2. **Observe**: LED indicates looping OFF
3. Send NRPN #103 value 1a
4. **Observe**: LED indicates looping ON

#### Part B: Settings

**Test B1 – Brightness**

1. Send NRPN #200 (Brightness) value 0
2. **Observe**: LEDs are very dim
3. Send NRPN #200 value 15
4. **Observe**: LEDs are at full brightness

**Test B2 – Voice Mode**

1. Send NRPN #208 (Voice Mode) value 0 (Poly12)
2. Play 3+ notes simultaneously → should hear all notes
3. Send NRPN #208 value 3 (Unison)
4. Play notes → should hear single voice, thicker/detuned sound (all oscillators on same note)
5. **Observe**: LED shows voice mode changed (e.g., from "P" to "U")

**Test B3 – Pickup Mode**

1. Turn physical knob (e.g., Volume) fully left
2. Send NRPN #202 (Pickup Mode) value 1 (ON)
3. Return DAW control to center position
4. **Observe**: Volume should jump to physical knob position, not jump around 50%
5. Send NRPN #202 value 0 (OFF)
6. Move DAW control to opposite side
7. **Observe**: Volume can jump discontinuously (pickup off)

#### Part C: Operator Parameters

**Test C1 – OP1 Level**

1. Send NRPN #2002 (OP1 Level) value 0
2. Play note → OP1 is silent
3. Send NRPN #2002 value 255
4. Play note → OP1 is loud
5. Send NRPN #2002 value 50
6. Play note → OP1 is quiet

**Test C2 – OP1 Attack**

1. Send NRPN #2003 (OP1 Attack) value 0
2. Play note → attack is instant (sharp click)
3. Send NRPN #2003 value 200
4. Play note → attack is slow (ramping up)

**Test C3 – OP2 Multiplier**

1. Send NRPN #3001 (OP2 Multiplier) value 50
2. Play note → listen to pitch content from OP2
3. Send NRPN #3001 value 150
4. Play same note → harmonic content should change (OP2 ratio higher)

#### Pass Criteria

- [ ] LFO Shape changed reflected in sound or LED
- [ ] LFO Rate controls modulation speed
- [ ] LFO Looping LED toggles
- [ ] Brightness LED levels change
- [ ] Voice Mode audibly changes sound and LED
- [ ] Pickup Mode affects parameter response
- [ ] Operator levels, attack, and multiplier all affect sound as expected

---

## C2: Transmission — Manual MIDI Monitoring

### Objective
Verify that hardware sends changed values via NRPN, with deduplication working.

### Setup

- MEGAfm hardware connected
- MIDI Monitor app (e.g., MidiOX on Windows, MIDI Monitor on macOS)
- DAW open but MIDI input from MEGAfm optional

### Procedure

#### Part A: Manual Knob → NRPN

1. Open MIDI Monitor, set to display all messages
2. On MEGAfm hardware, turn physical knob (e.g., LFO1 Rate) slightly
3. **Monitor shows**: 4 CC messages (CC 99, 98, 6, 38) = 1 NRPN message
4. Turn same knob more (but NOT to exact same position)
5. **Monitor shows**: Another set of 4 CCs for new position
6. Turn knob back to exactly the previous position
7. **Monitor shows**: NO new NRPN message (deduplication cache working!)
8. Turn different knob (e.g., Volume)
9. **Monitor shows**: New NRPN messages for this parameter

#### Part B: Hardware State Changes

1. Press Button (e.g., toggle MIDI Thru on/off via button)
2. **Monitor shows**: NRPN for MIDI Thru state change
3. Press same button again
4. **Monitor shows**: NRPN sent again (state changed)

#### Part C: NRPN Message Format

1. Select one NRPN message in monitor (4 consecutive CCs)
2. Verify bytes are in format: CC99 CC98 CC6 CC38
3. Verify values are:
   - CC 99: NRPN number MSB (bits 8-14)
   - CC 98: NRPN number LSB (bits 0-7)
   - CC 6: Value MSB (bits 8-14)
   - CC 38: Value LSB (bits 0-7)
4. Example: NRPN 222 (LFO1 Rate) value 200
   - CC 99 value = 1 (222 >> 7)
   - CC 98 value = 94 (222 & 0x7F)
   - CC 6 value = 1 (200 >> 7)
   - CC 38 value = 72 (200 & 0x7F)

#### Pass Criteria

- [ ] Manual knob turns produce NRPN messages
- [ ] Deduplication works (exact position repeat = no message)
- [ ] Different parameters each send their own NRPN
- [ ] Button state changes produce NRPNs
- [ ] NRPN format is correct (4 CCs in sequence)

---

## C3: State Consistency — Manual Round-Trip

### Objective
Verify that dumped preset matches what was set, and round-trip restore is correct.

### Setup

- MEGAfm hardware connected to Ableton with Max for Live device loaded
- Audio routing working (can hear MEGAfm output)
- Audio recorder or DAW with recording capability

### Procedure

#### Part A: Baseline Preset Recording

1. Load a distinctive preset (e.g., bank 2, program 45)
2. Play a simple phrase (few notes, short duration, 5-10 seconds)
3. **Record audio** of this preset's output to baseline_preset.wav
4. **Note**: This is your "original" sound

#### Part B: Dump SysEx

1. In Max device, click "DUMP" button (exports current preset as SysEx)
2. Open MIDI monitor → should see large SysEx message (starts F0, ends F7)
3. **Save this SysEx** to file for later comparison

#### Part C: Modify Preset in Max

1. In Max device UI, change several parameters:
   - Volume: increase to 90%
   - LFO1 Rate: change to 120
   - Voice Mode: change to "Unison"
   - Algorithm: change to different value
2. Play same phrase → **Record to modified_preset.wav**
3. **Expect**: Sound is noticeably different

#### Part D: Restore from Backup

1. In Max device, click "RESTORE" button
2. Load the saved SysEx from Part B
3. This should send the backup back to MEGAfm
4. **Wait 2 seconds** for hardware to update
5. Play same phrase → **Record to restored_preset.wav**

#### Part E: Manual Comparison

1. Listen to baseline_preset.wav
2. Listen to modified_preset.wav (should be different)
3. Listen to restored_preset.wav (should match baseline)
4. **Optional**: Use audio editor to overlay waveforms (should be nearly identical)

#### Pass Criteria

- [ ] Original preset sounds distinctive
- [ ] SysEx dump is received (visible in MIDI monitor)
- [ ] Modified preset sounds audibly different
- [ ] Restored preset sounds identical to original

---

## C4: Stress Test — Manual Rapid Automation

### Objective
Verify no audio glitches, stuck notes, or parameter corruption under rapid NRPN load.

### Setup

- MEGAfm hardware connected and playing
- Continuous audio output
- DAW capable of fast automation (Ableton, Logic, Bitwig)

### Procedure

#### Part A: Continuous Playback

1. In MEGAfm or synth app: Start a continuous note/arpeggio playing
2. **Listen for any audio artifacts** (clicks, pops, glitches)
3. **Record clean baseline** to baseline_stress.wav (30 seconds)

#### Part B: Rapid Automation — Single Parameter

1. In DAW: Create automation curve on 1 NRPN parameter
   - Use LFO1 Rate (NRPN 222)
   - Automate from 0 → 255 over 3 seconds (dense automation)
2. Play the synth while automation is running
3. **Listen for**:
   - Stuck notes (sustained sound after release)
   - Audio glitches or clicks
   - Instrument becoming unresponsive
4. Let it stop and play a final note
5. **Expect**: All notes release cleanly, no hanging audio

#### Part C: Rapid Automation — Multiple Parameters

1. Create simultaneous automation on multiple parameters:
   - Automation 1: LFO1 Rate (0 → 255 over 3 sec)
   - Automation 2: LFO1 Depth (0 → 255 over 3 sec)
   - Automation 3: Volume (100 → 50 → 100 over 3 sec)
   - Automation 4: Algorithm (various values, stepping every 200ms)
   - **Result**: 300+ MIDI/NRPN messages in 3 seconds
2. Play the synth
3. **Listen for audio artifacts or glitches**
4. Play final notes after automation stops
5. **Expect**: Clean playback, no corruption

#### Part D: Final State Check

1. After stress test, request SysEx dump
2. In MIDI monitor: should see complete SysEx
3. Compare to expected final state from automation curves
4. **Expect**: Values match last automation point

#### Part E: Audio Recording

1. Record stress test to stress_test.wav (60 seconds)
2. Listen back for clicks/pops/glitches
3. **Expect**: Clean audio (might be harsh from modulation, but no digital artifacts)

#### Pass Criteria

- [ ] Continuous playback during rapid automation
- [ ] No stuck notes after stress test
- [ ] No audio clicks or glitches (excluding intended modulation)
- [ ] Synth remains responsive
- [ ] Final state dump is valid
- [ ] Audio recording is clean (subjective but important)

---

## C5: Edge Cases — Directional Parameters

### Objective
Verify correct handling of receive-only vs. both-direction parameters.

### Setup

- MEGAfm hardware connected
- DAW with NRPN support
- MIDI monitor (optional)

### Procedure

#### Part A: Receive-Only Parameter (Envelope Mode)

**Background**: Envelope Mode (NRPNs 2008, 3008, 4008, 5008 for OP 1-4) are **receive-only**.
The firmware applies them but does NOT send them back in SysEx or when preset dumped.

1. Send NRPN #2008 (OP1 Envelope Mode) value 2 (Ping Pong)
2. Play synth → note envelope should ping-pong
3. Request SysEx dump via NRPN #10
4. Open SysEx dump in hex editor or parser
5. **Verify**: Envelope Mode byte is NOT in the dump (or is default 0/1)
6. Manually turn physical OP1 knob (any knob) to trigger different preset dump
7. In fresh dump, Envelope Mode still not present
8. **Expect**: Receive-only parameters NOT echoed back

#### Part B: Both-Direction Parameter (LFO Shape)

**Background**: LFO Shape (NRPNs 100-102) are **both-direction**.
The firmware applies them AND sends them back when preset is dumped.

1. Send NRPN #100 (LFO1 Shape) value 5 (Random 2-step)
2. Request SysEx dump
3. Parse SysEx and find LFO1 Shape byte
4. **Verify**: Shape value is 5 in dump
5. Manually turn physical LFO1 Shape knob to different position (e.g., position for value 2)
6. Request new SysEx dump
7. Parse SysEx and find LFO1 Shape byte
8. **Verify**: Shape value now matches physical knob position (e.g., 2)

#### Pass Criteria

- [ ] Receive-only parameters (Envelope Mode) are applied but not echoed in SysEx
- [ ] Both-direction parameters (LFO Shape) are applied AND echoed in SysEx
- [ ] Manual physical control overrides NRPN when hardware state is dumped
- [ ] No parameter corruption between requests
