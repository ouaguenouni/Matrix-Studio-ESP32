#!/usr/bin/env python3
"""Embed the editable browser UI in the firmware; no build dependencies."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
codec = (root / 'web/codec.mjs').read_text().replace('export function ', 'function ')
decoder = (root / 'web/vendor/omggif.mjs').read_text().replace('export {GifReader, GifWriter};', '')
gif = '\n'.join(line for line in (root / 'web/gif.mjs').read_text().splitlines()
                if not line.startswith('import ')).replace('export function ', 'function ')
player = (root / 'web/gif-player.js').read_text()
html = (root / 'web/interface.html').read_text().replace('/*__CODEC__*/', codec)
html = html.replace('/*__GIF__*/', decoder + '\n' + gif).replace('/*__GIF_PLAYER__*/', player)
assert ')MATRIX_UI"' not in html
(root / 'include/web_ui.h').write_text(
    '#pragma once\n#include <Arduino.h>\n'
    'static const char WEB_UI[] PROGMEM = R"MATRIX_UI(' + html + ')MATRIX_UI";\n')
(root / 'web/preview.html').write_text(html)
print(f'Embedded {len(html.encode())} bytes of HTML; wrote web/preview.html.')
