#!/bin/bash

# compile
pio run

# upload
pio run --target upload

# serial
minicom -D /dev/ttyUSB0 -b 115200 -t vt100
