# Quick Reference: How to Run NRPN Tests

## All Tests (Complete Suite)

### 1️⃣ Unit Tests (Always Pass First)

**Time**: 1 second | **Hardware**: Not required | **Setup**: None

```bash
node unit/test_nrpn_unit.mjs
```

**Expected**: 
```
╔════════════════════════════════════════════════════════════╗
║  SUMMARY                                                   ║
║  Passed: 4269                                              ║
║  Failed: 0                                                 ║
╚════════════════════════════════════════════════════════════╝
```

✓ **Status**: Ready to run anytime

---

### 2️⃣ Integration Tests (Requires Hardware)

**Time**: 5-10 minutes | **Hardware**: MEGAfm + USB MIDI | **Setup**: npm install

#### Setup (First Time Only)

```bash
cd integration
npm install
```

#### Find Your MIDI Ports

```bash
npm run list-devices
```

**Example Output**:
```
Available MIDI outputs (synth):
  0: MEGAfm
Available MIDI inputs (synth):
  0: MEGAfm
```

#### Run Tests

```bash
node test_nrpn_integration.mjs \
  --synth-out "MEGAfm" \
  --synth-in "MEGAfm" \
  --channel 0
```

**Expected**:
```
╔════════════════════════════════════════════════════════════╗
║  MEGAfm NRPN Integration Tests                             ║
╚════════════════════════════════════════════════════════════╝

Output: UMC404HD 192k MIDI Out
Input: UMC404HD 192k MIDI In
Channel: 5
Bank: 0, Program: 0

Turning off MIDI Feedback

=== Test 1a: Ingestion (Send → Hardware State) ===
LFO1 Shape (100): sending values 0, 4, 8
LFO2 Shape (101): sending values 0, 4, 8

[... more tests ...]

=== Test 1b: LFO Links (Special Multi-Value NRPNs) ===

╔════════════════════════════════════════════════════════════╗
║  SUMMARY                                                   ║
║  Passed: 597                                               ║
║  Failed: 0                                                 ║
╚════════════════════════════════════════════════════════════╝
```

✓ **Status**: Ready when hardware available

---

### 3️⃣ Manual Hardware Procedures (Requires DAW + Hardware)

**Time**: 30-60 minutes | **Hardware**: MEGAfm + DAW | **Setup**: Read procedures

#### Location

```
manual/NRPN_MANUAL_TEST_PROCEDURES.md
```

#### 5 Procedures

- **C1**: Ingestion (LED/audio response) — 9 tests
- **C2**: Transmission (MIDI format) — 3 tests  
- **C3**: Consistency (audio round-trip) — 5 tests
- **C4**: Stress (rapid automation) — 5 tests
- **C5**: Edge cases (param directionality) — 2 tests

#### Run

1. Open the manual procedures file
2. Follow each section (C1-C5)
3. Check off pass criteria
4. Sign off at bottom
