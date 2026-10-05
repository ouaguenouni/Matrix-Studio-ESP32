let giftDirty=false;
let giftPreviewScene=0;
let giftPreviewAt=performance.now();
const giftForm=$('giftForm');
const giftTime=n=>`${String(Math.floor(n/60)).padStart(2,'0')}:${String(n%60).padStart(2,'0')}`;
function syncGift(data){
  const g=data.gift;if(!g)return;
  if(!giftDirty){
    $('giftName').value=g.name;$('giftMessage').value=g.message;$('giftSurprises').value=g.surprises;
    $('giftBirthday').value=g.month?`${String(g.month).padStart(2,'0')}-${String(g.day).padStart(2,'0')}`:'';
    $('giftBirthYear').value=g.birthYear;$('giftSeconds').value=g.seconds;$('giftStudio').checked=g.studio;
    $('giftNight').checked=g.nightEnabled;$('giftNightStart').value=giftTime(g.nightStart);$('giftNightEnd').value=giftTime(g.nightEnd);$('giftNightDim').value=g.nightDim;$('giftFocusMinutes').value=g.focusMinutes;
  }
  const focus=g.timer?.running?(g.timer.paused?'Focus paused.':'Focus timer running.'):'';
  $('giftStatus').textContent=[g.studio?'Studio rotation on.':'Studio rotation off.',focus,g.nightActive?'Night schedule active.':'',g.month?`Birthday: ${g.day}/${g.month}.`:'Birthday disabled.'].filter(Boolean).join(' ');
}
giftForm.addEventListener('input',()=>{giftDirty=true;});
function giftFields(){
  const date=$('giftBirthday').value.trim();
  if(date&&!/^\d{2}-\d{2}$/.test(date))throw new Error('Use MM-DD for the birthday, or leave it empty.');
  const [month=0,day=0]=date?date.split('-').map(Number):[];
  const minutes=id=>{const [h,m]=$(id).value.split(':').map(Number);return h*60+m;};
  return {name:$('giftName').value.trim(),message:$('giftMessage').value.trim(),surprises:$('giftSurprises').value,month,day,birthYear:Number($('giftBirthYear').value),seconds:Number($('giftSeconds').value),studio:$('giftStudio').checked,nightEnabled:$('giftNight').checked,nightStart:minutes('giftNightStart'),nightEnd:minutes('giftNightEnd'),nightDim:Number($('giftNightDim').value),focusMinutes:Number($('giftFocusMinutes').value)};
}
async function giftAction(action,save=false){
  try{
    const fields=save?giftFields():{};liveEpoch++;
    await request('/api/gift',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({...fields,action})});
    if(save)giftDirty=false;
    message(action==='save'?'Inge’s settings saved.':'Display updated.');await refreshStatus();
  }catch(error){message(error.message,true);}
}
giftForm.onsubmit=event=>{event.preventDefault();giftAction('save',true);};
for(const [id,action]of [['giftStart','studio'],['giftArt','art'],['giftSurprise','surprise'],['giftBirthdayPlay','birthday'],['giftFocusStart','focus_start'],['giftFocusPause','focus_pause'],['giftFocusResume','focus_resume'],['giftDismiss','dismiss']]){
  $(id).onclick=()=>giftAction(action,['studio','birthday','focus_start'].includes(action));
}
$('giftPreview').onchange=()=>{giftPreviewScene=Number($('giftPreview').value);giftPreviewAt=performance.now();renderGiftPreview();};
function renderGiftPreview(){
  const g=liveReport.gift||{name:'Inge',message:'ADHD future millionaire',age:32,tick:0,timer:{}};
  const delta=Math.max(0,performance.now()-reportAt);const scene=g.overlay>=0?g.overlay:giftPreviewScene;
  const tick=g.overlay>=0?(g.tick||0)+delta:performance.now()-giftPreviewAt;
  renderGift(ctx,{scene:scene===0?Math.floor(tick/12000)%2:scene,tick,palette:liveAppearance.global,name:g.name,message:scene===3?(g.selectedMessage||g.message):g.message,age:g.age??32,motion:!window.matchMedia('(prefers-reduced-motion: reduce)').matches,paused:!!g.timer?.paused,left:Math.max(0,(g.timer?.leftMs||0)-(g.timer?.running&&!g.timer.paused?delta:0)),total:g.timer?.totalMs||1});
}
