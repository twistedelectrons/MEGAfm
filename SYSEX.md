# MEGAfm SysEx Reference

The MEGAfm uses System Exclusive (SysEx) messages for dumping and restoring complete presets, including all parameters documented in [NRPN.md](NRPN.md).

## Manufacturer ID

All MEGAfm SysEx messages use the manufacturer ID `00 21 44` (hex). Messages with a different manufacturer ID are ignored.

## Message Format

Every SysEx message follows this structure:

```
F0  00 21 44  [command]  [payload...]  F7
```

| Byte(s)      | Description                          |
|--------------|--------------------------------------|
| `F0`         | SysEx start                          |
| `00 21 44`   | Manufacturer ID                      |
| command      | Message type (see below)             |
| payload      | Command-specific data                |
| `F7`         | SysEx end                            |

## Commands

| Command | Direction | Description              |
|---------|-----------|--------------------------|
| `5B` (91)  | Receive   | Full preset load      |
| `5C` (92)  | Send      | Full preset dump      |
| `5D` (93)  | Receive   | Arp sequence load     |
| `5E` (94)  | Send      | Arp sequence dump     |

---

## Full Preset Dump (Command 92 / 5C)

Sent by the MEGAfm when you:
- Send NRPN 10 (Dump Settings) with any value
- Load a preset on the device
- Trigger a random shuffle

The payload contains the arp sequence followed by all preset parameters encoded as NRPN data.

### Structure

```
F0 00 21 44 5C [arpLen] [arp data...] [NRPN data...] F7
```

| Section    | Format                                                       |
|------------|--------------------------------------------------------------|
| Arp length | 1 byte: number of arp steps (0–16)                           |
| Arp data   | 2 bytes per step: `[value >> 7] [value & 7F]` (MSB/LSB, 255 = rest) |
| NRPN data  | 4 bytes per parameter: `[nrpn MSB] [nrpn LSB] [value MSB] [value LSB]` |

The NRPN data contains 220 parameter entries covering all settings described in [NRPN.md](NRPN.md):

| Count | Parameters                                                        |
|-------|-------------------------------------------------------------------|
| 15    | LFO settings (shape, looping, retrigger, clock sync, vel/mod/at) |
| 13    | Continuous parameters (fine tune, glide, LFO rates/depths, fat, volume, feedback, algorithm, note priority) |
| 10    | Global settings (brightness, MIDI thru, pickup, stereo ch3, MPE, fat spread, ignore volume, fat mode, voice mode, octave offset) |
| 4     | Arpeggiator (mode, clock sync, rate, range)                       |
| 3     | Vibrato (clock sync, rate, depth)                                 |
| 135   | LFO links (3 LFOs x 45 target pots)                              |
| 40    | Operators (4 operators x 10 parameters)                           |
| **220** | **Total**                                                       |

Each 4-byte NRPN entry uses 7-bit encoding (values split into MSB/LSB with 7 bits each), matching the standard MIDI data byte format.

---

## Full Preset Load (Command 91 / 5B)

Send this to the MEGAfm to restore a previously dumped preset. The format is identical to the preset dump, but with command byte `5B` instead of `5C`:

```
F0 00 21 44 5B [arpLen] [arp data...] [NRPN data...] F7
```

The device applies each NRPN parameter in sequence using the same handler as individual NRPN messages. During SysEx processing, outgoing MIDI feedback is suppressed to avoid echo loops.

### Validation

The device checks:
- Minimum message length of 7 bytes
- Manufacturer ID matches `00 21 44`
- Arp length is 0–16
- Total message length is consistent with the declared arp length
- Remaining bytes after arp data are a multiple of 4 (complete NRPN entries)

---

## Arp Sequence Dump (Command 94 / 5E)

Sent by the MEGAfm to export just the arp sequence without the full preset.

```
F0 00 21 44 5E [arpLen] [arp data...] F7
```

| Section    | Format                                                       |
|------------|--------------------------------------------------------------|
| Arp length | 1 byte: number of steps (0–16)                               |
| Arp data   | 2 bytes per step: `[value >> 7] [value & 7F]` (MSB/LSB)     |

Step values are note numbers (0–254). A value of 255 represents a rest.

---

## Arp Sequence Load (Command 93 / 5D)

Send this to load an arp sequence into the MEGAfm:

```
F0 00 21 44 5D [arpLen] [arp data...] F7
```

Same payload format as the arp dump. The device validates that the arp length is 0–16 and the total message length matches.

---

## Display Feedback

After processing a SysEx message, the MEGAfm display shows a two-character status:

| Display | Meaning             |
|---------|---------------------|
| S 0     | Success             |
| S 1     | Byte error          |
| S 2     | Header mismatch     |
| S 3     | Length error         |
| S 4     | Dump length error   |
| S 5     | Unknown command     |

---

## Triggering a Dump

To request a full preset dump from a DAW or controller, send **NRPN 10** with any value (see [NRPN.md](NRPN.md#commands-receive-only)). The MEGAfm will respond with a full preset dump (command 92).

## Typical Workflow

1. **Save:** Send NRPN 10 to request a dump. Capture the resulting SysEx message (command `5C`).
2. **Restore:** Send the captured SysEx back, changing command byte `5C` to `5B`.
3. **Arp only:** Use commands `5E`/`5D` to save and restore just the arp sequence.
