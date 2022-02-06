#!/bin/bash
#TODO(montag): this does not work yet, since sendmidi does not throttle sysex
set -x
sendmidi dev "mio" syf "firmware.syx"
