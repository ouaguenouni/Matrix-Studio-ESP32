#!/usr/bin/env python3
"""Export five complete STL kits. OPENSCAD may be a binary or WASM .js path."""
import concurrent.futures, json, os, shlex, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parent
cmd=shlex.split(os.environ.get('OPENSCAD','openscad'))
styles=[('01-line',190,110,82,5,164,84,14),('02-orbit',202,122,84,16,164,84,20),
        ('03-gallery',206,156,84,8,180,130,20),('04-vault',206,156,110,8,180,130,40),
        ('05-console',202,122,96,7,164,84,30)]
parts={'bezel':1,'body':1,'lid':1,'keeper':1,'bridge':2,'carrier':1,'rail':2,'spacer':4,'ports':1,'cassette':1,'feet':2,'coupon':1,'ports_usb_c':0}
manifest={'units':'mm','hardware_fit_verified':False,'acrylic_thickness_mm':1,'variants':[],'quantities':parts}
jobs=[]
for v,(name,w,h,d,r,lw,lh,ch) in enumerate(styles,1):
    folder=ROOT/name;folder.mkdir(exist_ok=True)
    (folder/'stl').mkdir(exist_ok=True)
    (folder/'model.scad').write_text(f'// Set part in Customizer; see ../PRINT-GUIDE.md\nstyle_id = {v};\ninclude <../family.scad>\n')
    manifest['variants'].append({'id':v,'name':name,'width':w,'height':h,'body_depth':d,'overall_depth':d+12.5,'radius':r,'lens':[lw,lh,1],'cassette_height':ch})
    for part in parts:
        jobs.append((v,part,folder/'stl'/f'{part}.stl'))
    jobs.append((v,'acrylic_template',folder/'acrylic-outline.svg'))
(ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
def export(job):
    v,part,out=job
    args=cmd+(['--export-format','binstl'] if out.suffix=='.stl' else [])+['-o',str(out),'-D',f'variant={v}','-D',f'part="{part}"',str(ROOT/'family.scad')]
    result=subprocess.run(args,capture_output=True,text=True,timeout=180)
    log=result.stdout+'\n'+result.stderr
    (out.parent/(out.stem+'.log')).write_text(log)
    if result.returncode or not out.exists() or 'ERROR:' in log:
        raise RuntimeError(f'{v}/{part}: '+log[-2500:])
    return f'{v}/{part}'
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
    for done in pool.map(export,jobs): print(done,flush=True)
print('All five kits exported.',flush=True)
