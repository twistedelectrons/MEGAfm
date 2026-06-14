# MEGAfm NRPN Reference

The MEGAfm supports NRPN (Non-Registered Parameter Number) messages for controlling nearly all synthesis parameters from a DAW, sequencer, or MIDI controller.

## How NRPN Works

An NRPN message consists of four MIDI CC (Control Change) messages sent in sequence:

| Step | CC # | Purpose           |
|------|------|-------------------|
| 1    | 99   | NRPN number MSB   |
| 2    | 98   | NRPN number LSB   |
| 3    | 6    | Value MSB         |
| 4    | 38   | Value LSB         |

All NRPN messages are sent and received on the **master MIDI channel**.

Most DAWs and MIDI controllers have built-in support for sending NRPN — you typically just enter the NRPN number and value, and the software handles the four CC messages for you.

> **Note:** Most parameters use values 0–255 (the lower 8 bits). Boolean parameters treat 0 as off and any value above 0 as on.

---

## Commands (Receive Only)

| Parameter        | NRPN # | Values                                        |
|------------------|--------|-----------------------------------------------|
| Dump Settings    | 10     | Any value — triggers a SysEx dump             |
| MIDI Feedback    | 11     | On/Off - update display on MIDI/NRPN messages |
| Program Change   | 20     | 0–599 (bank 0–5 × 100 + program 0–99)         |

---

## LFO Parameters

Each LFO (1, 2, 3) has its own NRPN number. Add 0/1/2 to the base number for LFO 1/2/3 (e.g. 100 = LFO 1, 101 = LFO 2, 102 = LFO 3)

| Parameter          | NRPN #      | Values               | Direction |
|--------------------|-------------|----------------------|-----------|
| Shape              | 100-102     | 0–8 (see below)      | Both      |
| Looping            | 103–105     | On/Off  (0, !=0)     | Both      |
| Retrigger          | 106–108     | On/Off               | Both      |
| MIDI Clock Sync    | 109–111     | On/Off               | Both      |
| Rate               | 222-224     | 0–255                | Both      |
| Depth              | 225-227     | 0–255                | Both      |
| LFO1 Velocity      | 112         | On/Off               | Both      |
| LFO2 Mod Wheel     | 113         | On/Off               | Both      |
| LFO3 Aftertouch    | 114         | On/Off               | Both      |

### LFO Shape Values

| Value | Shape              |
|-------|--------------------|
| 0     | Square             |
| 1     | Inverted Square    |
| 2     | Triangle           |
| 3     | Saw                |
| 4     | Inverted Saw       |
| 5     | Random (2-step)    |
| 6     | Random (3-step)    |
| 7     | Random (4-step)    |
| 8     | Random (5-step)    |

---

## LFO Link

Links an LFO to a target parameter (pot). The value encodes both the target and the link state in a single number.

| Parameter          | NRPN #       | Direction |
|--------------------|-------------|-----------|
| LFO1 Link          | 1000        | Both      |
| LFO2 Link          | 1001        | Both      |
| LFO3 Link          | 1002        | Both      |

**Value encoding:** `target × 2 + linked`.

- To **link** LFO to target pot 10: send value **21** (10 × 2 + 1)
- To **unlink** LFO from target pot 10: send value **20** (10 × 2 + 0)

### LFO Link Target Pots

| Pot # | Parameter          | Pot # | Parameter          |
|-------|--------------------|-------|--------------------|
| 0     | OP1 Detune         | 27    | OP4 Detune         |
| 1     | OP1 Multiplier     | 28    | OP4 Multiplier     |
| 2     | OP1 Level          | 29    | OP4 Level          |
| 4     | OP1 Attack         | 31    | OP4 Attack         |
| 5     | OP1 Decay          | 32    | OP4 Decay          |
| 6     | OP1 Sustain Rate   | 33    | OP4 Sustain Rate   |
| 7     | OP1 Sustain Level  | 34    | OP4 Sustain Level  |
| 8     | OP1 Release        | 35    | OP4 Release        |
| 9     | OP3 Detune         | 36    | LFO1 Rate          |
| 10    | OP3 Multiplier     | 37    | LFO1 Depth         |
| 11    | OP3 Level          | 38    | LFO2 Rate          |
| 13    | OP3 Attack         | 39    | LFO2 Depth         |
| 14    | OP3 Decay          | 40    | LFO3 Rate          |
| 15    | OP3 Sustain Rate   | 41    | LFO3 Depth         |
| 16    | OP3 Sustain Level  | 42    | Algorithm          |
| 17    | OP3 Release        | 43    | Feedback           |
| 18    | OP2 Detune         | 46    | Arp Rate           |
| 19    | OP2 Multiplier     | 47    | Arp Range          |
| 20    | OP2 Level          | 48    | Vibrato Rate       |
| 22    | OP2 Attack         | 49    | Vibrato Depth      |
| 23    | OP2 Decay          | 50    | Fat                |
| 24    | OP2 Sustain Rate   |       |                    |
| 25    | OP2 Sustain Level  |       |                    |
| 26    | OP2 Release        |       |                    |

> **Note:** Volume, Fine Tune, and Glide are not available as LFO link targets.

---

## Settings

| Parameter              | NRPN # | Values                                              | Direction |
|------------------------|--------|-----------------------------------------------------|-----------|
| Brightness             | 200    | 0–15                                                | Both      |
| MIDI Thru              | 201    | On/Off                                              | Both      |
| Pickup Mode            | 202    | On/Off                                              | Both      |
| Stereo Channel 3       | 203    | On/Off                                              | Both      |
| MPE Mode               | 204    | On/Off                                              | Both      |
| Fat Spread             | 205    | On/Off                                              | Both      |
| Ignore Preset Volume   | 206    | On/Off                                              | Both      |
| Fat Mode               | 207    | 0 = Semitone, 1 = Octave                            | Both      |
| Voice Mode             | 208    | 0–5 (see below)                                     | Both      |
| Octave Offset          | 209    | 0–3                                                 | Both      |

### Voice Modes

| Value | Mode       |
|-------|------------|
| 0     | Poly 12    |
| 1     | Wide 6     |
| 2     | Dual Ch3   |
| 3     | Unison     |
| 4     | Wide 4     |
| 5     | Wide 3     |

---

## Continuous Parameters

| Parameter        | NRPN # | Values  | Direction |
|------------------|--------|---------|-----------|
| Fine Tune        | 220    | 0–255   | Both      |
| Glide            | 221    | 0–255   | Both      |
| Fat              | 228    | 0–255   | Both      |
| Volume           | 229    | 0–255   | Both      |
| Feedback         | 230    | 0–255   | Both      |
| Algorithm        | 231    | 0–255   | Both      |
| Note Priority    | 232    | 0 = Lowest, 1 = Highest, 2 = Last | Both |

---

## Arpeggiator

| Parameter        | NRPN # | Values               | Direction |
|------------------|--------|----------------------|-----------|
| Arp Mode         | 300    | 0–7 (see below)      | Both      |
| Arp Clock Sync   | 301    | On/Off               | Both      |
| Arp Rate         | 302    | 0–255                | Both      |
| Arp Range        | 303    | 0–255                | Both      |
| Arp Set Step     | 304    | Special (see below)  | Both      |

### Arp Modes

| Value | Mode       |
|-------|------------|
| 0     | Off        |
| 1     | Up         |
| 2     | Down       |
| 3     | Up/Down    |
| 4     | Random 1   |
| 5     | Random 2   |
| 6     | Sequence 1 |
| 7     | Sequence 2 |

### Arp Set Step

The 14-bit value encodes both the step index and step value:
- **Upper byte (bits 8–15):** step index
- **Lower byte (bits 0–7):** step value (255 = reset/clear)

---

## Vibrato

| Parameter          | NRPN # | Values  | Direction |
|--------------------|--------|---------|-----------|
| Vibrato Clock Sync | 500    | On/Off  | Both      |
| Vibrato Rate       | 501    | 0–255   | Both      |
| Vibrato Depth      | 502    | 0–255   | Both      |

---

## Operator Parameters

Each of the four operators shares the same set of parameters. Add the operator's base number to the parameter offset:

| Operator | Base NRPN # |
|----------|-------------|
| OP 1     | 2000        |
| OP 2     | 3000        |
| OP 3     | 4000        |
| OP 4     | 5000        |

For example, OP 2 Attack = 3000 + 3 = **3003**.

| Parameter      | Offset | Values                                     | Direction |
|----------------|--------|--------------------------------------------|-----------|
| Detune         | +0     | 0–255                                      | Both      |
| Multiplier     | +1     | 0–255                                      | Both      |
| Level          | +2     | 0–255                                      | Both      |
| Attack         | +3     | 0–255                                      | Both      |
| Decay          | +4     | 0–255                                      | Both      |
| Sustain Level  | +5     | 0–255                                      | Both      |
| Sustain Rate   | +6     | 0–255                                      | Both      |
| Release        | +7     | 0–255                                      | Both      |
| Envelope Mode  | +8     | 0 = Off, 1 = Forward, 2 = Ping Pong        | Receive   |
| Rate Scaling   | +9     | 0–255                                      | Both      |

---

## Quick Reference — All NRPN Numbers

| NRPN #    | Parameter                |
|-----------|--------------------------|
| 10        | Dump Settings            |
| 20        | Program Change           |
| 100–102   | LFO 1–3 Shape            |
| 103–105   | LFO 1–3 Looping          |
| 106–108   | LFO 1–3 Retrigger        |
| 109–111   | LFO 1–3 Clock Sync       |
| 112       | LFO1 Velocity            |
| 113       | LFO2 Mod Wheel           |
| 114       | LFO3 Aftertouch          |
| 200–209   | Settings                 |
| 220–232   | Continuous Parameters    |
| 300–304   | Arpeggiator              |
| 500–502   | Vibrato                  |
| 1000–1002 | LFO 1–3 Link             |
| 2000–2009 | Operator 1               |
| 3000–3009 | Operator 2               |
| 4000–4009 | Operator 3               |
| 5000–5009 | Operator 4               |
