import {execFileSync} from 'node:child_process';
import assert from 'node:assert/strict';
import {renderGift} from '../web/gift.mjs';
import {PALETTES} from '../web/display.mjs';
const expected=new Map(execFileSync(process.env.MATRIX_GIFT_DUMP,{encoding:'utf8'}).trim().split('\n').map(line=>line.split(':')));
let checked=0;
for(let scene=0;scene<6;scene++)for(let palette=0;palette<6;palette++)for(let motion=0;motion<2;motion++)for(let scenario=0;scenario<5;scenario++){
 const pixels=new Uint32Array(2048);const c={fillStyle:'#000000',fillRect(x,y,w,h){assert([x,y,w,h].every(Number.isInteger));for(let dy=0;dy<h;dy++)for(let dx=0;dx<w;dx++)if(x+dx>=0&&x+dx<64&&y+dy>=0&&y+dy<32)pixels[(y+dy)*64+x+dx]=parseInt(this.fillStyle.slice(1),16);}};
 renderGift(c,{scene,palette:PALETTES[palette],motion,tick:scenario*7000,age:32,name:'Inge',message:'ADHD future millionaire',left:scenario*600000,total:3000000,paused:scenario===1});
 let hash=2166136261;for(const rgb of pixels)hash=Math.imul(hash^rgb,16777619)>>>0;const key=[scene,palette,motion,scenario].join(',');assert.equal(String(hash),expected.get(key),key);checked++;
}
console.log(`${checked} gift firmware/browser pixel comparisons passed.`);
