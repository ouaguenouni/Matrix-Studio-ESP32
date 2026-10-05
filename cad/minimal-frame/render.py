#!/usr/bin/env python3
"""Render actual exported STL geometry. Hardware and LED pixels are illustrative."""
import numpy as np
import trimesh
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from geometry import ROOT, P, assembly, back, box, load, moved
from raster import rasterize

BG = '#f3f1ea'
INK = '#20363c'
COLORS = {'frame': '#41575a', 'rear': '#41575a', 'panel': '#101a21',
          'pcb': '#4b9d7b', 'usb': '#c5cace', 'power': '#343d3f', 'acrylic': '#b9dadd',
          'pcb_tie': '#c5a66d', 'power_tie0': '#c5a66d', 'power_tie1': '#c5a66d'}


def picture(items, camera, pixels=False, width=1250, height=1000):
    triangles, colours = [], []
    light = np.array(camera, dtype=float) + [-.4, .8, 0]
    light /= np.linalg.norm(light)

    def append(mesh, color):
        shade = np.clip(.70 + .28 * (mesh.face_normals @ light), .35, 1)
        rgb = np.array(matplotlib.colors.to_rgb(color))
        triangles.append(mesh.triangles)
        colours.append(rgb[None, :] * shade[:, None])

    for name, mesh in items.items():
        if name != 'acrylic':  # The optical sheet is clear, not a simulated opaque surface.
            append(mesh, COLORS.get(name, '#829892'))
    if pixels:
        for row in range(32):
            for column in range(64):
                bar_height = int(7 + 5 * np.sin(column * .28) + 4 * np.sin(column * .63))
                on = 31 - bar_height <= row < 29
                append(box([1.35, 1.35, .15], [-78.75 + column * 2.5, 38.75 - row * 2.5,
                                               P['panel_front'] - .1]), '#69bfa6' if on else '#26343b')
    return rasterize(np.concatenate(triangles), np.concatenate(colours), camera, width, height)


def main():
    plt.rcParams.update({'font.family': 'DejaVu Sans', 'text.color': INK, 'axes.titlecolor': INK})
    parts = assembly()
    fig, axes = plt.subplots(1, 2, figsize=(14, 7.5), facecolor=BG)
    for ax, camera, title, subtitle in [
        (axes[0], (.6, .4, -1.5), 'Close-fitting front frame', '160 × 80 mm panel · 1 mm acrylic'),
        (axes[1], (-.55, .4, 1.6), 'Closed, ventilated rear', 'Existing DC inlet + ESP32 USB-C face backwards')]:
        ax.imshow(picture(parts, camera, pixels=True))
        ax.axis('off')
        ax.set_title(title, fontsize=17, fontweight='bold')
        ax.text(.5, -.02, subtitle, transform=ax.transAxes, ha='center', fontsize=10)
    fig.suptitle('MINIMAL FRAME / TWO PRINTED PARTS', fontsize=23, fontweight='bold', y=.96)
    fig.text(.5, .87, '176 × 96 × 108 mm  •  1 frame + 1 rear shell  •  Integral board guides', ha='center', fontsize=12)
    fig.text(.06, .035, 'Actual STL geometry. Hardware dimensions provisional; physical fit and print time unverified.', fontsize=10, color='#805130')
    fig.subplots_adjust(top=.78, bottom=.10, left=.02, right=.98, wspace=.04)
    fig.savefig(ROOT / 'preview.png', dpi=150, facecolor=BG)
    plt.close(fig)

    # Remove only shell walls on the viewer side to expose the real internal fixtures.
    section = {key: value for key, value in parts.items() if key not in ['frame', 'panel', 'acrylic']}
    cutouts = [box([220, 30, 150], [0, 42, 60]), box([30, 120, 150], [-88, 0, 60])]
    section['rear'] = trimesh.boolean.difference([section['rear'], *cutouts], engine='manifold')
    fig, axes = plt.subplots(1, 2, figsize=(14, 7.5), facecolor=BG)
    axes[0].imshow(picture(section, (-.65, .8, -1.2)))
    axes[0].axis('off')
    axes[0].set_title('Rear shell — two walls cut away', fontsize=16, fontweight='bold')
    axes[0].text(.5, -.02, 'Left: vertical PCB guides  /  Right: DC cradle', transform=axes[0].transAxes, ha='center', fontsize=10)
    # Side section is an explicitly schematic dimensioned view of the same positions.
    ax = axes[1]
    from matplotlib.patches import Rectangle
    ax.add_patch(Rectangle((0, -48), P['frame_depth'], 96, fill=False, linewidth=2, edgecolor='#41575a'))
    ax.add_patch(Rectangle((P['frame_depth'], -48), P['leg_h'], 96, fill=False, linewidth=2, edgecolor='#41575a'))
    ax.add_patch(Rectangle((P['panel_front'], -40), P['panel_depth'], 80, color='#15222b'))
    ax.add_patch(Rectangle((.3, -42), 1, 84, color='#9bc9d0'))
    ax.add_patch(Rectangle((P['panel_back'], -24), 20, 48, color='#d9b473', alpha=.35))
    ax.add_patch(Rectangle((P['rear_z'] - P['pcb_stop'] - P['pcb_l'], -14.5), P['pcb_l'], 29, color='#4b9d7b'))
    ax.add_patch(Rectangle((P['rear_z'] - P['pcb_stop'] - 5.5, -4.5), 7, 9, color='#abb8bb'))
    ax.annotate('', (0, -58), (108, -58), arrowprops={'arrowstyle': '<->', 'color': INK})
    ax.text(54, -65, '108 mm overall depth', ha='center', fontsize=11)
    ax.annotate('Panel', (13, 15), (13, 56), ha='center', fontsize=11,
                arrowprops={'arrowstyle': '-', 'color': INK})
    ax.annotate('ESP32 on edge', (74, 12), (76, 56), ha='center', fontsize=11,
                arrowprops={'arrowstyle': '-', 'color': INK})
    ax.text(31.8, -31, 'Wiring\nspace', ha='center', fontsize=10)
    ax.annotate('USB-C', (108, 0), (119, 25), fontsize=11,
                arrowprops={'arrowstyle': '->', 'color': INK})
    ax.text(0, -76, 'Front', ha='center', fontsize=11)
    ax.text(108, -76, 'Back', ha='center', fontsize=11)
    ax.set_aspect('equal'); ax.set_xlim(-12, 145); ax.set_ylim(-85, 70); ax.axis('off')
    ax.set_title('Why the rear has this depth', fontsize=16, fontweight='bold')
    fig.suptitle('INTEGRAL FIXTURES / REAR-FACING SOCKETS', fontsize=22, fontweight='bold', y=.96)
    fig.text(.5, .87, 'One tie retains the PCB; two ties secure the existing power adapter.', ha='center', fontsize=12)
    fig.text(.06, .035, 'Actual fixtures; simplified tie bands; wiring omitted. Side section is schematic. PCB envelope: 58 × 29 × 1.6 mm.', fontsize=10, color='#805130')
    fig.subplots_adjust(top=.78, bottom=.10, left=.02, right=.98, wspace=.10)
    fig.savefig(ROOT / 'inside.png', dpi=150, facecolor=BG)
    plt.close(fig)
    print('preview.png and inside.png', flush=True)


if __name__ == '__main__':
    main()
