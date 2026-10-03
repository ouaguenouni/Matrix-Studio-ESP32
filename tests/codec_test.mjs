import assert from 'node:assert/strict';
import {rgbaTo565, textMaskToRgba, imagePlacement} from '../web/codec.mjs';
const rgba=new Uint8ClampedArray([
  255,0,0,255, 0,255,0,255, 0,0,255,255,
  255,255,255,255, 255,0,0,0, 255,0,0,128
]);
assert.deepEqual([...rgbaTo565(rgba,6,1)], [0,248,224,7,31,0,255,255,0,0,0,128]);
assert.equal(rgbaTo565(new Uint8ClampedArray(64*32*4)).length,4096);
assert.throws(()=>rgbaTo565(new Uint8Array(3)));
// Violet shades must not acquire green during packing or endian conversion.
for (const [r,b] of [[255,255],[128,192],[64,96]]) {
  const bytes=rgbaTo565(new Uint8ClampedArray([r,0,b,255]),1,1);
  const pixel=bytes[0]|(bytes[1]<<8);
  assert.equal((pixel>>5)&63,0);
  assert((pixel>>11)>0&&(pixel&31)>0);
}
const mask=new Uint8ClampedArray([255,255,255,0, 255,255,255,127, 255,255,255,128, 255,255,255,255]);
const sharp=rgbaTo565(textMaskToRgba(mask,'#ffffff','#000000'),4,1);
assert.deepEqual([...sharp], [0,0, 0,0, 255,255, 255,255]);
const smooth=textMaskToRgba(mask,'#ffffff','#000000',false);
assert.deepEqual([...smooth.slice(4,8)], [127,127,127,255]);
const coloured=rgbaTo565(textMaskToRgba(mask,'#0000ff','#ff0000'),4,1);
assert.deepEqual([...coloured], [0,248, 0,248, 31,0, 31,0]);
assert.deepEqual(imagePlacement(100,100,'contain'),{x:16,y:0,w:32,h:32});
assert.deepEqual(imagePlacement(100,100,'cover'),{x:0,y:-16,w:64,h:64});
assert.deepEqual(imagePlacement(100,100,'stretch'),{x:0,y:0,w:64,h:32});
assert.throws(()=>imagePlacement(0,2,'contain'));
console.log('Canvas codec: RGB primaries, alpha, dimensions, fit modes, and sharp/smooth text colours passed.');
