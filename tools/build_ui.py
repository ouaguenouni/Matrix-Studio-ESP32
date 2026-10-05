#!/usr/bin/env python3
"""Embed the editable browser UI in the firmware; no build dependencies."""
from pathlib import Path
import runpy

root = Path(__file__).resolve().parents[1]
runpy.run_path(str(root / 'tools/build_display.py'))
codec = (root / 'web/codec.mjs').read_text().replace('export function ', 'function ')
decoder = (root / 'web/vendor/omggif.mjs').read_text().replace('export {GifReader, GifWriter};', '')
gif = '\n'.join(line for line in (root / 'web/gif.mjs').read_text().splitlines()
                if not line.startswith('import ')).replace('export function ', 'function ')
player = (root / 'web/gif-player.js').read_text()

def embed_module(name):
    lines = [line for line in (root / name).read_text().splitlines() if not line.startswith('import ')]
    return '\n'.join(lines).replace('export function ', 'function ').replace('export const ', 'const ')

info = '\n'.join(embed_module(name) for name in ('web/weather.mjs', 'web/clock.mjs', 'web/youtube.mjs', 'web/display-data.mjs', 'web/display.mjs', 'web/gift.mjs'))
modules = '\n'.join((root / name).read_text() for name in ('web/weather.js', 'web/clock.js', 'web/youtube.js', 'web/gift.js'))
html = (root / 'web/interface.html').read_text().replace('/*__CODEC__*/', codec)
html = html.replace('/*__GIF__*/', decoder + '\n' + gif).replace('/*__GIF_PLAYER__*/', player)
html = html.replace('/*__INFO__*/', info).replace('/*__MODULES__*/', modules)
assert ')MATRIX_UI"' not in html
(root / 'include/web_ui.h').write_text(
    '#pragma once\n#include <Arduino.h>\n'
    'static const char WEB_UI[] PROGMEM = R"MATRIX_UI(' + html + ')MATRIX_UI";\n')
(root / 'web/preview.html').write_text(html)
print(f'Embedded {len(html.encode())} bytes of HTML; wrote web/preview.html.')
