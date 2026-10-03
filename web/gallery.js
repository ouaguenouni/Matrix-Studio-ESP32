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
