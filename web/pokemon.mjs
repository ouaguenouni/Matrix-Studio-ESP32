export function pokemonQuery(value) {
  const query=String(value).trim().toLowerCase().replace(/^#/, '');
  if (/^\d+$/.test(query) && Number(query)>0) return String(Number(query));
  if (/^[a-z][a-z0-9-]*$/.test(query)) return query;
  throw new Error('Enter a Pokémon name or Pokédex number, such as Haunter or 93.');
}

export function pokemonEntries(data) {
  const entries=new Map();
  for (const item of data.results || []) {
    const match=/^https:\/\/pokeapi\.co\/api\/v2\/pokemon\/(\d+)\/$/.exec(item.url || '');
    if (match && /^[a-z][a-z0-9-]*$/.test(item.name))
      entries.set(Number(match[1]), {id:Number(match[1]),name:item.name});
  }
  if (!entries.size) throw new Error('The Pokémon catalogue is empty. Try again.');
  return [...entries.values()].sort((a,b)=>a.id-b.id);
}

export function filterPokemon(entries, scope, search='') {
  const query=search.trim().toLowerCase().replace(/^#/, '');
  return entries.filter(entry=>(scope!=='gen1' || entry.id<=151) &&
    (!query || (/^\d+$/.test(query) ? entry.id===Number(query) : entry.name.includes(query))));
}

export function createPokemonClient(fetcher=fetch, decode=null) {
  const imageCache=new Map(), detailsCache=new Map();
  let cataloguePromise=null;
  const prefix='matrix-pokemon-v1:', cacheAge=7*24*60*60*1000;
  function readCache(key) {
    try {
      const saved=JSON.parse(localStorage.getItem(prefix+key));
      return saved && Date.now()-saved.at<cacheAge ? saved.data : null;
    } catch { return null; }
  }
  function writeCache(key,data) {
    try { localStorage.setItem(prefix+key,JSON.stringify({at:Date.now(),data})); } catch {}
  }
  async function getJSON(key,url,project=value=>value) {
    const cached=readCache(key);if(cached)return cached;
    const controller=new AbortController(), timer=setTimeout(()=>controller.abort(),12000);
    try {
      const response=await fetcher(url,{signal:controller.signal,cache:'force-cache',credentials:'omit'});
      if(response.status===404)throw new Error('No Pokémon found with that name or number.');
      if(!response.ok)throw new Error('Pokémon service unavailable. Try again later.');
      const data=project(await response.json());writeCache(key,data);return data;
    } catch(error) {
      if(error.name==='AbortError')throw new Error('Pokémon download timed out. Check your internet connection.');
      throw error;
    } finally {clearTimeout(timer);}
  }
  async function defaultDecode(url) {
    return new Promise((resolve,reject)=>{
      const image=new Image(),timer=setTimeout(()=>{image.src='';reject(new Error('Sprite download timed out.'));},12000);
      image.crossOrigin='anonymous';
      image.onload=()=>{clearTimeout(timer);resolve(image);};
      image.onerror=()=>{clearTimeout(timer);reject(new Error('Could not download the sprite. Check your internet connection.'));};
      image.src=url;
    });
  }
  return {
    catalogue() {
      if(!cataloguePromise)cataloguePromise=getJSON('catalogue','https://pokeapi.co/api/v2/pokemon?limit=10000')
        .then(pokemonEntries).catch(error=>{cataloguePromise=null;throw error;});
      return cataloguePromise;
    },
    async load(value) {
      const query=pokemonQuery(value);
      let data=detailsCache.get(query);
      if(!data) {
        data=await getJSON('pokemon:'+query,'https://pokeapi.co/api/v2/pokemon/'+query,
          value=>({id:value.id,name:value.name,sprites:{front_default:value.sprites?.front_default || null}}));
        if(!Number.isInteger(data.id)||data.id<=0||!/^[a-z][a-z0-9-]*$/.test(data.name))
          throw new Error('Invalid Pokémon response. Try again.');
        detailsCache.set(query,data);detailsCache.set(String(data.id),data);detailsCache.set(data.name,data);
      }
      const url=data.sprites?.front_default;
      if(!url) {const error=new Error('No front sprite is available for '+data.name+'.');error.code='NO_SPRITE';throw error;}
      if(!url.startsWith('https://raw.githubusercontent.com/PokeAPI/sprites/'))throw new Error('Unsupported sprite source.');
      let image=imageCache.get(url);
      if(!image) {
        image=await (decode || defaultDecode)(url);
        imageCache.set(url,image);
        if(imageCache.size>32)imageCache.delete(imageCache.keys().next().value);
      }
      return {id:data.id,name:data.name,image};
    }
  };
}
