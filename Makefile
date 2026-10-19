all: firmware.syx

fmt:
	find src/ -type f -exec clang-format -i '{}' +
	find include/ -type f -exec clang-format -i '{}' +

check_fmt:
	find src/ -type f -exec clang-format --dry-run --Werror '{}' +
	find include/ -type f -exec clang-format --dry-run --Werror '{}' +

.pio/build/ATmega1284P/firmware.hex: $(wildcard src/* include/*)
	platformio run
	touch $@ # because platformio is strange

firmware.syx: .pio/build/ATmega1284P/firmware.hex
	PYTHONPATH=3rdparty python2 3rdparty/tools/hex2sysex/hex2sysex.py -s -o $@ -v 0x7f $<

clean:
	platformio run -t clean
	rm -f firmware.syx

.PHONY: clean fmt check_fmt

# Host regression tests; sanitizers catch invalid MIDI array accesses.
test_midi:
	@set -e; test_binary=$$(mktemp); trap 'rm -f "$$test_binary"' EXIT; \
	$(CXX) -std=c++11 -fsanitize=address,undefined,bounds -fno-sanitize-recover=all test/native/midi_pedal_test.cpp -o "$$test_binary"; \
	"$$test_binary"

.PHONY: test_midi
