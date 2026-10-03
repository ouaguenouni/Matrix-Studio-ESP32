let gifCurrent = null, imageGif = null, gifFrameIndex = 0;
let gifGeneration = 0, gifTimer = null, gifRunning = false, gifPanelPlaying = false, gifLoading = false;
const gifScratch = document.createElement('canvas');
const gifScratchCtx = gifScratch.getContext('2d');

function updateGifButtons() {
  $('imageGifPlay').disabled = gifLoading;
  $('imageGifPlay').textContent = gifPanelPlaying && gifCurrent?.owner === 'image'
    ? 'Stop GIF playback' : 'Play GIF on panel';
  updateSendButton();
}

function stopGifPlayback() {
  gifGeneration++;
  clearTimeout(gifTimer);
  gifRunning = false;
  gifPanelPlaying = false;
  gifLoading = false;
  updateGifButtons();
}

function renderGifFrame() {
  if (!gifCurrent) return;
  const animation = gifCurrent.animation;
  const frame = animation.frames[gifFrameIndex % animation.frames.length];
  gifScratch.width = animation.width;
  gifScratch.height = animation.height;
  gifScratchCtx.putImageData(new ImageData(frame.rgba, animation.width, animation.height), 0, 0);
  background();
  ctx.imageSmoothingEnabled = $('smoothing').value === 'smooth';
  const placement = imagePlacement(animation.width, animation.height, $('fit').value);
  ctx.drawImage(gifScratch, placement.x, placement.y, placement.w, placement.h);
}

function startGifPreview() {
  gifRunning = true;
  gifPanelPlaying = false;
  gifFrameIndex = 0;
  updateGifButtons();
  gifTick(gifGeneration);
}

async function gifTick(generation) {
  if (!gifRunning || generation !== gifGeneration) return;
  const started = performance.now();
  try {
    renderGifFrame();
    let sent = true;
    if (gifPanelPlaying) sent = await sendCanvas();
    if (!gifRunning || generation !== gifGeneration) return;
    const delay = gifCurrent.animation.frames[gifFrameIndex % gifCurrent.animation.frames.length].delay;
    if (sent) {
      if (gifPanelPlaying) message('Playing GIF: ' + gifCurrent.name + '.');
      gifFrameIndex = (gifFrameIndex + 1) % gifCurrent.animation.frames.length;
    }
    gifTimer = setTimeout(() => gifTick(generation), sent ? Math.max(0, Math.max(40, delay) - (performance.now() - started)) : 40);
  } catch (error) {
    if (generation === gifGeneration) {
      stopGifPlayback();
      message(error.message, true);
    }
  }
}

function playCurrentGif() {
  if (gifPanelPlaying) {
    stopScroll();
    message('GIF playback stopped. The last frame remains on the panel.');
    return;
  }
  stopScroll();
  gifCurrent = imageGif;
  if (!gifCurrent) {
    message('Choose a GIF first.', true);
    return;
  }
  if (document.hidden) {
    message('Keep this page visible during GIF playback.', true);
    return;
  }
  gifRunning = gifPanelPlaying = true;
  gifFrameIndex = 0;
  updateGifButtons();
  gifTick(gifGeneration);
}

$('imageGifPlay').onclick = () => playCurrentGif();
