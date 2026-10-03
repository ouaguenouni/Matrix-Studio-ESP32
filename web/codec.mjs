export function rgbaTo565(rgba, width = 64, height = 32) {
  if (rgba.length !== width * height * 4) throw new Error('Unexpected canvas dimensions');
  const result = new Uint8Array(width * height * 2);
  for (let i = 0; i < width * height; i++) {
    const a = rgba[i * 4 + 3] / 255;
    const r = Math.round(rgba[i * 4] * a);
    const g = Math.round(rgba[i * 4 + 1] * a);
    const b = Math.round(rgba[i * 4 + 2] * a);
    const pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    result[i * 2] = pixel & 255;
    result[i * 2 + 1] = pixel >> 8;
  }
  return result;
}

// Render glyph coverage using the selected colours. Sharp mode removes dim
// antialiased edges so small LED text uses full foreground/background pixels.
export function textMaskToRgba(mask, foreground, background, sharp = true) {
  const rgb = hex => [1, 3, 5].map(offset => parseInt(hex.slice(offset, offset + 2), 16));
  const fg = rgb(foreground), bg = rgb(background);
  const result = new Uint8ClampedArray(mask.length);
  for (let i = 0; i < mask.length; i += 4) {
    const coverage = sharp ? (mask[i + 3] >= 128 ? 1 : 0) : mask[i + 3] / 255;
    for (let channel = 0; channel < 3; ++channel)
      result[i + channel] = Math.round(bg[channel] + (fg[channel] - bg[channel]) * coverage);
    result[i + 3] = 255;
  }
  return result;
}

export function imagePlacement(iw, ih, mode, w = 64, h = 32) {
  if (!(iw > 0 && ih > 0)) throw new Error('Image has no dimensions');
  if (mode === 'stretch') return { x: 0, y: 0, w, h };
  const scale = mode === 'cover' ? Math.max(w / iw, h / ih) : Math.min(w / iw, h / ih);
  return { x: (w - iw * scale) / 2, y: (h - ih * scale) / 2, w: iw * scale, h: ih * scale };
}

export function opaqueBounds(rgba,width,height) {
  if(rgba.length!==width*height*4)throw new Error('Unexpected image dimensions');
  let left=width,top=height,right=-1,bottom=-1;
  for(let y=0;y<height;y++)for(let x=0;x<width;x++)if(rgba[(y*width+x)*4+3]) {
    left=Math.min(left,x);right=Math.max(right,x);top=Math.min(top,y);bottom=Math.max(bottom,y);
  }
  return right<0 ? null : {x:left,y:top,w:right-left+1,h:bottom-top+1};
}
