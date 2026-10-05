#!/usr/bin/env python3
"""Export the two-part enclosure, optional fit gauges and reference geometry."""
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import trimesh

ROOT = Path(__file__).resolve().parent


def main():
    source = ROOT / 'model.scad'
    text = source.read_text()
    values = {k: float(v) for k, v in re.findall(r'^([a-z_]+)\s*=\s*([\d.]+)\s*;', text, re.M)}
    values['panel_front'] = values['acrylic_t'] + .3 + values['front_lip'] + 2.9
    values['panel_back'] = values['panel_front'] + values['panel_depth']
    values['frame_depth'] = values['panel_back'] + 3
    values['leg_h'] = values['rear_z'] - values['frame_depth']
    values['cx'] = values['outer_w'] / 2 - 4
    values['cy'] = values['outer_h'] / 2 - 4
    # power_x is a negative literal and is deliberately parsed separately.
    values['power_x'] = float(re.search(r'^power_x\s*=\s*(-?[\d.]+)', text, re.M)[1])
    outputs = {p: ROOT / 'stl' / f'{p}.stl' for p in ['frame', 'rear']}
    outputs.update({p: ROOT / 'fit-tests' / f'{p}.stl' for p in ['fit_corner', 'board_gauge']})
    outputs.update({p: ROOT / 'reference' / f'{p}.stl'
                    for p in ['panel_mock', 'pcb_mock', 'usb_mock', 'power_mock']})
    outputs['acrylic_template'] = ROOT / 'acrylic-outline.svg'
    cmd = shlex.split(os.environ.get('OPENSCAD', 'openscad'))
    hashes = {}
    removed_faces = {}
    for part, output in outputs.items():
        output.parent.mkdir(parents=True, exist_ok=True)
        pending = output.with_name(output.stem + '.pending' + output.suffix)
        result = subprocess.run(cmd + (['--export-format', 'binstl'] if output.suffix == '.stl' else [])
                                + ['-o', str(pending), '-D', f'part="{part}"', str(source)],
                                capture_output=True, text=True, timeout=180)
        log = result.stdout + result.stderr
        if result.returncode or 'ERROR:' in log or 'WARNING:' in log or not pending.exists():
            pending.unlink(missing_ok=True)
            raise RuntimeError(f'{part}: {log}')
        if output.suffix == '.stl':
            # Binary STL quantization can leave zero-area triangles on coplanar
            # Manifold seams. Remove only collapsed faces; never fill mesh holes.
            mesh = trimesh.load_mesh(pending)
            keep = mesh.nondegenerate_faces(height=1e-8)
            removed_faces[part] = int((~keep).sum())
            mesh.update_faces(keep)
            mesh.remove_unreferenced_vertices()
            if not mesh.is_watertight or not mesh.is_winding_consistent or mesh.volume <= 0:
                raise RuntimeError(f'{part}: exported mesh is not a closed, oriented volume')
            mesh.export(pending, file_type='stl')
        pending.replace(output)
        hashes[str(output.relative_to(ROOT))] = hashlib.sha256(output.read_bytes()).hexdigest()
        print(part, flush=True)
    manifest = {'name': 'Minimal frame / enclosed rear', 'revision': 1, 'units': 'mm',
                'installed_prints': {'frame': 1, 'rear': 1}, 'optional_fit_tests': ['fit_corner', 'board_gauge'],
                'physical_fit_verified': False, 'slicer_time_verified': False,
                'parameters': values, 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
                'collapsed_export_faces_removed': removed_faces,
                'export_sha256': hashes}
    (ROOT / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')


if __name__ == '__main__':
    main()
