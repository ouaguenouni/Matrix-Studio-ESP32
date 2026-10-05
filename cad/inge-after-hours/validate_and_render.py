#!/usr/bin/env python3
"""Validate exported prototype meshes and render their actual geometry.
Requires trimesh, scipy and matplotlib. Does not certify assembled hardware fit.
"""
from pathlib import Path
import json
import numpy as np
import trimesh
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

ROOT = Path(__file__).resolve().parent
meshes = {}
report = {'status': 'prototype_geometry_only', 'units': 'mm',
          'hardware_fit_verified': False, 'printer_bed_assumption': [220, 220, 250],
          'parts': {}}
for path in sorted((ROOT / 'prototype-stl').glob('*.stl')):
    mesh = trimesh.load_mesh(path)
    assert mesh.is_watertight, f'{path.name}: open mesh'
    assert mesh.is_winding_consistent, f'{path.name}: inconsistent normals'
    assert mesh.volume > 0, f'{path.name}: nonpositive volume'
    assert mesh.body_count == 1, f'{path.name}: disconnected pieces'
    assert np.all(mesh.extents <= [220, 220, 250]), f'{path.name}: exceeds bed'
    assert abs(mesh.bounds[0][2]) < 0.01, f'{path.name}: not on print plane'
    meshes[path.stem] = mesh
    report['parts'][path.stem] = {
        'watertight': bool(mesh.is_watertight), 'consistent_winding': bool(mesh.is_winding_consistent),
        'connected_bodies': int(mesh.body_count),
        'dimensions_mm': mesh.extents.round(3).tolist(),
        'volume_mm3': round(float(mesh.volume), 2), 'triangles': len(mesh.faces),
        'fits_220mm_bed': True,
    }
assert len(meshes) == 8, 'Missing part mesh'
(ROOT / 'validation.json').write_text(json.dumps(report, indent=2) + '\n')

plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 10})
fig = plt.figure(figsize=(14, 9), facecolor='#f3f1ea')
fig.suptitle('INGE / AFTER HOURS', x=0.06, y=0.95, ha='left', fontsize=25, fontweight='bold', color='#14252b')
fig.text(.06,.905,'ENCLOSURE PROTOTYPE  /  160 × 80 mm panel  /  replaceable rear interfaces',fontsize=12,color='#526467')
ax = fig.add_axes([0.02,0.12,0.67,0.77], facecolor='#f3f1ea')
render_triangles=[]
render_colours=[]

def plot(mesh, offset=(0,0,0), color='#8c969b', rotate=None):
    m = mesh.copy()
    if rotate is not None:
        m.apply_transform(trimesh.transformations.rotation_matrix(rotate[0], rotate[1]))
    m.apply_translation(offset)
    triangles = m.triangles
    normals = m.face_normals
    light = np.array([-.4,-.5,.85]); light /= np.linalg.norm(light)
    shades = np.clip(.55+.4*(normals@light), .23, .95)
    rgb = np.array(matplotlib.colors.to_rgb(color))
    colors = np.clip(rgb[None,:]*shades[:,None],0,1)
    render_triangles.append(triangles)
    render_colours.append(colors)

def rasterize(triangles, colours, camera=(0.6,-1,0.8), width=1250, height=1150):
    # A depth buffer avoids mplot3d's per-face painter-order artifacts.
    forward=np.array(camera,dtype=float);forward/=np.linalg.norm(forward)
    right=np.cross([0,0,1],forward);right/=np.linalg.norm(right)
    up=np.cross(forward,right)
    projected=triangles@np.stack([right,up,forward],axis=1)
    flat=projected.reshape(-1,3)
    low=flat[:,:2].min(axis=0);high=flat[:,:2].max(axis=0)
    scale=min((width-60)/(high[0]-low[0]),(height-60)/(high[1]-low[1]))
    projected[:,:,0]=(projected[:,:,0]-(low[0]+high[0])/2)*scale+width/2
    projected[:,:,1]=height/2-(projected[:,:,1]-(low[1]+high[1])/2)*scale
    image=np.empty((height,width,3),dtype=np.uint8);image[:]=[243,241,234]
    depth=np.full((height,width),-np.inf)
    for tri,colour in zip(projected,colours):
        x0=max(0,int(np.floor(tri[:,0].min())));x1=min(width-1,int(np.ceil(tri[:,0].max())))
        y0=max(0,int(np.floor(tri[:,1].min())));y1=min(height-1,int(np.ceil(tri[:,1].max())))
        if x1<x0 or y1<y0: continue
        x,y=np.meshgrid(np.arange(x0,x1+1)+.5,np.arange(y0,y1+1)+.5)
        a,b,c=tri
        denom=(b[1]-c[1])*(a[0]-c[0])+(c[0]-b[0])*(a[1]-c[1])
        if abs(denom)<1e-8: continue
        u=((b[1]-c[1])*(x-c[0])+(c[0]-b[0])*(y-c[1]))/denom
        v=((c[1]-a[1])*(x-c[0])+(a[0]-c[0])*(y-c[1]))/denom
        w=1-u-v;z=u*a[2]+v*b[2]+w*c[2]
        region=depth[y0:y1+1,x0:x1+1];mask=(u>=0)&(v>=0)&(w>=0)&(z>region)
        region[mask]=z[mask];image[y0:y1+1,x0:x1+1][mask]=(colour*255).astype(np.uint8)
    return image

plot(meshes['shell'],color='#91a1a7')
plot(meshes['front_spacer'],(0,0,-14),color='#607079')
lens=trimesh.creation.box(extents=[164,84,2]);lens.apply_translation([0,0,-32]);plot(lens,color='#b5d7df')
plot(meshes['lid'],(0,0,103),color='#b2bfc3')
plot(meshes['carrier'],(39,11,84),color='#70b9ab',rotate=(np.pi,[1,0,0]))
plot(meshes['ports'],(-32,-24,134),color='#a1c7bb')
for x in [-84.4,84.4]:
    for y in [-22,22]:
        m=meshes['clip'].copy(); m.apply_translation([-2,0,0])
        if x<0: m.apply_transform(trimesh.transformations.rotation_matrix(np.pi,[0,0,1]))
        plot(m,(x,y,29),color='#73baaa')
ax.imshow(rasterize(np.concatenate(render_triangles),np.concatenate(render_colours)))
ax.set_axis_off()

labels = [
('01  Removable rear plate','Power jack + USB-C data extension\nMount dimensions await chosen connectors.'),
('02  Rear cover + controller tray','Screw access, cooling slots and board stand-offs.\nBOOT / EN accessible with cover removed.'),
('03  Cable routing','Lower power channel + six tie anchors.\nUpper ribbon route; keep vents unobstructed.'),
('04  Removable acrylic front','164 × 84 mm cut from your 180 × 130 mm sheet.\nPrinted spacer keeps it clear of the LEDs.'),
('05  Two printed desk wedges','10° viewing angle, add non-slip pads.\nFit coupon included for ports and screw holes.')]
for i,(title,body) in enumerate(labels):
    yy=.805-i*.125
    fig.text(.69,yy,title,fontsize=11,fontweight='bold',color='#17343a')
    fig.text(.69,yy-.046,body,fontsize=9,color='#546368',linespacing=1.5)
fig.text(.06,.062,'PROVISIONAL FIT — actual STL geometry shown; PCB, connector, panel depth and wiring dimensions need measuring.',
         fontsize=10,color='#9b4c27',fontweight='bold')
fig.text(.06,.034,'Body 182.8 × 102.8 × 60 mm + lid  |  Geometry checked; not physically fit-tested or print-approved.',
         fontsize=10,color='#526467')
fig.savefig(ROOT/'prototype-overview.png',dpi=170,facecolor=fig.get_facecolor())
# Front and rear assembled views of the exported meshes. The dark panel is an
# illustrative envelope, not a manufactured part or an assertion of hardware fit.
render_triangles.clear();render_colours.clear()
plot(meshes['shell'],color='#48565c')
plot(meshes['lid'],(0,0,60),color='#48565c')
plot(meshes['ports'],(-32,-24,63),color='#86bdae')
panel=trimesh.creation.box(extents=[160,80,2]);panel.apply_translation([0,0,7.8])
plot(panel,color='#121b21')
# Subtle lens tint is illustrative; exact transmission is not simulated.
lens=trimesh.creation.box(extents=[164,84,2]);lens.apply_translation([0,0,4]);plot(lens,color='#263b43')
triangles=np.concatenate(render_triangles);colours=np.concatenate(render_colours)
hero,axes=plt.subplots(1,2,figsize=(14,6),facecolor='#f3f1ea')
for axis,camera,title in zip(axes,[(.55,-.65,-1),(.55,-.65,1)],['Front — illustrative panel face','Rear — removable cover and connector plate']):
    axis.imshow(rasterize(triangles,colours,camera,1000,750));axis.set_axis_off();axis.set_title(title,color='#17343a',fontsize=12)
hero.suptitle('INGE / AFTER HOURS · assembled prototype',color='#14252b',fontsize=20)
hero.text(.05,.04,'182.8 × 102.8 × 66 mm overall · fit must be checked on real parts before final printing',color='#9b4c27',fontsize=11)
hero.savefig(ROOT/'assembled-preview.png',dpi=160,facecolor=hero.get_facecolor())
print(json.dumps(report,indent=2))
