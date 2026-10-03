const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {chromium}=require(require.resolve('playwright',{paths:[process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES||process.cwd()]}));
const root=path.resolve(__dirname,'..'),html=fs.readFileSync(path.join(root,'web/preview.html'),'utf8');
const catalogue=JSON.parse(fs.readFileSync(path.join(root,'assets/crystal/catalogue.json')));
catalogue.entries.sort((a,b)=>a.variant.localeCompare(b.variant)||a.id-b.id||a.key.localeCompare(b.key));
(async()=>{
  const browser=await chromium.launch({headless:true,...(process.env.MATRIX_TEST_CHROMIUM?{executablePath:process.env.MATRIX_TEST_CHROMIUM}:{})});
  try {
    const page=await browser.newPage({viewport:{width:390,height:844}});
    const errors=[];page.on('pageerror',error=>errors.push(error.message));
    let frames=[],rejectFrame=false,slowResolve=null,slow=false;
    await page.route('http://matrix.test/**',async route=>{
      const request=route.request(),url=new URL(request.url());
      if(url.pathname==='/')return route.fulfill({contentType:'text/html',body:html});
      if(url.pathname==='/sprites/crystal.json')return route.fulfill({contentType:'application/json',body:JSON.stringify(catalogue)});
      if(url.pathname.startsWith('/sprites/crystal/')) {
        if(slow&&url.pathname==='/sprites/crystal/shiny/93.gif')await new Promise(resolve=>{slowResolve=resolve;});
        const filename=path.join(root,'assets/crystal',url.pathname.slice('/sprites/crystal/'.length));
        return route.fulfill({contentType:'image/gif',body:fs.readFileSync(filename)});
      }
      if(url.pathname==='/api/status')return route.fulfill({contentType:'application/json',body:JSON.stringify({width:64,height:32,connected:true,brightness:30,frames:frames.length,ip:'192.168.1.39',hostname:'matrix-test.local'})});
      if(url.pathname==='/api/frame') {
        assert.equal(request.headers()['x-matrix-control'],'1');
        if(rejectFrame)return route.fulfill({status:503,contentType:'application/json',body:'{"error":"Panel unavailable"}'});
        const body=request.postDataBuffer(),boundary=request.headers()['content-type'].split('boundary=')[1];
        const start=body.indexOf(Buffer.from('\r\n\r\n'))+4,end=body.indexOf(Buffer.from('\r\n--'+boundary),start);
        assert.equal(end-start,4096);frames.push(Buffer.from(body.subarray(start,end)));
        return route.fulfill({contentType:'application/json',body:'{"ok":true}'});
      }
      return route.fulfill({status:404,body:'{}'});
    });
    await page.goto('http://matrix.test/');await page.locator('#tab-gallery').click();
    await page.getByText('276 matching GIFs · 552 downloaded sprites.',{exact:true}).waitFor();
    assert.equal(await page.locator('.gallery-tile').count(),24);
    assert.equal(await page.locator('#gifPage').textContent(),'1 / 12');
    await page.locator('#gifNextPage').click();assert.equal(await page.locator('#gifPage').textContent(),'2 / 12');
    await page.locator('#gifQuery').fill('#93');assert.equal(await page.locator('.gallery-tile').count(),1);
    await page.getByRole('button',{name:'Load #93 · Haunter',exact:true}).click();
    await page.getByText('#93 · Haunter ready. Choose “Play GIF on panel”.',{exact:true}).waitFor();
    assert.equal(frames.length,0); // Preview must animate without changing hardware.
    const previewBefore=await page.locator('#canvas').evaluate(c=>c.toDataURL());
    await page.waitForFunction(before=>document.querySelector('#canvas').toDataURL()!==before,previewBefore);
    await page.locator('#gifPlay').click();await page.getByText('Playing GIF: #93 · Haunter.',{exact:true}).waitFor();
    const animationDeadline=Date.now()+10000;
    while(frames.length<5&&Date.now()<animationDeadline)await page.waitForTimeout(25);
    assert(frames.length>=5);
    assert(new Set(frames.map(frame=>frame.toString('hex'))).size>=2);
    await page.locator('#gifPlay').click();let before=frames.length;
    await page.waitForTimeout(300);assert.equal(frames.length,before);
    await page.locator('#gifVariant').selectOption('both');assert.equal(await page.locator('.gallery-tile').count(),2);
    await page.locator('#gifInterval').fill('2');
    await page.locator('#gifCycle').click();
    await page.getByText('Cycling GIF: #93 · Haunter · Shiny.',{exact:true}).waitFor();
    assert.equal(await page.getByRole('button',{name:'Load #93 · Haunter · Shiny',exact:true}).getAttribute('aria-pressed'),'true');
    await page.locator('#gifCycle').click();before=frames.length;
    await page.waitForTimeout(300);assert.equal(frames.length,before);
    await page.locator('#gifQuery').fill('');await page.locator('#gifRegion').selectOption('unown');
    await page.getByText('52 matching GIFs · 552 downloaded sprites.',{exact:true}).waitFor();
    await page.locator('#gifVariant').selectOption('normal');assert.equal(await page.locator('.gallery-tile').count(),24);
    await page.locator('#gifQuery').fill('nothing');await page.locator('#gifCycle').click();
    await page.getByText('No matching GIFs to cycle. Clear or change the search.',{exact:true}).waitFor();
    assert(await page.locator('#send').isEnabled());
    // Selecting a new image during an outstanding GIF load cancels that preview.
    await page.locator('#gifRegion').selectOption('all');await page.locator('#gifVariant').selectOption('shiny');
    await page.locator('#gifQuery').fill('93');
    // The cached Haunter is already decoded, so use a new page to test a slow load.
    await page.reload();await page.locator('#tab-gallery').click();
    await page.getByText('276 matching GIFs · 552 downloaded sprites.',{exact:true}).waitFor();
    await page.locator('#gifVariant').selectOption('shiny');await page.locator('#gifQuery').fill('93');slow=true;
    await page.getByRole('button',{name:'Load #93 · Haunter · Shiny',exact:true}).click();
    while(!slowResolve)await page.waitForTimeout(10);
    await page.locator('#tab-text').click();slow=false;slowResolve();await page.waitForTimeout(150);
    assert.equal(await page.locator('#tab-text').getAttribute('aria-selected'),'true');
    // Local GIF uploads use the same compositor and animated frame transport.
    await page.locator('#tab-image').click();
    await page.locator('#imageFile').setInputFiles(path.join(root,'assets/crystal/normal/93.gif'));
    await page.getByText('GIF ready. Choose “Play GIF on panel” to animate it.',{exact:true}).waitFor();
    await page.locator('#imageGifPlay').click();await page.getByText('Playing GIF: 93.gif.',{exact:true}).waitFor();
    await page.waitForTimeout(600);before=frames.length;assert(before>=5);
    await page.locator('#tab-draw').click();await page.waitForTimeout(300);assert.equal(frames.length,before);
    await page.locator('#tab-image').click();rejectFrame=true;await page.locator('#imageGifPlay').click();
    await page.getByText('Panel unavailable',{exact:true}).waitFor();
    assert.equal(await page.locator('#imageGifPlay').textContent(),'Play GIF on panel');
    assert(await page.locator('#send').isEnabled());rejectFrame=false;
    await page.locator('#imageGifPlay').click();await page.getByText('Playing GIF: 93.gif.',{exact:true}).waitFor();
    await page.evaluate(()=>{Object.defineProperty(document,'hidden',{configurable:true,get:()=>true});document.dispatchEvent(new Event('visibilitychange'));});
    before=frames.length;await page.waitForTimeout(300);assert.equal(frames.length,before);
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
    assert(await page.locator('.tab').evaluateAll(tabs=>tabs.every(button=>button.scrollWidth<=button.clientWidth)));
    await page.locator('#tab-gallery').click();await page.locator('#gifVariant').selectOption('normal');
    await page.screenshot({path:'/tmp/matrix-gif-gallery-mobile.png',fullPage:true});
    assert.deepEqual(errors,[]);
    console.log('GIF browser tests passed: gallery, paging, search, preview, multiple uploaded animation frames, cycling, shiny/Unown filters, stale loads, local GIFs, cancellation, error handling, hidden-page pause, and mobile layout.');
  }finally {await browser.close();}
})().catch(error=>{console.error(error);process.exit(1);});
