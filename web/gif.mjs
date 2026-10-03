import {GifReader} from './vendor/omggif.mjs';
import {opaqueBounds} from './codec.mjs';

// Decode and composite once; browser playback then sends normal RGB565 frames.
export function decodeGIF(buffer) {
  const bytes=buffer instanceof Uint8Array?buffer:new Uint8Array(buffer);
  const reader=new GifReader(bytes),width=reader.width,height=reader.height,count=reader.numFrames();
  if(!count||width>2048||height>2048||width*height*count>16*1024*1024)
    throw new Error('This GIF is too large to animate. Choose a smaller GIF.');
  const background=[0,0,0,0];
  if(bytes[10]&128) {
    const index=13+bytes[11]*3;
    background.splice(0,4,bytes[index],bytes[index+1],bytes[index+2],255);
  }
  let pixels=new Uint8ClampedArray(width*height*4),bounds=null;
  const frames=[];
  function clear(info) {
    const colour=info.transparent_index!==null?[0,0,0,0]:background;
    for(let y=info.y;y<info.y+info.height;y++)for(let x=info.x;x<info.x+info.width;x++)
      pixels.set(colour,(y*width+x)*4);
  }
  for(let i=0;i<count;i++) {
    const info=reader.frameInfo(i);
    if(info.x+info.width>width||info.y+info.height>height)throw new Error('Invalid GIF frame dimensions.');
    if(!i&&info.transparent_index===null)clear({x:0,y:0,width,height,transparent_index:null});
    const previous=info.disposal===3?pixels.slice():null;
    reader.decodeAndBlitFrameRGBA(i,pixels);
    const visible=opaqueBounds(pixels,width,height);
    if(visible) {
      if(!bounds)bounds={...visible};
      else {
        const right=Math.max(bounds.x+bounds.w,visible.x+visible.w),bottom=Math.max(bounds.y+bounds.h,visible.y+visible.h);
        bounds.x=Math.min(bounds.x,visible.x);bounds.y=Math.min(bounds.y,visible.y);
        bounds.w=right-bounds.x;bounds.h=bottom-bounds.y;
      }
    }
    // Match common browser treatment of zero/one-centisecond GIF delays.
    frames.push({rgba:pixels.slice(),delay:info.delay<=1?100:info.delay*10});
    if(info.disposal===2)clear(info);
    else if(previous)pixels=previous;
  }
  return {width,height,frames,bounds:bounds||{x:0,y:0,w:width,h:height}};
}
