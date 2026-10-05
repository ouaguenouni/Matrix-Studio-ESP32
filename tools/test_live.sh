#!/bin/sh
# Run after pio run installs the pinned ArduinoJson and HUB75 dependencies.
set -eu
cd "$(dirname "$0")/.."
workdir=$(mktemp -d /tmp/matrix-tests.XXXXXX)
trap 'rm -rf "$workdir"' EXIT
c++ -std=c++11 -Wall -Wextra -Werror -Iinclude -I.pio/libdeps/esp32dev/ArduinoJson/src tests/live_data_test.cpp -o "$workdir/live-data"
"$workdir/live-data"
c++ -std=c++11 -Wall -Wextra -Werror -Iinclude tests/display_dump.cpp -o "$workdir/display-dump"
MATRIX_DISPLAY_DUMP="$workdir/display-dump" node tests/display_test.mjs
c++ -std=c++11 -Wall -Wextra -Werror tests/frame_protocol_test.cpp -o "$workdir/frame"
"$workdir/frame"
c++ -std=c++11 -Wall -Wextra -Werror -DPIXEL_COLOR_DEPTH_BITS=6 -I '.pio/libdeps/esp32dev/ESP32 HUB75 LED MATRIX PANEL DMA Display/src' tests/panel_colour_test.cpp -o "$workdir/colour"
"$workdir/colour"
c++ -std=c++11 -Wall -Wextra -Werror -Iinclude tests/gift_state_test.cpp -o "$workdir/gift-state"
"$workdir/gift-state"
c++ -std=c++11 -Wall -Wextra -Werror -Iinclude tests/gift_display_dump.cpp -o "$workdir/gift-dump"
MATRIX_GIFT_DUMP="$workdir/gift-dump" node tests/gift_display_test.mjs
