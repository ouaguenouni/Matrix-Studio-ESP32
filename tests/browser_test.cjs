const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const {chromium}=require(require.resolve('playwright',{paths:[process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES||process.cwd()]}));
const root=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(root,'web/preview.html'),'utf8');

(async()=>{
  const browser=await chromium.launch({headless:true, ...(process.env.MATRIX_TEST_CHROMIUM ? {executablePath:process.env.MATRIX_TEST_CHROMIUM} : {})});
  const page=await browser.newPage({viewport:{width:1280,height:960}});
  const errors=[];page.on('pageerror',e=>errors.push(e.message));
  let frame=null, frames=0, brightness=30, wifi=null, rejectNext=false, liveState={mode:'',theme:'neon'};
  const appearance={global:'neon',clock:{palette:'global',layout:'large',motion:'subtle'},weather:{palette:'global',layout:'icon',motion:'subtle'},youtube:{palette:'global',layout:'rotate',motion:'subtle'}};
  const services={weather:{state:'loading',message:'Loading from the service.',ageSeconds:null},youtube:{state:'unconfigured',message:'Add a YouTube Data API key.',ageSeconds:null}};
  const payload={};const refreshes=[];
  await page.route('http://matrix.test/**',async route=>{
    const req=route.request(),url=new URL(req.url());
    if(url.pathname==='/')return route.fulfill({contentType:'text/html',body:html});
    let result={ok:true},status=200;
    if(req.method()==='POST')assert.equal(req.headers()['x-matrix-control'],'1');
    if(url.pathname==='/api/status')result={width:64,height:32,connected:!!wifi,ip:wifi?'192.168.1.42':'',hostname:'matrix-test.local',ssid:wifi?.ssid||'',ap:'Matrix-test',brightness,frames,freeHeap:120000,live:liveState.mode,theme:liveState.theme,appearance,services,...payload};
    else if(url.pathname==='/api/frame'){
      const body=req.postDataBuffer();
      const boundary=req.headers()['content-type'].split('boundary=')[1];
      const start=body.indexOf(Buffer.from('\r\n\r\n'))+4;
      const end=body.indexOf(Buffer.from('\r\n--'+boundary),start);
      assert(start>3&&end>start);assert.equal(end-start,4096);
      if(rejectNext){status=400;result={error:'Test rejected frame'};rejectNext=false;}
      else{frame=Buffer.from(body.subarray(start,end));frames++;result={ok:true,frames};}
    }else if(url.pathname==='/api/brightness')brightness=Number(new URLSearchParams(req.postData()).get('value'));
    else if(url.pathname==='/api/wifi'){wifi=Object.fromEntries(new URLSearchParams(req.postData()));result={ok:true,message:'Saved; connecting now.'};}
    else if(url.pathname==='/api/live'){const body=Object.fromEntries(new URLSearchParams(req.postData()));if(body.mode==='off')liveState.mode='';else if(body.mode&&body.mode!=='save')liveState.mode=body.mode;if(body.theme){liveState.theme=body.theme;appearance.global=body.theme;}if(body.target)for(const key of ['palette','layout','motion'])if(body[key])appearance[body.target][key]=body[key];if(body.refresh)refreshes.push(body.refresh);result={ok:true};}
    else{status=404;result={error:'Not found'};}
    await route.fulfill({status,contentType:'application/json',body:JSON.stringify(result)});
  });
  try{
    await page.goto('http://matrix.test/');
    await page.getByText('Setup network',{exact:true}).waitFor();
    assert(await page.locator('#wifiDetails').evaluate(e=>e.open));
    assert.equal(await page.locator('#textColor').inputValue(),'#ffffff');
    assert(await page.locator('#sharpText').isChecked());
    await page.locator('#send').click();await page.getByText('Sent to your display.',{exact:true}).waitFor();assert.equal(frame.length,4096);assert(frame.some(x=>x!==0));
    const words=()=>Array.from({length:2048},(_,i)=>frame.readUInt16LE(i*2));
    assert(words().includes(0xffff));assert(words().every(pixel=>pixel===0||pixel===0xffff));
    await page.locator('#sharpText').uncheck();
    await page.locator('#send').click();await page.waitForFunction(()=>!document.querySelector('#send').disabled);
    assert(words().some(pixel=>pixel!==0&&pixel!==0xffff));
    await page.locator('#sharpText').check();
    await page.locator('#textColor').fill('#0000ff');await page.locator('#background').fill('#ff0000');
    await page.locator('#send').click();await page.waitForFunction(()=>!document.querySelector('#send').disabled);
    assert(words().includes(0x001f));assert(words().every(pixel=>pixel===0xf800||pixel===0x001f));
    await page.locator('#textColor').fill('#ffffff');await page.locator('#background').fill('#000000');
    await page.locator('#tab-image').click();await page.locator('#pattern').click();await page.locator('#send').click();
    await page.waitForFunction(()=>document.querySelector('#message').textContent==='Sent to your display.'&&!document.querySelector('#send').disabled);
    assert.equal(frame.readUInt16LE(0),0xf800);assert.equal(frame.readUInt16LE(21*2),0x07e0);assert.equal(frame.readUInt16LE(42*2),0x001f);
    await page.locator('#purplePattern').click();await page.locator('#send').click();
    await page.waitForFunction(()=>!document.querySelector('#send').disabled);
    const purpleWords=[0xf800,0x07e0,0x001f,0xf81f,0x8018,0x400c,0xb49c,0xffff];
    for(let y=0;y<32;y++)for(let x=0;x<64;x++)
      assert.equal(frame.readUInt16LE((y*64+x)*2),purpleWords[Math.floor(x/8)]);
    const png=await page.evaluate(()=>{const c=document.createElement('canvas');c.width=c.height=1;const x=c.getContext('2d');x.fillStyle='red';x.fillRect(0,0,1,1);return c.toDataURL().split(',')[1];});
    await page.locator('#imageFile').setInputFiles({name:'red.png',mimeType:'image/png',buffer:Buffer.from(png,'base64')});
    await page.getByText('Image ready. Choose “Send to display”.',{exact:true}).waitFor();
    await page.locator('#send').click();await page.waitForFunction(()=>!document.querySelector('#send').disabled);
    assert.equal(frame.readUInt16LE((16*64+32)*2),0xf800);assert.equal(frame.readUInt16LE(0),0);
    await page.locator('#brightness').fill('64');await page.locator('#brightness').dispatchEvent('change');
    await page.getByText('Brightness updated.',{exact:true}).waitFor();assert.equal(brightness,64);
    await page.locator('#clear').click();await page.locator('#send').click();await page.waitForFunction(()=>!document.querySelector('#send').disabled);assert(frame.every(x=>x===0));
    await page.locator('#tab-draw').click();const bounds=await page.locator('#canvas').boundingBox();
    await page.mouse.click(bounds.x+bounds.width/2,bounds.y+bounds.height/2);
    await page.locator('#send').click();await page.waitForFunction(()=>!document.querySelector('#send').disabled);assert(frame.some(x=>x!==0));
    rejectNext=true;await page.locator('#send').click();await page.getByText('Test rejected frame',{exact:true}).waitFor();
    await page.locator('#ssid').fill('Example-WiFi');await page.locator('#wifiPassword').fill('sample-password-123');await page.locator('#saveWiFi').click();
    await page.getByText('Saved; connecting now.',{exact:true}).waitFor();assert.equal(wifi.ssid,'Example-WiFi');assert.equal(await page.locator('#wifiPassword').inputValue(),'');
    await page.evaluate(()=>refreshStatus());await page.getByText('On your Wi-Fi',{exact:true}).waitFor();assert.equal(await page.locator('#networkInfo a').getAttribute('href'),'http://192.168.1.42');
    await page.locator('#tab-text').click();await page.locator('#text').fill('Bonjour !');
    let before=frames;await page.locator('#scroll').click();await page.waitForFunction(()=>document.querySelector('#scroll').textContent==='Stop scrolling');
    await page.waitForTimeout(450);assert(frames>before);
    assert(words().every(pixel=>pixel===0||pixel===0xffff));
    await page.locator('#scroll').click();await page.waitForFunction(()=>!document.querySelector('#send').disabled);before=frames;await page.waitForTimeout(300);assert.equal(frames,before);
    await page.locator('#tab-weather').click();
    await page.getByRole('button',{name:'Use my location',exact:true}).waitFor();
    await page.locator('#tab-youtube').click();
    await page.getByLabel('YouTube Data API key').waitFor();
    await page.getByText('Enable YouTube Data API v3.').waitFor();
    await page.locator('#tab-clock').click();
    await page.locator('#clockTheme').selectOption('neon');
    await page.waitForFunction(()=>document.querySelector('#message').textContent==='Appearance saved on the panel.');
    assert.equal(appearance.clock.palette,'neon');
    assert(await page.evaluate(()=>{
      const data=document.querySelector('#canvas').getContext('2d').getImageData(0,0,64,32).data;
      let cyan=false, magenta=false;
      for(let i=0;i<data.length;i+=4){
        if(data[i]<40&&data[i+1]>200&&data[i+2]>200) cyan=true;
        if(data[i]>200&&data[i+1]<80&&data[i+2]>=200) magenta=true;
      }
      return cyan&&magenta;
    }));
    await page.locator('#clockPlay').click();
    await page.waitForFunction(()=>document.querySelector('#clockPlay').textContent==='Stop clock');
    assert.equal(liveState.mode,'clock');
    assert.equal(appearance.clock.palette,'neon');
    await page.locator('#globalTheme').selectOption('sunset');
    await page.waitForFunction(()=>document.querySelector('#globalTheme').value==='sunset');
    await page.evaluate(()=>deviceRequests);assert.equal(appearance.global,'sunset');assert.equal(appearance.clock.palette,'neon');assert.equal(appearance.weather.palette,'global');
    await page.locator('#clockLayout').selectOption('date');await page.evaluate(()=>deviceRequests);assert.equal(appearance.clock.layout,'date');assert.equal(liveState.mode,'clock');
    await page.locator('#clockMotion').selectOption('off');await page.evaluate(()=>deviceRequests);assert.equal(appearance.clock.motion,'off');
    await page.locator('#tab-weather').click();assert(await page.locator('#editorActions').isHidden());
    await page.locator('#weatherRefresh').click();await page.evaluate(()=>deviceRequests);assert(refreshes.includes('weather'));
    payload.temp=0;payload.kind='snow';payload.place='Saint-Étienne';services.weather={state:'ready',message:'Up to date.',ageSeconds:3};
    await page.evaluate(()=>refreshStatus());assert((await page.locator('#weatherStatus').textContent()).includes('3s'));
    services.weather={state:'stale',message:'Network request failed.',ageSeconds:650};await page.evaluate(()=>refreshStatus());
    assert((await page.locator('#weatherStatus').textContent()).includes('Stale'));
    await page.locator('#weatherLayout').selectOption('detail');await page.evaluate(()=>deviceRequests);assert.equal(appearance.weather.layout,'detail');
    await page.locator('#tab-youtube').click();assert((await page.locator('#youtubeStatus').textContent()).includes('Add a YouTube'));
    services.youtube={state:'error',message:'YouTube rejected the key.',ageSeconds:null};await page.evaluate(()=>refreshStatus());
    assert((await page.locator('#youtubeStatus').textContent()).includes('rejected'));
    payload.channel='Cosmic Hippo Sounds';payload.subs='0';payload.views='123.4M';payload.videos='42';services.youtube={state:'ready',message:'Up to date.',ageSeconds:0};
    await page.locator('#youtubeLayout').selectOption('subscribers');await page.evaluate(()=>deviceRequests);assert.equal(appearance.youtube.layout,'subscribers');
    await page.locator('#youtubeRefresh').click();await page.evaluate(()=>deviceRequests);assert(refreshes.includes('youtube'));
    await page.reload();await page.waitForFunction(()=>document.querySelector('#globalTheme').value==='sunset');
    await page.locator('#tab-clock').click();assert.equal(await page.locator('#clockTheme').inputValue(),'neon');assert.equal(await page.locator('#clockLayout').inputValue(),'date');
    assert.equal(await page.locator('#clockMotion').inputValue(),'off');
    await page.locator('#clockPlay').click();
    await page.waitForFunction(()=>document.querySelector('#clockPlay').textContent==='Show clock on panel');
    await page.locator('#tab-text').click();
    await page.locator('#wifiDetails').evaluate(e=>e.open=false);
    await page.screenshot({path:'/tmp/matrix-studio-desktop.png',fullPage:true});
    await page.setViewportSize({width:390,height:844});
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
    await page.screenshot({path:'/tmp/matrix-studio-mobile.png',fullPage:true});
    assert.deepEqual(errors,[]);
    console.log('Browser tests passed: sharp white/colour text, smooth edges, RGB frame order, image upload, drawing, brightness, errors, Wi-Fi form, sharp scrolling, and mobile layout.');
  }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
