"""Load exported meshes and place actual parts for assembly verification/rendering."""
from pathlib import Path
import json, numpy as np, trimesh
ROOT=Path(__file__).resolve().parent
MANIFEST=json.loads((ROOT/'manifest.json').read_text())
PARAMETERS=MANIFEST.get('hardware_parameters',{})
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
 add('bezel','bezel',(0,0,-30*explode)); add('body','body',(0,0,6.5))
 keeper_z=3+v['lens'][2]+.3
 add('keeper','keeper',(0,0,keeper_z-12*explode));add('lid','lid',(0,0,z+70*explode))
 add('ports','ports',(0,-h/2+21,z+3+90*explode))
 add('carrier','carrier',(43,12,z+45*explode),True)
 add('cassette','cassette',(-44,36 if h>=150 else 12,z+45*explode),True)
 hole_l=PARAMETERS.get('pcb_hole_length',52);hole_w=PARAMETERS.get('pcb_hole_width',23)
 standoff=PARAMETERS.get('pcb_standoff',20)
 panel_depth=PARAMETERS.get('panel_depth',14.5);panel_back=8.5+panel_depth
 # The bridges and their seats follow the measured panel depth.
 for sy in [-1,1]:items[f'bridge{sy}']=moved(m['bridge'],[0,sy*32.5,panel_back+12*explode])
 for sy in [-1,1]:
  add(f'rail{sy}','rail',(43,12+sy*hole_w/2,z-9+40*explode),True)
  for sx in [-1,1]:add(f'spacer{sx}_{sy}','spacer',(43+sx*hole_l/2,12+sy*hole_w/2,z-12+40*explode),True)
 mat=np.eye(4);mat[:3,:3]=np.array([[0,1,0],[0,0,1],[1,0,0]])
 for sx in [-1,1]:add(f'foot{sx}','feet',(sx*(w/2-22)-7,-h/2-4,-5),matrix=mat)
 if hardware:
  def box(name,size,center):items[name]=moved(trimesh.creation.box(size),center)
  box('panel',[160,80,panel_depth],[0,0,8.5+panel_depth/2])
  box('acrylic',v['lens'],[0,0,3+v['lens'][2]/2-20*explode])
  pcb_t=PARAMETERS.get('pcb_thickness',1.6)
  pcb=trimesh.creation.box([PARAMETERS.get('pcb_length',58),PARAMETERS.get('pcb_width',29),pcb_t])
  pcb=moved(pcb,[43,12,z-12-standoff-pcb_t/2+40*explode])
  # Represent real mounting holes so fastener clearances can be checked.
  cutters=[moved(trimesh.creation.cylinder(radius=1.6,height=pcb_t+2,sections=24),
           [43+sx*hole_l/2,12+sy*hole_w/2,z-12-standoff-pcb_t/2+40*explode])
           for sx in [-1,1] for sy in [-1,1]]
  items['pcb']=trimesh.boolean.difference([pcb,*cutters],engine='manifold')
  def head(name,xy,start,diam,height):
   items[name]=moved(trimesh.creation.cylinder(radius=diam/2,height=height,sections=32),[xy[0],xy[1],start+height/2])
  for sx in [-1,1]:
   for sy in [-1,1]:
    head(f'keeper_head{sx}_{sy}',[sx*(v['lens'][0]/2+3),sy*(v['lens'][1]/2-9)],keeper_z+2-12*explode,5.5,2.5)
    head(f'front_head{sx}_{sy}',[sx*(w/2-7),sy*(h/2-7)],-2.4-30*explode,5.6,2.4)
    head(f'lid_head{sx}_{sy}',[sx*(w/2-7),sy*(h/2-7)],z+3+70*explode,5.6,2.4)
    head(f'panel_head{sx}_{sy}',[sx*62.5,sy*32.5],panel_back+3+12*explode,5.6,2.4)
    head(f'bridge_head{sx}_{sy}',[sx*(w/2-8),sy*32.5],panel_back+3+12*explode,5.6,2.4)
    head(f'pcb_head{sx}_{sy}',[43+sx*hole_l/2,12+sy*hole_w/2],z-12-standoff-pcb_t-2.4+40*explode,5.6,2.4)
 return items
