import {DISPLAY_DATA} from './display-data.mjs';
import {pixelLabel,pixelWidth} from './display.mjs';
export function renderGift(c,m){
  const [bg,fg,accent]=DISPLAY_DATA.palettes[m.palette]||DISPLAY_DATA.palettes.neon;
  const rect=(x,y,w,h,col)=>{c.fillStyle=col;c.fillRect(x,y,w,h);};
  const text=(t,x,y,s,col)=>{for(const ch of t){const rows=DISPLAY_DATA.font[ch]||DISPLAY_DATA.font['?'];rows.forEach((row,dy)=>[...row].forEach((v,dx)=>{if(v==='1')rect(x+dx*s,y+dy*s,s,s,col);}));x+=(rows[0].length+1)*s;}};
  const centre=(t,y,s,col)=>text(t,Math.trunc((64-pixelWidth(t,s))/2),y,s,col);
  const ticker=(input,y,col,tick=m.tick)=>{const t=pixelLabel(input),w=pixelWidth(t);if(w<=60){centre(t,y,1,col);return;}if(!m.motion){text(t.slice(0,15),2,y,1,col);return;}const offset=Math.max(0,Math.floor(tick/180)%(w+32)-12);text(t,2-offset,y,1,col);text(t,2-offset+w+20,y,1,col);};
  rect(0,0,64,32,bg);const tick=m.motion?m.tick:0;
  if(m.scene===0){
    [9,15,11,20,13,17,10,14].forEach((h,i)=>{const x=i*8;rect(x,25-h,6,1,accent);rect(x,25-h,1,h,accent);rect(x+5,25-h,1,h,accent);for(let row=0;row<Math.trunc((h-3)/4);row++)if((i+row+Math.floor(tick/1300))%4!==0)rect(x+2,28-h+row*4,2,1,fg);});
    rect(0,25,64,1,accent);ticker(m.name+' / AFTER HOURS',27,fg);
  }else if(m.scene===1){
    for(let i=0;i<6;i++){const y=8+i*4,inset=12-i*2;rect(inset,y,64-inset*2,1,accent);}
    for(let i=0;i<7;i++)for(let y=8;y<28;y++){const x=32+Math.trunc((i-3)*(y+4)/3);if(x>=0&&x<64)rect(x,y,1,1,accent);}
    rect(0,8+Math.floor(tick/350)%20,64,1,fg);centre('AFTER HOURS',1,1,fg);
  }else if(m.scene===2){
    if(m.tick<6500){
      centre('FOR '+pixelLabel(m.name),0,1,fg);rect(19,20,26,8,accent);rect(17,27,30,2,fg);rect(19,19,26,2,fg);
      for(let i=0;i<3;i++){const x=24+i*7;rect(x,13,1,6,fg);rect(x,10-(Math.floor(tick/250)+i)%2,1,2,accent);}
      for(let i=0;i<6;i++)rect((i*13+3)%64,7+(Math.floor(tick/240)+i*3)%18,1,1,accent);
    }else if(m.tick<13000){centre('HAPPY BIRTHDAY',2,1,accent);centre(String(m.age),12,3,fg);}
    else if(m.tick<19000){ticker(m.name,6,fg);centre('MAKE SOME NOISE',22,1,accent);}
    else{centre('FOR YOU',4,1,accent);ticker(m.message,15,fg,m.tick-19000);rect(28,27,8,1,accent);}
  }else if(m.scene===3){
    centre('HEY '+pixelLabel(m.name),2,1,accent);ticker(m.message,14,fg);rect(5+Math.floor(tick/160)%54,27,3,1,accent);
  }else if(m.scene===4){
    centre(m.paused?'PAUSED':'IN THE FLOW',1,1,accent);const seconds=Math.ceil(m.left/1000);const time=String(Math.floor(seconds/60)).padStart(2,'0')+':'+String(seconds%60).padStart(2,'0');
    centre(time,10,pixelWidth(time,3)<=60?3:2,fg);rect(2,29,60,1,'#18202a');rect(2,29,m.total?Math.trunc(m.left*60/m.total):0,1,accent);
  }else{centre('NICE WORK',5,1,accent);centre('BREATHE',16,2,fg);}
}
