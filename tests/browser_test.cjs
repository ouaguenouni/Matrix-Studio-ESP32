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
  let frame=null, frames=0, brightness=30, wifi=null, rejectNext=false;
  await page.route('http://matrix.test/**',async route=>{
    const req=route.request(),url=new URL(req.url());
    if(url.pathname==='/')return route.fulfill({contentType:'text/html',body:html});
    let result={ok:true},status=200;
    if(req.method()==='POST')assert.equal(req.headers()['x-matrix-control'],'1');
    if(url.pathname==='/api/status')result={width:64,height:32,connected:!!wifi,ip:wifi?'192.168.1.42':'',hostname:'matrix-test.local',ssid:wifi?.ssid||'',ap:'Matrix-test',brightness,frames,freeHeap:120000};
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
    await page.locator('#wifiDetails').evaluate(e=>e.open=false);
    await page.screenshot({path:'/tmp/matrix-studio-desktop.png',fullPage:true});
    await page.setViewportSize({width:390,height:844});
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
    await page.screenshot({path:'/tmp/matrix-studio-mobile.png',fullPage:true});
    assert.deepEqual(errors,[]);
    console.log('Browser tests passed: sharp white/colour text, smooth edges, RGB frame order, image upload, drawing, brightness, errors, Wi-Fi form, sharp scrolling, and mobile layout.');
  }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
