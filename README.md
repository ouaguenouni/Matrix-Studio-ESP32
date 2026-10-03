# Matrix Studio: ESP32 + Waveshare 64x32

A local browser interface hosted on your ESP32. Send text, resized still images,
pixel drawings, Pokémon sprites, animated GIFs, or browser-streamed scrolling text
to the panel. No cloud account
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
The browser interaction tests pass against simulated endpoints in local Chromium.
The live browser fetched the online catalogue, sent Haunter to the ESP32, and
cycled from Haunter to Gengar with acknowledged frame uploads. Physical colour
appearance still needs the viewer's confirmation.
Crystal GIF storage mounting, animated Haunter uploads, and cycling between
normal and shiny Haunter have also been verified through the live browser.

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
   pio run --target uploadfs
   pio device monitor
   ```

4. **Upload Filesystem Image** (`pio run --target uploadfs`) installs the 552 downloaded
   Crystal GIF sprites. Do this once after installing this version, and again if
   gallery assets change. `partitions.csv` reserves one 2.5 MB firmware slot for
   Wi-Fi, the panel and Bluetooth audio. Uploads use USB; OTA is not implemented.
   NVS and the 1.4 MB LittleFS retain their original offsets and sizes, preserving
   saved Wi-Fi credentials and installed GIFs when updating the firmware.
5. If multiple USB serial devices are connected, select the ESP32 explicitly with
   `pio run --target upload --upload-port /dev/cu.YOUR_PORT` and
   `pio device monitor --port /dev/cu.YOUR_PORT` (use the appropriate port on your OS).
   Run `pio device list` to see available ports.
6. If uploading waits at “Connecting…”, hold BOOT until writing starts, then release it.
7. The serial monitor uses **115200 baud**. Press the board's **EN/RST** button
   to view startup messages.

Firmware lives in `src/main.cpp`; headers live in `include/`. The build automatically
regenerates `include/web_ui.h` from `web/interface.html`, `web/codec.mjs`, and
`web/pokemon.mjs`, `web/gif.mjs`, `web/gallery.js`, `web/speaker.js`, and the bundled GIF decoder using
PlatformIO's own Python interpreter. The same build repacks original GIFs from
`assets/crystal/` into `data/` for the LittleFS upload; no external server is needed.

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
- **Image:** local PNG, JPEG, WebP, BMP, or animated GIF; fit, crop, or stretch.
  Images are resized on your phone/computer, then sent as 4096-byte pixel frames.
  GIFs animate in the preview. Click **Play GIF on panel** to stream the animation,
  or **Send to display** to send a still frame. Playback loops while the page stays
  visible; Stop, Clear, or switching tabs stops uploads.
- **GIFs:** an offline gallery of 552 downloaded Pokémon Crystal animations
  (276 normal and 276 shiny, including Unown A–Z). Search by English name or number,
  filter normal/shiny, Kanto/Johto/Unown, and browse pages of 24 thumbnails. Select
  a thumbnail, then **Play GIF on panel**. **Cycle GIF gallery on panel** animates
  every matching sprite across all pages, starting at the selection and wrapping
  around; **Seconds per Pokémon** controls when it changes sprites (2–60 seconds).
  The browser decodes transparency, frame disposal and delays, then sends one frame
  at a time. Frame delays have a 40 ms minimum and uploads can slow playback when
  Wi-Fi is busy. Sprite scaling uses one bounding box across the whole animation
  so the image stays aligned. Native GIF loop counts are ignored for continuous
  playback. The gallery needs no internet access after the filesystem is uploaded.
  **Play matching Pokémon cries on JBL** is enabled by default. With a speaker
  connected, the first accepted frame of each selected/cycled sprite queues its
  legacy cry once. GIF loops stay silent until the next Pokémon appears. Normal,
  shiny, and every Unown letter share their species cry. Previews and local GIF
  uploads stay silent. Muting, stopping, switching tabs or hiding the browser
  stops the current cry. GIFs still play if the JBL is disconnected.
- **Pokémon:** search the complete PokéAPI catalogue by English name or number
  (for example, `haunter` or `93`). Select **Load sprite**, then **Send to display**.
  Matching names appear in the list; Previous/Next browse in Pokédex order.
  Choose all Pokémon/forms or the first generation, set 2–60 seconds per sprite,
  and click **Start cycling on panel** to send sprites automatically, starting
  with the current selection and wrapping around the chosen catalogue.
  Keep this browser page visible and awake. Stop, switching content tabs, or hiding
  the page ends cycling; the last sent sprite remains. Unavailable front sprites
  are skipped during cycling. Internet access is needed to load new sprites.
  Catalogue and metadata are cached in the browser for seven days; images load
  on demand from PokéAPI's sprite repository, rather than occupying ESP32 flash.
  Transparent borders are cropped and resizing uses nearest-neighbour pixels.
- **Draw:** tap or drag on the preview to paint pixels; use the eraser as needed.
- **Send to display:** commits the preview. Editing alone does not update the panel.
- **Clear:** clears the preview; then press Send to clear the physical panel.
- **Brightness:** changes the physical panel immediately; initial value is 30/255.
- **Bluetooth speaker:** expand the JBL Go 4 card. Turn on the speaker and press
  its Bluetooth button, then **Scan for speakers**. Choose the JBL by name/address
  and click **Connect speaker**. Connection status comes from the ESP32's real
  Bluetooth Classic A2DP source, not the phone's Bluetooth. Connecting is silent.
  **Play a 2-second test sound** sends a quiet 440 Hz tone; **Speaker audio volume**
  scales both cries and that tone, independently of the JBL's hardware volume.
  **Disconnect** ends the link or
  cancels a scan/connection. Bluetooth starts only on Scan and remains initialized
  until reboot; connections are not restored automatically. No music source is
  included. Scanning enables Wi-Fi modem sleep, which the ESP32 requires for
  Wi-Fi/Bluetooth coexistence. Bluetooth and Wi-Fi share the radio, so GIF uploads
  may slow during audio playback or discovery. The Bluetooth source uses no
  panel pins or I2S.
  The browser serializes ESP32 requests, including gallery thumbnails. Firmware
  bounds Wi-Fi to two static RX and four dynamic RX/TX buffers, one Bluetooth
  audio peer, and 512-byte multipart upload chunks so those features can share
  RAM without reducing the panel's colour depth or double buffering.
- **Scrolling:** sends approximately eight frames per second, subject to Wi-Fi speed.
  Keep the browser page visible and awake. It stops if you leave the tab or close it.

The last sent frame remains while the ESP32 is powered, even if the browser closes.
Display content and brightness are **not** restored after power loss; Wi-Fi settings are.
Videos, saved playlists, and remote control from outside your network
are extension points, not included features. This interface has no user login;
clients on the same LAN or password-protected setup network can control it.

## Pin mapping

The table records the reported ribbon-wire connections and corrected firmware
LED channels. The direct RGB test rendered red, blue, green, so firmware swaps
green and blue on both panel halves. Wire colours identify the ribbon conductors;
they do not identify LED colours.

| Wire number | Wire colour | Firmware channel/signal | ESP32 printed label |
|---|---|---|---|
| 1 | Brown — outer edge | R1 | G25 |
| 2 | Red | B1 | G26 |
| 3 | Orange | G1 | G27 |
| 4 | Yellow | GND | GND |
| 5 | Green | R2 | G14 |
| 6 | Blue | B2 | G12 |
| 7 | Purple | G2 | G13 |
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
| `/api/frame` | POST | One multipart file named `frame`; optional `cry=1..251` field | Atomically replace all pixels and queue a matching cry after accepting the frame |
| `/api/brightness` | POST | URL-encoded `value=0..255` | Set brightness |
| `/api/wifi` | POST | URL-encoded `ssid` and `password` | Save credentials and connect |
| `/api/speaker` | GET | None | Speaker state and discovered audio devices |
| `/api/speaker/scan` | POST | None | Initialize Classic Bluetooth and scan for 12.8 seconds |
| `/api/speaker/connect` | POST | URL-encoded discovered `address` | Connect to an A2DP speaker |
| `/api/speaker/disconnect` | POST | None | Cancel discovery/connection or disconnect |
| `/api/speaker/test` | POST | None | Send a two-second 440 Hz test tone |
| `/api/speaker/volume` | POST | URL-encoded `value=0..100` | Set cry/test sound volume |
| `/api/speaker/cry/stop` | POST | None | Stop the current Pokémon cry without disconnecting |
| `/sprites/crystal.json` | GET | None | Downloaded Crystal GIF catalogue |
| `/sprites/crystal/{normal,shiny}/{number}.gif` | GET | None | Original GIF, served with gzip content encoding |

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

To edit the embedded interface, modify `web/interface.html`, `web/codec.mjs`, or
`web/pokemon.mjs`, `web/gif.mjs`, or `web/gallery.js`,
then build and upload the firmware again. PlatformIO regenerates the embedded UI
automatically; you can also run `python3 tools/build_ui.py` manually.
`web/preview.html` is the generated UI for local inspection; a normal local preview
does not control hardware because it has no ESP32 server behind it.

## Verification and troubleshooting

Run `node tests/codec_test.mjs` and compile/run `tests/frame_protocol_test.cpp` with
a standard C++ compiler. `node tests/pokemon_test.mjs` checks catalogue/search,
sprite caching, and cropping. `tests/browser_test.cjs` and
`tests/pokemon_browser_test.cjs` run the browser interface against simulated
endpoints using Playwright, including sprite playback and cancellation. These
do not replace hardware testing.

`node tests/gif_test.mjs` compares every archived GIF's composited frames against
Pillow reference hashes in `tests/fixtures/crystal_frames.json`, and checks disposal,
timing and gallery filters. `tests/gif_browser_test.cjs` tests animated preview,
multiple changing RGB565 frame uploads, cycling, local GIF uploads, cancellation
and error handling. The small MIT-licensed omggif decoder is bundled locally with
its license retained in `web/vendor/`.

Original GIFs are in `assets/crystal/normal/` and `assets/crystal/shiny/`.
Run `python3 tools/prepare_crystal.py` to regenerate the compressed pack manually.
The roughly 1.1 MB pack plus catalogue fits the default 1.4 MB filesystem.
The catalogue is also served compressed (about 6 KB) to keep loading fast when
Bluetooth shares the Wi-Fi radio.

All 251 legacy Pokémon cries are downloaded from the pinned
[PokéAPI cries archive](https://github.com/PokeAPI/cries) in `assets/cries/legacy/`.
`assets/cries/adpcm/` contains 8 kHz mono IMA ADPCM blocks. The normal offline
build concatenates them and embeds the roughly 840 KB pack in firmware flash;
no LittleFS space is needed for audio. The existing 2.5 MB app slot is nearly
full (about 99%), so future additions may require changing storage/compression.
The Bluetooth callback decodes from flash and interpolates to 44.1 kHz stereo
without allocating a full cry buffer. It feeds a short silence tail before
suspending A2DP so the queued end of the cry can finish transmitting.
The panel settings and filesystem layout
remain unchanged. Bluetooth speaker buffering can add a short delay to audible
output; the image and cry cue are submitted together, not synchronized to each
individual GIF animation frame.

`python3 tools/prepare_cries.py --download --convert` explicitly downloads and
converts audio using curl and ffmpeg. Normal builds do not require ffmpeg or
internet access. Original/converted SHA-256 hashes are in the audio catalogue.
`python3 tests/cry_audio_test.py` compares all 251 firmware decodes against
FFmpeg with address/undefined-behavior sanitizers, and checks interpolation,
replacement and end-of-stream. `tests/cry_browser_test.cjs` verifies first-frame
species matching, once-per-appearance playback, cycling, shiny/Unown mapping,
mute, cancellation/restart ordering, disconnected fallback, and a maximum of
one simultaneous ESP32 request, including thumbnails and polling.

`tests/speaker_browser_test.cjs` checks discovery results, distinct speakers with
the same name, asynchronous connection status, disconnect, test tone/volume
controls, request errors and mobile layout against simulated endpoints. Actual
JBL pairing and audible output require the speaker to be available in pairing mode.
Hardware checks verified Bluetooth startup, discovery and an A2DP connection to
the JBL Go 4. Haunter's first-frame cry completed through the A2DP start/suspend
acknowledgements while GIF uploads continued; listening confirmed the full cry
after the memory and audio-tail fixes. Repeated playback stayed responsive with
stable free memory and no restarts during the live test.

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
the corresponding signal wires and panel hardware: G1 → GPIO27, B1 → GPIO26,
G2 → GPIO13, B2 → GPIO12. Turn off power before reseating wires.

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
- Pokémon catalogue API: https://pokeapi.co/docs/v2
- Pokémon sprite repository: https://github.com/PokeAPI/sprites
- Crystal GIF archive: https://bluemoonfalls.com/pages/general/crystal-gif-archive
- GIF decoder (MIT): https://github.com/deanm/omggif

Prepared 2026-10-03. No real Wi-Fi credentials are included.
