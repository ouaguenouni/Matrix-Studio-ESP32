#!/usr/bin/env python3
"""Check exported meshes, mating parts, hardware envelopes and loading paths."""
import hashlib
import json
from itertools import combinations
import numpy as np
import trimesh
from geometry import ROOT, MANIFEST, P, assembly, back, box, load, moved


def intersection_volume(a, b):
    result = trimesh.boolean.intersection([a, b], engine='manifold')
    if not len(result.faces):
        return 0.
    # Touching faces can produce a zero-volume result whose center of mass is
    # undefined. Integrate volume directly without asking for mass properties.
    triangles = result.triangles - result.vertices.mean(axis=0)
    return abs(np.einsum('ij,ij->i', triangles[:, 0],
                         np.cross(triangles[:, 1], triangles[:, 2])).sum() / 6)


def main():
    failures = []
    checks = {}

    def check(name, okay, detail):
        checks[name] = {'pass': bool(okay), 'detail': detail}
        if not okay:
            failures.append(name)

    source_hash = hashlib.sha256((ROOT / 'model.scad').read_bytes()).hexdigest()
    check('current_source', source_hash == MANIFEST['source_sha256'], source_hash)
    for relative, expected in MANIFEST['export_sha256'].items():
        check('current_export/' + relative,
              hashlib.sha256((ROOT / relative).read_bytes()).hexdigest() == expected, expected)
    dimensions = {}
    total_volume = 0
    for part, folder in [('frame', 'stl'), ('rear', 'stl'), ('fit_corner', 'fit-tests'), ('board_gauge', 'fit-tests')]:
        mesh = load(f'{folder}/{part}.stl')
        count = len(mesh.split(only_watertight=False))
        check(part + '/solid', mesh.is_watertight and mesh.is_winding_consistent and mesh.volume > 0 and count == 1,
              {'watertight': mesh.is_watertight, 'connected_solids': count})
        check(part + '/bed', abs(mesh.bounds[0, 2]) < .001 and max(mesh.extents[:2]) + 10 <= 220,
              'Z=0; fits a 220 x 220 mm bed with 5 mm brim on each side')
        dimensions[part] = {'size_mm': np.round(mesh.extents, 3).tolist(),
                            'solid_volume_cm3': round(mesh.volume / 1000, 3)}
        if folder == 'stl':
            total_volume += mesh.volume
            wanted = [P['outer_w'], P['outer_h'], P['frame_depth'] if part == 'frame' else P['leg_h']]
            check(part + '/dimensions', np.allclose(mesh.extents, wanted, atol=.002), wanted)

    items = assembly()
    for a, b in combinations(items, 2):
        volume = intersection_volume(items[a], items[b])
        check('assembly/' + a + '/' + b, volume < .02, {'overlap_mm3': round(volume, 6)})

    # Mock plug bodies extend 20 mm beyond the back and enter 1.5 mm into it.
    usb_center_x = P['pcb_x'] + P['pcb_t'] + 1.6
    keepouts = {
        'usb_plug_10x14': box([10, 14, 21.5], [usb_center_x, 0, P['rear_z'] + 9.25]),
        'dc_plug_d12': moved(trimesh.creation.cylinder(radius=6, height=21.5, sections=40),
                              [P['power_x'], 0, P['rear_z'] + 9.25]),
        'panel_wiring_20mm': box([148, 48, 20], [0, 0, P['panel_back'] + 10]),
        'gpio_wiring_25mm': back(box([25, P['pcb_w'] - 5, P['pcb_l'] - 14],
                                    [P['pcb_x'] + P['pcb_t'] + 12.5, 0,
                                     P['pcb_stop'] + 8 + (P['pcb_l'] - 14) / 2])),
        'pcb_back_8mm': back(box([8, P['pcb_w'] - 4, P['pcb_l'] - 10],
                                  [P['pcb_x'] - 4, 0, P['pcb_stop'] + P['pcb_l'] / 2])),
    }
    for label, space in keepouts.items():
        for part in ['frame', 'rear']:
            volume = intersection_volume(items[part], space)
            check('access/' + label + '/' + part, volume < .02, {'overlap_mm3': round(volume, 6)})

    # Front-loading the display must remain possible with all mounting ears integral.
    panel_sweep = box([P['panel_w'], P['panel_h'], P['panel_back'] + 20],
                       [0, 0, (P['panel_back'] - 20) / 2])
    volume = intersection_volume(items['frame'], panel_sweep)
    check('panel_front_loading', volume < .02, {'overlap_mm3': round(volume, 6)})

    # Long swept boxes test the entire insertion path into the rear guides/cradle.
    pcb_sweep = box([P['pcb_t'], P['pcb_w'], P['leg_h'] + 20 - P['pcb_stop']],
                     [P['pcb_x'] + P['pcb_t'] / 2, 0, (P['pcb_stop'] + P['leg_h'] + 20) / 2])
    power_sweep = box([P['power_w'], P['power_h'], P['leg_h'] + 20 - P['rear_t']],
                       [P['power_x'], 0, (P['rear_t'] + P['leg_h'] + 20) / 2])
    rear_print = load('stl/rear.stl')
    for label, space in [('pcb', pcb_sweep), ('power', power_sweep)]:
        volume = intersection_volume(rear_print, space)
        check(label + '_insertion', volume < .02, {'overlap_mm3': round(volume, 6)})

    # Head envelopes: front closing screws and rear panel screws need working space.
    for sx in [-1, 1]:
        for sy in [-1, 1]:
            front_head = moved(trimesh.creation.cylinder(radius=2.8, height=2.4, sections=32),
                                [sx * P['cx'], sy * P['cy'], -1.2])
            panel_head = moved(trimesh.creation.cylinder(radius=2.8, height=2.4, sections=32),
                                [sx * 62.5, sy * 32.5, P['frame_depth'] + 1.2])
            for name, head in [('front', front_head), ('panel', panel_head)]:
                volume = sum(intersection_volume(head, m) for m in items.values())
                check(f'fastener_head/{name}/{sx}/{sy}', volume < .02, {'overlap_mm3': round(volume, 6)})
    engagement = 30 - P['frame_depth']
    check('closure_screw_engagement', 5 <= engagement <= 10,
          {'M3x30_engagement_mm': round(engagement, 3), 'pilot_depth_mm': 11})

    report = {'passed': not failures, 'checks_count': len(checks), 'failures': failures,
              'source_sha256': source_hash, 'installed_prints': 2, 'parts': dimensions,
              'total_solid_volume_cm3': round(total_volume / 1000, 3),
              'solid_PLA_mass_at_1_24_g_cm3': round(total_volume / 1000 * 1.24, 1),
              'physical_fit_verified': False, 'slicer_time_verified': False,
              'limitations': ['Board, USB socket and DC adapter dimensions are provisional.',
                              'Clearances use simple hardware and plug envelopes, not measured component CAD.',
                              'Printability, strength, wire routing, thermal performance and hardware fit need physical checks.',
                              'Solid volume/mass is not slicer filament consumption or a print-time estimate.'],
              'checks': checks}
    (ROOT / 'validation.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ['passed', 'checks_count', 'failures', 'parts', 'total_solid_volume_cm3']}, indent=2))
    if failures:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
