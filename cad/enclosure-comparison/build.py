#!/usr/bin/env python3
"""Export five STL kits using a native OpenSCAD binary or its Node WASM CLI."""
import argparse
import concurrent.futures
import hashlib
import json
import os
import shlex
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
STYLES = [('01-line',190,110,82,5,164,84,14),('02-orbit',202,122,84,16,164,84,20),
          ('03-gallery',206,156,84,8,180,130,20),('04-vault',206,156,110,8,180,130,40),
          ('05-console',202,122,96,7,164,84,30)]
PARTS = {'bezel':1,'body':1,'lid':1,'keeper':1,'bridge':2,'carrier':1,'rail':2,
         'spacer':4,'ports':1,'cassette':1,'feet':2,'coupon':1,'ports_usb_c':0}
HARDWARE_PARAMETERS = ['panel_depth', 'acrylic_thickness', 'pcb_length', 'pcb_width',
                       'pcb_thickness', 'pcb_hole_length', 'pcb_hole_width', 'pcb_standoff']

def source_defaults():
    import re
    source = (ROOT/'family.scad').read_text()
    return {name:float(re.search(rf'^\s*{name}\s*=\s*([\d.]+)\s*;',source,re.M)[1])
            for name in HARDWARE_PARAMETERS}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--jobs',type=int,default=3)
    for parameter in HARDWARE_PARAMETERS:
        parser.add_argument('--'+parameter.replace('_','-'),type=float)
    args = parser.parse_args()
    cmd = shlex.split(os.environ.get('OPENSCAD','openscad'))
    if not cmd or not shutil.which(cmd[0]):
        parser.error('OpenSCAD not found. Set OPENSCAD="/path/to/openscad" or "node /path/to/openscad.js".')
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    parameters = source_defaults()
    for name in parameters:
        if getattr(args,name) is not None:
            parameters[name] = getattr(args,name)
    definitions = [item for k,val in parameters.items() for item in ['-D',f'{k}={val}']]
    manifest = {'units':'mm','revision':3,'hardware_fit_verified':False,
                'acrylic_thickness_mm':parameters['acrylic_thickness'],
                'hardware_parameters':parameters,'variants':[],'quantities':PARTS,
                'source_sha256':hashlib.sha256((ROOT/'family.scad').read_bytes()).hexdigest()}
    jobs = []
    for v,(name,w,h,d,r,lw,lh,ch) in enumerate(STYLES,1):
        folder = ROOT/name
        (folder/'stl').mkdir(parents=True,exist_ok=True)
        overrides = ''.join(f'{p} = {value};\n' for p,value in parameters.items())
        (folder/'model.scad').write_text(f'// Set part in Customizer; see ../PRINT-GUIDE.md\nstyle_id = {v};\ninclude <../family.scad>\n'+overrides)
        manifest['variants'].append({'id':v,'name':name,'width':w,'height':h,'body_depth':d,
            'overall_depth':d+12.5,'radius':r,'lens':[lw,lh,parameters['acrylic_thickness']],
            'cassette_height':ch})
        jobs.extend((v,part,folder/'stl'/f'{part}.stl') for part in PARTS)
        jobs.append((v,'acrylic_template',folder/'acrylic-outline.svg'))
    def export(job):
        v,part,out = job
        temporary = out.with_name(out.stem+'.pending'+out.suffix)
        result = subprocess.run(cmd+(['--export-format','binstl'] if out.suffix=='.stl' else [])+
            ['-o',str(temporary),'-D',f'variant={v}','-D',f'part="{part}"']+definitions+
            [str(ROOT/'family.scad')],capture_output=True,text=True,timeout=180)
        log = result.stdout+'\n'+result.stderr
        (out.parent/(out.stem+'.log')).write_text(log)
        if result.returncode or not temporary.exists() or 'ERROR:' in log:
            temporary.unlink(missing_ok=True)
            raise RuntimeError(f'{v}/{part}: '+log[-2500:])
        temporary.replace(out)
        return f'{v}/{part}'
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for done in pool.map(export,jobs):
            print(done,flush=True)
    (ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print('All five kits exported. Run validate.py before packaging.',flush=True)

if __name__ == '__main__':
    main()
