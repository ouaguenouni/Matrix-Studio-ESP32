#include "gift.h"
#include "GiftState.h"
#include "GiftDisplay.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <esp_system.h>

namespace {
Preferences* prefs=nullptr;
struct Config {
  String name="Inge",message="ADHD future millionaire",surprises="ONE MORE TRACK\nBUILD SOMETHING BEAUTIFUL\nYOUR IDEAS DESERVE TO EXIST";
  bool studio=false,nightEnabled=false;
  unsigned month=10,day=9,birthYear=1994,seconds=15,start=22*60,end=8*60,dim=8,focusMinutes=25;
} config;
Gift::Timer timer;
int overlay=-1; // -1 normal; 0/1 art; 2 birthday; 3 surprise; 4 focus; 5 finished
uint32_t sceneAt=0,studioAt=0,lastBirthday=0,wakeAt=0;
bool awakeOverride=false,nightActive=false;
int birthdayAge=32;
String selectedMessage;
void writeConfig(JsonObject o,const Config& c){
  o["name"]=c.name;o["message"]=c.message;o["surprises"]=c.surprises;o["studio"]=c.studio;
  o["birthYear"]=c.birthYear;o["month"]=c.month;o["day"]=c.day;o["seconds"]=c.seconds;o["nightEnabled"]=c.nightEnabled;
  o["nightStart"]=c.start;o["nightEnd"]=c.end;o["nightDim"]=c.dim;o["focusMinutes"]=c.focusMinutes;
}
void save(){if(!prefs)return;DynamicJsonDocument d(2048);writeConfig(d.to<JsonObject>(),config);String s;serializeJson(d,s);prefs->putString("giftCfg",s);}
bool parse(JsonObjectConst q,Config& c,String& error){
  auto number=[&](const char* key,unsigned& value,unsigned min,unsigned max){if(!q.containsKey(key))return true;if(!q[key].is<unsigned>()||q[key].as<unsigned>()<min||q[key].as<unsigned>()>max){error=String("Invalid ")+key+".";return false;}value=q[key].as<unsigned>();return true;};
  auto text=[&](const char* key,String& value,unsigned limit){if(!q.containsKey(key))return true;if(!q[key].is<const char*>()||strlen(q[key].as<const char*>())>limit){error=String("Invalid ")+key+".";return false;}value=q[key].as<const char*>();value.trim();return true;};
  if(!text("name",c.name,32)||!text("message",c.message,120)||!text("surprises",c.surprises,480))return false;
  if(c.name.isEmpty()||c.message.isEmpty()){error="Name and birthday message cannot be empty.";return false;}
  for(const char* key:{"studio","nightEnabled"})if(q.containsKey(key)&&!q[key].is<bool>()){error="Use true or false for switches.";return false;}
  if(q.containsKey("studio"))c.studio=q["studio"].as<bool>();if(q.containsKey("nightEnabled"))c.nightEnabled=q["nightEnabled"].as<bool>();
  if(!number("birthYear",c.birthYear,1900,2100)||!number("month",c.month,0,12)||!number("day",c.day,0,31)||!number("seconds",c.seconds,5,120)||!number("nightStart",c.start,0,1439)||!number("nightEnd",c.end,0,1439)||!number("nightDim",c.dim,0,30)||!number("focusMinutes",c.focusMinutes,1,180))return false;
  if(!Gift::validDate(c.month,c.day)){error="Choose a real birthday, or clear both day and month.";return false;}
  if(c.nightEnabled&&c.start==c.end){error="Night start and end must differ.";return false;}
  return true;
}
void beginScene(int scene,uint32_t now){overlay=scene;sceneAt=now;wakeAt=now;awakeOverride=true;}
uint32_t birthdayDuration(){return 19000U+std::max(11000,Display::width(Display::label(config.message.c_str()))*180+4000);}
uint32_t surpriseDuration(){return std::max(14000,Display::width(Display::label(selectedMessage.c_str()))*180+4000);}
}
void giftBegin(Preferences& settings){
  prefs=&settings;DynamicJsonDocument d(2048);String value=settings.getString("giftCfg","");String error;
  if(!deserializeJson(d,value)){Config restored;if(parse(d.as<JsonObjectConst>(),restored,error))config=restored;}
  lastBirthday=settings.getUInt("giftBirthday",0);studioAt=millis();
}
void giftManual(){
  if(config.studio){config.studio=false;save();}overlay=-1;timer.cancel();wakeAt=millis();awakeOverride=true;
}
void giftSurprise(uint32_t now){
  timer.cancel();String lines=config.surprises;unsigned count=1;for(unsigned i=0;i<lines.length();++i)if(lines[i]=='\n')++count;
  unsigned pick=esp_random()%count;int start=0;
  while(pick--){start=lines.indexOf('\n',start)+1;}int end=lines.indexOf('\n',start);
  selectedMessage=lines.substring(start,end<0?lines.length():end);selectedMessage.trim();if(selectedMessage.isEmpty())selectedMessage="HELLO "+config.name;
  beginScene(3,now);
}
bool giftConfigure(JsonObjectConst request,String& error){
  Config next=config;if(!parse(request,next,error))return false;
  const char* action=request["action"] | "save";
  const char* actions[]={"save","studio","art","surprise","birthday","focus_start","focus_pause","focus_resume","dismiss"};bool known=false;for(auto a:actions)if(!strcmp(a,action))known=true;
  if(!known){error="Unknown gift action.";return false;}
  if((!strcmp(action,"focus_pause")||!strcmp(action,"focus_resume"))&&!timer.running){error="No focus timer is running.";return false;}
  if(!strcmp(action,"studio"))next.studio=true;
  config=next;save();uint32_t now=millis();
  if(!strcmp(action,"save"))return true;
  if(!strcmp(action,"studio")){overlay=-1;timer.cancel();studioAt=now;}
  if(!strcmp(action,"art")){timer.cancel();beginScene(0,now);}
  if(!strcmp(action,"surprise"))giftSurprise(now);
  if(!strcmp(action,"birthday")){timer.cancel();beginScene(2,now);}
  if(!strcmp(action,"focus_start")){timer.start(now,config.focusMinutes);beginScene(4,now);}
  if(!strcmp(action,"focus_pause"))timer.pause(now);
  if(!strcmp(action,"focus_resume"))timer.resume(now);
  if(!strcmp(action,"dismiss")){overlay=-1;timer.cancel();}
  return true;
}
void giftTick(uint32_t now,const tm* local,bool canCelebrate){
  if(local)birthdayAge=std::max(0,local->tm_year+1900-int(config.birthYear));
  if(timer.tick(now))beginScene(5,now);
  uint32_t elapsed=now-sceneAt;
  if((overlay==2&&elapsed>=birthdayDuration())||(overlay==3&&elapsed>=surpriseDuration())||(overlay==5&&elapsed>=10000)){overlay=-1;studioAt=now;}
  if(!local||!canCelebrate||timer.running||overlay>=0||!config.month)return;
  uint32_t stamp=(local->tm_year+1900)*10000U+(local->tm_mon+1)*100+local->tm_mday;
  bool sleeping=config.nightEnabled&&Gift::night(local->tm_hour*60+local->tm_min,config.start,config.end);
  if(!sleeping&&unsigned(local->tm_mon+1)==config.month&&unsigned(local->tm_mday)==config.day&&stamp!=lastBirthday){lastBirthday=stamp;prefs->putUInt("giftBirthday",stamp);beginScene(2,now);}
}
int giftMode(uint32_t now,bool youtube){return config.studio?Gift::studioMode(now-studioAt,config.seconds,youtube):0;}
uint8_t giftBrightness(uint8_t requested,uint32_t now,const tm* local){
  if(awakeOverride&&uint32_t(now-wakeAt)>=60000)awakeOverride=false;
  nightActive=config.nightEnabled&&local&&Gift::night(local->tm_hour*60+local->tm_min,config.start,config.end);
  return nightActive&&!awakeOverride?std::min(unsigned(requested),config.dim):requested;
}
void giftStatus(JsonObject out,uint32_t now){
  writeConfig(out,config);out["age"]=birthdayAge;out["overlay"]=overlay;out["tick"]=uint32_t(now-sceneAt);out["studioTick"]=uint32_t(now-studioAt);out["nightActive"]=nightActive;
  out["selectedMessage"]=selectedMessage;auto t=out.createNestedObject("timer");t["running"]=timer.running;t["paused"]=timer.paused;t["leftMs"]=timer.left(now);t["totalMs"]=timer.total;
}
bool giftRender(MatrixPanel_I2S_DMA* panel,uint8_t palette,uint32_t now){
  int scene=overlay;
  if(scene<0)return false;
  Gift::Frame f;f.scene=scene==0?int((now-sceneAt)/12000)%2:scene;f.tick=now-sceneAt;f.palette=palette;f.age=birthdayAge;f.name=config.name.c_str();f.message=(scene==3?selectedMessage:config.message).c_str();f.left=timer.left(now);f.total=timer.total;f.paused=timer.paused;
  struct Canvas {MatrixPanel_I2S_DMA* panel;void rect(int x,int y,int w,int h,uint32_t rgb){panel->fillRect(x,y,w,h,panel->color565(rgb>>16,(rgb>>8)&255,rgb&255));}} c{panel};
  Gift::draw(c,f);panel->flipDMABuffer();return true;
}
// Studio architectural interludes use the same renderer without changing overlay state.
void giftRenderArt(MatrixPanel_I2S_DMA* panel,uint8_t palette,uint32_t now){
  Gift::Frame f;f.scene=int((now-studioAt)/(config.seconds*1000U)/2)%2;f.tick=now-studioAt;f.palette=palette;f.age=birthdayAge;f.name=config.name.c_str();
  struct Canvas {MatrixPanel_I2S_DMA* panel;void rect(int x,int y,int w,int h,uint32_t rgb){panel->fillRect(x,y,w,h,panel->color565(rgb>>16,(rgb>>8)&255,rgb&255));}} c{panel};
  Gift::draw(c,f);panel->flipDMABuffer();
}
