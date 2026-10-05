const fs=require('fs'),assert=require('node:assert/strict');
const {chromium}=require('playwright');
(async()=>{
 const browser=await chromium.launch({headless:true});const page=await browser.newPage({viewport:{width:390,height:844}});
 const html=fs.readFileSync('web/preview.html','utf8'),requests=[],errors=[];page.on('pageerror',e=>errors.push(e.message));
 let gift={name:'Inge',message:'ADHD future millionaire',surprises:'ONE MORE TRACK',month:10,day:9,birthYear:1994,seconds:15,studio:false,nightEnabled:false,nightStart:1320,nightEnd:480,nightDim:8,focusMinutes:25,overlay:-1,tick:0,age:32,timer:{running:false,paused:false,leftMs:0,totalMs:0}};
 await page.route('http://matrix.test/**',async route=>{
  const req=route.request(),path=new URL(req.url()).pathname;
  if(path==='/')return route.fulfill({contentType:'text/html',body:html});
  let data={ok:true};
  if(path==='/api/status')data={width:64,height:32,connected:true,ip:'192.168.1.39',hostname:'matrix.local',ssid:'Test',brightness:30,live:'clock',theme:'neon',gift};
  if(path==='/api/gift'){
   assert.equal(req.headers()['x-matrix-control'],'1');const body=JSON.parse(req.postData());requests.push(body);gift={...gift,...body};
   if(body.action==='studio')gift.studio=true;
   if(body.action==='birthday')gift.overlay=2;
   if(body.action==='focus_start'){gift.overlay=4;gift.timer={running:true,paused:false,leftMs:body.focusMinutes*60000,totalMs:body.focusMinutes*60000};}
   if(body.action==='focus_pause')gift.timer.paused=true;
   if(body.action==='focus_resume')gift.timer.paused=false;
   if(body.action==='dismiss'){gift.overlay=-1;gift.timer.running=false;}
  }
  await route.fulfill({contentType:'application/json',body:JSON.stringify(data)});
 });
 try{
  await page.goto('http://matrix.test/');await page.locator('#tab-studio').click();assert.equal(requests.length,0);
  assert.equal(await page.locator('#giftBirthday').inputValue(),'10-09');assert.equal(await page.locator('#giftBirthYear').inputValue(),'1994');
  await page.locator('#giftMessage').fill('ADHD future millionaire!');await page.evaluate(()=>refreshStatus());assert.equal(await page.locator('#giftMessage').inputValue(),'ADHD future millionaire!');
  await page.getByRole('button',{name:'Save gift settings',exact:true}).click();await page.getByText('Inge’s settings saved.',{exact:true}).waitFor();
  assert.equal(requests.at(-1).message,'ADHD future millionaire!');assert.equal(requests.at(-1).day,9);assert.equal(requests.at(-1).month,10);
  await page.locator('#giftBirthdayPlay').click();await page.evaluate(()=>deviceRequests);assert.equal(requests.at(-1).action,'birthday');
  await page.locator('#giftFocusMinutes').fill('45');await page.locator('#giftFocusStart').click();await page.evaluate(()=>deviceRequests);assert.equal(requests.at(-1).focusMinutes,45);
  await page.locator('#giftFocusPause').click();await page.evaluate(()=>deviceRequests);assert(gift.timer.paused);
  await page.locator('#giftFocusResume').click();await page.evaluate(()=>deviceRequests);assert(!gift.timer.paused);
  await page.locator('#giftNight').check();await page.locator('#giftNightDim').fill('0');
  await page.getByRole('button',{name:'Save gift settings',exact:true}).click();await page.evaluate(()=>deviceRequests);assert(gift.nightEnabled);assert.equal(gift.nightDim,0);
  await page.locator('#giftStart').click();await page.evaluate(()=>deviceRequests);assert(gift.studio);
  await page.locator('#giftDismiss').click();await page.evaluate(()=>deviceRequests);assert.equal(gift.overlay,-1);
  await page.locator('#giftBirthday').fill('');await page.getByRole('button',{name:'Save gift settings',exact:true}).click();await page.evaluate(()=>deviceRequests);assert.equal(gift.day,0);assert.equal(gift.month,0);
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));assert.deepEqual(errors,[]);
  console.log('Gift browser: editable birthday, dirty-form preservation, studio, focus, night dimming, controls and mobile layout passed.');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
