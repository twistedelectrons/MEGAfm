/**
 * MEGAfm SysEx Parser
 *
 * Parses MEGAfm SysEx dump messages to extract all 88 parameter values.
 * SysEx format: F0 7E 00 09 01 <data> F7
 *
 * The data section contains all parameters encoded in a specific order.
 */

/**
 * Parse a complete MEGAfm SysEx preset dump
 * @param {Uint8Array} sysex - The SysEx message bytes
 * @returns {Object} Map of NRPN numbers to arrays of values
 *   - Most NRPNs: single-element array [value]
 *   - LFO Links (1000-1002): 45-element array (one per pot)
 */
export function parseSysEx(sysex) {
	// Validate SysEx format
	if (!sysex || sysex.length < 8) {
		throw new Error('Invalid SysEx: too short');
	}

	if (sysex[0] !== 0xF0 || sysex[sysex.length - 1] !== 0xF7) {
		throw new Error('Invalid SysEx: missing start (F0) or end (F7)');
	}

	// MEGAfm SysEx Format: F0 00 21 44 5C [arpLen] [arp data...] [NRPN data...] F7
	// Manufacturer ID: 00 21 44
	// Command: 5C = Full preset dump
	if (sysex[1] !== 0x00 || sysex[2] !== 0x21 || sysex[3] !== 0x44 || sysex[4] !== 0x5C) {
		throw new Error('Invalid SysEx: unknown header (expected F0 00 21 44 5C)');
	}

	// Payload format: [arpLen] [arp data...] [NRPN data...] F7
	// - arpLen: 1 byte (0-16)
	// - arp data: 2 bytes per step [value >> 7] [value & 0x7F]
	// - NRPN data: 4 bytes per parameter [nrpn MSB] [nrpn LSB] [value MSB] [value LSB]

	let offset = 5; // Skip F0 00 21 44 5C
	const arpLen = sysex[offset];
	offset += 1;

	if (arpLen > 16) {
		throw new Error(`Invalid SysEx: arp length > 16 (got ${arpLen})`);
	}

	// Skip arp data (2 bytes per step)
	offset += arpLen * 2;

	const result = {};

	// Parse NRPN data (4 bytes per parameter)
	// Store as arrays to support multi-value NRPNs like LFO Links
	while (offset < sysex.length - 1) {
		// Check if we have at least 4 bytes left (before F7)
		if (offset + 4 > sysex.length - 1) {
			break;
		}

		const nrpnMSB = sysex[offset];
		const nrpnLSB = sysex[offset + 1];
		const valueMSB = sysex[offset + 2];
		const valueLSB = sysex[offset + 3];

		// Decode 14-bit values: MSB is 7 bits, LSB is 7 bits
		const nrpn = (nrpnMSB << 7) | nrpnLSB;
		const value = (valueMSB << 7) | valueLSB;

		// Initialize array if this is the first occurrence
		if (!(nrpn in result)) {
			result[nrpn] = [];
		}

		// Append value to array
		result[nrpn].push(value);
		offset += 4;
	}

	return result;
}

/**
 * Build a map from parameter index (0-87) to NRPN number
 * Must match the nrpnIndex() function in firmware
 */
function buildIndexToNRPNMap() {
	const map = {};

	// LFO parameters (100-114) => indices 0-14
	for (let i = 0; i < 15; i++) {
		map[i] = 100 + i;
	}

	// Settings (200-209) => indices 15-24
	for (let i = 0; i < 10; i++) {
		map[15 + i] = 200 + i;
	}

	// Continuous (220-232) => indices 25-37
	for (let i = 0; i < 13; i++) {
		map[25 + i] = 220 + i;
	}

	// Arp (300-303) => indices 38-41
	for (let i = 0; i < 4; i++) {
		map[38 + i] = 300 + i;
	}

	// Vibrato (500-502) => indices 42-44
	for (let i = 0; i < 3; i++) {
		map[42 + i] = 500 + i;
	}

	// LFO Link (1000-1002) => indices 45-47
	for (let i = 0; i < 3; i++) {
		map[45 + i] = 1000 + i;
	}

	// Operators 1-4 (2000-2009, 3000-3009, 4000-4009, 5000-5009) => indices 48-87
	for (let op = 0; op < 4; op++) {
		const baseNRPN = 2000 + op * 1000;
		const baseIdx = 48 + op * 10;
		for (let i = 0; i < 10; i++) {
			map[baseIdx + i] = baseNRPN + i;
		}
	}

	return map;
}

/**
 * Create a SysEx message from parsed parameter map
 * Used for testing round-trip (parse → encode → parse)
 *
 * @param {Object} params - Map of NRPN to value
 * @returns {Uint8Array} SysEx bytes
 */
export function createSysEx(params) {
	const indexToNRPN = buildIndexToNRPNMap();
	const nrpnToIndex = {};

	for (const [idx, nrpn] of Object.entries(indexToNRPN)) {
		nrpnToIndex[nrpn] = parseInt(idx);
	}

	// Build data payload
	const data = [];

	// Program (NRPN 20)
	data.push(params[20] || 0);

	// 88 parameters
	for (let idx = 0; idx < 88; idx++) {
		const nrpn = indexToNRPN[idx];
		data.push(params[nrpn] || 0);
	}

	// Build SysEx: F0 7E 00 09 01 <data> F7
	const sysex = new Uint8Array([0xF0, 0x7E, 0x00, 0x09, 0x01, ...data, 0xF7]);

	return sysex;
}

/**
 * Convert byte array from MIDI input (0-127 range) to SysEx format
 * MIDI transmits 7-bit values; SysEx may use full 8-bit in some cases
 *
 * @param {number[]} bytes - MIDI bytes (0-127)
 * @returns {Uint8Array} SysEx bytes
 */
export function midiToSysEx(bytes) {
	return new Uint8Array(bytes);
}

/**
 * Wait for SysEx message on given MIDI input port
 * @param {Object} input - EasyMIDI input port
 * @param {number} timeoutMs - Timeout in milliseconds
 * @param {boolean} debug - Enable debug output
 * @returns {Promise<Uint8Array>} SysEx bytes when received
 */
export async function waitForSysEx(input, timeoutMs = 1000, debug = false) {
	return new Promise((resolve, reject) => {
		let receivedSysex = false;
		const timer = setTimeout(() => {
			input.removeListener('sysex', handler);
			if (debug) console.log(`[waitForSysEx] Timeout after ${timeoutMs}ms (received: ${receivedSysex})`);
			reject(new Error(`SysEx timeout after ${timeoutMs}ms`));
		}, timeoutMs);

		const handler = (bytes) => {
			if (debug) console.log(`[waitForSysEx] Handler called with:`, typeof bytes, Array.isArray(bytes) ? `(${bytes.length} bytes)` : '(not array)', bytes ? Object.keys(bytes) : 'null/undefined');
			
			receivedSysex = true;
			
			// Handle both array and object formats
			let data = bytes;
			if (bytes && typeof bytes === 'object' && !Array.isArray(bytes)) {
				// Might be an object with byte data
				if (bytes.bytes) data = bytes.bytes;
				if (debug) console.log(`[waitForSysEx] Extracted bytes from object:`, data);
			}
			
			if (Array.isArray(data) && data.length > 0) {
				if (debug) console.log(`[waitForSysEx] Got array: ${data.length} bytes, starts with 0x${data[0]?.toString(16)}, ends with 0x${data[data.length-1]?.toString(16)}`);
				
				if (data[0] === 0xF0 && data[data.length - 1] === 0xF7) {
					if (debug) console.log(`[waitForSysEx] ✓ Valid SysEx, resolving`);
					input.removeListener('sysex', handler);
					clearTimeout(timer);
					resolve(new Uint8Array(data));
				} else {
					if (debug) console.log(`[waitForSysEx] ✗ Not valid SysEx format (F0=${data[0]===0xF0}, F7=${data[data.length-1]===0xF7})`);
				}
			} else {
				if (debug) console.log(`[waitForSysEx] ✗ Not an array or empty`);
			}
		};

		if (debug) console.log(`[waitForSysEx] Listening for SysEx...`);
		input.on('sysex', handler);
	});
}

/**
 * Send NRPN message via MIDI output
 * NRPN = CC(99) CC(98) CC(6) CC(38)
 *
 * @param {Object} output - EasyMIDI output port
 * @param {number} nrpn - NRPN number
 * @param {number} value - NRPN value (0-16383 for 14-bit)
 * @param {number} channel - MIDI channel (0-15)
 */
export function sendNRPN(output, nrpn, value, channel = 0) {
	const nrpnMsb = (nrpn >> 7) & 0x7F;
	const nrpnLsb = nrpn & 0x7F;
	const valueMsb = (value >> 7) & 0x7F;
	const valueLsb = value & 0x7F;

	// Easy MIDI API: output.send(type, params)
	output.send('cc', { controller: 99, value: nrpnMsb, channel });
	output.send('cc', { controller: 98, value: nrpnLsb, channel });
	output.send('cc', { controller: 6, value: valueMsb, channel });
	output.send('cc', { controller: 38, value: valueLsb, channel });
}

/**
 * Sleep for N milliseconds
 */
export function sleep(ms) {
	return new Promise((resolve) => setTimeout(resolve, ms));
}
