#!/usr/bin/env python3
"""Topology, actual assembly intersections, source-backed clearances and layer checks.
No claim of thermal, electrical or physical fit certification is made.
"""
import itertools, json, math, numpy as np, trimesh
from geometry import ROOT,MANIFEST,meshes,assembly,moved
report={'status':'digital_checks_only','hardware_fit_verified':False,'variants':[]}
failures=[]
for v in MANIFEST['variants']:
 ms=meshes(v);r={'name':v['name'],'parts':{},'collisions':[],'envelopes':[]}
 for name,m in ms.items():
  ok=m.is_watertight and m.is_winding_consistent and m.volume>0 and m.body_count==1
  bed=bool(np.all(m.extents[:2]+10<=220) and m.extents[2]<=250)
  flat=bool(abs(m.bounds[0,2])<.01)
  r['parts'][name]={'watertight':bool(m.is_watertight),'consistent_winding':bool(m.is_winding_consistent),'bodies':int(m.body_count),'positive_volume':bool(m.volume>0),'bed_with_5mm_brim':bed,'on_z0':flat,'size_mm':m.extents.round(3).tolist(),'solid_volume_cm3':round(float(m.volume)/1000,2)}
  if not(ok and bed and flat):failures.append(f'{v["name"]}/{name}: mesh/bed/base')
 parts=assembly(v)
 for (an,a),(bn,b) in itertools.combinations(parts.items(),2):
  if np.any(a.bounds[0]>=b.bounds[1]-.001) or np.any(b.bounds[0]>=a.bounds[1]-.001):continue
  inter=trimesh.boolean.intersection([a,b],engine='manifold')
  vol=0 if inter is None else abs(float(inter.volume))
  if vol>.05:
   r['collisions'].append({'a':an,'b':bn,'volume_mm3':round(vol,3)})
   failures.append(f'{v["name"]}: {an}/{bn}: {vol:.3f} mm3')
 # Reserve a 20mm connector zone behind panel; board keepout is wider than nominal PCB.
 z=v['body_depth']+6.5
 envelopes={
  'panel_connector_zone':moved(trimesh.creation.box([148,48,20]),[0,0,36]),
  'usb_plug_zone':moved(trimesh.creation.box([26,12,10]),[1,12,z-36]),
  'pcb_and_wiring_zone':moved(trimesh.creation.box([64,36,38]),[43,12,z-20]),
 }
 for name,env in envelopes.items():
  for other in (['body','lid','cassette','carrier'] if name=='panel_connector_zone' else ['body','panel','cassette']):
   obj=parts[other]
   if np.any(env.bounds[0]>=obj.bounds[1]-.001) or np.any(obj.bounds[0]>=env.bounds[1]-.001):continue
   inter=trimesh.boolean.intersection([env,obj],engine='manifold');vol=abs(float(inter.volume))
   if vol>.05:r['envelopes'].append({'envelope':name,'part':other,'intersection_mm3':round(vol,2)});failures.append(f'{v["name"]}: {name}/{other}')
 # Usable one-layer-wide coil,6mm effective diameter,0.2m per revolution,2mm headroom.
 turns=max(0,math.floor((v['cassette_height']-4.4)/6))
 r['conservative_6mm_coil_m']=round(turns*.2,1)
 r['acrylic_to_panel_gap_mm']=4.5
 r['panel_face_to_keeper_gap_mm']=2.2
 r['panel_mount_spacing_mm']=[125,65]
 r['assembled_printed_mass_solid_upper_bound_g']=round(sum(ms[n].volume/1000*1.27*q for n,q in MANIFEST['quantities'].items() if n!='coupon'),1)
 report['variants'].append(r)
 print(v['name'], 'collisions',r['collisions'],'envelopes',r['envelopes'],flush=True)
report['failures']=failures;report['passed']=not failures
(ROOT/'validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS' if not failures else '\n'.join(failures),flush=True)
raise SystemExit(bool(failures))
