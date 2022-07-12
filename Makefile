all: firmware.syx

.pio/build/ATmega1284P/firmware.hex: $(wildcard src/* include/*)
	platformio run
	touch $@ # because platformio is strange

firmware.syx: .pio/build/ATmega1284P/firmware.hex
	PYTHONPATH=3rdparty python2 3rdparty/tools/hex2sysex/hex2sysex.py -s -o $@ -v 0x7f $<

clean:
	platformio run -t clean
	rm -f firmware.syx

.PHONY: clean
