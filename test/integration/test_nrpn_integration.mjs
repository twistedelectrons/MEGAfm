#!/usr/bin/env node

/**
 * NRPN Integration Tests
 *
 * Tests end-to-end NRPN communication with MEGAfm hardware.
 *
 * Usage:
 *   npm install  # First time only
 *   node test_nrpn_integration.mjs --list-devices
 *   node test_nrpn_integration.mjs \
 *     --synth-out "MEGAfm" --synth-in "MEGAfm" \
 *     --bank 0 --program 0
 *
 * Requires:
 *   - MEGAfm connected via USB MIDI
 */

import EasyMIDI from 'easymidi';
import { parseArgs } from 'util';
import {
	parseSysEx,
	createSysEx,
	waitForSysEx,
	sendNRPN,
	sleep,
} from './sysex_parser.mjs';
import { NRPN } from '../shared/nrpn_constants.mjs';

// Test framework
let g_testsPassed = 0;
let g_testsFailed = 0;
let g_testName = '';

function test(condition, message) {
	const tag = condition ? '✓' : '✗';
	if (!condition) {
		console.log(`  ${tag} ${message}`);
		g_testsFailed++;
	} else {
		g_testsPassed++;
	}
}

function section(name) {
	g_testName = name;
	console.log(`\n=== ${name} ===`);
}

// List available MIDI devices
function listDevices() {
	console.log('Available MIDI outputs (synth):');
	EasyMIDI.getOutputs().forEach((out, i) => {
		console.log(`  ${i}: ${out}`);
	});

	console.log('\nAvailable MIDI inputs (synth):');
	EasyMIDI.getInputs().forEach((inp, i) => {
		console.log(`  ${i}: ${inp}`);
	});
}

// Parse command-line arguments
function parseArgs_() {
	const options = {
		help: { type: 'boolean', short: 'h' },
		'list-devices': { type: 'boolean' },
		'synth-out': { type: 'string' },
		'synth-in': { type: 'string' },
		channel: { type: 'string', default: '0' },
		bank: { type: 'string', default: '0' },
		program: { type: 'string', default: '0' },
		timeout: { type: 'string', default: '1000' },
	};

	const { values } = parseArgs({ options });
	return values;
}

// Test Suite 1a: Ingestion
async function testIngestion(output, input, channel) {
	section('Test 1a: Ingestion (Send → Hardware State)');

	// Comprehensive test cases covering all 88 distinct NRPNs
	// Organized by category with min/middle/max values
	const testCases = [
		// === LFO Shapes (100-102) ===
		{ nrpn: 100, values: [0, 4, 8], name: 'LFO1 Shape (100)' },
		{ nrpn: 101, values: [0, 4, 8], name: 'LFO2 Shape (101)' },
		{ nrpn: 102, values: [0, 4, 8], name: 'LFO3 Shape (102)' },

		// === LFO Looping (103-105) ===
		{ nrpn: 103, values: [0, 1, 0], name: 'LFO1 Looping (103)' },
		{ nrpn: 104, values: [0, 1, 0], name: 'LFO2 Looping (104)' },
		{ nrpn: 105, values: [0, 1, 0], name: 'LFO3 Looping (105)' },

		// === LFO Retrigger (106-108) ===
		{ nrpn: 106, values: [0, 1, 0], name: 'LFO1 Retrigger (106)' },
		{ nrpn: 107, values: [0, 1, 0], name: 'LFO2 Retrigger (107)' },
		{ nrpn: 108, values: [0, 1, 0], name: 'LFO3 Retrigger (108)' },

		// === LFO Clock Sync (109-111) ===
		{ nrpn: 109, values: [0, 1, 0], name: 'LFO1 Clock Sync (109)' },
		{ nrpn: 110, values: [0, 1, 0], name: 'LFO2 Clock Sync (110)' },
		{ nrpn: 111, values: [0, 1, 0], name: 'LFO3 Clock Sync (111)' },

		// === LFO Modulation Sources (112-114) ===
		{ nrpn: 112, values: [0, 1, 0], name: 'LFO1 Velocity (112)' },
		{ nrpn: 113, values: [0, 1, 0], name: 'LFO2 Mod Wheel (113)' },
		{ nrpn: 114, values: [0, 1, 0], name: 'LFO3 Aftertouch (114)' },

		// === Settings (200-209) ===
		{ nrpn: 200, values: [0, 8, 15], name: 'Brightness (200)' },
		{ nrpn: 201, values: [0, 1, 0], name: 'MIDI Thru (201)' },
		// Set to zero after test!
		{ nrpn: 202, values: [0, 1, 0], name: 'Pickup Mode (202)' },
		{ nrpn: 203, values: [0, 1, 0], name: 'Stereo CH3 (203)' },
		// Turn off again after test!
		{ nrpn: 204, values: [0, 1, 0], name: 'MPE Mode (204)' },
		{ nrpn: 205, values: [0, 1, 0], name: 'Fat Spread (205)' },
		{ nrpn: 206, values: [0, 1, 0], name: 'Ignore Volume (206)' },
		{ nrpn: 207, values: [0, 1, 0], name: 'Fat Mode (207)' },
		{ nrpn: 208, values: [0, 3, 5, 0], name: 'Voice Mode (208)' },
		{ nrpn: 209, values: [0, 2, 3, 0], name: 'Octave Offset (209)' },

		// === Continuous Parameters (220-232) ===
		{ nrpn: 220, values: [0, 128, 255, 0], name: 'Fine Tune (220)' },
		{ nrpn: 221, values: [0, 7 << 4, 15 << 4, 0], name: 'Glide (221)', expectedValues: [0, 7 << 4, 15 << 4, 0] },
		{ nrpn: 222, values: [0, 128, 255, 0], name: 'LFO1 Rate (222)' },
		{ nrpn: 223, values: [0, 128, 255, 0], name: 'LFO2 Rate (223)' },
		{ nrpn: 224, values: [0, 128, 255, 0], name: 'LFO3 Rate (224)' },
		{ nrpn: 225, values: [0, 128, 255, 0], name: 'LFO1 Depth (225)' },
		{ nrpn: 226, values: [0, 128, 255, 0], name: 'LFO2 Depth (226)' },
		{ nrpn: 227, values: [0, 128, 255, 0], name: 'LFO3 Depth (227)' },
		{ nrpn: 228, values: [0, 128, 255, 0], name: 'Fat (228)' },
		{ nrpn: 229, values: [0, 128, 254], name: 'Volume (229)*' },  // *vol=(128-data>>1)<<1
		{ nrpn: 230, values: [0, 128, 255, 0], name: 'Feedback (230)' },
		{ nrpn: 231, values: [0, 128, 255, 0], name: 'Algorithm (231)' },
		{ nrpn: 232, values: [0, 1, 2, 0], name: 'Note Priority (232)' },

		// === Arpeggiator (300-303) ===
		{ nrpn: 300, values: [0, 4, 7, 0], name: 'Arp Mode (300)' },
		{ nrpn: 301, values: [0, 1, 0], name: 'Arp Clock Sync (301)' },
		{ nrpn: 302, values: [0, 128, 255, 0], name: 'Arp Rate (302)' },
		{ nrpn: 303, values: [0, 128, 255, 0], name: 'Arp Range (303)' },

		// === Vibrato (500-502) ===
		{ nrpn: 500, values: [0, 1, 0], name: 'Vibrato Clock Sync (500)' },
		{ nrpn: 501, values: [0, 128, 255, 0], name: 'Vibrato Rate (501)' },
		{ nrpn: 502, values: [0, 128, 255, 0], name: 'Vibrato Depth (502)' },

		// === Operator 1 (2000-2009) ===
		{ nrpn: 2000, values: [0, 128, 255, 0], name: 'OP1 Detune (2000)' },
		{ nrpn: 2001, values: [0, 128, 255, 0], name: 'OP1 Multiplier (2001)' },
		{ nrpn: 2002, values: [0, 128, 255], name: 'OP1 Level (2002)' },
		{ nrpn: 2003, values: [0, 128, 255], name: 'OP1 Attack (2003)' },
		{ nrpn: 2004, values: [0, 128, 255], name: 'OP1 Decay (2004)' },
		{ nrpn: 2005, values: [0, 128, 255], name: 'OP1 Sustain Lvl (2005)' },
		{ nrpn: 2006, values: [0, 128, 255], name: 'OP1 Sustain Rate (2006)' },
		{ nrpn: 2007, values: [0, 128, 255, 0], name: 'OP1 Release (2007)' },
		{ nrpn: 2008, values: [0, 1, 2, 0], name: 'OP1 Envelope Mode (2008)' },
		{ nrpn: 2009, values: [0, 128, 255, 0], name: 'OP1 Rate Scaling (2009)' },

		// === Operator 2 (3000-3009) ===
		{ nrpn: 3000, values: [0, 128, 255, 0], name: 'OP2 Detune (3000)' },
		{ nrpn: 3001, values: [0, 128, 255, 0], name: 'OP2 Multiplier (3001)' },
		{ nrpn: 3002, values: [0, 128, 255], name: 'OP2 Level (3002)' },
		{ nrpn: 3003, values: [0, 128, 255], name: 'OP2 Attack (3003)' },
		{ nrpn: 3004, values: [0, 128, 255], name: 'OP2 Decay (3004)' },
		{ nrpn: 3005, values: [0, 128, 255], name: 'OP2 Sustain Lvl (3005)' },
		{ nrpn: 3006, values: [0, 128, 255], name: 'OP2 Sustain Rate (3006)' },
		{ nrpn: 3007, values: [0, 128, 255, 0], name: 'OP2 Release (3007)' },
		{ nrpn: 3008, values: [0, 1, 2, 0], name: 'OP2 Envelope Mode (3008)' },
		{ nrpn: 3009, values: [0, 128, 255, 0], name: 'OP2 Rate Scaling (3009)' },

		// === Operator 3 (4000-4009) ===
		{ nrpn: 4000, values: [0, 128, 255, 0], name: 'OP3 Detune (4000)' },
		{ nrpn: 4001, values: [0, 128, 255, 0], name: 'OP3 Multiplier (4001)' },
		{ nrpn: 4002, values: [0, 128, 255], name: 'OP3 Level (4002)' },
		{ nrpn: 4003, values: [0, 128, 255], name: 'OP3 Attack (4003)' },
		{ nrpn: 4004, values: [0, 128, 255], name: 'OP3 Decay (4004)' },
		{ nrpn: 4005, values: [0, 128, 255], name: 'OP3 Sustain Lvl (4005)' },
		{ nrpn: 4006, values: [0, 128, 255], name: 'OP3 Sustain Rate (4006)' },
		{ nrpn: 4007, values: [0, 128, 255, 0], name: 'OP3 Release (4007)' },
		{ nrpn: 4008, values: [0, 1, 2, 0], name: 'OP3 Envelope Mode (4008)' },
		{ nrpn: 4009, values: [0, 128, 255, 0], name: 'OP3 Rate Scaling (4009)' },

		// === Operator 4 (5000-5009) ===
		{ nrpn: 5000, values: [0, 128, 255, 0], name: 'OP4 Detune (5000)' },
		{ nrpn: 5001, values: [0, 128, 255, 0], name: 'OP4 Multiplier (5001)' },
		{ nrpn: 5002, values: [0, 128, 255], name: 'OP4 Level (5002)' },
		{ nrpn: 5003, values: [0, 128, 255], name: 'OP4 Attack (5003)' },
		{ nrpn: 5004, values: [0, 128, 255], name: 'OP4 Decay (5004)' },
		{ nrpn: 5005, values: [0, 128, 255], name: 'OP4 Sustain Lvl (5005)' },
		{ nrpn: 5006, values: [0, 128, 255], name: 'OP4 Sustain Rate (5006)' },
		{ nrpn: 5007, values: [0, 128, 255, 0], name: 'OP4 Release (5007)' },
		{ nrpn: 5008, values: [0, 1, 2, 0], name: 'OP4 Envelope Mode (5008)' },
		{ nrpn: 5009, values: [0, 128, 255, 0], name: 'OP4 Rate Scaling (5009)' },
	];

	for (const tc of testCases) {
		for (let idx = 0; idx < tc.values.length; idx++) {
			const value = tc.values[idx];
			try {
				// Send NRPN
				sendNRPN(output, tc.nrpn, value, channel);
				await sleep(150);  // Allow firmware time to apply change

				// Request dump
				sendNRPN(output, 10, 0, channel);  // NRPN_DUMP_CURRENT_SETTINGS
				// await sleep(100);

				// Wait for SysEx response
				const sysex = await waitForSysEx(input, 4000);

				// Parse and verify
				const parsed = parseSysEx(sysex);
				const values = parsed[tc.nrpn];
				test(
					Array.isArray(values) && values.length === 1,
					`${tc.name}: sent ${value} — exactly 1 value in SysEx`
				);

				if (Array.isArray(values) && values.length > 0) {
					const actual = values[0];
					const expected = tc.expectedValues ? tc.expectedValues[idx] : value;
					test(
						actual === expected,
						`${tc.name}: sent ${value}, got ${actual}`
					);
				}
			} catch (err) {
				test(false, `${tc.name}: sent ${value} — ${err.message}`);
			}
		}
	}
}

// Test Suite 1b: LFO Links
async function testLFOLinks(output, input, channel) {
	section('Test 1b: LFO Links (Special Multi-Value NRPNs)');

	// LFO Links: Each LFO sends 45 NRPN messages (one per target pot) in a dump.
	// Format: value = (targetPot << 1) | linked
	//   - Bits 1-7: target pot number
	//   - Bit 0: link state (0=unlinked, 1=linked)
	//
	// Parser now correctly captures all 45 values as an array.
	// Each LFO can have multiple pots linked simultane ously.

	// Pots excluded from LFO link dumps (operator rate scaling + unused)
	const excludedPots = new Set([3, 12, 21, 30, 44, 45]);

	const testCases = [
		{ lfoNrpn: 1000, name: 'LFO1 Link' },
		{ lfoNrpn: 1001, name: 'LFO2 Link' },
		{ lfoNrpn: 1002, name: 'LFO3 Link' },
	];

	for (const tc of testCases) {
		try {
			// Step 1: Link all pots
			for (let pot = 0; pot < 51; pot++) {
				if (excludedPots.has(pot)) continue;
				sendNRPN(output, tc.lfoNrpn, (pot << 1) | 1, channel);
				await sleep(10);
			}
			await sleep(150);

			// Dump and verify all linked
			sendNRPN(output, 10, 0, channel);
			await sleep(100);
			const sysex1 = await waitForSysEx(input, 4000);
			const parsed1 = parseSysEx(sysex1);
			const values1 = parsed1[tc.lfoNrpn];

			test(
				Array.isArray(values1) && values1.length === 45,
				`${tc.name}: 45 pot entries captured in SysEx dump`
			);

			if (Array.isArray(values1) && values1.length === 45) {
				let allLinked = true;
				for (let i = 0; i < values1.length; i++) {
					if ((values1[i] & 1) !== 1) {
						allLinked = false;
						break;
					}
				}
				test(allLinked, `${tc.name}: all pots linked`);
			}

			// Step 2: Unlink all pots
			for (let pot = 0; pot < 51; pot++) {
				if (excludedPots.has(pot)) continue;
				sendNRPN(output, tc.lfoNrpn, pot << 1, channel);
				await sleep(10);
			}
			await sleep(150);

			// Dump and verify all unlinked
			sendNRPN(output, 10, 0, channel);
			await sleep(100);
			const sysex2 = await waitForSysEx(input, 4000);
			const parsed2 = parseSysEx(sysex2);
			const values2 = parsed2[tc.lfoNrpn];

			if (Array.isArray(values2) && values2.length === 45) {
				let allUnlinked = true;
				for (let i = 0; i < values2.length; i++) {
					if ((values2[i] & 1) !== 0) {
						allUnlinked = false;
						break;
					}
				}
				test(allUnlinked, `${tc.name}: all pots unlinked`);
			}
		} catch (err) {
			test(false, `${tc.name}: ${err.message}`);
		}
	}
}

// Main
async function main() {
	const args = parseArgs_();

	if (args.help) {
		console.log(`
NRPN Integration Tests for MEGAfm

Usage:
  node test_nrpn_integration.mjs [options]

Options:
  --list-devices           List available MIDI devices
  --synth-out <name>       MIDI output port name (e.g., "MEGAfm")
  --synth-in <name>        MIDI input port name (e.g., "MEGAfm")
  --channel <n>            MIDI channel (0-15, default 0)
  --bank <n>               Bank (0-5, default 0)
  --program <n>            Program (0-99, default 0)
  --timeout <ms>           SysEx timeout in ms (default 1000)
  -h, --help               Show this help
		`);
		return;
	}

	if (args['list-devices']) {
		listDevices();
		return;
	}

	if (!args['synth-out'] || !args['synth-in']) {
		console.error(
			'Error: --synth-out and --synth-in required (use --list-devices)'
		);
		process.exit(1);
	}

	console.log('\n╔════════════════════════════════════════════════════════════╗');
	console.log('║  MEGAfm NRPN Integration Tests                             ║');
	console.log('╚════════════════════════════════════════════════════════════╝\n');

	try {
		const output = new EasyMIDI.Output(args['synth-out']);
		const input = new EasyMIDI.Input(args['synth-in']);
		const channel = parseInt(args.channel);

		console.log(`Output: ${args['synth-out']}`);
		console.log(`Input: ${args['synth-in']}`);
		console.log(`Channel: ${channel}`);
		console.log(`Bank: ${args.bank}, Program: ${args.program}\n`);

		// Run test suites
		await testIngestion(output, input, channel);
		await testLFOLinks(output, input, channel);

		// Print summary
		console.log(
			'\n╔════════════════════════════════════════════════════════════╗'
		);
		console.log('║  SUMMARY                                                   ║');
		console.log(
			`║  Passed: ${g_testsPassed.toString().padEnd(47)}║`
		);
		console.log(
			`║  Failed: ${g_testsFailed.toString().padEnd(47)}║`
		);
		console.log(
			'╚════════════════════════════════════════════════════════════╝\n'
		);

		output.close();
		input.close();

		process.exit(g_testsFailed > 0 ? 1 : 0);
	} catch (err) {
		console.error(`Error: ${err.message}`);
		process.exit(1);
	}
}

main().catch(console.error);
