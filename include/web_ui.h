#pragma once
#include <Arduino.h>
static const char WEB_UI[] PROGMEM = R"MATRIX_UI(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Matrix Studio</title>
<style>
:root{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;color:#182a29;background:#f4f5ee;font-size:15px;--green:#176b57;--border:#d8dfd6}
*{box-sizing:border-box}body{margin:0}main{max-width:1080px;margin:auto;padding:30px 22px 55px}header{display:flex;justify-content:space-between;align-items:center;gap:18px;margin-bottom:34px}.logo{display:flex;gap:12px;align-items:center}.mark{font-size:24px;letter-spacing:2px;color:var(--green)}h1{font-size:23px;letter-spacing:-.8px;margin:0}h2{font-size:18px;margin:0 0 15px}p{line-height:1.6}.muted{color:#657470;font-size:13px}.badge{padding:8px 12px;border:1px solid var(--border);border-radius:30px;font-size:12px;background:#fff}.lead{font-size:36px;letter-spacing:-1.5px;font-weight:600;margin:0 0 7px}.sub{margin:0 0 28px;color:#657470}.layout{display:grid;grid-template-columns:minmax(0,1.2fr) minmax(290px,1fr);gap:22px}.card{background:#fff;border:1px solid var(--border);border-radius:18px;padding:22px}.preview{background:#0a1110;border-radius:12px;padding:18px;margin:18px 0;color:#95ada4}.preview canvas{display:block;width:100%;height:auto;aspect-ratio:2;background:#000;image-rendering:pixelated;touch-action:none;cursor:crosshair}.caps{text-transform:uppercase;letter-spacing:1.2px;font-size:11px;color:#64756c;display:flex;justify-content:space-between;gap:10px}label{display:block;font-size:13px;font-weight:600;margin:15px 0 6px}input:not([type=color]):not([type=range]),textarea,select{font:inherit;width:100%;padding:10px 11px;border:1px solid var(--border);border-radius:8px;background:#fff;color:#182a29}textarea{resize:vertical;min-height:88px}input[type=color]{width:100%;height:41px;border:1px solid var(--border);border-radius:8px;padding:4px;background:#fff}input[type=range]{width:100%;accent-color:var(--green)}button{font:inherit;cursor:pointer;border:1px solid var(--border);border-radius:9px;padding:10px 14px;background:#fff;color:#182a29;font-weight:600}button:hover{background:#edf4ee}button:disabled{opacity:.55;cursor:wait}.primary{background:var(--green);color:#fff;border-color:var(--green)}.primary:hover{background:#105642}.wide{width:100%}.row{display:flex;gap:10px;align-items:center}.row>*{flex:1}.tabs{display:flex;gap:5px;border-bottom:1px solid var(--border);padding-bottom:12px;margin-bottom:18px}.tab{flex:1;font-size:13px;padding:8px}.tab[aria-selected=true]{background:#e8f2eb;color:#125944;border-color:#91b5a5}.pane[hidden]{display:none}.message{min-height:24px;font-size:13px;line-height:1.5;margin-top:12px}.error{color:#a33028}.success{color:var(--green)}.connection{margin-top:22px}summary{cursor:pointer;font-size:15px;font-weight:600}details[open] summary{margin-bottom:15px}a{color:var(--green);overflow-wrap:anywhere}footer{font-size:12px;color:#657470;margin-top:25px}.hint{background:#f0f5ef;border-radius:9px;padding:10px 12px;font-size:13px;line-height:1.5}.selected{background:#e8f2eb!important;border-color:#91b5a5!important}small{font-size:12px}button:focus-visible,input:focus-visible,textarea:focus-visible,select:focus-visible,summary:focus-visible{outline:3px solid #e6a558;outline-offset:2px}@media(max-width:730px){main{padding:22px 15px}.layout{grid-template-columns:1fr}.lead{font-size:29px}header{margin-bottom:25px}.card{padding:18px}.preview{padding:10px}}
</style>
<style>
.tabs{flex-wrap:wrap}.tab{min-width:48px}.gif-gallery{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:8px;margin:14px 0}.gallery-tile{min-width:0;padding:8px 4px;font-size:11px;line-height:1.4}.gallery-tile img{display:block;margin:0 auto 5px;width:56px;height:56px;image-rendering:pixelated;object-fit:contain}.gallery-tile[aria-pressed=true]{background:#e8f2eb;border-color:#91b5a5}.gallery-tile span{display:block;overflow-wrap:anywhere}.gallery-pages{font-size:12px;text-align:center}.gif-gallery .muted{grid-column:1/-1}
@media(max-width:730px){.tabs{display:grid;grid-template-columns:repeat(3,minmax(0,1fr))}}
</style>
</head>
<body><main>
<header><div class="logo"><span class="mark" aria-hidden="true">▦</span><h1>Matrix Studio</h1></div><span class="badge" id="connectionBadge">Connecting…</span></header>
<p class="lead">Your 64 × 32 canvas.</p><p class="sub">Make something here. Send it to your display.</p>
<div class="layout">
<section class="card" aria-label="Display preview">
<div class="caps"><span>Preview</span><span>2,048 pixels · RGB</span></div>
<div class="preview"><canvas id="canvas" width="64" height="32" aria-label="64 by 32 pixel preview"></canvas></div>
<div class="row"><button class="primary" id="send">Send to display</button><button id="clear">Clear</button></div>
<div class="message" id="message" role="status" aria-live="polite">Edits stay in the preview until you send them.</div>
<label for="brightness">Panel brightness <span class="muted" id="brightnessValue">12%</span></label>
<input type="range" id="brightness" min="0" max="255" value="30">
<p class="muted">Brightness changes apply directly to the panel.</p>
</section>
<section class="card" aria-label="Canvas tools">
<div class="tabs" role="tablist" aria-label="Content type"><button class="tab" role="tab" id="tab-text" aria-controls="pane-text" aria-selected="true" data-tab="text">Text</button><button class="tab" role="tab" id="tab-image" aria-controls="pane-image" aria-selected="false" data-tab="image">Image</button><button class="tab" role="tab" id="tab-pokemon" aria-controls="pane-pokemon" aria-selected="false" data-tab="pokemon">Pokémon</button><button class="tab" role="tab" id="tab-gallery" aria-controls="pane-gallery" aria-selected="false" data-tab="gallery">GIFs</button><button class="tab" role="tab" id="tab-draw" aria-controls="pane-draw" aria-selected="false" data-tab="draw">Draw</button></div>
<div class="pane" id="pane-text" role="tabpanel" aria-labelledby="tab-text">
<label for="text">Message</label><textarea id="text" maxlength="500">Hello!</textarea>
<div class="row"><div><label for="textColor">Text colour</label><input id="textColor" type="color" value="#ffffff"></div><div><label for="background">Background</label><input id="background" type="color" value="#000000"></div></div>
<div class="row"><div><label for="fontSize">Size in pixels</label><input id="fontSize" type="number" value="10" min="6" max="32"></div><div><label for="align">Alignment</label><select id="align"><option value="center">Centre</option><option value="left">Left</option><option value="right">Right</option></select></div></div>
<label for="font">Font</label><select id="font"><option value="monospace">Monospace</option><option value="sans-serif">Sans serif</option><option value="serif">Serif</option></select>
<label for="sharpText"><input id="sharpText" type="checkbox" checked style="width:auto"> Sharp text pixels</label>
<p class="muted">Solid colour pixels for clearer letters. Uncheck for smooth edges.</p>
<button id="scroll" class="wide" style="margin-top:18px">Start scrolling on panel</button>
<p class="muted">Scrolling streams from this page. Keep it open and visible; stopping leaves the last frame on the panel.</p>
</div>
<div class="pane" id="pane-image" role="tabpanel" aria-labelledby="tab-image" hidden>
<label for="imageFile">Choose an image</label><input type="file" id="imageFile" accept="image/png,image/jpeg,image/webp,image/gif,image/bmp">
<label for="fit">Fit to display</label><select id="fit"><option value="contain">Fit whole image</option><option value="cover">Fill and crop</option><option value="stretch">Stretch</option></select>
<label for="smoothing">Resizing</label><select id="smoothing"><option value="smooth">Smooth for photos</option><option value="pixel">Sharp for pixel art</option></select>
<p class="muted">PNG, JPEG, WebP, BMP and animated GIF. Images are resized here before sending. Keep this page visible during GIF playback.</p>
<button id="imageGifPlay" class="wide" hidden>Play GIF on panel</button>
<button id="pattern" class="wide">Preview a colour test</button>
<button id="purplePattern" class="wide" style="margin-top:10px">Preview a purple test</button>
</div>
<div class="pane" id="pane-pokemon" role="tabpanel" aria-labelledby="tab-pokemon" hidden>
<form id="pokemonSearchForm">
<label for="pokemonQuery">Pokémon name or number</label><input id="pokemonQuery" type="search" value="haunter" placeholder="Haunter or 93" maxlength="60">
<button id="pokemonLoad" class="wide" style="margin-top:10px" type="submit">Load sprite</button>
</form>
<label for="pokemonScope">Catalogue</label><select id="pokemonScope"><option value="all">All Pokémon and forms</option><option value="gen1">First generation (1–151)</option></select>
<label for="pokemonResults">Matching Pokémon</label><select id="pokemonResults" size="5" aria-label="Matching Pokémon"></select>
<p id="pokemonStatus" class="muted" role="status" aria-live="polite">Search the online sprite catalogue.</p>
<div class="row"><button id="pokemonPrevious">Previous</button><button id="pokemonNext">Next</button></div>
<label for="pokemonInterval">Seconds per sprite</label><input id="pokemonInterval" type="number" value="5" min="2" max="60">
<button id="pokemonCycle" class="wide" style="margin-top:12px">Start cycling on panel</button>
<p class="muted">Cycles the selected catalogue in Pokédex order. Keep this page open and visible; stopping leaves the last sprite on the panel.</p>
<p class="muted">Internet is needed to load new sprites. <a href="https://pokeapi.co/" target="_blank" rel="noopener noreferrer">Sprites and catalogue from PokéAPI</a>.</p>
</div>
<div class="pane" id="pane-gallery" role="tabpanel" aria-labelledby="tab-gallery" hidden>
<p class="muted">Pokémon Crystal · 552 animated sprites stored on your ESP32.</p>
<label for="gifQuery">Search GIFs by name or number</label><input type="search" id="gifQuery" placeholder="Haunter or 93" maxlength="60">
<div class="row"><div><label for="gifVariant">Colours</label><select id="gifVariant"><option value="normal">Normal</option><option value="shiny">Shiny</option><option value="both">Normal and shiny</option></select></div><div><label for="gifRegion">Pokémon</label><select id="gifRegion"><option value="all">All Crystal sprites</option><option value="kanto">Kanto (1–151)</option><option value="johto">Johto (152–251)</option><option value="unown">Unown forms</option></select></div></div>
<p id="gifStatus" class="muted" role="status" aria-live="polite">Loading the GIF gallery…</p>
<div id="gifGallery" class="gif-gallery" aria-label="Crystal GIF gallery"></div>
<div class="row"><button id="gifPreviousPage">Previous page</button><span id="gifPage" class="gallery-pages"></span><button id="gifNextPage">Next page</button></div>
<button id="gifPlay" class="wide" style="margin-top:16px" disabled>Play GIF on panel</button>
<label for="gifCries"><input id="gifCries" type="checkbox" checked style="width:auto"> Play matching Pokémon cries on JBL</label>
<p id="gifCryStatus" class="muted" role="status">Matching cries play once when each Pokémon appears. Connect your JBL below; its volume slider controls cries too.</p>
<label for="gifInterval">Seconds per Pokémon</label><input id="gifInterval" type="number" min="2" max="60" value="5">
<button id="gifCycle" class="wide" style="margin-top:12px">Cycle GIF gallery on panel</button>
<p class="muted">Cycling uses every matching sprite across all pages. Keep this page open and visible. Stop leaves the last frame on the panel.</p>
<p class="muted">GIFs downloaded from <a href="https://bluemoonfalls.com/pages/general/crystal-gif-archive" target="_blank" rel="noopener noreferrer">Blue Moon Falls</a>. Original Pokémon artwork © Nintendo / Game Freak.</p>
</div>
<div class="pane" id="pane-draw" role="tabpanel" aria-labelledby="tab-draw" hidden>
<p>Draw directly on the preview with your finger or mouse.</p><label for="brush">Brush colour</label><input id="brush" type="color" value="#ffb267">
<div class="row" style="margin-top:18px"><button id="pencil" class="selected">Pencil</button><button id="eraser">Eraser</button></div>
<p class="muted">Choose “Send to display” when your drawing is ready.</p>
</div>
</section>
</div>
<details class="card connection" id="speakerDetails"><summary>Bluetooth speaker · JBL Go 4</summary>
<p class="muted">Turn on your JBL Go 4 and press its Bluetooth button to enter pairing mode. Scan, select the speaker, then connect.</p>
<p class="hint" id="speakerStatus" role="status" aria-live="polite">Bluetooth is off until you scan for a speaker.</p>
<button id="speakerScan" class="wide">Scan for speakers</button>
<label for="speakerDevice">Available speakers</label><select id="speakerDevice"><option value="">Scan to find your JBL Go 4</option></select>
<div class="row" style="margin-top:12px"><button id="speakerConnect" class="primary" disabled>Connect speaker</button><button id="speakerDisconnect" disabled>Disconnect</button></div>
<label for="speakerVolume">Speaker audio volume <span id="speakerVolumeValue" class="muted">25%</span></label><input id="speakerVolume" type="range" min="0" max="100" value="25">
<button id="speakerTest" class="wide" style="margin-top:12px" disabled>Play a 2-second test sound</button>
<p class="muted">The ESP32 plays matching cries during GIF playback. This volume controls cries and the test sound; you can also adjust volume on the JBL.</p>
<div class="message" id="speakerMessage" role="status" aria-live="polite"></div>
</details>
<details class="card connection" id="wifiDetails"><summary>Wi-Fi connection</summary>
<p id="networkInfo" class="hint">Checking the ESP32…</p>
<form id="wifiForm"><div class="row"><div><label for="ssid">Home Wi-Fi name (2.4 GHz)</label><input id="ssid" name="ssid" maxlength="32" autocomplete="off" required></div><div><label for="wifiPassword">Wi-Fi password</label><input id="wifiPassword" name="password" type="password" minlength="8" maxlength="63" autocomplete="new-password" required></div></div>
<button type="submit" id="saveWiFi" style="margin-top:15px">Save and connect</button></form>
<p class="muted">The credentials are saved on the ESP32. If joining fails, its setup network stays available.</p><div id="wifiMessage" class="message" role="status"></div>
</details>
<footer>Local display control · The last image stays until you replace it or restart the ESP32.</footer>
</main><script>
function rgbaTo565(rgba, width = 64, height = 32) {
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
function textMaskToRgba(mask, foreground, background, sharp = true) {
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

function imagePlacement(iw, ih, mode, w = 64, h = 32) {
  if (!(iw > 0 && ih > 0)) throw new Error('Image has no dimensions');
  if (mode === 'stretch') return { x: 0, y: 0, w, h };
  const scale = mode === 'cover' ? Math.max(w / iw, h / ih) : Math.min(w / iw, h / ih);
  return { x: (w - iw * scale) / 2, y: (h - ih * scale) / 2, w: iw * scale, h: ih * scale };
}

function opaqueBounds(rgba,width,height) {
  if(rgba.length!==width*height*4)throw new Error('Unexpected sprite dimensions');
  let left=width,top=height,right=-1,bottom=-1;
  for(let y=0;y<height;y++)for(let x=0;x<width;x++)if(rgba[(y*width+x)*4+3]) {
    left=Math.min(left,x);right=Math.max(right,x);top=Math.min(top,y);bottom=Math.max(bottom,y);
  }
  return right<0 ? null : {x:left,y:top,w:right-left+1,h:bottom-top+1};
}

function pokemonQuery(value) {
  const query=String(value).trim().toLowerCase().replace(/^#/, '');
  if (/^\d+$/.test(query) && Number(query)>0) return String(Number(query));
  if (/^[a-z][a-z0-9-]*$/.test(query)) return query;
  throw new Error('Enter a Pokémon name or Pokédex number, such as Haunter or 93.');
}

function pokemonEntries(data) {
  const entries=new Map();
  for (const item of data.results || []) {
    const match=/^https:\/\/pokeapi\.co\/api\/v2\/pokemon\/(\d+)\/$/.exec(item.url || '');
    if (match && /^[a-z][a-z0-9-]*$/.test(item.name))
      entries.set(Number(match[1]), {id:Number(match[1]),name:item.name});
  }
  if (!entries.size) throw new Error('The Pokémon catalogue is empty. Try again.');
  return [...entries.values()].sort((a,b)=>a.id-b.id);
}

function filterPokemon(entries, scope, search='') {
  const query=search.trim().toLowerCase().replace(/^#/, '');
  return entries.filter(entry=>(scope!=='gen1' || entry.id<=151) &&
    (!query || (/^\d+$/.test(query) ? entry.id===Number(query) : entry.name.includes(query))));
}

function createPokemonClient(fetcher=fetch, decode=null) {
  const imageCache=new Map(), detailsCache=new Map();
  let cataloguePromise=null;
  const prefix='matrix-pokemon-v1:', cacheAge=7*24*60*60*1000;
  function readCache(key) {
    try {
      const saved=JSON.parse(localStorage.getItem(prefix+key));
      return saved && Date.now()-saved.at<cacheAge ? saved.data : null;
    } catch { return null; }
  }
  function writeCache(key,data) {
    try { localStorage.setItem(prefix+key,JSON.stringify({at:Date.now(),data})); } catch {}
  }
  async function getJSON(key,url,project=value=>value) {
    const cached=readCache(key);if(cached)return cached;
    const controller=new AbortController(), timer=setTimeout(()=>controller.abort(),12000);
    try {
      const response=await fetcher(url,{signal:controller.signal,cache:'force-cache',credentials:'omit'});
      if(response.status===404)throw new Error('No Pokémon found with that name or number.');
      if(!response.ok)throw new Error('Pokémon service unavailable. Try again later.');
      const data=project(await response.json());writeCache(key,data);return data;
    } catch(error) {
      if(error.name==='AbortError')throw new Error('Pokémon download timed out. Check your internet connection.');
      throw error;
    } finally {clearTimeout(timer);}
  }
  async function defaultDecode(url) {
    return new Promise((resolve,reject)=>{
      const image=new Image(),timer=setTimeout(()=>{image.src='';reject(new Error('Sprite download timed out.'));},12000);
      image.crossOrigin='anonymous';
      image.onload=()=>{clearTimeout(timer);resolve(image);};
      image.onerror=()=>{clearTimeout(timer);reject(new Error('Could not download the sprite. Check your internet connection.'));};
      image.src=url;
    });
  }
  return {
    catalogue() {
      if(!cataloguePromise)cataloguePromise=getJSON('catalogue','https://pokeapi.co/api/v2/pokemon?limit=10000')
        .then(pokemonEntries).catch(error=>{cataloguePromise=null;throw error;});
      return cataloguePromise;
    },
    async load(value) {
      const query=pokemonQuery(value);
      let data=detailsCache.get(query);
      if(!data) {
        data=await getJSON('pokemon:'+query,'https://pokeapi.co/api/v2/pokemon/'+query,
          value=>({id:value.id,name:value.name,sprites:{front_default:value.sprites?.front_default || null}}));
        if(!Number.isInteger(data.id)||data.id<=0||!/^[a-z][a-z0-9-]*$/.test(data.name))
          throw new Error('Invalid Pokémon response. Try again.');
        detailsCache.set(query,data);detailsCache.set(String(data.id),data);detailsCache.set(data.name,data);
      }
      const url=data.sprites?.front_default;
      if(!url) {const error=new Error('No front sprite is available for '+data.name+'.');error.code='NO_SPRITE';throw error;}
      if(!url.startsWith('https://raw.githubusercontent.com/PokeAPI/sprites/'))throw new Error('Unsupported sprite source.');
      let image=imageCache.get(url);
      if(!image) {
        image=await (decode || defaultDecode)(url);
        imageCache.set(url,image);
        if(imageCache.size>32)imageCache.delete(imageCache.keys().next().value);
      }
      return {id:data.id,name:data.name,image};
    }
  };
}

// (c) Dean McNamee <dean@gmail.com>, 2013.
//
// https://github.com/deanm/omggif
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// omggif is a JavaScript implementation of a GIF 89a encoder and decoder,
// including animation and compression.  It does not rely on any specific
// underlying system, so should run in the browser, Node, or Plask.

"use strict";

function GifWriter(buf, width, height, gopts) {
  var p = 0;

  var gopts = gopts === undefined ? { } : gopts;
  var loop_count = gopts.loop === undefined ? null : gopts.loop;
  var global_palette = gopts.palette === undefined ? null : gopts.palette;

  if (width <= 0 || height <= 0 || width > 65535 || height > 65535)
    throw new Error("Width/Height invalid.");

  function check_palette_and_num_colors(palette) {
    var num_colors = palette.length;
    if (num_colors < 2 || num_colors > 256 ||  num_colors & (num_colors-1)) {
      throw new Error(
          "Invalid code/color length, must be power of 2 and 2 .. 256.");
    }
    return num_colors;
  }

  // - Header.
  buf[p++] = 0x47; buf[p++] = 0x49; buf[p++] = 0x46;  // GIF
  buf[p++] = 0x38; buf[p++] = 0x39; buf[p++] = 0x61;  // 89a

  // Handling of Global Color Table (palette) and background index.
  var gp_num_colors_pow2 = 0;
  var background = 0;
  if (global_palette !== null) {
    var gp_num_colors = check_palette_and_num_colors(global_palette);
    while (gp_num_colors >>= 1) ++gp_num_colors_pow2;
    gp_num_colors = 1 << gp_num_colors_pow2;
    --gp_num_colors_pow2;
    if (gopts.background !== undefined) {
      background = gopts.background;
      if (background >= gp_num_colors)
        throw new Error("Background index out of range.");
      // The GIF spec states that a background index of 0 should be ignored, so
      // this is probably a mistake and you really want to set it to another
      // slot in the palette.  But actually in the end most browsers, etc end
      // up ignoring this almost completely (including for dispose background).
      if (background === 0)
        throw new Error("Background index explicitly passed as 0.");
    }
  }

  // - Logical Screen Descriptor.
  // NOTE(deanm): w/h apparently ignored by implementations, but set anyway.
  buf[p++] = width & 0xff; buf[p++] = width >> 8 & 0xff;
  buf[p++] = height & 0xff; buf[p++] = height >> 8 & 0xff;
  // NOTE: Indicates 0-bpp original color resolution (unused?).
  buf[p++] = (global_palette !== null ? 0x80 : 0) |  // Global Color Table Flag.
             gp_num_colors_pow2;  // NOTE: No sort flag (unused?).
  buf[p++] = background;  // Background Color Index.
  buf[p++] = 0;  // Pixel aspect ratio (unused?).

  // - Global Color Table
  if (global_palette !== null) {
    for (var i = 0, il = global_palette.length; i < il; ++i) {
      var rgb = global_palette[i];
      buf[p++] = rgb >> 16 & 0xff;
      buf[p++] = rgb >> 8 & 0xff;
      buf[p++] = rgb & 0xff;
    }
  }

  if (loop_count !== null) {  // Netscape block for looping.
    if (loop_count < 0 || loop_count > 65535)
      throw new Error("Loop count invalid.");
    // Extension code, label, and length.
    buf[p++] = 0x21; buf[p++] = 0xff; buf[p++] = 0x0b;
    // NETSCAPE2.0
    buf[p++] = 0x4e; buf[p++] = 0x45; buf[p++] = 0x54; buf[p++] = 0x53;
    buf[p++] = 0x43; buf[p++] = 0x41; buf[p++] = 0x50; buf[p++] = 0x45;
    buf[p++] = 0x32; buf[p++] = 0x2e; buf[p++] = 0x30;
    // Sub-block
    buf[p++] = 0x03; buf[p++] = 0x01;
    buf[p++] = loop_count & 0xff; buf[p++] = loop_count >> 8 & 0xff;
    buf[p++] = 0x00;  // Terminator.
  }


  var ended = false;

  this.addFrame = function(x, y, w, h, indexed_pixels, opts) {
    if (ended === true) { --p; ended = false; }  // Un-end.

    opts = opts === undefined ? { } : opts;

    // TODO(deanm): Bounds check x, y.  Do they need to be within the virtual
    // canvas width/height, I imagine?
    if (x < 0 || y < 0 || x > 65535 || y > 65535)
      throw new Error("x/y invalid.");

    if (w <= 0 || h <= 0 || w > 65535 || h > 65535)
      throw new Error("Width/Height invalid.");

    if (indexed_pixels.length < w * h)
      throw new Error("Not enough pixels for the frame size.");

    var using_local_palette = true;
    var palette = opts.palette;
    if (palette === undefined || palette === null) {
      using_local_palette = false;
      palette = global_palette;
    }

    if (palette === undefined || palette === null)
      throw new Error("Must supply either a local or global palette.");

    var num_colors = check_palette_and_num_colors(palette);

    // Compute the min_code_size (power of 2), destroying num_colors.
    var min_code_size = 0;
    while (num_colors >>= 1) ++min_code_size;
    num_colors = 1 << min_code_size;  // Now we can easily get it back.

    var delay = opts.delay === undefined ? 0 : opts.delay;

    // From the spec:
    //     0 -   No disposal specified. The decoder is
    //           not required to take any action.
    //     1 -   Do not dispose. The graphic is to be left
    //           in place.
    //     2 -   Restore to background color. The area used by the
    //           graphic must be restored to the background color.
    //     3 -   Restore to previous. The decoder is required to
    //           restore the area overwritten by the graphic with
    //           what was there prior to rendering the graphic.
    //  4-7 -    To be defined.
    // NOTE(deanm): Dispose background doesn't really work, apparently most
    // browsers ignore the background palette index and clear to transparency.
    var disposal = opts.disposal === undefined ? 0 : opts.disposal;
    if (disposal < 0 || disposal > 3)  // 4-7 is reserved.
      throw new Error("Disposal out of range.");

    var use_transparency = false;
    var transparent_index = 0;
    if (opts.transparent !== undefined && opts.transparent !== null) {
      use_transparency = true;
      transparent_index = opts.transparent;
      if (transparent_index < 0 || transparent_index >= num_colors)
        throw new Error("Transparent color index.");
    }

    if (disposal !== 0 || use_transparency || delay !== 0) {
      // - Graphics Control Extension
      buf[p++] = 0x21; buf[p++] = 0xf9;  // Extension / Label.
      buf[p++] = 4;  // Byte size.

      buf[p++] = disposal << 2 | (use_transparency === true ? 1 : 0);
      buf[p++] = delay & 0xff; buf[p++] = delay >> 8 & 0xff;
      buf[p++] = transparent_index;  // Transparent color index.
      buf[p++] = 0;  // Block Terminator.
    }

    // - Image Descriptor
    buf[p++] = 0x2c;  // Image Seperator.
    buf[p++] = x & 0xff; buf[p++] = x >> 8 & 0xff;  // Left.
    buf[p++] = y & 0xff; buf[p++] = y >> 8 & 0xff;  // Top.
    buf[p++] = w & 0xff; buf[p++] = w >> 8 & 0xff;
    buf[p++] = h & 0xff; buf[p++] = h >> 8 & 0xff;
    // NOTE: No sort flag (unused?).
    // TODO(deanm): Support interlace.
    buf[p++] = using_local_palette === true ? (0x80 | (min_code_size-1)) : 0;

    // - Local Color Table
    if (using_local_palette === true) {
      for (var i = 0, il = palette.length; i < il; ++i) {
        var rgb = palette[i];
        buf[p++] = rgb >> 16 & 0xff;
        buf[p++] = rgb >> 8 & 0xff;
        buf[p++] = rgb & 0xff;
      }
    }

    p = GifWriterOutputLZWCodeStream(
            buf, p, min_code_size < 2 ? 2 : min_code_size, indexed_pixels);

    return p;
  };

  this.end = function() {
    if (ended === false) {
      buf[p++] = 0x3b;  // Trailer.
      ended = true;
    }
    return p;
  };

  this.getOutputBuffer = function() { return buf; };
  this.setOutputBuffer = function(v) { buf = v; };
  this.getOutputBufferPosition = function() { return p; };
  this.setOutputBufferPosition = function(v) { p = v; };
}

// Main compression routine, palette indexes -> LZW code stream.
// |index_stream| must have at least one entry.
function GifWriterOutputLZWCodeStream(buf, p, min_code_size, index_stream) {
  buf[p++] = min_code_size;
  var cur_subblock = p++;  // Pointing at the length field.

  var clear_code = 1 << min_code_size;
  var code_mask = clear_code - 1;
  var eoi_code = clear_code + 1;
  var next_code = eoi_code + 1;

  var cur_code_size = min_code_size + 1;  // Number of bits per code.
  var cur_shift = 0;
  // We have at most 12-bit codes, so we should have to hold a max of 19
  // bits here (and then we would write out).
  var cur = 0;

  function emit_bytes_to_buffer(bit_block_size) {
    while (cur_shift >= bit_block_size) {
      buf[p++] = cur & 0xff;
      cur >>= 8; cur_shift -= 8;
      if (p === cur_subblock + 256) {  // Finished a subblock.
        buf[cur_subblock] = 255;
        cur_subblock = p++;
      }
    }
  }

  function emit_code(c) {
    cur |= c << cur_shift;
    cur_shift += cur_code_size;
    emit_bytes_to_buffer(8);
  }

  // I am not an expert on the topic, and I don't want to write a thesis.
  // However, it is good to outline here the basic algorithm and the few data
  // structures and optimizations here that make this implementation fast.
  // The basic idea behind LZW is to build a table of previously seen runs
  // addressed by a short id (herein called output code).  All data is
  // referenced by a code, which represents one or more values from the
  // original input stream.  All input bytes can be referenced as the same
  // value as an output code.  So if you didn't want any compression, you
  // could more or less just output the original bytes as codes (there are
  // some details to this, but it is the idea).  In order to achieve
  // compression, values greater then the input range (codes can be up to
  // 12-bit while input only 8-bit) represent a sequence of previously seen
  // inputs.  The decompressor is able to build the same mapping while
  // decoding, so there is always a shared common knowledge between the
  // encoding and decoder, which is also important for "timing" aspects like
  // how to handle variable bit width code encoding.
  //
  // One obvious but very important consequence of the table system is there
  // is always a unique id (at most 12-bits) to map the runs.  'A' might be
  // 4, then 'AA' might be 10, 'AAA' 11, 'AAAA' 12, etc.  This relationship
  // can be used for an effecient lookup strategy for the code mapping.  We
  // need to know if a run has been seen before, and be able to map that run
  // to the output code.  Since we start with known unique ids (input bytes),
  // and then from those build more unique ids (table entries), we can
  // continue this chain (almost like a linked list) to always have small
  // integer values that represent the current byte chains in the encoder.
  // This means instead of tracking the input bytes (AAAABCD) to know our
  // current state, we can track the table entry for AAAABC (it is guaranteed
  // to exist by the nature of the algorithm) and the next character D.
  // Therefor the tuple of (table_entry, byte) is guaranteed to also be
  // unique.  This allows us to create a simple lookup key for mapping input
  // sequences to codes (table indices) without having to store or search
  // any of the code sequences.  So if 'AAAA' has a table entry of 12, the
  // tuple of ('AAAA', K) for any input byte K will be unique, and can be our
  // key.  This leads to a integer value at most 20-bits, which can always
  // fit in an SMI value and be used as a fast sparse array / object key.

  // Output code for the current contents of the index buffer.
  var ib_code = index_stream[0] & code_mask;  // Load first input index.
  var code_table = { };  // Key'd on our 20-bit "tuple".

  emit_code(clear_code);  // Spec says first code should be a clear code.

  // First index already loaded, process the rest of the stream.
  for (var i = 1, il = index_stream.length; i < il; ++i) {
    var k = index_stream[i] & code_mask;
    var cur_key = ib_code << 8 | k;  // (prev, k) unique tuple.
    var cur_code = code_table[cur_key];  // buffer + k.

    // Check if we have to create a new code table entry.
    if (cur_code === undefined) {  // We don't have buffer + k.
      // Emit index buffer (without k).
      // This is an inline version of emit_code, because this is the core
      // writing routine of the compressor (and V8 cannot inline emit_code
      // because it is a closure here in a different context).  Additionally
      // we can call emit_byte_to_buffer less often, because we can have
      // 30-bits (from our 31-bit signed SMI), and we know our codes will only
      // be 12-bits, so can safely have 18-bits there without overflow.
      // emit_code(ib_code);
      cur |= ib_code << cur_shift;
      cur_shift += cur_code_size;
      while (cur_shift >= 8) {
        buf[p++] = cur & 0xff;
        cur >>= 8; cur_shift -= 8;
        if (p === cur_subblock + 256) {  // Finished a subblock.
          buf[cur_subblock] = 255;
          cur_subblock = p++;
        }
      }

      if (next_code === 4096) {  // Table full, need a clear.
        emit_code(clear_code);
        next_code = eoi_code + 1;
        cur_code_size = min_code_size + 1;
        code_table = { };
      } else {  // Table not full, insert a new entry.
        // Increase our variable bit code sizes if necessary.  This is a bit
        // tricky as it is based on "timing" between the encoding and
        // decoder.  From the encoders perspective this should happen after
        // we've already emitted the index buffer and are about to create the
        // first table entry that would overflow our current code bit size.
        if (next_code >= (1 << cur_code_size)) ++cur_code_size;
        code_table[cur_key] = next_code++;  // Insert into code table.
      }

      ib_code = k;  // Index buffer to single input k.
    } else {
      ib_code = cur_code;  // Index buffer to sequence in code table.
    }
  }

  emit_code(ib_code);  // There will still be something in the index buffer.
  emit_code(eoi_code);  // End Of Information.

  // Flush / finalize the sub-blocks stream to the buffer.
  emit_bytes_to_buffer(1);

  // Finish the sub-blocks, writing out any unfinished lengths and
  // terminating with a sub-block of length 0.  If we have already started
  // but not yet used a sub-block it can just become the terminator.
  if (cur_subblock + 1 === p) {  // Started but unused.
    buf[cur_subblock] = 0;
  } else {  // Started and used, write length and additional terminator block.
    buf[cur_subblock] = p - cur_subblock - 1;
    buf[p++] = 0;
  }
  return p;
}

function GifReader(buf) {
  var p = 0;

  // - Header (GIF87a or GIF89a).
  if (buf[p++] !== 0x47 ||            buf[p++] !== 0x49 || buf[p++] !== 0x46 ||
      buf[p++] !== 0x38 || (buf[p++]+1 & 0xfd) !== 0x38 || buf[p++] !== 0x61) {
    throw new Error("Invalid GIF 87a/89a header.");
  }

  // - Logical Screen Descriptor.
  var width = buf[p++] | buf[p++] << 8;
  var height = buf[p++] | buf[p++] << 8;
  var pf0 = buf[p++];  // <Packed Fields>.
  var global_palette_flag = pf0 >> 7;
  var num_global_colors_pow2 = pf0 & 0x7;
  var num_global_colors = 1 << (num_global_colors_pow2 + 1);
  var background = buf[p++];
  buf[p++];  // Pixel aspect ratio (unused?).

  var global_palette_offset = null;
  var global_palette_size   = null;

  if (global_palette_flag) {
    global_palette_offset = p;
    global_palette_size = num_global_colors;
    p += num_global_colors * 3;  // Seek past palette.
  }

  var no_eof = true;

  var frames = [ ];

  var delay = 0;
  var transparent_index = null;
  var disposal = 0;  // 0 - No disposal specified.
  var loop_count = null;

  this.width = width;
  this.height = height;

  while (no_eof && p < buf.length) {
    switch (buf[p++]) {
      case 0x21:  // Graphics Control Extension Block
        switch (buf[p++]) {
          case 0xff:  // Application specific block
            // Try if it's a Netscape block (with animation loop counter).
            if (buf[p   ] !== 0x0b ||  // 21 FF already read, check block size.
                // NETSCAPE2.0
                buf[p+1 ] == 0x4e && buf[p+2 ] == 0x45 && buf[p+3 ] == 0x54 &&
                buf[p+4 ] == 0x53 && buf[p+5 ] == 0x43 && buf[p+6 ] == 0x41 &&
                buf[p+7 ] == 0x50 && buf[p+8 ] == 0x45 && buf[p+9 ] == 0x32 &&
                buf[p+10] == 0x2e && buf[p+11] == 0x30 &&
                // Sub-block
                buf[p+12] == 0x03 && buf[p+13] == 0x01 && buf[p+16] == 0) {
              p += 14;
              loop_count = buf[p++] | buf[p++] << 8;
              p++;  // Skip terminator.
            } else {  // We don't know what it is, just try to get past it.
              p += 12;
              while (true) {  // Seek through subblocks.
                var block_size = buf[p++];
                // Bad block size (ex: undefined from an out of bounds read).
                if (!(block_size >= 0)) throw Error("Invalid block size");
                if (block_size === 0) break;  // 0 size is terminator
                p += block_size;
              }
            }
            break;

          case 0xf9:  // Graphics Control Extension
            if (buf[p++] !== 0x4 || buf[p+4] !== 0)
              throw new Error("Invalid graphics extension block.");
            var pf1 = buf[p++];
            delay = buf[p++] | buf[p++] << 8;
            transparent_index = buf[p++];
            if ((pf1 & 1) === 0) transparent_index = null;
            disposal = pf1 >> 2 & 0x7;
            p++;  // Skip terminator.
            break;

          // Plain Text Extension could be present and we just want to be able
          // to parse past it.  It follows the block structure of the comment
          // extension enough to reuse the path to skip through the blocks.
          case 0x01:  // Plain Text Extension (fallthrough to Comment Extension)
          case 0xfe:  // Comment Extension.
            while (true) {  // Seek through subblocks.
              var block_size = buf[p++];
              // Bad block size (ex: undefined from an out of bounds read).
              if (!(block_size >= 0)) throw Error("Invalid block size");
              if (block_size === 0) break;  // 0 size is terminator
              // console.log(buf.slice(p, p+block_size).toString('ascii'));
              p += block_size;
            }
            break;

          default:
            throw new Error(
                "Unknown graphic control label: 0x" + buf[p-1].toString(16));
        }
        break;

      case 0x2c:  // Image Descriptor.
        var x = buf[p++] | buf[p++] << 8;
        var y = buf[p++] | buf[p++] << 8;
        var w = buf[p++] | buf[p++] << 8;
        var h = buf[p++] | buf[p++] << 8;
        var pf2 = buf[p++];
        var local_palette_flag = pf2 >> 7;
        var interlace_flag = pf2 >> 6 & 1;
        var num_local_colors_pow2 = pf2 & 0x7;
        var num_local_colors = 1 << (num_local_colors_pow2 + 1);
        var palette_offset = global_palette_offset;
        var palette_size = global_palette_size;
        var has_local_palette = false;
        if (local_palette_flag) {
          var has_local_palette = true;
          palette_offset = p;  // Override with local palette.
          palette_size = num_local_colors;
          p += num_local_colors * 3;  // Seek past palette.
        }

        var data_offset = p;

        p++;  // codesize
        while (true) {
          var block_size = buf[p++];
          // Bad block size (ex: undefined from an out of bounds read).
          if (!(block_size >= 0)) throw Error("Invalid block size");
          if (block_size === 0) break;  // 0 size is terminator
          p += block_size;
        }

        frames.push({x: x, y: y, width: w, height: h,
                     has_local_palette: has_local_palette,
                     palette_offset: palette_offset,
                     palette_size: palette_size,
                     data_offset: data_offset,
                     data_length: p - data_offset,
                     transparent_index: transparent_index,
                     interlaced: !!interlace_flag,
                     delay: delay,
                     disposal: disposal});
        break;

      case 0x3b:  // Trailer Marker (end of file).
        no_eof = false;
        break;

      case 0x00:  // Tolerate NUL padding between blocks (Crystal shiny Magmar).
        break;

      default:
        throw new Error("Unknown gif block: 0x" + buf[p-1].toString(16));
        break;
    }
  }

  this.numFrames = function() {
    return frames.length;
  };

  this.loopCount = function() {
    return loop_count;
  };

  this.frameInfo = function(frame_num) {
    if (frame_num < 0 || frame_num >= frames.length)
      throw new Error("Frame index out of range.");
    return frames[frame_num];
  };

  this.decodeAndBlitFrameBGRA = function(frame_num, pixels) {
    var frame = this.frameInfo(frame_num);
    var num_pixels = frame.width * frame.height;
    var index_stream = new Uint8Array(num_pixels);  // At most 8-bit indices.
    GifReaderLZWOutputIndexStream(
        buf, frame.data_offset, index_stream, num_pixels);
    var palette_offset = frame.palette_offset;

    // NOTE(deanm): It seems to be much faster to compare index to 256 than
    // to === null.  Not sure why, but CompareStub_EQ_STRICT shows up high in
    // the profile, not sure if it's related to using a Uint8Array.
    var trans = frame.transparent_index;
    if (trans === null) trans = 256;

    // We are possibly just blitting to a portion of the entire frame.
    // That is a subrect within the framerect, so the additional pixels
    // must be skipped over after we finished a scanline.
    var framewidth  = frame.width;
    var framestride = width - framewidth;
    var xleft       = framewidth;  // Number of subrect pixels left in scanline.

    // Output index of the top left corner of the subrect.
    var opbeg = ((frame.y * width) + frame.x) * 4;
    // Output index of what would be the left edge of the subrect, one row
    // below it, i.e. the index at which an interlace pass should wrap.
    var opend = ((frame.y + frame.height) * width + frame.x) * 4;
    var op    = opbeg;

    var scanstride = framestride * 4;

    // Use scanstride to skip past the rows when interlacing.  This is skipping
    // 7 rows for the first two passes, then 3 then 1.
    if (frame.interlaced === true) {
      scanstride += width * 4 * 7;  // Pass 1.
    }

    var interlaceskip = 8;  // Tracking the row interval in the current pass.

    for (var i = 0, il = index_stream.length; i < il; ++i) {
      var index = index_stream[i];

      if (xleft === 0) {  // Beginning of new scan line
        op += scanstride;
        xleft = framewidth;
        if (op >= opend) { // Catch the wrap to switch passes when interlacing.
          scanstride = framestride * 4 + width * 4 * (interlaceskip-1);
          // interlaceskip / 2 * 4 is interlaceskip << 1.
          op = opbeg + (framewidth + framestride) * (interlaceskip << 1);
          interlaceskip >>= 1;
        }
      }

      if (index === trans) {
        op += 4;
      } else {
        var r = buf[palette_offset + index * 3];
        var g = buf[palette_offset + index * 3 + 1];
        var b = buf[palette_offset + index * 3 + 2];
        pixels[op++] = b;
        pixels[op++] = g;
        pixels[op++] = r;
        pixels[op++] = 255;
      }
      --xleft;
    }
  };

  // I will go to copy and paste hell one day...
  this.decodeAndBlitFrameRGBA = function(frame_num, pixels) {
    var frame = this.frameInfo(frame_num);
    var num_pixels = frame.width * frame.height;
    var index_stream = new Uint8Array(num_pixels);  // At most 8-bit indices.
    GifReaderLZWOutputIndexStream(
        buf, frame.data_offset, index_stream, num_pixels);
    var palette_offset = frame.palette_offset;

    // NOTE(deanm): It seems to be much faster to compare index to 256 than
    // to === null.  Not sure why, but CompareStub_EQ_STRICT shows up high in
    // the profile, not sure if it's related to using a Uint8Array.
    var trans = frame.transparent_index;
    if (trans === null) trans = 256;

    // We are possibly just blitting to a portion of the entire frame.
    // That is a subrect within the framerect, so the additional pixels
    // must be skipped over after we finished a scanline.
    var framewidth  = frame.width;
    var framestride = width - framewidth;
    var xleft       = framewidth;  // Number of subrect pixels left in scanline.

    // Output index of the top left corner of the subrect.
    var opbeg = ((frame.y * width) + frame.x) * 4;
    // Output index of what would be the left edge of the subrect, one row
    // below it, i.e. the index at which an interlace pass should wrap.
    var opend = ((frame.y + frame.height) * width + frame.x) * 4;
    var op    = opbeg;

    var scanstride = framestride * 4;

    // Use scanstride to skip past the rows when interlacing.  This is skipping
    // 7 rows for the first two passes, then 3 then 1.
    if (frame.interlaced === true) {
      scanstride += width * 4 * 7;  // Pass 1.
    }

    var interlaceskip = 8;  // Tracking the row interval in the current pass.

    for (var i = 0, il = index_stream.length; i < il; ++i) {
      var index = index_stream[i];

      if (xleft === 0) {  // Beginning of new scan line
        op += scanstride;
        xleft = framewidth;
        if (op >= opend) { // Catch the wrap to switch passes when interlacing.
          scanstride = framestride * 4 + width * 4 * (interlaceskip-1);
          // interlaceskip / 2 * 4 is interlaceskip << 1.
          op = opbeg + (framewidth + framestride) * (interlaceskip << 1);
          interlaceskip >>= 1;
        }
      }

      if (index === trans) {
        op += 4;
      } else {
        var r = buf[palette_offset + index * 3];
        var g = buf[palette_offset + index * 3 + 1];
        var b = buf[palette_offset + index * 3 + 2];
        pixels[op++] = r;
        pixels[op++] = g;
        pixels[op++] = b;
        pixels[op++] = 255;
      }
      --xleft;
    }
  };
}

function GifReaderLZWOutputIndexStream(code_stream, p, output, output_length) {
  var min_code_size = code_stream[p++];

  var clear_code = 1 << min_code_size;
  var eoi_code = clear_code + 1;
  var next_code = eoi_code + 1;

  var cur_code_size = min_code_size + 1;  // Number of bits per code.
  // NOTE: This shares the same name as the encoder, but has a different
  // meaning here.  Here this masks each code coming from the code stream.
  var code_mask = (1 << cur_code_size) - 1;
  var cur_shift = 0;
  var cur = 0;

  var op = 0;  // Output pointer.

  var subblock_size = code_stream[p++];

  // TODO(deanm): Would using a TypedArray be any faster?  At least it would
  // solve the fast mode / backing store uncertainty.
  // var code_table = Array(4096);
  var code_table = new Int32Array(4096);  // Can be signed, we only use 20 bits.

  var prev_code = null;  // Track code-1.

  while (true) {
    // Read up to two bytes, making sure we always 12-bits for max sized code.
    while (cur_shift < 16) {
      if (subblock_size === 0) break;  // No more data to be read.

      cur |= code_stream[p++] << cur_shift;
      cur_shift += 8;

      if (subblock_size === 1) {  // Never let it get to 0 to hold logic above.
        subblock_size = code_stream[p++];  // Next subblock.
      } else {
        --subblock_size;
      }
    }

    // TODO(deanm): We should never really get here, we should have received
    // and EOI.
    if (cur_shift < cur_code_size)
      break;

    var code = cur & code_mask;
    cur >>= cur_code_size;
    cur_shift -= cur_code_size;

    // TODO(deanm): Maybe should check that the first code was a clear code,
    // at least this is what you're supposed to do.  But actually our encoder
    // now doesn't emit a clear code first anyway.
    if (code === clear_code) {
      // We don't actually have to clear the table.  This could be a good idea
      // for greater error checking, but we don't really do any anyway.  We
      // will just track it with next_code and overwrite old entries.

      next_code = eoi_code + 1;
      cur_code_size = min_code_size + 1;
      code_mask = (1 << cur_code_size) - 1;

      // Don't update prev_code ?
      prev_code = null;
      continue;
    } else if (code === eoi_code) {
      break;
    }

    // We have a similar situation as the decoder, where we want to store
    // variable length entries (code table entries), but we want to do in a
    // faster manner than an array of arrays.  The code below stores sort of a
    // linked list within the code table, and then "chases" through it to
    // construct the dictionary entries.  When a new entry is created, just the
    // last byte is stored, and the rest (prefix) of the entry is only
    // referenced by its table entry.  Then the code chases through the
    // prefixes until it reaches a single byte code.  We have to chase twice,
    // first to compute the length, and then to actually copy the data to the
    // output (backwards, since we know the length).  The alternative would be
    // storing something in an intermediate stack, but that doesn't make any
    // more sense.  I implemented an approach where it also stored the length
    // in the code table, although it's a bit tricky because you run out of
    // bits (12 + 12 + 8), but I didn't measure much improvements (the table
    // entries are generally not the long).  Even when I created benchmarks for
    // very long table entries the complexity did not seem worth it.
    // The code table stores the prefix entry in 12 bits and then the suffix
    // byte in 8 bits, so each entry is 20 bits.

    var chase_code = code < next_code ? code : prev_code;

    // Chase what we will output, either {CODE} or {CODE-1}.
    var chase_length = 0;
    var chase = chase_code;
    while (chase > clear_code) {
      chase = code_table[chase] >> 8;
      ++chase_length;
    }

    var k = chase;

    var op_end = op + chase_length + (chase_code !== code ? 1 : 0);
    if (op_end > output_length) {
      console.log("Warning, gif stream longer than expected.");
      return;
    }

    // Already have the first byte from the chase, might as well write it fast.
    output[op++] = k;

    op += chase_length;
    var b = op;  // Track pointer, writing backwards.

    if (chase_code !== code)  // The case of emitting {CODE-1} + k.
      output[op++] = k;

    chase = chase_code;
    while (chase_length--) {
      chase = code_table[chase];
      output[--b] = chase & 0xff;  // Write backwards.
      chase >>= 8;  // Pull down to the prefix code.
    }

    if (prev_code !== null && next_code < 4096) {
      code_table[next_code++] = prev_code << 8 | k;
      // TODO(deanm): Figure out this clearing vs code growth logic better.  I
      // have an feeling that it should just happen somewhere else, for now it
      // is awkward between when we grow past the max and then hit a clear code.
      // For now just check if we hit the max 12-bits (then a clear code should
      // follow, also of course encoded in 12-bits).
      if (next_code >= code_mask+1 && cur_code_size < 12) {
        ++cur_code_size;
        code_mask = code_mask << 1 | 1;
      }
    }

    prev_code = code;
  }

  if (op !== output_length) {
    console.log("Warning, gif stream shorter than expected.");
  }

  return output;
}

// CommonJS.
try { exports.GifWriter = GifWriter; exports.GifReader = GifReader } catch(e) {}




// Decode and composite once; browser playback then sends normal RGB565 frames.
function decodeGIF(buffer) {
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

function filterCrystal(entries,query='',variant='normal',region='all') {
  const search=query.trim().toLowerCase().replace(/^#/,'');
  return entries.filter(entry=>(variant==='both'||entry.variant===variant)&&
    (region==='all'||(region==='kanto'&&entry.id<=151)||(region==='johto'&&entry.id>151)||(region==='unown'&&entry.id===201))&&
    (!search||(/^\d+$/.test(search)?entry.id===Number(search):entry.name.includes(search))));
}
const $ = id => document.getElementById(id);
const canvas = $('canvas'), ctx = canvas.getContext('2d', {willReadFrequently:true});
const textMask = document.createElement('canvas'); textMask.width=64; textMask.height=32;
const textCtx = textMask.getContext('2d', {willReadFrequently:true});
let tab = 'text', loadedImage = null, imageGeneration = 0, erasing = false, drawing = false;
let scrolling = false, scrollOffset = 64, streamTimer = null, busy = false, streamGeneration = 0;
let lastStatus = null, initialized = false, wifiWasEdited = false, statusRefreshing = false;
const pokemonClient=createPokemonClient();
let pokemonCatalog=[],pokemonCurrent=null,pokemonCycling=false,pokemonTimer=null,pokemonGeneration=0,pokemonLoading=false;
let pokemonPool=[],pokemonCursor=0;
function message(text, error = false) { $('message').textContent = text; $('message').className = 'message ' + (error ? 'error' : 'success'); }
let deviceRequests=Promise.resolve();
function withDeviceRequest(work) {
  const pending=deviceRequests.then(work);deviceRequests=pending.catch(()=>{});return pending;
}
function request(path, options = {}) {
  return withDeviceRequest(()=>performRequest(path,options));
}
async function performRequest(path, options = {}) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 8000);
  try {
    const response = await fetch(path, {...options, signal:controller.signal, cache:'no-store',
      headers:{'X-Matrix-Control':'1', ...(options.headers || {})}});
    const data = await response.json();
    if (!response.ok) throw new Error(data.error || `Request failed (${response.status})`);
    return data;
  } catch (error) {
    if (error.name === 'AbortError') throw new Error('The ESP32 did not respond. Check your Wi-Fi connection.');
    throw error;
  } finally { clearTimeout(timer); }
}
function background(colour = '#000000') { ctx.fillStyle = colour; ctx.fillRect(0,0,64,32); }
function size() { return Math.max(6,Math.min(32,Number($('fontSize').value) || 10)); }
function textStyle() {
  textCtx.clearRect(0,0,64,32); textCtx.font=`${size()}px ${$('font').value}`;
  textCtx.textBaseline='middle'; textCtx.fillStyle='#ffffff';
}
function compositeText() {
  const pixels=textCtx.getImageData(0,0,64,32);
  pixels.data.set(textMaskToRgba(pixels.data,$('textColor').value,$('background').value,$('sharpText').checked));
  ctx.putImageData(pixels,0,0);
}
function renderText() {
  textStyle(); textCtx.textAlign=$('align').value;
  const lines=$('text').value.split('\n'), step=size()+1;
  const x=textCtx.textAlign==='left'?1:textCtx.textAlign==='right'?63:32;
  lines.forEach((line,i)=>textCtx.fillText(line,x,Math.round(16+(i-(lines.length-1)/2)*step)));
  compositeText();
}
function renderImage() {
  if(imageGif){gifCurrent=imageGif;renderGifFrame();return;}
  if(!loadedImage) return;
  background();ctx.imageSmoothingEnabled=$('smoothing').value==='smooth';
  const p=imagePlacement(loadedImage.width,loadedImage.height,$('fit').value);
  ctx.drawImage(loadedImage,p.x,p.y,p.w,p.h);
}
function setTab(next) {
  stopScroll(); tab=next;
  document.querySelectorAll('[data-tab]').forEach(b=>b.setAttribute('aria-selected',String(b.dataset.tab===next)));
  document.querySelectorAll('.pane').forEach(p=>p.hidden=p.id!==`pane-${next}`);
  if(next==='text')renderText(); else if(next==='image')renderImage();
  else if(next==='pokemon') {
    if(pokemonCurrent)renderPokemon();
    loadPokemonCatalog().catch(error=>message(error.message,true));
  }
  else if(next==='gallery') {
    gifCurrent=galleryGif;if(gifCurrent)renderGifFrame();else background();
    loadGifCatalogue().catch(error=>message(error.message,true));
  }
}
function updateSendButton() { $('send').disabled=busy||pokemonLoading||gifLoading; }
function stopPokemonCycle() {
  pokemonCycling=false;pokemonGeneration++;clearTimeout(pokemonTimer);pokemonLoading=false;
  $('pokemonCycle').textContent='Start cycling on panel';$('pokemonLoad').disabled=false;updateSendButton();
}
function pokemonLabel(pokemon) { return '#'+pokemon.id+' · '+pokemon.name.split('-').map(word=>word[0].toUpperCase()+word.slice(1)).join(' '); }
function refreshPokemonResults() {
  const matches=filterPokemon(pokemonCatalog,$('pokemonScope').value,$('pokemonQuery').value);
  $('pokemonResults').replaceChildren();
  for(const pokemon of matches.slice(0,100)) {
    const option=document.createElement('option');option.value=String(pokemon.id);option.textContent=pokemonLabel(pokemon);
    option.selected=pokemonCurrent?.id===pokemon.id;$('pokemonResults').append(option);
  }
  $('pokemonStatus').textContent=pokemonCatalog.length
    ? `${pokemonCatalog.length} catalogue entries loaded; ${matches.length} matches${matches.length>100?' (first 100 shown)':''}.`
    : 'Loading Pokémon catalogue…';
}
async function loadPokemonCatalog() {
  if(!pokemonCatalog.length)pokemonCatalog=await pokemonClient.catalogue();
  refreshPokemonResults();return pokemonCatalog;
}
function renderPokemon() {
  if(!pokemonCurrent)return;
  const image=pokemonCurrent.image,source=document.createElement('canvas');
  source.width=image.naturalWidth||image.width;source.height=image.naturalHeight||image.height;
  const sourceCtx=source.getContext('2d',{willReadFrequently:true});sourceCtx.drawImage(image,0,0);
  const bounds=opaqueBounds(sourceCtx.getImageData(0,0,source.width,source.height).data,source.width,source.height);
  if(!bounds)throw new Error('This sprite is empty. Choose another Pokémon.');
  const scale=Math.min(64/bounds.w,32/bounds.h),w=Math.max(1,Math.floor(bounds.w*scale)),h=Math.max(1,Math.floor(bounds.h*scale));
  background();ctx.imageSmoothingEnabled=false;
  ctx.drawImage(image,bounds.x,bounds.y,bounds.w,bounds.h,Math.floor((64-w)/2),Math.floor((32-h)/2),w,h);
  $('pokemonStatus').textContent=pokemonLabel(pokemonCurrent)+' ready.';
}
async function previewPokemon(query) {
  stopScroll();const generation=++pokemonGeneration;pokemonLoading=true;$('pokemonLoad').disabled=true;updateSendButton();
  try {
    const pokemon=await pokemonClient.load(query);
    if(generation!==pokemonGeneration||tab!=='pokemon')return;
    pokemonCurrent=pokemon;renderPokemon();message('Sprite ready. Choose “Send to display”.');
  } catch(error) {if(generation===pokemonGeneration)message(error.message,true);}
  finally {if(generation===pokemonGeneration){pokemonLoading=false;$('pokemonLoad').disabled=false;updateSendButton();}}
}
$('pokemonSearchForm').onsubmit=event=>{event.preventDefault();previewPokemon($('pokemonQuery').value);};
$('pokemonQuery').oninput=()=>{stopScroll();refreshPokemonResults();};
$('pokemonScope').onchange=()=>{stopScroll();refreshPokemonResults();};
$('pokemonResults').onchange=()=>previewPokemon($('pokemonResults').value);
async function stepPokemon(direction) {
  stopScroll();const generation=pokemonGeneration;
  try {
    await loadPokemonCatalog();if(generation!==pokemonGeneration||tab!=='pokemon')return;
    const pool=filterPokemon(pokemonCatalog,$('pokemonScope').value),current=pool.findIndex(pokemon=>pokemon.id===pokemonCurrent?.id);
    if(!pool.length)throw new Error('No Pokémon in this catalogue.');
    const index=current<0?(direction>0?0:pool.length-1):(current+direction+pool.length)%pool.length;
    $('pokemonQuery').value=pool[index].name;refreshPokemonResults();previewPokemon(pool[index].id);
  } catch(error){if(generation===pokemonGeneration)message(error.message,true);}
}
$('pokemonPrevious').onclick=()=>stepPokemon(-1);$('pokemonNext').onclick=()=>stepPokemon(1);
async function pokemonTick(generation,skipped=0) {
  if(!pokemonCycling||generation!==pokemonGeneration)return;
  try {
    pokemonLoading=true;updateSendButton();
    const pokemon=await pokemonClient.load(pokemonPool[pokemonCursor].id);
    if(!pokemonCycling||generation!==pokemonGeneration)return;
    pokemonCurrent=pokemon;renderPokemon();pokemonLoading=false;updateSendButton();
    const sent=await sendCanvas();if(!pokemonCycling||generation!==pokemonGeneration)return;
    if(sent){message('Cycling: '+pokemonLabel(pokemon)+'.');pokemonCursor=(pokemonCursor+1)%pokemonPool.length;}
    const seconds=Math.max(2,Math.min(60,Number($('pokemonInterval').value)||5));
    pokemonTimer=setTimeout(()=>pokemonTick(generation),sent?seconds*1000:200);
  } catch(error) {
    if(generation!==pokemonGeneration)return;
    if(error.code==='NO_SPRITE'&&skipped+1<pokemonPool.length) {
      pokemonCursor=(pokemonCursor+1)%pokemonPool.length;
      pokemonTimer=setTimeout(()=>pokemonTick(generation,skipped+1),200);
    } else {stopPokemonCycle();message(error.message,true);}
  }
}
$('pokemonCycle').onclick=async()=>{
  if(pokemonCycling){stopScroll();message('Cycling stopped. The last sprite remains on the panel.');return;}
  stopScroll();const generation=++pokemonGeneration;
  try {
    if(document.hidden)throw new Error('Keep this page visible to cycle sprites.');
    await loadPokemonCatalog();if(generation!==pokemonGeneration||tab!=='pokemon')return;
    pokemonPool=filterPokemon(pokemonCatalog,$('pokemonScope').value);
    if(!pokemonPool.length)throw new Error('No Pokémon in this catalogue.');
    pokemonCursor=Math.max(0,pokemonPool.findIndex(pokemon=>pokemon.id===pokemonCurrent?.id));
    pokemonCycling=true;$('pokemonCycle').textContent='Stop cycling';pokemonTick(generation);
  } catch(error){if(generation===pokemonGeneration){stopPokemonCycle();message(error.message,true);}}
};
document.querySelectorAll('[data-tab]').forEach(button=>button.onclick=()=>setTab(button.dataset.tab));
['text','fontSize','font','textColor','background','align','sharpText'].forEach(id=>$(id).addEventListener('input',()=>{stopScroll();renderText();}));
['fit','smoothing'].forEach(id=>$(id).onchange=renderImage);
$('imageFile').onchange=async()=>{
  const file=$('imageFile').files[0]; if(!file)return;
  stopScroll();
  const generation=++imageGeneration;
  const gifToken=gifGeneration;
  try {
    if(file.size>20*1024*1024)throw new Error('Choose an image smaller than 20 MB.');
    const signature=new TextDecoder().decode(await file.slice(0,6).arrayBuffer());
    if(signature==='GIF87a'||signature==='GIF89a') {
      gifLoading=true;updateSendButton();
      const animation=decodeGIF(await file.arrayBuffer());
      if(generation!==imageGeneration||gifToken!==gifGeneration||tab!=='image')return;
      if(loadedImage?.close)loadedImage.close();loadedImage=null;
      imageGif={animation,owner:'image',name:file.name};gifCurrent=imageGif;gifFrameIndex=0;
      $('imageGifPlay').hidden=false;$('smoothing').value='pixel';
      gifLoading=false;startGifPreview();message('GIF ready. Choose “Play GIF on panel” to animate it.');return;
    }
    let image;
    if(typeof createImageBitmap==='function') image=await createImageBitmap(file);
    else image=await new Promise((resolve,reject)=>{
      const url=URL.createObjectURL(file), img=new Image();
      img.onload=()=>{URL.revokeObjectURL(url);resolve(img);};
      img.onerror=()=>{URL.revokeObjectURL(url);reject(new Error('This image format could not be opened.'));};img.src=url;
    });
    if(generation!==imageGeneration||gifToken!==gifGeneration||tab!=='image'){if(image.close)image.close();return;}
    imageGif=null;$('imageGifPlay').hidden=true;
    if(loadedImage&&loadedImage.close)loadedImage.close();loadedImage=image;renderImage();
    message('Image ready. Choose “Send to display”.');
  }catch(error){if(gifToken===gifGeneration)message(error.message,true);}
  finally {if(gifToken===gifGeneration){gifLoading=false;updateSendButton();}}
};
$('pattern').onclick=()=>{
  stopScroll();background();
  ['#ff0000','#00ff00','#0000ff'].forEach((c,i)=>{ctx.fillStyle=c;ctx.fillRect(i*21,0,i===2?22:21,24);});
  ctx.fillStyle='#ffffff';ctx.font='7px monospace';ctx.textAlign='left';ctx.textBaseline='top';ctx.fillText('RGB TEST',2,25);
};
$('purplePattern').onclick=()=>{
  stopScroll();background();
  ['#ff0000','#00ff00','#0000ff','#ff00ff','#8000c0','#400060','#b090e0','#ffffff'].forEach((colour,i)=>{
    ctx.fillStyle=colour;ctx.fillRect(i*8,0,8,32);
  });
  message('Left to right: red, green, blue, magenta, purple, dark purple, lavender, white. Send to compare.');
};
function paint(event){
  const r=canvas.getBoundingClientRect();const x=Math.floor((event.clientX-r.left)*64/r.width),y=Math.floor((event.clientY-r.top)*32/r.height);
  if(x<0||x>=64||y<0||y>=32)return;
  ctx.fillStyle=erasing?'#000000':$('brush').value;ctx.fillRect(x,y,1,1);
}
canvas.onpointerdown=event=>{if(tab!=='draw')return;drawing=true;canvas.setPointerCapture(event.pointerId);paint(event);};
canvas.onpointermove=event=>{if(drawing)paint(event);};
canvas.onpointerup=canvas.onpointercancel=()=>drawing=false;
$('pencil').onclick=()=>{erasing=false;$('pencil').classList.add('selected');$('eraser').classList.remove('selected');};
$('eraser').onclick=()=>{erasing=true;$('eraser').classList.add('selected');$('pencil').classList.remove('selected');};
async function sendCanvas(cryId=0,generation=null){
  if(busy)return false;
  busy=true;updateSendButton();
  try{
    if(cryId)await gifCryStop;
    if(generation!==null&&generation!==gifGeneration)return false;
    const pixels=rgbaTo565(ctx.getImageData(0,0,64,32).data);
    const form=new FormData();form.append('frame',new Blob([pixels],{type:'application/octet-stream'}),'frame.rgb565');
    if(cryId)form.append('cry',String(cryId));
    const uploading=request('/api/frame',{method:'POST',body:form});
    if(cryId){gifCryOwned=true;gifCryUpload=uploading;}
    const result=await uploading;
    if(cryId){
      if(gifCryUpload===uploading)gifCryUpload=null;
      if((generation===null||generation===gifGeneration)&&$('gifCries').checked)cryNotice(result.cry);
    }
    return true;
  }finally{busy=false;updateSendButton();}
}
$('send').onclick=async()=>{stopScroll();try{if(await sendCanvas())message('Sent to your display.');}catch(e){message(e.message,true);}};
$('clear').onclick=()=>{stopScroll();background();message('Preview cleared. Send it to clear the panel.');};
function stopScroll(){scrolling=false;streamGeneration++;clearTimeout(streamTimer);$('scroll').textContent='Start scrolling on panel';stopGifPlayback();stopPokemonCycle();}
async function streamTick(generation){
  if(!scrolling||generation!==streamGeneration)return;
  const started=performance.now();
  textStyle();textCtx.textAlign='left';
  const text=$('text').value.replace(/\n/g,' ');
  textCtx.fillText(text,scrollOffset,16);const width=textCtx.measureText(text).width;
  compositeText();
  try{if(await sendCanvas()){if(generation!==streamGeneration)return;scrollOffset-=1;if(scrollOffset < -width)scrollOffset=64;}}
  catch(e){if(generation===streamGeneration){stopScroll();message(e.message,true);}return;}
  if(scrolling&&generation===streamGeneration)streamTimer=setTimeout(()=>streamTick(generation),Math.max(0,125-(performance.now()-started)));
}
$('scroll').onclick=()=>{if(scrolling){stopScroll();message('Scrolling stopped. The last frame remains on the panel.');return;}
  if(!$('text').value.trim()){message('Enter a message first.',true);return;}
  scrolling=true;scrollOffset=64;$('scroll').textContent='Stop scrolling';message('Streaming. Keep this page open and visible.');streamTick(++streamGeneration);};
document.addEventListener('visibilitychange',()=>{if(document.hidden&&(scrolling||pokemonCycling||pokemonLoading||gifRunning||gifLoading)){stopScroll();message('Playback paused because this page was hidden.');}});
$('brightness').oninput=()=>$('brightnessValue').textContent=Math.round(Number($('brightness').value)/255*100)+'%';
$('brightness').onchange=async()=>{try{await request('/api/brightness',{method:'POST',body:new URLSearchParams({value:$('brightness').value})});message('Brightness updated.');}catch(e){message(e.message,true);}};
$('ssid').oninput=()=>wifiWasEdited=true;
$('wifiForm').onsubmit=async event=>{
  event.preventDefault();stopScroll();$('saveWiFi').disabled=true;
  try{const result=await request('/api/wifi',{method:'POST',body:new URLSearchParams({ssid:$('ssid').value,password:$('wifiPassword').value})});
    $('wifiPassword').value='';$('wifiMessage').textContent=result.message;$('wifiMessage').className='message success';
  }catch(e){$('wifiMessage').textContent=e.message;$('wifiMessage').className='message error';}
  finally{$('saveWiFi').disabled=false;}
};
async function refreshStatus(){
  if(busy||statusRefreshing||document.hidden)return;
  statusRefreshing=true;
  try{
    const data=await request('/api/status');lastStatus=data;
    $('connectionBadge').textContent=data.connected?'On your Wi-Fi':'Setup network';
    $('networkInfo').replaceChildren();
    if(data.connected){
      $('networkInfo').append(document.createTextNode(`Connected to ${data.ssid}. On your home network, open `));
      const link=document.createElement('a');link.href='http://'+data.ip;link.textContent=link.href;$('networkInfo').append(link);
      $('networkInfo').append(document.createTextNode(`. You can also try http://${data.hostname}.`));
    }else{$('networkInfo').textContent='Enter your home Wi-Fi details below, or use the display directly through this setup network.';}
    if(!initialized){
      $('brightness').value=data.brightness;$('brightness').oninput();
      if(!wifiWasEdited)$('ssid').value=data.ssid;
      if(!data.connected)$('wifiDetails').open=true;
      initialized=true;
    }
  }catch(e){$('connectionBadge').textContent='ESP32 unreachable';}
  finally{statusRefreshing=false;}
}
let gifEntries=[],gifMatches=[],gifPageIndex=0,gifCataloguePromise=null;
let gifCurrent=null,imageGif=null,galleryGif=null,gifFrameIndex=0;
let gifGeneration=0,gifTimer=null,gifRunning=false,gifPanelPlaying=false,gifLoading=false,gifCycling=false;
let gifPool=[],gifPoolIndex=0,gifSwitchAt=0;
let gifCryPending=false,gifCryOwned=false,gifCryUpload=null,gifCryStop=Promise.resolve();
const gifCache=new Map(),gifScratch=document.createElement('canvas');
let gifThumbnailGeneration=0;
const gifThumbnailUrls=new Map();
const gifScratchCtx=gifScratch.getContext('2d');

function gifLabel(entry) {
  return '#'+entry.id+' · '+entry.name.split('-').map(word=>word[0].toUpperCase()+word.slice(1)).join(' ')+
    (entry.variant==='shiny'?' · Shiny':'');
}
function updateGifButtons() {
  $('gifPlay').disabled=!galleryGif||gifLoading;
  $('gifPlay').textContent=gifPanelPlaying&&gifCurrent?.owner==='gallery'?'Stop GIF playback':'Play GIF on panel';
  $('imageGifPlay').disabled=gifLoading;
  $('imageGifPlay').textContent=gifPanelPlaying&&gifCurrent?.owner==='image'?'Stop GIF playback':'Play GIF on panel';
  $('gifCycle').textContent=gifCycling?'Stop cycling GIFs':'Cycle GIF gallery on panel';
  document.querySelectorAll('.gallery-tile').forEach(button=>
    button.setAttribute('aria-pressed',String(button.dataset.key===galleryGif?.entry.key)));
  updateSendButton();
}
function stopGifPlayback() {
  stopGifCry();
  gifGeneration++;clearTimeout(gifTimer);gifRunning=false;gifPanelPlaying=false;gifCycling=false;gifLoading=false;
  $('gifCryStatus').textContent=$('gifCries').checked?'Matching cries play once when each Pokémon appears.':'Pokémon cries muted.';
  updateGifButtons();
}
function stopGifCry() {
  if(!gifCryOwned)return;
  gifCryOwned=false;const pending=gifCryUpload;
  // Wait for a pending first frame before stopping its sound. New playback
  // waits for this stop, so a slow old upload cannot start a stale cry later.
  gifCryStop=gifCryStop.then(()=>pending?.catch(()=>{}))
    .then(()=>request('/api/speaker/cry/stop',{method:'POST'})).catch(()=>{});
}
function cryNotice(result) {
  $('gifCryStatus').textContent=result==='queued'?'Matching cry sent to your JBL.':
    result==='disconnected'?'GIF playing silently. Connect your JBL below to hear Pokémon cries.':
    'Matching cries play once when each Pokémon appears.';
}
$('gifCries').onchange=()=>{
  if(!$('gifCries').checked){stopGifCry();gifCryPending=false;}
  $('gifCryStatus').textContent=$('gifCries').checked?
    'Matching cries play when playback starts or the next Pokémon appears.':'Pokémon cries muted.';
};
async function loadGifCatalogue() {
  if(!gifCataloguePromise)gifCataloguePromise=(async()=>{
    const data=await request('/sprites/crystal.json');
    if(!Array.isArray(data.entries)||!data.entries.length)throw new Error('The GIF gallery is empty.');
    for(const entry of data.entries) {
      if(!Number.isInteger(entry.id)||!/^[a-z0-9-]+$/.test(entry.name)||
        !/^\/sprites\/crystal\/(normal|shiny)\/\d+(-[a-z])?\.gif$/.test(entry.url))
        throw new Error('Invalid GIF catalogue. Upload the gallery again.');
    }
    gifEntries=data.entries;return gifEntries;
  })().catch(error=>{gifCataloguePromise=null;throw error;});
  await gifCataloguePromise;if(tab==='gallery')refreshGifGallery();return gifEntries;
}
function refreshGifGallery() {
  const thumbnails=[],generation=++gifThumbnailGeneration;
  gifMatches=filterCrystal(gifEntries,$('gifQuery').value,$('gifVariant').value,$('gifRegion').value);
  const pages=Math.max(1,Math.ceil(gifMatches.length/24));gifPageIndex=Math.min(gifPageIndex,pages-1);
  $('gifGallery').replaceChildren();
  for(const entry of gifMatches.slice(gifPageIndex*24,(gifPageIndex+1)*24)) {
    const button=document.createElement('button');button.className='gallery-tile';
    button.dataset.key=entry.key;
    button.setAttribute('aria-label','Load '+gifLabel(entry));
    button.setAttribute('aria-pressed',String(galleryGif?.entry.key===entry.key));
    const image=document.createElement('img');image.alt='';image.width=image.height=56;
    thumbnails.push({image,url:entry.url});
    const label=document.createElement('span');label.textContent=gifLabel(entry);
    button.append(image,label);button.onclick=()=>previewGalleryGif(entry);$('gifGallery').append(button);
  }
  if(!gifMatches.length) {
    const empty=document.createElement('p');empty.className='muted';empty.textContent=gifEntries.length?'No matching GIFs.':'Loading the GIF gallery…';
    $('gifGallery').append(empty);
  }
  $('gifStatus').textContent=gifEntries.length?`${gifMatches.length} matching GIFs · ${gifEntries.length} downloaded sprites.`:'Loading the GIF gallery…';
  $('gifPage').textContent=`${gifPageIndex+1} / ${pages}`;
  $('gifPreviousPage').disabled=gifPageIndex===0;$('gifNextPage').disabled=gifPageIndex>=pages-1;
  loadGifThumbnails(thumbnails,generation);
}
async function loadGifThumbnails(thumbnails,generation) {
  // Browser image loaders otherwise open six simultaneous TCP connections.
  // Share one request lane with frames/status to keep the ESP32's RAM bounded.
  for(const {image,url} of thumbnails) {
    if(generation!==gifThumbnailGeneration)return;
    try {
      let blobUrl=gifThumbnailUrls.get(url);
      if(!blobUrl) {
        const blob=await withDeviceRequest(async()=>{
          if(generation!==gifThumbnailGeneration)return null;
          const response=await fetch(url,{cache:'force-cache',signal:AbortSignal.timeout(8000)});
          if(!response.ok)throw new Error('Thumbnail unavailable');
          return response.blob();
        });
        if(!blob)return;
        blobUrl=URL.createObjectURL(blob);gifThumbnailUrls.set(url,blobUrl);
        if(gifThumbnailUrls.size>48) {
          const oldest=gifThumbnailUrls.keys().next().value;
          URL.revokeObjectURL(gifThumbnailUrls.get(oldest));gifThumbnailUrls.delete(oldest);
        }
      }
      if(generation!==gifThumbnailGeneration)return;
      image.src=blobUrl;
    }catch(_) { /* A failed thumbnail leaves its labelled button usable. */ }
  }
}
async function loadGalleryGif(entry) {
  let pending=gifCache.get(entry.url);
  if(!pending) {
    pending=withDeviceRequest(async()=>{
      const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),12000);
      try {
        const response=await fetch(entry.url,{signal:controller.signal,cache:'force-cache',credentials:'omit'});
        if(!response.ok)throw new Error('Could not load this GIF from the ESP32.');
        const bytes=await response.arrayBuffer();if(bytes.byteLength>20*1024*1024)throw new Error('GIF is too large.');
        return decodeGIF(bytes);
      }catch(error) {
        if(error.name==='AbortError')throw new Error('GIF download timed out. Check the ESP32 connection.');
        throw error;
      }finally {clearTimeout(timer);}
    });
    gifCache.set(entry.url,pending);
    if(gifCache.size>8)gifCache.delete(gifCache.keys().next().value);
    pending.catch(()=>{if(gifCache.get(entry.url)===pending)gifCache.delete(entry.url);});
  }
  return {animation:await pending,entry,owner:'gallery',name:gifLabel(entry)};
}
function renderGifFrame() {
  if(!gifCurrent)return;
  const animation=gifCurrent.animation,frame=animation.frames[gifFrameIndex%animation.frames.length];
  gifScratch.width=animation.width;gifScratch.height=animation.height;
  gifScratchCtx.putImageData(new ImageData(frame.rgba,animation.width,animation.height),0,0);
  background();
  if(gifCurrent.owner==='gallery') {
    const b=animation.bounds,scale=Math.min(64/b.w,32/b.h),w=Math.max(1,Math.floor(b.w*scale)),h=Math.max(1,Math.floor(b.h*scale));
    ctx.imageSmoothingEnabled=false;
    ctx.drawImage(gifScratch,b.x,b.y,b.w,b.h,Math.floor((64-w)/2),Math.floor((32-h)/2),w,h);
  }else {
    ctx.imageSmoothingEnabled=$('smoothing').value==='smooth';
    const p=imagePlacement(animation.width,animation.height,$('fit').value);
    ctx.drawImage(gifScratch,p.x,p.y,p.w,p.h);
  }
}
function startGifPreview() {
  gifRunning=true;gifPanelPlaying=false;gifCryPending=false;gifFrameIndex=0;updateGifButtons();gifTick(gifGeneration);
}
async function previewGalleryGif(entry) {
  stopScroll();const generation=gifGeneration;gifLoading=true;updateGifButtons();
  message('Loading '+gifLabel(entry)+'…');
  try {
    const loaded=await loadGalleryGif(entry);
    if(generation!==gifGeneration||tab!=='gallery')return;
    gifCurrent=galleryGif=loaded;gifLoading=false;refreshGifGallery();startGifPreview();
    message(loaded.name+' ready. Choose “Play GIF on panel”.');
  }catch(error){if(generation===gifGeneration)message(error.message,true);}
  finally{if(generation===gifGeneration){gifLoading=false;updateGifButtons();}}
}
async function gifTick(generation) {
  if(!gifRunning||generation!==gifGeneration)return;
  const started=performance.now();
  try {
    if(gifCycling&&performance.now()>=gifSwitchAt) {
      gifPoolIndex=(gifPoolIndex+1)%gifPool.length;gifLoading=true;updateGifButtons();
      const loaded=await loadGalleryGif(gifPool[gifPoolIndex]);
      if(generation!==gifGeneration)return;
      gifCurrent=galleryGif=loaded;gifFrameIndex=0;gifLoading=false;gifCryPending=true;
      gifSwitchAt=performance.now()+Math.max(2,Math.min(60,Number($('gifInterval').value)||5))*1000;
      updateGifButtons();
    }
    renderGifFrame();
    let sent=true;
    if(gifPanelPlaying) {
      const cry=gifCryPending&&gifCurrent.owner==='gallery'&&$('gifCries').checked?gifCurrent.entry.id:0;
      sent=await sendCanvas(cry,generation);
      if(sent&&generation===gifGeneration)gifCryPending=false;
    }
    if(!gifRunning||generation!==gifGeneration)return;
    const delay=gifCurrent.animation.frames[gifFrameIndex%gifCurrent.animation.frames.length].delay;
    if(sent) {
      if(gifPanelPlaying)message((gifCycling?'Cycling GIF: ':'Playing GIF: ')+gifCurrent.name+'.');
      gifFrameIndex=(gifFrameIndex+1)%gifCurrent.animation.frames.length;
    }
    gifTimer=setTimeout(()=>gifTick(generation),sent?Math.max(0,Math.max(40,delay)-(performance.now()-started)):40);
  }catch(error){if(generation===gifGeneration){stopGifPlayback();message(error.message,true);}}
}
function playCurrentGif(owner) {
  if(gifPanelPlaying){stopScroll();message('GIF playback stopped. The last frame remains on the panel.');return;}
  stopScroll();gifCurrent=owner==='gallery'?galleryGif:imageGif;
  if(!gifCurrent){message('Choose a GIF first.',true);return;}
  if(document.hidden){message('Keep this page visible during GIF playback.',true);return;}
  gifRunning=gifPanelPlaying=true;gifCryPending=true;gifFrameIndex=0;updateGifButtons();gifTick(gifGeneration);
}
$('gifPlay').onclick=()=>playCurrentGif('gallery');$('imageGifPlay').onclick=()=>playCurrentGif('image');
$('gifCycle').onclick=async()=>{
  if(gifCycling){stopScroll();message('GIF cycling stopped. The last frame remains on the panel.');return;}
  stopScroll();const generation=gifGeneration;
  try {
    if(document.hidden)throw new Error('Keep this page visible during GIF playback.');
    await loadGifCatalogue();if(generation!==gifGeneration||tab!=='gallery')return;
    gifPool=filterCrystal(gifEntries,$('gifQuery').value,$('gifVariant').value,$('gifRegion').value);
    if(!gifPool.length)throw new Error('No matching GIFs to cycle. Clear or change the search.');
    gifPoolIndex=Math.max(0,gifPool.findIndex(entry=>entry.key===galleryGif?.entry.key));gifLoading=true;updateGifButtons();
    const loaded=await loadGalleryGif(gifPool[gifPoolIndex]);if(generation!==gifGeneration)return;
    gifCurrent=galleryGif=loaded;gifLoading=false;gifFrameIndex=0;gifCryPending=true;gifRunning=gifPanelPlaying=gifCycling=true;
    gifSwitchAt=performance.now()+Math.max(2,Math.min(60,Number($('gifInterval').value)||5))*1000;
    updateGifButtons();gifTick(generation);
  }catch(error){if(generation===gifGeneration){stopGifPlayback();message(error.message,true);}}
};
['gifQuery','gifVariant','gifRegion'].forEach(id=>$(id).addEventListener(id==='gifQuery'?'input':'change',()=>{
  stopScroll();gifPageIndex=0;refreshGifGallery();
}));
$('gifPreviousPage').onclick=()=>{gifPageIndex=Math.max(0,gifPageIndex-1);refreshGifGallery();};
$('gifNextPage').onclick=()=>{gifPageIndex++;refreshGifGallery();};

let speakerState=null,speakerBusy=false,speakerRefreshing=false,speakerTimer=null;
function speakerMessage(text,error=false) {
  $('speakerMessage').textContent=text;$('speakerMessage').className='message '+(error?'error':'success');
}
function updateSpeakerControls() {
  const state=speakerState?.state||'off',connected=!!speakerState?.connected;
  const changing=['initializing','scanning','connecting','disconnecting'].includes(state);
  $('speakerScan').disabled=speakerBusy||changing||connected;
  $('speakerScan').textContent=state==='scanning'?'Scanning…':state==='initializing'?'Starting Bluetooth…':'Scan for speakers';
  $('speakerDevice').disabled=speakerBusy||connected||['connecting','disconnecting'].includes(state);
  $('speakerConnect').disabled=speakerBusy||connected||['initializing','connecting','disconnecting'].includes(state)||!$('speakerDevice').value;
  $('speakerDisconnect').disabled=speakerBusy||!(['scanning','connecting','connected'].includes(state));
  $('speakerTest').disabled=speakerBusy||!connected||!!speakerState?.testing||!!speakerState?.playing||state==='disconnecting';
  $('speakerTest').textContent=speakerState?.testing?'Playing test sound…':'Play a 2-second test sound';
}
function renderSpeakerStatus(data) {
  speakerState=data;
  const labels={off:'Bluetooth is off. Scan to find your JBL Go 4.',initializing:'Starting ESP32 Bluetooth…',
    scanning:'Scanning for speakers… Keep the JBL in pairing mode (about 13 seconds).',
    connecting:'Connecting to '+(data.name||'the speaker')+'…',
    connected:'Connected to '+(data.name||'the speaker')+'.'+(data.cryId?' Playing Pokémon cry #'+data.cryId+'.':''),disconnecting:'Disconnecting…',
    idle:'Not connected. Scan and select your JBL Go 4.',error:'Bluetooth could not start.'};
  $('speakerStatus').textContent=data.message||labels[data.state]||'Bluetooth status unavailable.';
  $('speakerStatus').className='hint'+(data.message?' error':'');
  const previous=$('speakerDevice').value;
  const devices=[...(data.devices||[])].sort((a,b)=>Number(!/jbl/i.test(a.name))-Number(!/jbl/i.test(b.name))||b.rssi-a.rssi);
  $('speakerDevice').replaceChildren();
  const placeholder=document.createElement('option');placeholder.value='';
  placeholder.textContent=devices.length?'Choose a speaker':data.state==='scanning'?'Searching…':'No speakers found — press JBL’s Bluetooth button and scan';
  $('speakerDevice').append(placeholder);
  for(const device of devices) {
    const option=document.createElement('option');option.value=device.address;
    option.textContent=device.name+' · '+device.address;
    $('speakerDevice').append(option);
  }
  if(devices.some(device=>device.address===previous))$('speakerDevice').value=previous;
  else if(data.connected&&devices.some(device=>device.address===data.address))$('speakerDevice').value=data.address;
  if(!$('speakerDevice').value) {
    const jbl=devices.find(device=>/^jbl go 4$/i.test(device.name));if(jbl)$('speakerDevice').value=jbl.address;
  }
  // Don't overwrite a slider while the user is changing it.
  if(document.activeElement!==$('speakerVolume')&&Number.isInteger(data.volume)) {
    $('speakerVolume').value=data.volume;$('speakerVolumeValue').textContent=data.volume+'%';
  }
  updateSpeakerControls();
}
async function refreshSpeakerStatus() {
  if(speakerRefreshing||!$('speakerDetails').open||document.hidden)return;
  speakerRefreshing=true;
  try {renderSpeakerStatus(await request('/api/speaker'));}
  catch(error){speakerMessage(error.message,true);}
  finally{speakerRefreshing=false;}
}
async function speakerAction(action,body=null) {
  if(speakerBusy)return;speakerBusy=true;updateSpeakerControls();
  try {
    const result=await request('/api/speaker/'+action,{method:'POST',...(body?{body:new URLSearchParams(body)}:{})});
    speakerMessage(result.message||(action==='volume'?'Speaker audio volume updated.':'Speaker updated.'));
    await refreshSpeakerStatus();
  }catch(error){speakerMessage(error.message,true);}
  finally{speakerBusy=false;updateSpeakerControls();}
}
$('speakerScan').onclick=()=>speakerAction('scan');
$('speakerDevice').onchange=updateSpeakerControls;
$('speakerConnect').onclick=()=>speakerAction('connect',{address:$('speakerDevice').value});
$('speakerDisconnect').onclick=()=>speakerAction('disconnect');
$('speakerTest').onclick=()=>speakerAction('test');
$('speakerVolume').oninput=()=>$('speakerVolumeValue').textContent=$('speakerVolume').value+'%';
$('speakerVolume').onchange=()=>speakerAction('volume',{value:$('speakerVolume').value});
$('speakerDetails').ontoggle=()=>{
  clearInterval(speakerTimer);speakerTimer=null;
  if($('speakerDetails').open){refreshSpeakerStatus();speakerTimer=setInterval(refreshSpeakerStatus,1500);}
};
document.addEventListener('visibilitychange',()=>{if(!document.hidden)refreshSpeakerStatus();});

renderText();refreshStatus();setInterval(refreshStatus,4000);
</script></body></html>
)MATRIX_UI";
