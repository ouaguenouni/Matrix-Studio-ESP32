import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import {GifWriter} from '../web/vendor/omggif.mjs';
import {decodeGIF,filterCrystal} from '../web/gif.mjs';

// Independent Pillow 12.1.1 reference, normalized to zero RGB on transparent pixels.
const references=JSON.parse(fs.readFileSync(new URL('./fixtures/crystal_frames.json',import.meta.url)));
let total=0;
for(const [path,reference] of Object.entries(references)) {
  const animation=decodeGIF(fs.readFileSync(new URL('../assets/crystal/'+path,import.meta.url)));
  assert.equal(animation.frames.length,reference.frames,path);
  const hashes=animation.frames.map(frame=>{
    const rgba=frame.rgba.slice();
    for(let i=0;i<rgba.length;i+=4)if(!rgba[i+3])rgba.fill(0,i,i+3);
    return crypto.createHash('sha256').update(rgba).digest('hex');
  }).join('');
  assert.equal(crypto.createHash('sha256').update(hashes).digest('hex'),reference.sha256,path);
  total+=reference.frames;
}
const buffer=new Uint8Array(4096);
const writer=new GifWriter(buffer,3,1,{palette:[0,0xff0000,0x00ff00,0x0000ff],loop:0});
writer.addFrame(0,0,3,1,[1,0,0],{transparent:0,disposal:1,delay:5});
writer.addFrame(1,0,1,1,[2],{transparent:0,disposal:3,delay:10});
writer.addFrame(2,0,1,1,[3],{transparent:0,disposal:2,delay:0});
writer.addFrame(1,0,1,1,[2],{transparent:0,disposal:1,delay:10});
const gif=decodeGIF(buffer.slice(0,writer.end()));
const R=[255,0,0,255],G=[0,255,0,255],B=[0,0,255,255],clear=[0,0,0,0];
assert.deepEqual([...gif.frames[0].rgba],[...R,...clear,...clear]);
assert.deepEqual([...gif.frames[1].rgba],[...R,...G,...clear]);
assert.deepEqual([...gif.frames[2].rgba],[...R,...clear,...B]);
assert.deepEqual([...gif.frames[3].rgba],[...R,...G,...clear]);
assert.deepEqual(gif.bounds,{x:0,y:0,w:3,h:1});
assert.deepEqual(gif.frames.map(f=>f.delay),[50,100,100,100]);
assert.throws(()=>decodeGIF(new Uint8Array([1,2,3])));
const huge=buffer.slice(0,writer.end());huge[6]=1;huge[7]=16;
assert.throws(()=>decodeGIF(huge),/too large/);
const catalogue=JSON.parse(fs.readFileSync(new URL('../assets/crystal/catalogue.json',import.meta.url))).entries;
assert.equal(filterCrystal(catalogue).length,276);
assert.equal(filterCrystal(catalogue,'','shiny').length,276);
assert.equal(filterCrystal(catalogue,'93','both').length,2);
assert.equal(filterCrystal(catalogue,'HAUNT','normal')[0].id,93);
assert.equal(filterCrystal(catalogue,'','normal','kanto').length,151);
assert.equal(filterCrystal(catalogue,'','normal','unown').length,26);
assert.equal(filterCrystal(catalogue,'','normal','johto').length,125);
console.log(`GIF tests passed: ${total} archive frames match Pillow, plus disposal 2/3, transparency, timing, bounds, limits, and filters.`);
