import {DISPLAY_DATA} from './display-data.mjs';
export const PALETTES = Object.keys(DISPLAY_DATA.palettes);
export const LAYOUTS = {clock: ['large', 'date'], weather: ['icon', 'detail'], youtube: ['rotate', 'subscribers']};
export function pixelLabel(input) {
  return String(input).normalize('NFD').replace(/[\u0300-\u036f]/g,'').toUpperCase().replace(/[^\x00-\x7f]/gu,'?');
}
export function pixelWidth(text, scale=1) {
  return [...text].reduce((w,c)=>w+((DISPLAY_DATA.font[c]||DISPLAY_DATA.font['?'])[0].length+1)*scale,0)-(text.length?scale:0);
}
export function defaultAppearance() {
  return {global:'neon',clock:{palette:'global',layout:'large',motion:'subtle'},weather:{palette:'global',layout:'icon',motion:'subtle'},youtube:{palette:'global',layout:'rotate',motion:'subtle'}};
}
export function effectivePalette(appearance, mode) {
  return appearance[mode].palette === 'global' ? appearance.global : appearance[mode].palette;
}
export function frenchTime(date) {
  const parts = Object.fromEntries(new Intl.DateTimeFormat('en-GB',{timeZone:'Europe/Paris',hour:'2-digit',minute:'2-digit',second:'2-digit',day:'2-digit',month:'2-digit',hourCycle:'h23'}).formatToParts(date).map(p=>[p.type,p.value]));
  return {hour:+parts.hour,minute:+parts.minute,second:+parts.second,day:+parts.day,month:+parts.month};
}
export function renderDisplay(c, m) {
  const palette = DISPLAY_DATA.palettes[m.palette] || DISPLAY_DATA.palettes.neon;
  const [bg,fg,accent] = palette;
  const rect=(x,y,w,h,col)=>{c.fillStyle=col;c.fillRect(x,y,w,h);};
  const text=(t,x,y,scale,col)=>{
    for(const ch of t){const rows=DISPLAY_DATA.font[ch]||DISPLAY_DATA.font['?'];
      rows.forEach((row,dy)=>[...row].forEach((bit,dx)=>{if(bit==='1')rect(x+dx*scale,y+dy*scale,scale,scale,col);}));
      x+=(rows[0].length+1)*scale;
    }
  };
  const centre=(t,y,scale,col)=>text(t,Math.trunc((64-pixelWidth(t,scale))/2),y,scale,col);
  const ticker=(input,y,col)=>{
    const t=pixelLabel(input),w=pixelWidth(t);
    if(w<=60){centre(t,y,1,col);return;}
    if(!m.motion){text(t.slice(0,15),2,y,1,col);return;}
    const offset=Math.max(0,Math.floor(m.tick/(m.motion===2?110:180))%(w+32)-12);
    text(t,2-offset,y,1,col);text(t,2-offset+w+20,y,1,col);
  };
  const icon=(name,x,y)=>{
    const p=m.motion?Math.floor(m.tick/(m.motion===2?150:300))%8:0;
    if(name==='SUN'){
      rect(x+5,y+5,7,7,accent);rect(x+7,y+1,3,2,accent);rect(x+7,y+14,3,2,accent);
      rect(x+1,y+7,2,3,accent);rect(x+14,y+7,2,3,accent);
      if(m.motion===2 && p%2){rect(x+2,y+2,2,2,accent);rect(x+13,y+13,2,2,accent);}return;
    }
    if(name==='FOG'){for(let r=0;r<3;r++)rect(x+2+(p+r)%2,y+4+r*4,12,1,fg);return;}
    rect(x+5,y+3,7,5,fg);rect(x+2,y+6,14,5,fg);
    if(name==='RAIN'||name==='STORM'){
      for(let r=0;r<3;r++)rect(x+3+r*5,y+12+(p+r*2)%4,1,2,accent);
      if(name==='STORM'){rect(x+9,y+10,2,4,accent);rect(x+7,y+13,3,2,accent);}
    }else if(name==='SNOW'){for(let r=0;r<3;r++)rect(x+3+r*5,y+12+(p+r)%4,2,2,accent);}
  };
  rect(0,0,64,32,bg);
  if(m.mode==='clock'){
    const time=m.hour<0?'--:--':`${String(m.hour).padStart(2,'0')}:${String(m.minute).padStart(2,'0')}`;
    const y=m.layout?3:8;centre(time,y,3,fg);
    if(m.hour>=0 && m.motion && m.second%2)rect(Math.trunc((64-pixelWidth(time,3))/2)+24,y,3,15,bg);
    if(m.layout)centre(m.hour<0?'SYNC':`${String(m.day).padStart(2,'0')}/${String(m.month).padStart(2,'0')}`,25,1,accent);
    else {rect(2,28,60,1,'#18202a');if(m.hour>=0)rect(2,28,m.second+1,1,accent);}
    if(m.motion===2 && m.hour>=0)rect(Math.floor(m.tick/120)%64,0,1,1,accent);
    return;
  }
  if(!m.valid){
    centre(m.mode==='weather'?'WEATHER':'YOUTUBE',3,1,accent);centre('--',11,2,fg);
    centre(m.state==='unconfigured'?'ADD KEY':m.state==='error'?'CHECK WEB':'LOADING',25,1,accent);return;
  }
  if(m.mode==='weather'){
    const temp=String(m.temperature),kind=m.kind.toUpperCase();
    if(!m.layout){icon(kind,1,2);const scale=pixelWidth(temp,3)<=35?3:2;const w=pixelWidth(temp,scale);text(temp,22+Math.trunc((36-w)/2),3,scale,fg);rect(60,3,2,2,accent);}
    else {centre(temp,1,3,fg);centre(kind,19,1,accent);}
    ticker(m.city,26,fg);
  }else{
    const slide=m.layout?0:Math.floor(m.tick/5000)%3;
    const value=[m.subs,m.views,m.videos][slide];
    centre(['SUBSCRIBERS','VIEWS','VIDEOS'][slide],1,1,accent);
    centre(value,10,pixelWidth(value,3)<=60?3:2,fg);ticker(m.title,27,accent);
    if(!m.layout)for(let i=0;i<3;i++)rect(26+i*5,7,3,1,i===slide?accent:'#18202a');
  }
  if(m.stale)rect(62,0,2,2,'#ffb040');
}
