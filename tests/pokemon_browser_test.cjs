const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {chromium}=require(require.resolve('playwright',{paths:[process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES||process.cwd()]}));
const html=fs.readFileSync(path.resolve(__dirname,'../web/preview.html'),'utf8');
(async()=>{
  const browser=await chromium.launch({headless:true,...(process.env.MATRIX_TEST_CHROMIUM?{executablePath:process.env.MATRIX_TEST_CHROMIUM}:{})});
  try{
    const page=await browser.newPage({viewport:{width:390,height:844}});
    const errors=[];page.on('pageerror',e=>errors.push(e.message));
    // Transparent padding around a solid violet rectangle should be cropped,
    // fitted to the panel and packed without acquiring a green channel.
    await page.goto('about:blank');
    const png=Buffer.from(await page.evaluate(()=>{
      const c=document.createElement('canvas');c.width=c.height=96;
      const ctx=c.getContext('2d');ctx.fillStyle='#8000c0';ctx.fillRect(30,40,20,10);
      return c.toDataURL().split(',')[1];
    }),'base64');
    const entries=[{id:1,name:'bulbasaur'},{id:2,name:'ivysaur'},{id:93,name:'haunter'},
      {id:94,name:'gengar'},{id:10001,name:'deoxys-attack'}];
    let frames=[],rejectFrame=false,slowResolve=null;
    await page.route('https://pokeapi.co/**',async route=>{
      const url=new URL(route.request().url()),key=url.pathname.split('/').filter(Boolean).at(-1);
      let data;
      if(url.search)data={results:entries.map(e=>({name:e.name,url:`https://pokeapi.co/api/v2/pokemon/${e.id}/`}))};
      else {
        if(key==='slow')await new Promise(resolve=>{slowResolve=resolve;});
        const entry=key==='slow'?entries[2]:entries.find(e=>String(e.id)===key||e.name===key);
        if(!entry)return route.fulfill({status:404,headers:{'Access-Control-Allow-Origin':'*'},body:'{}'});
        data={...entry,sprites:{front_default:entry.id===2?null:`https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/${entry.id}.png`}};
      }
      await route.fulfill({contentType:'application/json',headers:{'Access-Control-Allow-Origin':'*'},body:JSON.stringify(data)});
    });
    await page.route('https://raw.githubusercontent.com/**',route=>route.fulfill({contentType:'image/png',headers:{'Access-Control-Allow-Origin':'*'},body:png}));
    await page.route('http://matrix.test/**',async route=>{
      const url=new URL(route.request().url());
      if(url.pathname==='/')return route.fulfill({contentType:'text/html',body:html});
      if(url.pathname==='/api/status')return route.fulfill({contentType:'application/json',body:JSON.stringify({width:64,height:32,connected:true,brightness:30,frames:frames.length,ip:'192.168.1.39',hostname:'matrix-test.local'})});
      if(url.pathname==='/api/frame'){
        assert.equal(route.request().headers()['x-matrix-control'],'1');
        if(rejectFrame)return route.fulfill({status:503,contentType:'application/json',body:JSON.stringify({error:'Panel unavailable'})});
        const body=route.request().postDataBuffer(),boundary=route.request().headers()['content-type'].split('boundary=')[1];
        const start=body.indexOf(Buffer.from('\r\n\r\n'))+4,end=body.indexOf(Buffer.from('\r\n--'+boundary),start);
        assert.equal(end-start,4096);frames.push(Buffer.from(body.subarray(start,end)));
        return route.fulfill({contentType:'application/json',body:'{"ok":true}'});
      }
      return route.fulfill({status:404,body:'{}'});
    });
    await page.goto('http://matrix.test/');await page.locator('#tab-pokemon').click();
    await page.getByText('5 catalogue entries loaded; 1 matches.',{exact:true}).waitFor();
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
    await page.locator('#pokemonLoad').click();await page.getByText('#93 · Haunter ready.',{exact:true}).waitFor();
    await page.locator('#send').click();await page.getByText('Sent to your display.',{exact:true}).waitFor();
    for(let i=0;i<2048;i++)assert.equal(frames[0].readUInt16LE(i*2),0x8018);
    await page.locator('#pokemonQuery').fill('#0093');await page.locator('#pokemonLoad').click();
    await page.getByText('#93 · Haunter ready.',{exact:true}).waitFor();
    await page.locator('#pokemonNext').click();await page.getByText('#94 · Gengar ready.',{exact:true}).waitFor();
    await page.locator('#pokemonPrevious').click();await page.getByText('#93 · Haunter ready.',{exact:true}).waitFor();
    await page.locator('#pokemonQuery').fill('');
    assert.equal(await page.locator('#pokemonResults option').count(),5);
    await page.locator('#pokemonScope').selectOption('gen1');
    assert.equal(await page.locator('#pokemonResults option').count(),4);
    await page.locator('#pokemonInterval').fill('2');
    let before=frames.length;await page.locator('#pokemonCycle').click();
    await page.getByText('Cycling: #93 · Haunter.',{exact:true}).waitFor();
    await page.getByText('Cycling: #94 · Gengar.',{exact:true}).waitFor();
    assert.equal(frames.length,before+2);
    await page.locator('#pokemonCycle').click();before=frames.length;
    await page.waitForTimeout(2200);assert.equal(frames.length,before);
    // Missing images are skipped instead of terminating the sequence.
    await page.locator('#pokemonQuery').fill('1');await page.locator('#pokemonLoad').click();
    await page.getByText('#1 · Bulbasaur ready.',{exact:true}).waitFor();
    await page.locator('#pokemonCycle').click();await page.getByText('Cycling: #1 · Bulbasaur.',{exact:true}).waitFor();
    await page.getByText('Cycling: #93 · Haunter.',{exact:true}).waitFor();
    await page.locator('#tab-text').click();before=frames.length;
    await page.waitForTimeout(2200);assert.equal(frames.length,before);
    // A cancelled slow fetch must not overwrite a newly selected sprite.
    await page.locator('#tab-pokemon').click();await page.locator('#pokemonQuery').fill('slow');
    await page.locator('#pokemonLoad').click();await page.waitForFunction(()=>document.querySelector('#pokemonLoad').disabled);
    while(!slowResolve)await page.waitForTimeout(10);
    await page.locator('#pokemonQuery').fill('94');await page.locator('#pokemonLoad').click();
    await page.getByText('#94 · Gengar ready.',{exact:true}).waitFor();slowResolve();
    await page.waitForTimeout(100);assert.equal(await page.locator('#pokemonStatus').textContent(),'#94 · Gengar ready.');
    await page.locator('#pokemonQuery').fill('unknown');await page.locator('#pokemonLoad').click();
    await page.getByText('No Pokémon found with that name or number.',{exact:true}).waitFor();
    assert(await page.locator('#send').isEnabled());
    rejectFrame=true;await page.locator('#pokemonCycle').click();await page.getByText('Panel unavailable',{exact:true}).waitFor();
    assert.equal(await page.locator('#pokemonCycle').textContent(),'Start cycling on panel');
    assert(await page.locator('#send').isEnabled());rejectFrame=false;
    await page.locator('#pokemonCycle').click();await page.getByText('Cycling: #94 · Gengar.',{exact:true}).waitFor();
    await page.evaluate(()=>{Object.defineProperty(document,'hidden',{configurable:true,get:()=>true});document.dispatchEvent(new Event('visibilitychange'));});
    before=frames.length;await page.waitForTimeout(2200);assert.equal(frames.length,before);
    assert.equal(await page.locator('#pokemonCycle').textContent(),'Start cycling on panel');
    await page.screenshot({path:'/tmp/matrix-pokemon-mobile.png',fullPage:true});
    assert.deepEqual(errors,[]);
    console.log('Pokémon browser tests passed: search, purple RGB565, transparent cropping, catalogue scope, navigation, cycling, missing-sprite skipping, stop, stale loads, network errors, mobile layout, and hidden-page pause.');
  }finally{await browser.close();}
})().catch(error=>{console.error(error);process.exit(1);});
