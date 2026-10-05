"""Place exported geometry in front-to-back assembly coordinates (millimetres)."""
import json
from pathlib import Path
import numpy as np
import trimesh

ROOT = Path(__file__).resolve().parent
MANIFEST = json.loads((ROOT / 'manifest.json').read_text())
P = MANIFEST['parameters']


def load(relative):
    return trimesh.load_mesh(ROOT / relative)


def moved(mesh, offset=(0, 0, 0), matrix=None):
    result = mesh.copy()
    if matrix is not None:
        result.apply_transform(matrix)
    result.apply_translation(offset)
    return result


def box(size, center):
    return moved(trimesh.creation.box(size), center)


def back(mesh, explode=0):
    matrix = np.diag([1., 1., -1., 1.])
    return moved(mesh, (0, 0, P['rear_z'] + explode), matrix)


def tie_loop(size, center):
    """Simplified 2.5 mm wide, 0.8 mm thick tie; buckle omitted."""
    outer = box([*size, 2.5], center)
    inner = box([size[0] - 1.6, size[1] - 1.6, 4.5], center)
    return trimesh.boolean.difference([outer, inner], engine='manifold')


def assembly(explode=0):
    result = {'frame': moved(load('stl/frame.stl'), (0, 0, P['frame_depth']),
                             trimesh.transformations.rotation_matrix(np.pi, [1, 0, 0])),
              'rear': back(load('stl/rear.stl'), explode),
              'panel': load('reference/panel_mock.stl')}
    for name in ['pcb', 'usb', 'power']:
        result[name] = back(load(f'reference/{name}_mock.stl'), explode)
    result['pcb_tie'] = back(tie_loop([7.8, P['pcb_w'] + 3.8],
                                      [P['pcb_x'] + .8, 0, P['pcb_stop'] + P['pcb_l'] + 1.45]), explode)
    for i, depth in enumerate([12.65, 27.65]):
        result[f'power_tie{i}'] = back(tie_loop([P['power_w'] + 8.4, P['power_h'] + 2],
                                               [P['power_x'], 0, depth]), explode)
    # Rounded acrylic comes from the same 2D profile, matching the SVG outline.
    lens = trimesh.creation.box([P['acrylic_w'] - 2, P['acrylic_h'] - 2, P['acrylic_t']])
    corners = [moved(trimesh.creation.cylinder(radius=1, height=P['acrylic_t'], sections=40),
                     (x * (P['acrylic_w'] / 2 - 1), y * (P['acrylic_h'] / 2 - 1), 0))
               for x in [-1, 1] for y in [-1, 1]]
    result['acrylic'] = moved(trimesh.util.concatenate([lens, *corners]).convex_hull,
                              (0, 0, .3 + P['acrylic_t'] / 2 - explode / 3))
    return result
