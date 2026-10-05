#include "live.h"
#include "local_config.h"
#include "LiveData.h"
#include "LiveState.h"
#include "Display.h"
#include "gift.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <time.h>

using LiveState::Appearance;
static Preferences* store=nullptr;
static MatrixPanel_I2S_DMA* matrix=nullptr;
static uint8_t mode=1, globalPalette=2, requestedBrightness=30, appliedBrightness=30;
static Appearance appearance[3];
static bool hold=false, ntpStarted=false, wasOnline=false, workerBusy=false;
static uint32_t drawnAt=0, modeAt=0;
static LiveState::Service services[2];
static LiveData::Weather weather;
static LiveData::Youtube youtube;
static LiveData::Place place;
static bool hasPlace=false;
static String cityQuery, youtubeKey;
static bool locatePending=false;
static QueueHandle_t jobs=nullptr, results=nullptr;

// Queues copy fixed-size values: no String ownership or shared Preferences/display access.
struct Job {
  uint8_t service=0; uint32_t generation=0;
  bool hasPlace=false, locate=false;
  LiveData::Place place;
  char city[241]={}, key[129]={};
};
struct Result {
  uint8_t service=0; uint32_t generation=0;
  bool ok=false, hasPlace=false;
  LiveData::Place place;
  LiveData::Weather weather;
  LiveData::Youtube youtube;
  char error[120]={};
};
class BoundedBody : public Stream {
public:
  String value;
  size_t write(uint8_t c) override { return write(&c,1); }
  size_t write(const uint8_t* bytes,size_t size) override {
    if(value.length()+size>12000) return 0;
    return value.concat(reinterpret_cast<const char*>(bytes),size)?size:0;
  }
  int available() override {return 0;} int read() override {return -1;} int peek() override {return -1;} void flush() override {}
};
static String urlEncode(const String& value) {
  String out;
  for(size_t i=0;i<value.length();++i){unsigned char c=value[i];
    if(isalnum(c)||c=='-'||c=='_'||c=='.')out+=char(c);
    else {char b[4];snprintf(b,sizeof(b),"%%%02X",c);out+=b;}}
  return out;
}
static bool getJson(const String& url,JsonDocument& doc,Result& r,bool yt=false) {
  HTTPClient http;WiFiClient plain;WiFiClientSecure tls;
  tls.setInsecure(); // Retains existing device transport policy.
  tls.setHandshakeTimeout(6);
  http.setConnectTimeout(5000);http.setTimeout(6000);http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  const bool started=url.startsWith("https:")?http.begin(tls,url):http.begin(plain,url);
  int code=started?http.GET():-1;
  BoundedBody body;
  bool received=false;
  if(code>0 && http.getSize()<=12000)received=http.writeToStream(&body)>=0;
  http.end();
  const auto parsed=deserializeJson(doc,body.value);
  if(code!=200){snprintf(r.error,sizeof(r.error),"%s",LiveData::apiError(code,doc.as<JsonVariantConst>(),yt));return false;}
  if(!received || parsed){snprintf(r.error,sizeof(r.error),"Invalid or oversized service response.");return false;}
  return true;
}
static void networkWorker(void*) {
  Job job;
  for(;;) {
    if(xQueueReceive(jobs,&job,portMAX_DELAY)!=pdTRUE)continue;
    Result r;r.service=job.service;r.generation=job.generation;
    DynamicJsonDocument doc(16384);
    if(job.service==0) {
      r.place=job.place;r.hasPlace=job.hasPlace;
      if(job.locate || job.city[0] || !job.hasPlace) {
        const bool ip=!job.city[0];
        String url=ip?String("http://ip-api.com/json/?fields=status,city,lat,lon"):
          String("https://geocoding-api.open-meteo.com/v1/search?count=1&language=en&name=")+urlEncode(job.city);
        if(getJson(url,doc,r) && LiveData::place(doc.as<JsonVariantConst>(),ip,r.place))r.hasPlace=true;
        else {r.hasPlace=false;if(!r.error[0])snprintf(r.error,sizeof(r.error),"Location not found. Choose a city in the web controls.");}
      }
      if(r.hasPlace && !r.error[0]) {
        doc.clear();
        String url=String("https://api.open-meteo.com/v1/forecast?current=temperature_2m,weather_code&latitude=")+String(r.place.lat,4)+"&longitude="+String(r.place.lon,4);
        if(getJson(url,doc,r)) {
          r.ok=LiveData::weather(doc.as<JsonVariantConst>(),r.weather);
          if(!r.ok)snprintf(r.error,sizeof(r.error),"Weather response is missing valid current measurements.");
        }
      }
    } else {
      const String url=String("https://www.googleapis.com/youtube/v3/channels?part=statistics,snippet&forHandle=CosmicHippoSounds&fields=items(snippet(title),statistics)&key=")+urlEncode(job.key);
      if(getJson(url,doc,r,true)) {
        r.ok=LiveData::youtube(doc.as<JsonVariantConst>(),r.youtube);
        if(!r.ok)snprintf(r.error,sizeof(r.error),"No channel statistics returned. Check the channel and API access.");
      }
    }
    memset(job.key,0,sizeof(job.key));
    xQueueSend(results,&r,portMAX_DELAY);
  }
}
static String abbreviate(uint64_t count) {
  const char* units[]={"","K","M","B","T","Q"};double n=double(count);int u=0;
  while(n>=999.5 && u<5){n/=1000;++u;}
  char out[24];
  if(!u)snprintf(out,sizeof(out),"%llu",static_cast<unsigned long long>(count));
  else if(n>=100 || fabs(n-round(n))<0.05)snprintf(out,sizeof(out),"%.0f%s",n,units[u]);
  else snprintf(out,sizeof(out),"%.1f%s",n,units[u]);
  return out;
}
static void persist() {
  if(!store)return;
  store->putUChar("live",mode);store->putBool("liveSet",true);
  store->putUChar("globalPal",globalPalette);
  for(int i=0;i<3;++i){char key[12];
    snprintf(key,sizeof(key),"pal%d",i);store->putChar(key,appearance[i].palette);
    snprintf(key,sizeof(key),"layout%d",i);store->putUChar(key,appearance[i].layout);
    snprintf(key,sizeof(key),"motion%d",i);store->putUChar(key,appearance[i].motion);}
}
void liveBegin(Preferences& settings,MatrixPanel_I2S_DMA* panel) {
  store=&settings;matrix=panel;giftBegin(settings);
  mode=LiveState::restoredMode(settings.getBool("liveSet",false),settings.getUChar("live",1));
  globalPalette=settings.getUChar("globalPal",LiveState::migrateTheme(settings.getUChar("theme",2)));
  if(globalPalette>5)globalPalette=2;
  for(int i=0;i<3;++i){char key[12];
    snprintf(key,sizeof(key),"pal%d",i);appearance[i].palette=settings.getChar(key,-1);
    snprintf(key,sizeof(key),"layout%d",i);appearance[i].layout=settings.getUChar(key,0);
    snprintf(key,sizeof(key),"motion%d",i);appearance[i].motion=settings.getUChar(key,1);
    if(appearance[i].palette < -1 || appearance[i].palette>5)appearance[i].palette=-1;
    if(appearance[i].layout>1)appearance[i].layout=0;
    if(appearance[i].motion>2)appearance[i].motion=1;}
  place.lat=settings.getFloat("lat",0);place.lon=settings.getFloat("lon",0);
  snprintf(place.name,sizeof(place.name),"%s",settings.getString("city","").c_str());
  hasPlace=settings.isKey("lat") && settings.isKey("lon") && place.name[0] && std::isfinite(place.lat) && std::isfinite(place.lon) && fabs(place.lat)<=90 && fabs(place.lon)<=180;
  youtubeKey=settings.getString("ytKey","");
  if(youtubeKey.isEmpty() && YOUTUBE_API_KEY[0]){youtubeKey=YOUTUBE_API_KEY;settings.putString("ytKey",youtubeKey);}
  persist();
  jobs=xQueueCreate(1,sizeof(Job));results=xQueueCreate(1,sizeof(Result));
  if(!jobs || !results || xTaskCreate(networkWorker,"matrix-fetch",12288,nullptr,1,nullptr)!=pdPASS){
    if(jobs)vQueueDelete(jobs);if(results)vQueueDelete(results);jobs=nullptr;results=nullptr;
    for(auto& s:services){s.failed=true;snprintf(s.error,sizeof(s.error),"Network worker unavailable. Restart the panel.");}}
}
void liveHold(){hold=true;giftManual();}
void liveNextMode(){giftManual();mode=mode==1?2:mode==2 && youtubeKey.length()?3:1;hold=false;modeAt=millis();persist();}

bool liveConfigure(const LiveRequest& q,String& error) {
  // Validate every field before touching state, preferences, or pending work.
  uint8_t nextMode=mode,nextGlobal=globalPalette;Appearance next[3];memcpy(next,appearance,sizeof(next));
  if(q.mode.length() && q.mode!="save") {int n=LiveState::find(q.mode.c_str(),LiveState::modes,4);if(n<0){error="Unknown display mode.";return false;}nextMode=n;}
  if(q.theme.length()){int n=LiveState::find(q.theme.c_str(),LiveState::palettes,6);if(n<0){error="Unknown global palette.";return false;}nextGlobal=n;}
  const int target=LiveState::find(q.target.c_str(),LiveState::modes,4)-1;
  if(q.target.length() && target<0){error="Choose clock, weather, or YouTube for appearance.";return false;}
  if(q.palette.length() || q.layout.length() || q.motion.length()) {
    if(target<0){error="Appearance changes need a target mode.";return false;}
    if(q.palette.length()){int n=q.palette=="global"?-1:LiveState::find(q.palette.c_str(),LiveState::palettes,6);if(n<0 && q.palette!="global"){error="Unknown palette.";return false;}next[target].palette=n;}
    if(q.layout.length()){int n=LiveState::find(q.layout.c_str(),LiveState::layouts[target],2);if(n<0){error="Unknown layout.";return false;}next[target].layout=n;}
    if(q.motion.length()){int n=LiveState::find(q.motion.c_str(),LiveState::motions,3);if(n<0){error="Unknown motion setting.";return false;}next[target].motion=n;}
  }
  if(q.key.length()>128){error="That API key is too long.";return false;}
  if(q.city.length()>240){error="That city name is too long.";return false;}
  if(q.locate && q.city.length()){error="Choose automatic location or a city.";return false;}
  if(q.refresh.length() && q.refresh!="weather" && q.refresh!="youtube"){error="Unknown service to refresh.";return false;}
  if((q.mode=="youtube" || q.refresh=="youtube") && q.key.isEmpty() && youtubeKey.isEmpty()){error="Add a YouTube Data API key first.";return false;}
  if(q.key.length() && (!store || !store->putString("ytKey",q.key))){error="Could not save the YouTube key.";return false;}
  globalPalette=nextGlobal;memcpy(appearance,next,sizeof(next));
  if(q.key.length()){youtubeKey=q.key;services[1].invalidate(true);}
  if(q.locate || q.city.length()){cityQuery=q.city;locatePending=q.locate;services[0].invalidate(true);}
  if(q.refresh.length()){services[q.refresh=="weather"?0:1].invalidate();}
  if(q.mode.length() && q.mode!="save") {giftManual();mode=nextMode;hold=mode==0;modeAt=millis();}
  persist();return true;
}
static void serviceStatus(JsonObject out,int index,bool online,uint32_t now) {
  const auto& s=services[index];bool configured=index==0 || youtubeKey.length();
  out["state"]=s.state(online,configured,now);out["refreshing"]=s.loading;
  if(s.valid)out["ageSeconds"]=uint32_t(now-s.successAt)/1000;else out["ageSeconds"]=nullptr;
  if(!configured)out["message"]="Add a YouTube Data API key.";
  else if(!online)out["message"]="Waiting for Wi-Fi. Check the connection settings.";
  else if(s.error[0])out["message"]=s.error;
  else out["message"]=s.valid?"Up to date.":"Loading from the service…";
}
String liveStatusJson() {
  DynamicJsonDocument doc(6144);uint32_t now=millis();bool online=WiFi.status()==WL_CONNECTED;
  giftStatus(doc.createNestedObject("gift"),now);
  doc["effectiveBrightness"]=appliedBrightness;
  doc["displayMode"]=hold||mode==0?0:(giftMode(now,youtubeKey.length())?giftMode(now,youtubeKey.length()):mode);
  doc["live"]=hold?"off":LiveState::modes[mode];doc["savedMode"]=LiveState::modes[mode];doc["theme"]=LiveState::palettes[globalPalette];
  doc["liveTick"]=uint32_t(now-modeAt);doc["place"]=place.name;doc["youtube"]=youtubeKey.length()>0;
  struct tm clockTime;doc["clockSynced"]=getLocalTime(&clockTime,0);
  JsonObject a=doc.createNestedObject("appearance");a["global"]=LiveState::palettes[globalPalette];
  for(int i=0;i<3;++i){auto v=a.createNestedObject(LiveState::modes[i+1]);v["palette"]=appearance[i].palette<0?"global":LiveState::palettes[appearance[i].palette];v["layout"]=LiveState::layouts[i][appearance[i].layout];v["motion"]=LiveState::motions[appearance[i].motion];}
  auto svc=doc.createNestedObject("services");serviceStatus(svc.createNestedObject("weather"),0,online,now);serviceStatus(svc.createNestedObject("youtube"),1,online,now);
  if(services[0].valid){doc["temp"]=int(lroundf(weather.temperature));doc["weatherCode"]=weather.code;String k=Display::kind(weather.code);k.toLowerCase();doc["kind"]=k;}
  if(services[1].valid){doc["channel"]=youtube.title;doc["subs"]=youtube.hidden?String("--"):abbreviate(youtube.subs);doc["views"]=abbreviate(youtube.views);doc["videos"]=abbreviate(youtube.videos);}
  String out;serializeJson(doc,out);return ","+out.substring(1,out.length()-1);
}
static void processNetwork(uint32_t now,bool online) {
  if(online && !ntpStarted){setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);tzset();configTime(0,0,"pool.ntp.org","time.nist.gov");ntpStarted=true;}
  if(online!=wasOnline){for(auto& s:services)s.invalidate();wasOnline=online;}
  Result r;
  if(results && xQueueReceive(results,&r,0)==pdTRUE) {
    workerBusy=false;auto& s=services[r.service];
    if(s.finish(r.generation,r.ok,now,r.error)) {
      if(r.service==0 && r.hasPlace){place=r.place;hasPlace=true;cityQuery="";locatePending=false;
        store->putFloat("lat",place.lat);store->putFloat("lon",place.lon);store->putString("city",place.name);}
      if(r.ok){if(r.service==0)weather=r.weather;else youtube=r.youtube;}
    }
  }
  if(!online || workerBusy || !jobs)return;
  for(int i=0;i<2;++i) {
    if(i==1 && youtubeKey.isEmpty())continue;
    if(!services[i].due(now))continue;
    Job j;j.service=i;j.generation=services[i].generation;j.hasPlace=hasPlace;j.place=place;j.locate=locatePending;
    snprintf(j.city,sizeof(j.city),"%s",cityQuery.c_str());snprintf(j.key,sizeof(j.key),"%s",youtubeKey.c_str());
    if(xQueueSend(jobs,&j,0)==pdTRUE){workerBusy=true;services[i].start(now);}memset(j.key,0,sizeof(j.key));break;
  }
}
struct PanelCanvas {
  void rect(int x,int y,int w,int h,uint32_t rgb){matrix->fillRect(x,y,w,h,matrix->color565(rgb>>16,(rgb>>8)&255,rgb&255));}
};
void liveSetBrightness(uint8_t value){requestedBrightness=value;}
void liveSurprise(){hold=false;if(mode==0)mode=1;giftSurprise(millis());}
bool liveGiftConfigure(JsonObjectConst request,String& error){
  if(!giftConfigure(request,error))return false;
  const char* action=request["action"] | "save";
  if(strcmp(action,"save") && strcmp(action,"dismiss")){hold=false;if(mode==0)mode=1;}
  if(!strcmp(action,"studio"))persist();
  return true;
}
void liveTick(bool diagnostic) {
  uint32_t now=millis();bool online=WiFi.status()==WL_CONNECTED;
  processNetwork(now,online);
  struct tm t;const bool known=getLocalTime(&t,0);giftTick(now,known?&t:nullptr,!hold&&mode!=0);
  const uint8_t adjusted=diagnostic?requestedBrightness:giftBrightness(requestedBrightness,now,known?&t:nullptr);
  if(matrix&&adjusted!=appliedBrightness){matrix->setBrightness8(adjusted);appliedBrightness=adjusted;}
  if(!matrix || hold || mode==0 || uint32_t(now-drawnAt)<50)return;
  drawnAt=now;
  if(giftRender(matrix,globalPalette,now))return;
  int showing=giftMode(now,youtubeKey.length());if(!showing)showing=mode;
  if(showing==4){giftRenderArt(matrix,globalPalette,now);return;}
  Display::Model m;m.mode=showing;const auto& a=appearance[showing-1];m.palette=LiveState::effective(globalPalette,a);m.layout=a.layout;m.motion=a.motion;m.tick=now-modeAt;
  if(known){m.hour=t.tm_hour;m.minute=t.tm_min;m.second=t.tm_sec;m.day=t.tm_mday;m.month=t.tm_mon+1;}
  if(showing>1){const auto& s=services[showing-2];m.valid=s.valid;m.state=s.state(online,showing==2 || youtubeKey.length(),now);m.stale=m.state=="stale";}
  m.temperature=int(lroundf(weather.temperature));m.code=weather.code;m.city=place.name;m.title=youtube.title;
  m.subs=youtube.hidden?"--":abbreviate(youtube.subs).c_str();m.views=abbreviate(youtube.views).c_str();m.videos=abbreviate(youtube.videos).c_str();
  PanelCanvas canvas;Display::Painter<PanelCanvas>(canvas).render(m);matrix->flipDMABuffer();
}
