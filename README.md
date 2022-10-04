# MEGAfm

This is the Atmega1284 firmware for the [MEGAfm synthesizer](https://www.twistedelectrons.com/megafm).

Find the official releases at [the official product page](https://www.twistedelectrons.com/megafm) or the [github release page](https://github.com/twistedelectrons/MEGAfm/releases).

![Photo of the MEGAfm](https://static.wixstatic.com/media/b8c32b_6152ca5a9ba64d23bcfb972bbee82721~mv2.jpeg/v1/fill/w_1024,h_606,al_c,q_85,enc_auto/b8c32b_6152ca5a9ba64d23bcfb972bbee82721~mv2.jpeg)

## Building and flashing the source code:

Install `platformio` and `python2`. (Yes. I am so sorry :(.)

To compile the firmware and to generate the firmware update sysex, run:

```
make firmware.syx
```

Then put the device into bootloader mode by holding down the "arp mode" button while turning it on,
and send the sysex. Note that you need to wait about 200msec between each sysex.

## Contributing

If you want to fix a bug, add a feature or contribute in any other way to the development
of this firmware, we are glad to accept a pull request in this repository! Please make sure
your contribution adheres to the following:

- Please update [CHANGELOG.md](CHANGELOG.md) to contain a brief description of your changes.
- Ensure you do not break loading of old patches.

Thank you, we are looking forward to your contribution! :)

## License

The MEGAfm firmware under `src/` and `include/` is free software. It is released under the terms of
the [ISC License](LICENSE.md). The libraries and tools distributed in the `3rdparty/` and `lib/`
directories have their own copyright and license notes.

