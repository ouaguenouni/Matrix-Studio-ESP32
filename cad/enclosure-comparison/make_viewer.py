#!/usr/bin/env python3
"""Standalone offline WebGL viewer built from actual exported STL geometry."""
from pathlib import Path
import base64,json,numpy as np,matplotlib.colors
from geometry import ROOT,MANIFEST,assembly
colours=[('#3e515b','#7caeaa'),('#b8b0a4','#88586a'),('#b4b4a5','#547f71'),('#3b4c55','#ad9671'),('#434557','#9e7bac')]
models=[]
for v in MANIFEST['variants']:
 objects=[];shell,accent=colours[v['id']-1]
 for name,m in assembly(v).items():
  colour=shell
  if name.startswith(('bridge','keeper','rail','spacer')) or name=='ports':colour=accent
  if name in ['carrier','pcb']:colour='#5e9b88'
  if name=='cassette':colour='#b89b7a'
  if name=='panel':colour='#121a22'
  if name=='acrylic':colour='#a8c7d1'
  interleave=np.concatenate([m.triangles.reshape(-1,3),np.repeat(m.face_normals,3,axis=0)],axis=1).astype('<f4')
  objects.append({'name':name,'vertices':base64.b64encode(interleave.tobytes()).decode(),'count':len(interleave),'colour':list(matplotlib.colors.to_rgb(colour))})
 models.append({**v,'objects':objects})
html=Path(ROOT/'viewer-template.html').read_text().replace('/*MODEL_DATA*/',json.dumps(models,separators=(',',':')))
(ROOT/'viewer.html').write_text(html)
print('Offline viewer generated:',(ROOT/'viewer.html').stat().st_size,'bytes')
