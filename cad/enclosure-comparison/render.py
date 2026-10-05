#!/usr/bin/env python3
"""Actual STL comparison views. Pixel face is illustrative; optics not simulated."""
import json,numpy as np,trimesh,matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from geometry import ROOT,MANIFEST,assembly,moved
from raster import rasterize
BG='#f3f1ea';INK='#172e35'
PALETTES=[('#3e515b','#7caeaa'),('#b8b0a4','#88586a'),('#b4b4a5','#547f71'),('#3b4c55','#ad9671'),('#434557','#9e7bac')]
TITLES=['LINE / compact frame','ORBIT / rounded corners','GALLERY / uncut acrylic','VAULT / long cable storage','CONSOLE / fluted shell']
SUBTITLES=['Smaller footprint · cut acrylic','Soft outline · medium cable tray','Full 180 × 130 sheet · broad frame','Full sheet · deepest cable tray','Chamfered corners · larger coil space']
def picture(v,camera,explode=0,internals=False,w=1000,h=800):
 parts=assembly(v,True,explode)
 if internals:
  for n in ['lid','ports']:parts.pop(n)
 elif not explode:
  # Acrylic represented as clear by omitting it; LEDs are illustrated behind it.
  parts.pop('acrylic',None)
 triangles=[];colours=[]
 shell,accent=PALETTES[v['id']-1]
 def append(m,col):
  light=np.array([-.4,.7,-.8]);light/=np.linalg.norm(light)
  shade=np.clip(.68+.3*(m.face_normals@light),.3,1)
  rgb=np.array(matplotlib.colors.to_rgb(col))
  triangles.append(m.triangles);colours.append(rgb[None,:]*shade[:,None])
 for name,m in parts.items():
  col=shell
  if name.startswith(('bridge','keeper','spacer','rail')):col=accent
  if name in ['carrier','pcb']:col='#5e9b88'
  if name=='cassette':col='#b89b7a'
  if name=='ports':col=accent
  if name=='panel':col='#0a1118'
  if name=='acrylic':col='#b3d5dd'
  append(m,col)
 if not explode:
  # Sparse bright pixels suggest the matrix face; no firmware or optics claim.
  for row in range(32):
   for col in range(64):
    # stylised architectural skyline / techno bars, same graphic for fair comparison
    height=int(7+5*np.sin(col*.28)+4*np.sin(col*.63))
    on=row>=31-height and row<29
    rgb='#65a799' if on else '#202d33'
    px=trimesh.creation.box([1.35,1.35,.2]);px.apply_translation([-78.75+col*2.5,38.75-row*2.5,8.35])
    append(px,rgb)
 return rasterize(np.concatenate(triangles),np.concatenate(colours),camera,w,h)
plt.rcParams.update({'font.family':'DejaVu Sans'})
fig,axs=plt.subplots(2,3,figsize=(18,11),facecolor=BG)
for i,v in enumerate(MANIFEST['variants']):
 ax=axs.flat[i];ax.imshow(picture(v,(.65,.5,-1),w=1100,h=800));ax.axis('off')
 ax.set_title(TITLES[i],fontsize=15,fontweight='bold',color=INK)
 ax.text(.5,-.02,f"{v['width']} × {v['height']} × {v['overall_depth']:g} mm · {v['cassette_height']} mm tray",transform=ax.transAxes,ha='center',color=INK,fontsize=11)
 single,sa=plt.subplots(1,2,figsize=(14,6),facecolor=BG)
 sa[0].imshow(picture(v,(.65,.5,-1)));sa[1].imshow(picture(v,(.6,-.7,1)))
 for x in sa:x.axis('off')
 sa[0].set_title('Front · illustrative LED face',color=INK);sa[1].set_title('Rear · two cable/service openings',color=INK)
 single.suptitle(TITLES[i],fontsize=21,color=INK,fontweight='bold')
 single.text(.06,.025,f"{v['width']} × {v['height']} × {v['overall_depth']:g} mm including port plate; feet add 4 mm below. Actual STL geometry; physical fit unverified.",fontsize=10,color='#805130')
 single.savefig(ROOT/v['name']/'preview.png',dpi=140,facecolor=BG);plt.close(single)
 exploded,ea=plt.subplots(figsize=(11,10),facecolor=BG);ea.imshow(picture(v,(.6,-1,.9),explode=1,w=1200,h=1150));ea.axis('off')
 exploded.suptitle(TITLES[i]+' · exploded assembly',fontsize=20,color=INK)
 exploded.text(.07,.025,'Independent acrylic retention · M3 rear panel bridges · sliding PCB rails · removable cable cassette',color=INK,fontsize=10)
 exploded.savefig(ROOT/v['name']/'exploded.png',dpi=135,facecolor=BG);plt.close(exploded)
 print(v['name'],flush=True)
ax=axs.flat[5];ax.axis('off')
ax.text(.05,.94,'Shared mechanical corrections',fontsize=17,fontweight='bold',color=INK,va='top',transform=ax.transAxes)
ax.text(.05,.82,'1 mm acrylic; removable front bezel\n\nPanel supported by 125 × 65 mm M3 mounts\n\nSliding PCB rails; 20 mm wiring stand-offs\n\nR25 mm cable guide; soft strap retention\n\nTwo access openings; optional USB-C plate\n\nCradle feet; screw-serviceable enclosure',fontsize=12,linespacing=1.35,color=INK,va='top',transform=ax.transAxes)
ax.text(.05,.05,'Digital geometry validated.\nReal fit, temperature and cable-pull tests pending.',fontsize=11,color='#9b4c27',transform=ax.transAxes)
fig.suptitle('INGE / AFTER HOURS — five enclosure prototypes',fontsize=25,fontweight='bold',color=INK)
fig.subplots_adjust(top=.91,bottom=.065,hspace=.18,wspace=.05)
fig.savefig(ROOT/'comparison.png',dpi=145,facecolor=BG)
plt.close(fig)
# Top-down service view of recommended Vault, with cable paths as illustrations.
v=MANIFEST['variants'][3];parts=assembly(v)
service,ax=plt.subplots(figsize=(11,9),facecolor=BG)
ax.imshow(picture(v,(.03,-.03,1),internals=True,w=1150,h=1000));ax.axis('off')
service.suptitle('VAULT · service view with rear cover hidden',fontsize=20,color=INK)
service.text(.08,.04,'Left: removable coil cassette. Right: adjustable PCB carrier. Panel wiring runs in the clear middle depth.',fontsize=11,color=INK)
service.savefig(ROOT/'vault-service.png',dpi=140,facecolor=BG)
