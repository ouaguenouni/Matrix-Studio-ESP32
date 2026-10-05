function renderYoutube(){renderLivePreview();}
$('youtubeSave').onclick=async()=>{
  const key=$('youtubeKey').value.trim();
  if(!key){message('Paste an API key first.',true);return;}
  try{await postLive({mode:'save',key});$('youtubeKey').value='';message('Key saved. Loading channel statistics.');await refreshStatus();}
  catch(error){message(error.message,true);}
};
$('youtubePlay').onclick=async()=>{
  const fields={mode:deviceLive==='youtube'?'off':'youtube'};
  const key=$('youtubeKey').value.trim();if(key)fields.key=key;
  try{await postLive(fields);$('youtubeKey').value='';message('Display mode updated.');await refreshStatus();}
  catch(error){message(error.message,true);}
};
$('youtubeRefresh').onclick=()=>liveAction({refresh:'youtube'},'YouTube refresh requested.');
setInterval(()=>{if(!document.hidden)renderLivePreview();},50);
