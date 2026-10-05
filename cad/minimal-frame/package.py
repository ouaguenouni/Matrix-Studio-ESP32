#!/usr/bin/env python3
"""Package the validated source and fabrication files without caches or old exports."""
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

ROOT = Path(__file__).resolve().parent
manifest = json.loads((ROOT / 'manifest.json').read_text())
report = json.loads((ROOT / 'validation.json').read_text())
source_hash = hashlib.sha256((ROOT / 'model.scad').read_bytes()).hexdigest()
assert report['passed'] and report['source_sha256'] == manifest['source_sha256'] == source_hash
for name, expected in manifest['export_sha256'].items():
    assert hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == expected, name
files = [ROOT / name for name in ['README.md', 'FABLAB-FR.md', 'model.scad', 'build.py',
         'geometry.py', 'validate.py', 'render.py', 'raster.py', 'package.py', 'requirements.txt',
         'manifest.json', 'validation.json', 'preview.png', 'inside.png', 'acrylic-outline.svg']]
for folder in ['stl', 'fit-tests', 'reference']:
    files.extend(sorted((ROOT / folder).glob('*.stl')))
target = ROOT / 'minimal-frame.zip'
with ZipFile(target, 'w', compression=ZIP_DEFLATED) as archive:
    for path in files:
        archive.write(path, Path('minimal-frame') / path.relative_to(ROOT))
with ZipFile(target) as archive:
    assert archive.testzip() is None
print(f'{target.name}: {len(files)} files, {target.stat().st_size:,} bytes')
