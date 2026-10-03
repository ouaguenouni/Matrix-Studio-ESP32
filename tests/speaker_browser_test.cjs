const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {chromium}=require(require.resolve('playwright',{paths:[process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES||process.cwd()]}));
const html=fs.readFileSync(path.resolve(__dirname,'../web/preview.html'),'utf8');
(async()=>{
  const browser=await chromium.launch({headless:true,...(process.env.MATRIX_TEST_CHROMIUM?{executablePath:process.env.MATRIX_TEST_CHROMIUM}:{})});
  try {
    const page=await browser.newPage({viewport:{width:390,height:844}});
    const errors=[],posts=[];page.on('pageerror',error=>errors.push(error.message));
    let state={state:'off',connected:false,testing:false,volume:25,name:'',address:'',message:'',devices:[]};
    let rejectNext=false;
    const jbl=[{name:'JBL Go 4',address:'AA:BB:CC:DD:EE:01',rssi:-30},
      {name:'JBL Go 4',address:'AA:BB:CC:DD:EE:02',rssi:-70}];
    await page.route('http://matrix.test/**',async route=>{
      const req=route.request(),url=new URL(req.url());
      if(url.pathname==='/')return route.fulfill({contentType:'text/html',body:html});
      if(url.pathname==='/api/status')return route.fulfill({contentType:'application/json',body:JSON.stringify({width:64,height:32,connected:true,brightness:30,frames:0,ip:'192.168.1.39',hostname:'matrix-test.local'})});
      if(url.pathname==='/api/speaker')return route.fulfill({contentType:'application/json',body:JSON.stringify(state)});
      if(url.pathname.startsWith('/api/speaker/')&&req.method()==='POST') {
        assert.equal(req.headers()['x-matrix-control'],'1');
        const action=url.pathname.split('/').at(-1),body=Object.fromEntries(new URLSearchParams(req.postData()||''));posts.push({action,body});
        if(rejectNext){rejectNext=false;return route.fulfill({status:503,contentType:'application/json',body:'{"error":"Bluetooth could not start"}'});}
        if(action==='scan'){state={...state,state:'scanning',devices:jbl};setTimeout(()=>{state.state='idle';},200);}
        if(action==='connect'){
          const selected=jbl.find(device=>device.address===body.address);assert(selected);
          state={...state,state:'connecting',name:selected.name,address:selected.address};
          setTimeout(()=>{state.state='connected';state.connected=true;},200);
        }
        if(action==='disconnect'){state={...state,state:'idle',connected:false,testing:false};}
        if(action==='test'){state.testing=true;setTimeout(()=>{state.testing=false;},200);}
        if(action==='volume'){state.volume=Number(body.value);}
        return route.fulfill({contentType:'application/json',body:JSON.stringify({ok:true,message:action==='test'?'Playing a short test sound.':'Command accepted.'})});
      }
      return route.fulfill({status:404,body:'{}'});
    });
    await page.goto('http://matrix.test/');
    await page.getByText('Bluetooth speaker · JBL Go 4',{exact:true}).click();
    await page.getByText('Bluetooth is off. Scan to find your JBL Go 4.',{exact:true}).waitFor();
    assert(await page.locator('#speakerConnect').isDisabled());assert(await page.locator('#speakerTest').isDisabled());
    await page.locator('#speakerScan').click();
    await page.getByText('Scanning for speakers… Keep the JBL in pairing mode (about 13 seconds).',{exact:true}).waitFor();
    assert(await page.locator('#speakerScan').isDisabled());
    assert.equal(await page.locator('#speakerDevice').inputValue(),jbl[0].address);
    await page.locator('#speakerDevice').selectOption(jbl[1].address);
    await page.getByText('Not connected. Scan and select your JBL Go 4.',{exact:true}).waitFor();
    assert.equal(await page.locator('#speakerDevice').inputValue(),jbl[1].address);
    await page.locator('#speakerConnect').click();
    await page.getByText('Connected to JBL Go 4.',{exact:true}).waitFor();
    assert.equal(posts.find(post=>post.action==='connect').body.address,jbl[1].address);
    assert(await page.locator('#speakerScan').isDisabled());assert(await page.locator('#speakerDisconnect').isEnabled());
    assert(await page.locator('#speakerTest').isEnabled());
    await page.locator('#speakerVolume').fill('10');await page.locator('#speakerVolume').dispatchEvent('change');
    await page.waitForFunction(()=>!document.querySelector('#speakerTest').disabled);
    assert.equal(posts.find(post=>post.action==='volume').body.value,'10');
    await page.locator('#speakerTest').click();await page.getByText('Playing a short test sound.',{exact:true}).waitFor();
    assert(await page.locator('#speakerTest').isDisabled());
    await page.getByRole('button',{name:'Play a 2-second test sound',exact:true}).waitFor();
    await page.locator('#speakerDisconnect').click();await page.getByText('Not connected. Scan and select your JBL Go 4.',{exact:true}).waitFor();
    assert(await page.locator('#speakerTest').isDisabled());
    rejectNext=true;await page.locator('#speakerScan').click();await page.getByText('Bluetooth could not start',{exact:true}).waitFor();
    assert(await page.locator('#speakerScan').isEnabled());
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
    await page.screenshot({path:'/tmp/matrix-speaker-mobile.png',fullPage:true});assert.deepEqual(errors,[]);
    console.log('Speaker browser tests passed: scan, automatic JBL selection, distinct device addresses, persistent selection, asynchronous connection, test tone, volume, disconnect, errors, and mobile layout.');
  }finally{await browser.close();}
})().catch(error=>{console.error(error);process.exit(1);});
