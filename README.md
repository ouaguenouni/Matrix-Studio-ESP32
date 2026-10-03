# Matrix Studio: ESP32 + Waveshare 64x32

A local browser interface hosted on your ESP32. Send text, resized still images,
pixel drawings, or browser-streamed scrolling text to the panel. No cloud account
or separate server is needed. Upload this firmware once; ordinary display updates
then happen over Wi-Fi.

## Your hardware

- Classic ESP32-WROOM development board, configured as **esp32dev** in PlatformIO.
- One Waveshare HUB75 panel, 64x32, using the FM6124 driver. The earlier close-up
  appeared to show FM6124H chips. If a previous test sketch works with another driver
  or a different clock phase, carry those settings into `src/main.cpp`.
- Panel powered separately from a regulated 5 V supply with adequate current.
- ESP32 powered by computer USB while uploading; keep its external 5V pin unconnected
  during USB power. Panel and ESP32 grounds must be connected.
- Home Wi-Fi must offer **2.4 GHz**. Your computer/phone can use 5 GHz on the same
  router if the router allows communication between those devices.

This implementation compiles successfully with PlatformIO for **esp32dev** and
passes host-side frame-protocol tests, colour-conversion tests, and JavaScript/Python
syntax checks. USB firmware upload, Wi-Fi provisioning, reconnection, and serial
diagnostic controls have been verified on the connected ESP32. Panel timing is
being compared visually on hardware; the remaining text ghosting is unresolved.
The browser interaction test passes against simulated endpoints in local Chromium.
Physical browser-to-panel display checks remain.

## 1. Install and upload

1. Open this project's root folder in VS Code with the **PlatformIO IDE** extension,
   or use the PlatformIO Core CLI from this folder.
2. Connect the ESP32 by USB. `platformio.ini` selects **esp32dev**, the Arduino
   framework, and pinned versions of the ESP32 platform and display libraries.
   PlatformIO downloads dependencies automatically on the first build.
   `WiFi`, `WebServer`, `Preferences`, and `ESPmDNS` come with the framework.
3. Use PlatformIO's **Build**, **Upload**, and **Monitor** buttons, or run:

   ```bash
   pio run
   pio run --target upload
   pio device monitor
   ```

4. If multiple USB serial devices are connected, select the ESP32 explicitly with
   `pio run --target upload --upload-port /dev/cu.YOUR_PORT` and
   `pio device monitor --port /dev/cu.YOUR_PORT` (use the appropriate port on your OS).
   Run `pio device list` to see available ports.
5. If uploading waits at “Connecting…”, hold BOOT until writing starts, then release it.
6. The serial monitor uses **115200 baud**. Press the board's **EN/RST** button
   to view startup messages.

Firmware lives in `src/main.cpp`; headers live in `include/`. The build automatically
regenerates `include/web_ui.h` from `web/interface.html` and `web/codec.mjs` using
PlatformIO's own Python interpreter. No filesystem upload is required.

## 2. Join your home Wi-Fi

On the first boot, Serial Monitor prints something like:

```text
Network: Matrix-a1b2c3
Password: <your device's generated setup password>
Open http://192.168.4.1
```

1. On your phone or computer, join that **Matrix-xxxxxx** Wi-Fi network using the
   printed password. Stay connected even if your phone says “No internet”.
2. Manually open **http://192.168.4.1** in your browser. This is a direct web page,
   not an automatic captive portal.
3. Expand **Wi-Fi connection** if necessary. Enter your home network name and password.
   Use a normal personal/password-based network; enterprise sign-in and captive
   portals are not supported by this starter.
4. Click **Save and connect**. The page and Serial Monitor show the home-network IP.
5. Rejoin your home Wi-Fi and open that IP, such as `http://192.168.1.42`.
   You can also try the unique `.local` hostname printed by Serial Monitor.

Credentials are stored on the ESP32 and reused after restart. They are not placed
in this project or returned by the status API. On the setup session the temporary
network stays active, so you can recover from a mistyped password. It will not be
started on the next boot if the saved home-network connection succeeds within
20 seconds. If joining fails, setup mode returns with the same generated password.

The IP can change after a router restart. Use the `.local` name, check Serial
Monitor, or reserve an IP for the ESP32 in your router. `.local` support varies
by device and network. Guest Wi-Fi/client isolation may block access.

## 3. Use the interface

- **Text:** multiline text, font, pixel size, alignment, foreground/background colours.
  Text is rendered by your browser, including accents supported by its fonts.
  The default is pure white on black with **Sharp text pixels** enabled. Sharp
  mode uses full foreground/background pixels instead of dim antialiased edges,
  including during scrolling. Uncheck it to keep smooth edges.
  The preview shows clipping at the actual 64x32 resolution.
- **Image:** local PNG, JPEG, WebP, BMP, or GIF first frame; fit, crop, or stretch.
  Images are resized on your phone/computer, then sent as 4096-byte pixel frames.
- **Draw:** tap or drag on the preview to paint pixels; use the eraser as needed.
- **Send to display:** commits the preview. Editing alone does not update the panel.
- **Clear:** clears the preview; then press Send to clear the physical panel.
- **Brightness:** changes the physical panel immediately; initial value is 30/255.
- **Scrolling:** sends approximately eight frames per second, subject to Wi-Fi speed.
  Keep the browser page visible and awake. It stops if you leave the tab or close it.

The last sent frame remains while the ESP32 is powered, even if the browser closes.
Display content and brightness are **not** restored after power loss; Wi-Fi settings are.
Animations, videos, saved playlists, and remote control from outside your network
are extension points, not included features. This interface has no user login;
clients on the same LAN or password-protected setup network can control it.

## Pin mapping

The reported ribbon-wire connections match the assignments in `src/main.cpp`.
Wire colours identify the ribbon conductors; they do not identify LED colours.

| Wire number | Wire colour | HUB75 signal | ESP32 printed label |
|---|---|---|---|
| 1 | Brown — outer edge | R1 | G25 |
| 2 | Red | G1 | G26 |
| 3 | Orange | B1 | G27 |
| 4 | Yellow | GND | GND |
| 5 | Green | R2 | G14 |
| 6 | Blue | G2 | G12 |
| 7 | Purple | B2 | G13 |
| 8 | Grey | GND | GND |
| 9 | White | A | G23 |
| 10 | Black | B | G19 |
| 11 | Brown — second occurrence | C | G5 |
| 12 | Red — second occurrence | D | G17 |
| 13 | Orange — second occurrence | CLK | G16 |
| 14 | Yellow — second occurrence | LAT/STB | G4 |
| 15 | Green — second occurrence | OE | G15 |
| 16 | Blue — other outer edge | GND | GND |

The 64x32 panel does not use E. All 5 V power stays on the separate power connector;
none of the HUB75 signal wires connects to the ESP32 5V or 3V3 pins.

## Develop your own interface

The architecture is browser rendering → RGB565 frame upload → ESP32 DMA display.
You can replace the browser editor, generate a clock/dashboard elsewhere, or use
the included Python image sender without changing how the panel is wired.

| Endpoint | Method | Input | Effect |
|---|---|---|---|
| `/` | GET | None | Browser interface |
| `/api/status` | GET | None | Network, dimensions, brightness, frame count, free heap |
| `/api/frame` | POST | One multipart file named `frame` | Atomically replace all pixels |
| `/api/brightness` | POST | URL-encoded `value=0..255` | Set brightness |
| `/api/wifi` | POST | URL-encoded `ssid` and `password` | Save credentials and connect |

All POST requests require the header `X-Matrix-Control: 1`. It limits accidental
cross-site browser requests; it is **not** an authentication credential. CORS is
not enabled. Serve browser extensions from this device's page or use a backend
proxy on your own application when hosting an interface separately.

Frame format: exactly **4096 bytes**, 64x32 pixels in row order, top-left first.
Each pixel is RGB565 with the low byte first: red uses bits 11–15, green 5–10,
blue 0–4. Incomplete/oversized uploads are rejected without replacing the display.

```bash
curl -H 'X-Matrix-Control: 1' \
  -F 'frame=@frame.rgb565;type=application/octet-stream' \
  http://192.168.1.42/api/frame

curl -H 'X-Matrix-Control: 1' -d 'value=30' \
  http://192.168.1.42/api/brightness

python3 -m pip install Pillow
python3 tools/send_image.py http://192.168.1.42 photo.png
```

To edit the embedded interface, modify `web/interface.html` or `web/codec.mjs`,
then build and upload the firmware again. PlatformIO regenerates the embedded UI
automatically; you can also run `python3 tools/build_ui.py` manually.
`web/preview.html` is the generated UI for local inspection; a normal local preview
does not control hardware because it has no ESP32 server behind it.

## Verification and troubleshooting

Run `node tests/codec_test.mjs` and compile/run `tests/frame_protocol_test.cpp` with
a standard C++ compiler. `tests/browser_test.cjs` runs the real browser interface
against simulated endpoints using Playwright. These do not replace hardware testing.

The panel runs at six-bit colour depth. `platformio.ini` also defines
`PIXEL_COLOR_DEPTH_BITS=6` when compiling the HUB75 library, so its brightness
lookup table matches the six DMA bitplanes. An eight-bit table with only six
bitplanes discards the upper bits and wraps intermediate colours; full white
can still look correct. The firmware checks the configured depth at compile time.
The regression test exercises the pinned library's actual table:

```bash
c++ -std=c++11 -Wall -Wextra -Werror -DPIXEL_COLOR_DEPTH_BITS=6 \
  -I '.pio/libdeps/esp32dev/ESP32 HUB75 LED MATRIX PANEL DMA Display/src' \
  tests/panel_colour_test.cpp -o /tmp/matrix-panel-colour-test
/tmp/matrix-panel-colour-test
```

On hardware, first verify the previous RGB test sketch works. After uploading
this project, use **Image → Preview a colour test → Send to display**. Then verify
text, brightness, Wi-Fi reconnect after a reset, and preservation of the last frame
after closing your browser. Send at low brightness for the first tests.

To bypass browser rendering and frame uploads, open the serial monitor at 115200
baud and send uppercase `T`. The firmware displays red, green, and blue vertical
bands on the top 24 rows, with a white strip on the bottom 8 rows, using RGB888
directly. Send normal content from the web
interface to replace the test. If green and blue are black in this test too, check
the corresponding signal wires and panel hardware: G1 → GPIO26, B1 → GPIO27,
G2 → GPIO12, B2 → GPIO13. Turn off power before reseating wires.

Send uppercase `W` over serial to fill the entire panel with white at the current
brightness. Send normal content from the web interface to replace the white test.
Send uppercase `G` for eight grayscale swatches from black to white on the top
half and cream, pale yellow/red/green/blue/cyan/magenta, and white on the bottom.
This uses direct RGB888 values to check intermediate shades independently of
browser resizing and RGB565 conversion. Judge the colours directly: camera
exposure timing can introduce tints or bands when photographing scanned LEDs.
Send uppercase `V` for red, green, blue, magenta, purple, dark purple, lavender,
and white columns. The top half uses direct RGB888, the bottom RGB565 as used
by browser uploads. Magenta, purple and dark purple have zero green input.
In the browser's Image tab, **Preview a purple test → Send to display** sends
the same colours through the actual browser/upload path. Compare directly with
your eyes. Differences between panel halves can also indicate separate upper
and lower signal wiring, so they are not proof of an encoding fault by themselves.
Send uppercase `H` for a static white `Hello` on a black background. This is drawn
once using the on-device font, without browser rendering or continuous frame uploads.
Startup now uses profile **N**, the best of the latest visual comparison: 4 MHz,
clock phase false, four blanking clocks, and the weakest clock drive (level 0).
Other panel signals retain the library's drive level 3. The observed ranking was
K, then I; L and M were clearly worse. N subsequently looked best when compared
with K, O and P, and remained best against the slower Q, R and S trials. N is the
selected operating setting; complete removal of shifted ghosting is unconfirmed.
Send uppercase `C` over serial to switch between clock drive levels 0 and 3 while
the same image stays displayed. Reboot restores profile N.

Send uppercase `S` to compare timing profiles A–S. Each profile displays its letter,
two white `Hello` lines, and a thin vertical white line for 12 seconds; the sequence
repeats every 228 seconds. Brightness stays constant so the timing comparisons
use the same content and intensity. Ignore the brief transition between profiles.
Report the letter with the fewest ghost pixels, checking both halves of the panel.
Serial commands: `P` pauses/resumes, `N` advances, and `X` stops and restores the
settings used before the sweep. Sending normal content, changing brightness,
or using `T`, `W`, `G`, `V`, `H`, or `C` also stops the sweep and restores those settings.

Send uppercase `F` for the focused **N → Q → R → S** comparison (12 seconds per
test, 48 seconds per cycle). This retains N's weakest clock drive, phase false,
four blanking clocks and strong non-clock signals, comparing 4, 2, 1 and 0.5 MHz.
The slowest profile is eight times slower than N. These trials intentionally
lower scan refresh too; compare the shifted copies separately from whole-panel
flicker. Serial logs include estimated scan refresh, scaled from startup's DMA
calculation. The estimate is not an oscilloscope measurement.
The same pause, next, and stop controls apply.

| Test | Clock output | Clock phase | Blanking clocks | Clock drive level |
|---|---|---|---|---|
| A | 10 MHz | false | 4 | 2 (medium) |
| B | 5 MHz | false | 4 | 2 (medium) |
| C | 10 MHz | true | 4 | 2 (medium) |
| D | 5 MHz | true | 4 | 2 (medium) |
| E | 10 MHz | false | 2 | 2 (medium) |
| F | 5 MHz | false | 2 | 2 (medium) |
| G | 10 MHz | true | 2 | 2 (medium) |
| H | 5 MHz | true | 2 | 2 (medium) |
| I | 5 MHz | false | 4 | 1 (lower) |
| J | 5 MHz | false | 4 | 3 (maximum) |
| K | 5 MHz | false | 4 | 0 (weakest) |
| L | 5 MHz | false | 4 | 1 (lower) |
| M | 5 MHz | false | 4 | 1 (lower) |
| N | 4 MHz | false | 4 | 0 (weakest) |
| O | 5 MHz | true | 4 | 0 (weakest) |
| P | 4 MHz | true | 4 | 0 (weakest) |
| Q | 2 MHz | false | 4 | 0 (weakest) |
| R | 1 MHz | false | 4 | 0 (weakest) |
| S | 0.5 MHz | false | 4 | 0 (weakest) |

Profiles A–K and N–S leave non-clock signals at the library's drive level 3. L lowers only
LAT and OE to level 1; M lowers all non-clock data/address/control signals to
level 1. Each profile restores other signals explicitly so the previous profile
does not influence the next comparison. All profiles use the same brightness.

The sweep's clock-divider override targets the classic ESP32 and pinned HUB75
library. This library's nominal `HZ_8M` configuration uses an actual 10 MHz clock
on the classic ESP32 (80 MHz / divider 4 / 2). The 5 MHz profiles use divider 8;
4 MHz uses divider 10; 2, 1 and 0.5 MHz use dividers 20, 40 and 80 respectively.
The slower profiles also lower scan refresh, so report any
whole-panel flicker separately from shifted ghost pixels.
Startup applies profile N after the library initializes DMA, and its DMA refresh
calculation uses the matching 4 MHz clock. Sweep changes are temporary and do not
rewrite saved Wi-Fi credentials or the startup configuration. Reboot exits the
sweep. Profile labels describe the clock output; the calculated refresh value
describes startup profile N and is not updated by temporary sweep overrides.

- Blank panel with a reachable web page: check input connector, ground, pin mapping,
  and driver selection. Copy any successful settings from your earlier test sketch.
- Colours swapped: first confirm the wiring. Some panel variants need a software
  swap of green and blue pins, but do not change these blindly.
- Shifted pixels/ghosting: the firmware currently uses `config.clkphase = false;`,
  a 4 MHz clock, four clock cycles of latch blanking, and the weakest clock drive.
  Use the serial timing comparisons to evaluate other settings. Short, secure
  signal wires and a shared ground remain important if ghost pixels persist.
- Resets/flicker during Wi-Fi use: verify stable power and short, secure signal wires.
  Some panel variants need a proper 3.3 V to 5 V logic buffer for reliable operation.
- Panel initialization fails: read Serial Monitor, check library/board selection,
  and temporarily try `config.double_buff = false;` if memory allocation is failing
  (frame replacement may then visibly tear).
- Wi-Fi setup fails: check exact SSID/password, 2.4 GHz availability, and personal
  network security. Retry through the setup network; no new firmware upload is needed.

## References

- Espressif Wi-Fi API: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html
- Espressif WebServer example: https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer/examples/HelloServer
- HUB75 library: https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA
- Waveshare panel: https://docs.waveshare.com/RGB-Matrix-Px-64x32

Prepared 2026-10-03. No real Wi-Fi credentials are included.
