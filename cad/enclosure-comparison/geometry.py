"""Load exported meshes and place actual parts for assembly verification/rendering."""
from pathlib import Path
import json, numpy as np, trimesh
ROOT=Path(__file__).resolve().parent
MANIFEST=json.loads((ROOT/'manifest.json').read_text())
def meshes(v): return {p.stem:trimesh.load_mesh(p) for p in (ROOT/v['name']/'stl').glob('*.stl')}
def moved(mesh,offset=(0,0,0),flip=False,matrix=None):
 m=mesh.copy()
 if flip:m.apply_transform(trimesh.transformations.rotation_matrix(np.pi,[1,0,0]))
 if matrix is not None:m.apply_transform(matrix)
 m.apply_translation(offset);return m
def assembly(v,hardware=True,explode=0):
 m=meshes(v);w,h,d=v['width'],v['height'],v['body_depth'];z=d+6.5
 items={}
 def add(name,part,offset=(0,0,0),flip=False,matrix=None):items[name]=moved(m[part],offset,flip,matrix)
 add('bezel','bezel',(0,0,-25*explode)); add('body','body',(0,0,6.5))
 add('keeper','keeper',(0,0,4.3-10*explode));add('lid','lid',(0,0,z+65*explode))
 add('ports','ports',(0,-h/2+21,z+3+85*explode))
 for sy in [-1,1]:add(f'bridge{sy}','bridge',(0,sy*32.5,23+12*explode))
 add('carrier','carrier',(43,12,z+42*explode),True)
 add('cassette','cassette',(-44,36 if h>=150 else 12,z+42*explode),True)
 for sy in [-1,1]:
  add(f'rail{sy}','rail',(43,12+sy*11.5,z-9+42*explode),True)
  for sx in [-1,1]:add(f'spacer{sx}_{sy}','spacer',(43+sx*26,12+sy*11.5,z-12+42*explode),True)
 mat=np.eye(4);mat[:3,:3]=np.array([[0,1,0],[0,0,1],[1,0,0]])
 for sx in [-1,1]:add(f'foot{sx}','feet',(sx*(w/2-22)-7,-h/2-4,-5),matrix=mat)
 if hardware:
  def box(name,size,center):items[name]=moved(trimesh.creation.box(size),center)
  box('panel',[160,80,14.5],[0,0,15.75])
  box('acrylic',v['lens'],[0,0,3.5-18*explode])
  box('pcb',[58,29,1.6],[43,12,z-32-.8+42*explode])
  def head(name,xy,start,diam,height):
   items[name]=moved(trimesh.creation.cylinder(radius=diam/2,height=height,sections=32),[xy[0],xy[1],start+height/2])
  for sx in [-1,1]:
   for sy in [-1,1]:
    head(f'keeper_head{sx}_{sy}',[sx*(v['lens'][0]/2+3),sy*(v['lens'][1]/2-9)],6.3-10*explode,5.5,2.5)
    head(f'front_head{sx}_{sy}',[sx*(w/2-7),sy*(h/2-7)],-2.4-25*explode,5.6,2.4)
    head(f'lid_head{sx}_{sy}',[sx*(w/2-7),sy*(h/2-7)],z+3+65*explode,5.6,2.4)
 return items
