#!/usr/bin/env node

/**
 * NRPN Unit Tests
 *
 * Tests the core NRPN logic:
 *   - nrpnIndex() mapping accuracy (all 88 parameters)
 *   - Value bounds validation
 *   - Boolean parameter conversion
 *   - Deduplication cache logic
 *   - 16-bit value handling
 *
 * Run:
 *   node test_nrpn_unit.mjs
 */

import { NRPN, ALL_NRPNS } from '../shared/nrpn_constants.mjs';

// === Test Framework ===
let g_testsPassed = 0;
let g_testsFailed = 0;

function test(condition, message) {
	const tag = condition ? '✓' : '✗';
	if (!condition) {
		console.log(`  ${tag} ${message}`);
		g_testsFailed++;
	} else {
		g_testsPassed++;
	}
}

// === nrpnIndex() — Maps NRPN message number to cache index (0-87) ===
function nrpnIndex(msg) {
	if (msg >= NRPN.LFO_SHAPE && msg <= NRPN.LFO_AT)
		return msg - NRPN.LFO_SHAPE;
	if (msg >= NRPN.SET_BRIGHTNESS && msg <= NRPN.SET_OCT_OFFSET)
		return msg - 185;
	if (msg >= NRPN.FINE_TUNE && msg <= NRPN.NOTE_PRIORITY)
		return msg - 195;
	if (msg >= NRPN.ARP_MODE && msg <= NRPN.ARP_RANGE)
		return msg - 262;
	if (msg >= NRPN.VIB_CLOCK_SYNC && msg <= NRPN.VIB_DEPTH)
		return msg - 458;
	if (msg >= NRPN.LFO_LINK && msg <= NRPN.LFO_LINK + 2)
		return msg - 955;
	if (msg >= NRPN.OP1_BASE && msg <= NRPN.OP1_BASE + NRPN.OP_RATE_SCALE)
		return msg - 1952;
	if (msg >= NRPN.OP2_BASE && msg <= NRPN.OP2_BASE + NRPN.OP_RATE_SCALE)
		return msg - 2942;
	if (msg >= NRPN.OP3_BASE && msg <= NRPN.OP3_BASE + NRPN.OP_RATE_SCALE)
		return msg - 3932;
	if (msg >= NRPN.OP4_BASE && msg <= NRPN.OP4_BASE + NRPN.OP_RATE_SCALE)
		return msg - 4922;
	return -1;
}

// === Deduplication cache ===
const lastNRPN = new Int16Array(88);

function initLastNRPN() {
	lastNRPN.fill(-1);
}

function shouldSendNRPN(msg, value) {
	const idx = nrpnIndex(msg);
	if (idx < 0) return true;
	if (lastNRPN[idx] === value) return false;
	lastNRPN[idx] = value;
	return true;
}

// === TEST SUITE 1: nrpnIndex() Mapping Accuracy ===
function test_nrpnIndex_mapping() {
	console.log('\n=== TEST 1: nrpnIndex() Mapping Accuracy ===');

	// LFO parameters (100-114)
	test(nrpnIndex(100) === 0, 'nrpnIndex(100) => 0');
	test(nrpnIndex(114) === 14, 'nrpnIndex(114) => 14');

	// Settings (200-209)
	test(nrpnIndex(200) === 15, 'nrpnIndex(200) => 15');
	test(nrpnIndex(209) === 24, 'nrpnIndex(209) => 24');

	// Continuous (220-232)
	test(nrpnIndex(220) === 25, 'nrpnIndex(220) => 25');
	test(nrpnIndex(232) === 37, 'nrpnIndex(232) => 37');

	// Arp (300-303)
	test(nrpnIndex(300) === 38, 'nrpnIndex(300) => 38');
	test(nrpnIndex(303) === 41, 'nrpnIndex(303) => 41');

	// Vibrato (500-502)
	test(nrpnIndex(500) === 42, 'nrpnIndex(500) => 42');
	test(nrpnIndex(502) === 44, 'nrpnIndex(502) => 44');

	// LFO Link (1000-1002)
	test(nrpnIndex(1000) === 45, 'nrpnIndex(1000) => 45');
	test(nrpnIndex(1002) === 47, 'nrpnIndex(1002) => 47');

	// Operators 1-4
	test(nrpnIndex(2000) === 48, 'nrpnIndex(2000) => 48');
	test(nrpnIndex(2009) === 57, 'nrpnIndex(2009) => 57');
	test(nrpnIndex(3000) === 58, 'nrpnIndex(3000) => 58');
	test(nrpnIndex(3009) === 67, 'nrpnIndex(3009) => 67');
	test(nrpnIndex(4000) === 68, 'nrpnIndex(4000) => 68');
	test(nrpnIndex(4009) === 77, 'nrpnIndex(4009) => 77');
	test(nrpnIndex(5000) === 78, 'nrpnIndex(5000) => 78');
	test(nrpnIndex(5009) === 87, 'nrpnIndex(5009) => 87');

	// Invalid NRPNs should return -1
	test(nrpnIndex(99) === -1, 'nrpnIndex(99) => -1');
	test(nrpnIndex(115) === -1, 'nrpnIndex(115) => -1');
	test(nrpnIndex(500000) === -1, 'nrpnIndex(500000) => -1');
}

// === TEST SUITE 2: Value Bounds Validation ===
function test_value_bounds() {
	console.log('\n=== TEST 2: Value Bounds Validation ===');

	const inRange = (val, min, max) => val >= min && val <= max;

	// LFO Shape: valid 0-8
	test(inRange(0, 0, 8), 'LFO Shape 0 valid');
	test(inRange(8, 0, 8), 'LFO Shape 8 valid');
	test(!inRange(9, 0, 8), 'LFO Shape 9 invalid');

	// Voice Mode: valid 0-5
	test(inRange(0, 0, 5), 'Voice Mode 0 valid');
	test(inRange(5, 0, 5), 'Voice Mode 5 valid');
	test(!inRange(6, 0, 5), 'Voice Mode 6 invalid');

	// Octave Offset: valid 0-3
	test(inRange(0, 0, 3), 'Oct Offset 0 valid');
	test(inRange(3, 0, 3), 'Oct Offset 3 valid');
	test(!inRange(4, 0, 3), 'Oct Offset 4 invalid');

	// Brightness: valid 0-15
	test(inRange(0, 0, 15), 'Brightness 0 valid');
	test(inRange(15, 0, 15), 'Brightness 15 valid');
	test(!inRange(16, 0, 15), 'Brightness 16 invalid');

	// Arp Mode: valid 0-7
	test(inRange(0, 0, 7), 'Arp Mode 0 valid');
	test(inRange(7, 0, 7), 'Arp Mode 7 valid');
	test(!inRange(8, 0, 7), 'Arp Mode 8 invalid');

	// Envelope Mode: valid 0-2
	test(inRange(0, 0, 2), 'Env Mode 0 valid');
	test(inRange(2, 0, 2), 'Env Mode 2 valid');
	test(!inRange(3, 0, 2), 'Env Mode 3 invalid');

	// Continuous: valid 0-255
	test(inRange(0, 0, 255), 'Continuous 0 valid');
	test(inRange(255, 0, 255), 'Continuous 255 valid');
	test(!inRange(256, 0, 255), 'Continuous 256 invalid');
}

// === TEST SUITE 3: Boolean Parameter Conversion ===
function test_boolean_conversion() {
	console.log('\n=== TEST 3: Boolean Parameter Conversion ===');

	const byteToBool = (val) => val > 0;
	const boolToByte = (val) => val ? 1 : 0;

	test(byteToBool(0) === false, '0 => false');
	test(byteToBool(1) === true, '1 => true');
	test(byteToBool(127) === true, '127 => true');
	test(byteToBool(255) === true, '255 => true');

	test(boolToByte(false) === 0, 'false => 0');
	test(boolToByte(true) === 1, 'true => 1');
}

// === TEST SUITE 4: Deduplication Cache Logic ===
function test_deduplication() {
	console.log('\n=== TEST 4: Deduplication Cache Logic ===');

	initLastNRPN();

	// First send of value
	test(shouldSendNRPN(100, 5) === true, 'First send of NRPN 100 value 5');
	test(nrpnIndex(100) >= 0 && lastNRPN[nrpnIndex(100)] === 5, 'Cache updated');

	// Second send of same value
	test(shouldSendNRPN(100, 5) === false, 'Second send of NRPN 100 value 5 skipped');

	// Send different value
	test(shouldSendNRPN(100, 6) === true, 'Send NRPN 100 value 6');
	test(lastNRPN[nrpnIndex(100)] === 6, 'Cache updated to 6');

	// Send value back to previous
	test(shouldSendNRPN(100, 5) === true, 'Send NRPN 100 value 5 (new again)');
	test(lastNRPN[nrpnIndex(100)] === 5, 'Cache updated to 5');

	// Different parameter
	test(shouldSendNRPN(101, 3) === true, 'Different param (NRPN 101) first send');
	test(lastNRPN[nrpnIndex(100)] === 5, 'NRPN 101 cache independent from 100');

	// Invalid NRPN always sends
	test(shouldSendNRPN(9999, 100) === true, 'Invalid NRPN always sends');
}

// === TEST SUITE 5: 16-bit Value Handling ===
function test_16bit_values() {
	console.log('\n=== TEST 5: 16-bit Value Handling ===');

	const extractMsb = (val) => (val >> 7) & 0x7F;
	const extractLsb = (val) => val & 0x7F;
	const reconstruct = (msb, lsb) => (msb << 7) | lsb;

	// Test various 14-bit values
	const testValues = [0, 1, 127, 128, 255, 256, 1000, 2000, 16383];

	for (const val of testValues) {
		const msb = extractMsb(val);
		const lsb = extractLsb(val);
		const reconstructed = reconstruct(msb, lsb);
		test(
			(reconstructed & 0x3FFF) === (val & 0x3FFF),
			`14-bit value ${val}: MSB=${msb}, LSB=${lsb}, reconstructed=${reconstructed & 0x3FFF}`
		);
	}

	// Test LFO Link encoding (bit 0 = linked, bits 1-7 = target pot)
	const encodeLfoLink = (target, linked) => (target << 1) | (linked ? 1 : 0);
	const decodeLfoLink = (val) => ({ target: val >> 1, linked: (val & 1) === 1 });

	for (let target = 0; target < 51; target++) {
		for (const linked of [false, true]) {
			const encoded = encodeLfoLink(target, linked);
			const decoded = decodeLfoLink(encoded);
			test(
				decoded.target === target && decoded.linked === linked,
				`LFO Link: target=${target}, linked=${linked}`
			);
		}
	}

	// Test Arp Set Step encoding (upper byte = step, lower byte = value)
	const encodeArpStep = (step, value) => (step << 8) | value;
	const decodeArpStep = (val) => ({ step: val >> 8, value: val & 0xFF });

	for (let step = 0; step < 16; step++) {
		for (let value = 0; value < 256; value++) {
			const encoded = encodeArpStep(step, value);
			const decoded = decodeArpStep(encoded);
			test(
				decoded.step === step && decoded.value === value,
				`Arp Step: step=${step}, value=${value}`
			);
		}
	}
}

// === TEST SUITE 6: All 88 NRPN Numbers ===
function test_all_88_nrpns() {
	console.log('\n=== TEST 6: All 88 NRPN Numbers Mapped ===');

	const seenIndices = new Set();

	let count = 0;
	for (const nrpn of ALL_NRPNS) {
		const idx = nrpnIndex(nrpn);
		if (idx >= 0 && idx < 88) {
			seenIndices.add(idx);
			count++;
		}
	}

	test(count === 88, 'All 88 NRPNs map to unique indices');

	// Check for duplicates
	let hasDuplicates = false;
	for (const nrpn of ALL_NRPNS) {
		const idx = nrpnIndex(nrpn);
		if (idx >= 0) {
			for (const other of ALL_NRPNS) {
				if (nrpn !== other && nrpnIndex(nrpn) === nrpnIndex(other)) {
					hasDuplicates = true;
				}
			}
		}
	}
	test(!hasDuplicates, 'No duplicate index mappings');
}

// === Main ===
console.log('\n╔════════════════════════════════════════════════════════════╗');
console.log('║  MEGAfm NRPN Unit Tests                                    ║');
console.log('╚════════════════════════════════════════════════════════════╝');

test_nrpnIndex_mapping();
test_value_bounds();
test_boolean_conversion();
test_deduplication();
test_16bit_values();
test_all_88_nrpns();

console.log('\n╔════════════════════════════════════════════════════════════╗');
console.log('║  SUMMARY                                                   ║');
console.log(`║  Passed: ${g_testsPassed.toString().padEnd(47)}║`);
console.log(`║  Failed: ${g_testsFailed.toString().padEnd(47)}║`);
console.log('╚════════════════════════════════════════════════════════════╝\n');

process.exit(g_testsFailed > 0 ? 1 : 0);
