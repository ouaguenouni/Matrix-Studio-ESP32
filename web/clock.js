function renderClock(){renderLivePreview();}
$('clockPlay').onclick=()=>liveAction({mode:deviceLive==='clock'?'off':'clock'},'Display mode updated.');
