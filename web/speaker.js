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
