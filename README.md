# MEGAfm

Atmega1284 control software for the MEGAfm synthesizer.

## Architecture

(TODO)

## Development Guide

(TODO)

### Preparation

Install `platformio`.

Run `git clone https://github.com/pichenettes/avr-midi-bootloader`. Then, in
`tools/hex2sysex/hex2sysex.py`, find `default='\x00\x21\x02'` and change it to
`default='\x00\x21\x44'`.

### Building and flashing the source code:

Build the `.pio/build/ATmega1284P/firmware.hex` file using `platformio run`.

To generate the firmware update sysex, run

```bash
PYTHONPATH=/path/to/avr-midi-bootloader/ python2 /path/to/avr-midi-bootloader/tools/hex2sysex/hex2sysex.py -s -o firmware.syx -v 0x7f .pio/build/ATmega1284P/firmware.hex
```

Then put the device into bootloader mode by holding down the "arp mode" button while turning it on,
and send the sysex. Note that you need to wait about 200msec between each sysex.
