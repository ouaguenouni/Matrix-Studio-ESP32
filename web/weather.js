let deviceLive = '';
let liveReport = {};
let liveEpoch = 0;
let liveAppearance = defaultAppearance();
let reportAt = performance.now();
let appearancePending = 0;

function serviceDescription(service) {
  if (!service) return 'Waiting for the panel…';
  const age=service.ageSeconds;
  const updated=age==null?'':` Last updated ${age<60?`${age}s`:`${Math.floor(age/60)}m`} ago.`;
  return `${service.state[0].toUpperCase()+service.state.slice(1)} · ${service.message}${updated}${service.refreshing?' Refreshing…':''}`;
}
function syncLive(data) {
  liveReport=data||{};reportAt=performance.now();deviceLive=data.live||'';
  if(!appearancePending){
    if(data.appearance)liveAppearance=data.appearance;
    else if(PALETTES.includes(data.theme))liveAppearance.global=data.theme;
    $('globalTheme').value=liveAppearance.global;
    for(const name of ['clock','weather','youtube']) {
      $(name+'Theme').value=liveAppearance[name].palette;
      $(name+'Layout').value=liveAppearance[name].layout;
      $(name+'Motion').value=liveAppearance[name].motion;
    }
  }
  for(const name of ['clock','weather','youtube'])$(name+'Play').textContent=deviceLive===name?`Stop ${name==='youtube'?'channel':name}`:`Show ${name==='youtube'?'channel':name} on panel`;
  $('weatherStatus').textContent=serviceDescription(data.services?.weather);
  $('youtubeStatus').textContent=serviceDescription(data.services?.youtube);
  if(data.services?.weather)$('weatherStatus').dataset.state=data.services.weather.state;
  if(data.services?.youtube)$('youtubeStatus').dataset.state=data.services.youtube.state;
  syncGift(data);
  renderLivePreview();
}
function postLive(fields) {
  liveEpoch++;
  return request('/api/live',{method:'POST',body:new URLSearchParams(fields)});
}
async function applyAppearance(fields) {
  appearancePending++;
  try {await postLive({mode:'save',...fields});message('Appearance saved on the panel.');}
  catch(error){message(error.message,true);}
  finally {appearancePending--;await refreshStatus();renderLivePreview();}
}
$('globalTheme').onchange=()=>{
  liveAppearance.global=$('globalTheme').value;renderLivePreview();applyAppearance({theme:liveAppearance.global});
};
for(const name of ['clock','weather','youtube'])for(const [suffix,key] of [['Theme','palette'],['Layout','layout'],['Motion','motion']]){
  $(name+suffix).onchange=()=>{liveAppearance[name][key]=$(name+suffix).value;renderLivePreview();applyAppearance({target:name,[key]:liveAppearance[name][key]});};
}
function renderLivePreview() {
  if(tab==='studio'){renderGiftPreview();return;}
  if(!['clock','weather','youtube'].includes(tab))return;
  const a=liveAppearance[tab],service=liveReport.services?.[tab];
  const reduced=window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  const m={mode:tab,palette:effectivePalette(liveAppearance,tab),layout:LAYOUTS[tab].indexOf(a.layout),motion:reduced?0:['off','subtle','full'].indexOf(a.motion),
    tick:(liveReport.liveTick||0)+Math.max(0,performance.now()-reportAt),...frenchTime(new Date()),
    valid:tab==='weather'?liveReport.temp!=null:!!liveReport.channel,state:service?.state||'loading',stale:service?.state==='stale',
    temperature:liveReport.temp??0,kind:liveReport.kind||'cloud',city:liveReport.place||'',title:liveReport.channel||'',
    subs:liveReport.subs??'--',views:liveReport.views??'--',videos:liveReport.videos??'--'};
  if(liveReport.clockSynced===false)m.hour=-1;
  renderDisplay(ctx,m);
}
function renderWeather(){renderLivePreview();}
async function liveAction(fields,success) {
  try {await postLive(fields);message(success);await refreshStatus();}
  catch(error){message(error.message,true);}
}
$('weatherLocate').onclick=()=>liveAction({mode:'weather',locate:'1'},'Finding the network location.');
$('weatherSearch').onsubmit=event=>{
  event.preventDefault();const city=$('weatherCity').value.trim();
  if(!city){message('Enter a city name.',true);return;}
  liveAction({mode:'weather',city},'Looking up the city and its weather.');
};
$('weatherRefresh').onclick=()=>liveAction({refresh:'weather'},'Weather refresh requested.');
$('weatherPlay').onclick=()=>liveAction({mode:deviceLive==='weather'?'off':'weather'},'Display mode updated.');
