#!/usr/bin/env python3
"""Package checked STL kits and the complete offline review handoff."""
import hashlib
import json
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent

def check_report():
    report = json.loads((ROOT/'validation.json').read_text())
    if not report.get('passed'):
        raise SystemExit('Validation failed. Run build.py and validate.py before packaging.')
    hashes = report.get('checked_files_sha256',{})
    if not hashes:
        raise SystemExit('Missing checked-file hashes: rerun validate.py.')
    manifest = json.loads((ROOT/'manifest.json').read_text())
    expected = {'family.scad', 'manifest.json', 'geometry.py', 'validate.py'}
    for v in manifest['variants']:
        folder = v['name']
        expected.update(f'{folder}/stl/{part}.stl' for part in manifest['quantities'])
        expected.update([f'{folder}/model.scad',f'{folder}/acrylic-outline.svg'])
    if set(hashes) != expected:
        raise SystemExit('Checked-file inventory disagrees with manifest: rerun validate.py.')
    for name,digest in hashes.items():
        p = ROOT/name
        if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest() != digest:
            raise SystemExit(f'{name} changed since validation: regenerate and revalidate.')
    return manifest

def write_zip(path, paths, generated=None):
    temporary = path.with_suffix('.pending.zip')
    with zipfile.ZipFile(temporary,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for p in sorted(paths):
            z.write(p,str(p.relative_to(ROOT)))
        for name,body in (generated or {}).items():
            z.writestr(name,body)
    with zipfile.ZipFile(temporary) as z:
        if z.testzip():
            raise SystemExit(f'Archive integrity failed: {path}')
    temporary.replace(path)
    print(path.relative_to(ROOT),path.stat().st_size,'bytes',flush=True)

def main():
    manifest = check_report()
    required = ['README.md','PRINT-GUIDE.md','FABLAB-FR.md','AUDIT.md','viewer.html','comparison.png','vault-service.png']
    for name in required:
        if not (ROOT/name).is_file():
            raise SystemExit(f'Missing deliverable: {name}')
    (ROOT/'kits').mkdir(exist_ok=True)
    common = [ROOT/n for n in ['family.scad','manifest.json','validation.json','PRINT-GUIDE.md','FABLAB-FR.md','AUDIT.md']]
    common += list((ROOT/'evidence').glob('*'))
    for v in manifest['variants']:
        folder = ROOT/v['name']
        paths = common + [p for p in folder.rglob('*') if p.is_file() and p.suffix != '.log']
        readme = f"# {v['name']} — enclosure prototype kit\n\n"
        readme += 'Units: mm. Physical fit remains unverified. Read [PRINT-GUIDE.md](PRINT-GUIDE.md) before printing.\n\n'
        readme += f"Case: {v['width']} × {v['height']} × {v['overall_depth']} mm; acrylic {v['lens'][0]} × {v['lens'][1]} × {v['lens'][2]} mm.\n\n"
        readme += f"[Front/rear preview]({v['name']}/preview.png) · [Exploded view]({v['name']}/exploded.png) · [Editable model]({v['name']}/model.scad)\n\n"
        readme += 'Keep folder structure intact so the model finds `../family.scad`. Print individual STLs only.\n\n'
        readme += '| Part | Quantity |\n| --- | ---: |\n'
        readme += ''.join(f'| {part}.stl | {q} |\n' for part,q in manifest['quantities'].items())
        readme += '\n`ports_usb_c` is an optional replacement for `ports`. Test the coupon first.\n'
        write_zip(ROOT/'kits'/f"{v['name']}.zip",paths,{'README.md':readme})
    root_files = [p for p in ROOT.iterdir() if p.is_file() and p.suffix in {'.scad','.py','.md','.html','.png','.json','.ini','.txt'}]
    all_files = root_files + list((ROOT/'evidence').glob('*')) + list((ROOT/'kits').glob('*.zip'))
    for v in manifest['variants']:
        all_files.extend(p for p in (ROOT/v['name']).rglob('*') if p.is_file() and p.suffix != '.log')
    write_zip(ROOT/'five-enclosure-kits.zip',all_files)

if __name__ == '__main__':
    main()
