const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {chromium}=require(require.resolve('playwright',{paths:[process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES||process.cwd()]}));
const root=path.resolve(__dirname,'..'),html=fs.readFileSync(path.join(root,'web/preview.html'),'utf8');
const catalogue=JSON.parse(fs.readFileSync(path.join(root,'assets/crystal/catalogue.json')));
catalogue.entries.sort((a,b)=>a.variant.localeCompare(b.variant)||a.id-b.id||a.key.localeCompare(b.key));
(async()=>{
  const browser=await chromium.launch({headless:true,...(process.env.MATRIX_TEST_CHROMIUM?{executablePath:process.env.MATRIX_TEST_CHROMIUM}:{})});
  try {
    const page=await browser.newPage({viewport:{width:390,height:844}});
    const errors=[],events=[];page.on('pageerror',error=>errors.push(error.message));
    const activeRequests=new Set();let peakRequests=0;
    page.on('request',req=>{
      if(req.url().startsWith('http://matrix.test/')&&new URL(req.url()).pathname!=='/') {
        activeRequests.add(req);peakRequests=Math.max(peakRequests,activeRequests.size);
      }
    });
    page.on('requestfinished',req=>activeRequests.delete(req));
    page.on('requestfailed',req=>activeRequests.delete(req));
    let frames=0,connected=true,slowNext=false,resolveFrame=null;
    await page.route('http://matrix.test/**',async route=>{
      const req=route.request(),url=new URL(req.url());
      if(url.pathname==='/')return route.fulfill({contentType:'text/html',body:html});
      if(url.pathname==='/sprites/crystal.json')return route.fulfill({contentType:'application/json',body:JSON.stringify(catalogue)});
      if(url.pathname.startsWith('/sprites/crystal/'))return route.fulfill({contentType:'image/gif',body:fs.readFileSync(path.join(root,'assets/crystal',url.pathname.slice('/sprites/crystal/'.length)))});
      if(url.pathname==='/api/status')return route.fulfill({contentType:'application/json',body:JSON.stringify({width:64,height:32,connected:true,brightness:30,frames,ip:'192.168.1.39',hostname:'matrix-test.local'})});
      if(url.pathname==='/api/speaker')return route.fulfill({contentType:'application/json',body:JSON.stringify({state:connected?'connected':'idle',connected,testing:false,playing:false,volume:25,name:'JBL Go 4',devices:[]})});
      if(url.pathname==='/api/frame') {
        assert.equal(req.headers()['x-matrix-control'],'1');
        const body=req.postDataBuffer(),boundary=req.headers()['content-type'].split('boundary=')[1];
        const start=body.indexOf(Buffer.from('\r\n\r\n'))+4,end=body.indexOf(Buffer.from('\r\n--'+boundary),start);
        assert.equal(end-start,4096);frames++;
        const cry=body.toString('latin1').match(/name="cry"\r\n\r\n(\d+)\r\n/);
        if(cry)events.push({type:'cry',id:Number(cry[1]),frame:frames});
        if(slowNext&&cry){slowNext=false;await new Promise(resolve=>{resolveFrame=resolve;});}
        return route.fulfill({contentType:'application/json',body:JSON.stringify({ok:true,cry:cry?(connected?'queued':'disconnected'):'none'})});
      }
      if(url.pathname==='/api/speaker/cry/stop') {
        assert.equal(req.headers()['x-matrix-control'],'1');events.push({type:'stop'});
        return route.fulfill({contentType:'application/json',body:'{"ok":true}'});
      }
      return route.fulfill({status:404,body:'{}'});
    });
    const until=async predicate=>{
      const deadline=Date.now()+10000;
      while(!predicate()&&Date.now()<deadline)await page.waitForTimeout(25);
      assert(predicate(),'Timed out waiting for recorded audio/frame event');
    };
    const cries=()=>events.filter(event=>event.type==='cry');
    await page.goto('http://matrix.test/');await page.locator('#tab-gallery').click();
    await page.getByText('276 matching GIFs · 552 downloaded sprites.',{exact:true}).waitFor();
    await page.locator('#gifQuery').fill('93');await page.getByRole('button',{name:'Load #93 · Haunter',exact:true}).click();
    await page.getByText('#93 · Haunter ready. Choose “Play GIF on panel”.',{exact:true}).waitFor();
    assert.equal(cries().length,0); // Preview remains silent.
    await page.locator('#gifPlay').click();await until(()=>frames>=6);
    assert.equal(cries().length,1);assert.equal(cries()[0].id,93);assert.equal(cries()[0].frame,1);
    await page.getByText('Matching cry sent to your JBL.',{exact:true}).waitFor();
    await page.locator('#gifCries').uncheck();await until(()=>events.at(-1)?.type==='stop');
    const before=cries().length;await until(()=>frames>=9);assert.equal(cries().length,before);
    await page.locator('#gifPlay').click();await page.locator('#gifCries').check();
    // Different species are cued on their first frame while cycling.
    await page.locator('#gifQuery').fill('');await page.locator('#gifRegion').selectOption('kanto');
    await page.locator('#gifInterval').fill('2');const cycleStart=cries().length;
    await page.locator('#gifCycle').click();await until(()=>cries().length>=cycleStart+2);
    assert.deepEqual(cries().slice(cycleStart,cycleStart+2).map(event=>event.id),[93,94]);
    await page.locator('#gifCycle').click();await until(()=>events.at(-1)?.type==='stop');
    // Shiny Haunter and all Unown letters use their species IDs.
    await page.locator('#gifRegion').selectOption('all');await page.locator('#gifVariant').selectOption('shiny');
    await page.locator('#gifQuery').fill('93');await page.getByRole('button',{name:'Load #93 · Haunter · Shiny',exact:true}).click();
    await page.getByText('#93 · Haunter · Shiny ready. Choose “Play GIF on panel”.',{exact:true}).waitFor();
    const shinyStart=cries().length;await page.locator('#gifPlay').click();await until(()=>cries().length>shinyStart);
    assert.equal(cries().at(-1).id,93);await page.locator('#gifPlay').click();
    await page.locator('#gifVariant').selectOption('normal');await page.locator('#gifQuery').fill('unown-b');
    await page.getByRole('button',{name:'Load #201 · Unown B',exact:true}).click();
    await page.getByText('#201 · Unown B ready. Choose “Play GIF on panel”.',{exact:true}).waitFor();
    const unownStart=cries().length;await page.locator('#gifPlay').click();await until(()=>cries().length>unownStart);
    assert.equal(cries().at(-1).id,201);await page.locator('#gifPlay').click();
    // A late old upload must finish, stop its cry, then cue the restarted GIF.
    await until(()=>events.at(-1)?.type==='stop');const raceStart=events.length;
    slowNext=true;await page.locator('#gifPlay').click();await until(()=>!!resolveFrame);
    await page.locator('#gifPlay').click();await page.locator('#gifPlay').click();
    resolveFrame();resolveFrame=null;await until(()=>events.length>=raceStart+3);
    assert.deepEqual(events.slice(raceStart,raceStart+3).map(event=>event.type),['cry','stop','cry']);
    await page.locator('#gifPlay').click();await until(()=>events.at(-1)?.type==='stop');
    connected=false;await page.locator('#gifPlay').click();
    await page.getByText('GIF playing silently. Connect your JBL below to hear Pokémon cries.',{exact:true}).waitFor();
    const disconnectedFrames=frames;await until(()=>frames>=disconnectedFrames+2);
    await page.locator('#tab-text').click();await until(()=>events.at(-1)?.type==='stop');
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));assert.deepEqual(errors,[]);
    await page.locator('#tab-gallery').click();await page.screenshot({path:'/tmp/matrix-cry-gallery-mobile.png',fullPage:true});
    assert.equal(peakRequests,1,'ESP32 requests must stay serialized during thumbnails, frames, polling and cry stops');
    console.log('Cry browser tests passed: silent preview, atomic first-frame cue, one cry per appearance, cycling ID match, mute, shiny/Unown IDs, cancellation and restart ordering, disconnected fallback, and mobile layout.');
  }finally{await browser.close();}
})().catch(error=>{console.error(error);process.exit(1);});
