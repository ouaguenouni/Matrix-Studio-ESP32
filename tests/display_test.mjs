import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {renderDisplay, PALETTES, frenchTime, effectivePalette, defaultAppearance} from '../web/display.mjs';
const expected=new Map(execFileSync(process.env.MATRIX_DISPLAY_DUMP || '/tmp/matrix-display-dump',{encoding:'utf8'}).trim().split('\n').map(line=>line.split(':')));
let checked=0;
for(let mode=1;mode<=3;mode++)for(let layout=0;layout<2;layout++)for(let motion=0;motion<3;motion++)for(let palette=0;palette<6;palette++)for(let scenario=0;scenario<9;scenario++){
  const m={mode:['','clock','weather','youtube'][mode],layout,motion,palette:PALETTES[palette],tick:12570,hour:scenario===8?-1:9,minute:5,second:31,day:4,month:10,
    valid:scenario<6,stale:scenario===5,state:scenario===6?'loading':scenario===7?'error':'unconfigured',temperature:scenario===0?-99:scenario===1?0:24,
    kind:['snow','sun','fog','rain','storm','cloud'][scenario]||'cloud',city:'Saint-Étienne',title:'Cosmic Hippo Sounds',subs:scenario===0?'--':'12.3K',views:'123.4M',videos:'0'};
  const pixels=new Uint32Array(2048);
  const ctx={fillStyle:'#000000',fillRect(x,y,w,h){assert([x,y,w,h].every(Number.isInteger));const rgb=parseInt(this.fillStyle.slice(1),16);for(let dy=0;dy<h;dy++)for(let dx=0;dx<w;dx++)if(x+dx>=0&&x+dx<64&&y+dy>=0&&y+dy<32)pixels[(y+dy)*64+x+dx]=rgb;}};
  renderDisplay(ctx,m);
  let hash=2166136261;for(const rgb of pixels)hash=Math.imul(hash^rgb,16777619)>>>0;
  const key=[mode,layout,motion,palette,scenario].join(',');assert.equal(String(hash),expected.get(key),key);checked++;
}
assert.equal(frenchTime(new Date('2026-01-02T08:05:08Z')).hour,9);
assert.equal(frenchTime(new Date('2026-07-02T08:05:08Z')).hour,10);
const a=defaultAppearance();a.clock.palette='mint';a.global='sunset';assert.equal(effectivePalette(a,'clock'),'mint');assert.equal(effectivePalette(a,'weather'),'sunset');
console.log(`${checked} firmware/browser pixel comparisons passed, plus French timezone and palette isolation.`);
