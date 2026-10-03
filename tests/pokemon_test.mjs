import assert from 'node:assert/strict';
import {pokemonQuery,pokemonEntries,filterPokemon,createPokemonClient} from '../web/pokemon.mjs';
import {opaqueBounds} from '../web/codec.mjs';

assert.equal(pokemonQuery(' Haunter '),'haunter');
assert.equal(pokemonQuery('#0093'),'93');
for(const bad of ['',0,'../93','haunter?x=1'])assert.throws(()=>pokemonQuery(bad));
const raw={results:[{name:'haunter',url:'https://pokeapi.co/api/v2/pokemon/93/'},
  {name:'bulbasaur',url:'https://pokeapi.co/api/v2/pokemon/1/'},
  {name:'deoxys-attack',url:'https://pokeapi.co/api/v2/pokemon/10001/'},
  {name:'haunter',url:'https://pokeapi.co/api/v2/pokemon/93/'},
  {name:'bad',url:'https://other.test/1/'}]};
const entries=pokemonEntries(raw);
assert.deepEqual(entries.map(e=>e.id),[1,93,10001]);
assert.equal(filterPokemon(entries,'all').length,3);
assert.equal(filterPokemon(entries,'gen1').length,2);
assert.equal(filterPokemon(entries,'all','HAUN')[0].id,93);
assert.equal(filterPokemon(entries,'all','#0093')[0].name,'haunter');
assert.throws(()=>pokemonEntries({results:[]}));
const rgba=new Uint8ClampedArray(8*6*4);
assert.equal(opaqueBounds(rgba,8,6),null);
rgba[(2*8+3)*4+3]=255;rgba[(4*8+6)*4+3]=1;
assert.deepEqual(opaqueBounds(rgba,8,6),{x:3,y:2,w:4,h:3});
assert.throws(()=>opaqueBounds(rgba,1,1));

const saved=new Map();
globalThis.localStorage={getItem:key=>saved.get(key)||null,setItem:(key,value)=>saved.set(key,value)};
let fetches=0,decodes=0,failCatalogue=true;
const fetcher=async url=>{
  fetches++;
  if(url.includes('?limit=')){
    if(failCatalogue){failCatalogue=false;throw new Error('offline');}
    return {ok:true,json:async()=>raw};
  }
  const query=url.split('/').at(-1);
  if(query==='missing')return {status:404,ok:false};
  return {ok:true,json:async()=>({id:query==='empty'?10001:93,name:query==='empty'?'empty':'haunter',
    sprites:{front_default:query==='empty'?null:query==='unsafe'?'https://other.test/sprite.png':
      'https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/93.png'}})};
};
const decode=async url=>{decodes++;return {url,width:96,height:96};};
const client=createPokemonClient(fetcher,decode);
await assert.rejects(client.catalogue(),/offline/);
const [catalogue,other]=await Promise.all([client.catalogue(),client.catalogue()]);
assert.strictEqual(catalogue,other);assert.equal(fetches,2);
const haunter=await client.load('Haunter');
assert.strictEqual((await client.load('#93')).image,haunter.image);
assert.equal(fetches,3);assert.equal(decodes,1);
await assert.rejects(client.load('missing'),/No Pokémon found/);
await assert.rejects(client.load('empty'),e=>e.code==='NO_SPRITE');
await assert.rejects(client.load('unsafe'),/Unsupported sprite source/);
const cached=createPokemonClient(()=>{throw new Error('Should use saved cache');},decode);
assert.equal((await cached.catalogue()).length,3);
assert.equal((await cached.load('haunter')).id,93);
console.log('Pokémon tests passed: catalogue, search, retry, cached metadata/sprites, missing sprites, source validation, and alpha cropping.');
