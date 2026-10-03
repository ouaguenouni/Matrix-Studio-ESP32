#!/usr/bin/env python3
"""Verify firmware ADPCM decoding against FFmpeg, with ASan/UBSan enabled."""
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def chunk(name, data):
    return name + struct.pack('<I', len(data)) + data + (b'\0' if len(data) % 2 else b'')


with tempfile.TemporaryDirectory(prefix='matrix-cry-test-') as directory:
    target = Path(directory)
    catalogue = json.loads((ROOT / 'assets/cries/catalogue.json').read_text())

    def reference(entry):
        content = (ROOT / 'assets/cries/adpcm' / f'{entry["id"]}.ima').read_bytes()
        fmt = struct.pack('<HHIIHHHH', 17, 1, 8000, 8000 * 256 // 505, 256, 4, 2, 505)
        body = b'WAVE' + chunk(b'fmt ', fmt) + chunk(b'fact', struct.pack('<I', entry['samples'])) + chunk(b'data', content)
        wav = target / f'{entry["id"]}.wav'
        wav.write_bytes(b'RIFF' + struct.pack('<I', len(body)) + body)
        subprocess.run(['ffmpeg', '-v', 'error', '-i', str(wav), '-f', 's16le',
                        str(target / f'{entry["id"]}.pcm')], check=True)

    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(reference, catalogue['entries']))
    executable = target / 'cry-test'
    subprocess.run(['c++', '-std=c++11', '-O1', '-g', '-fsanitize=address,undefined',
                    str(ROOT / 'tests/cry_audio_test.cpp'), '-o', str(executable)], check=True)
    subprocess.run([str(executable), str(target)], cwd=ROOT, check=True)
