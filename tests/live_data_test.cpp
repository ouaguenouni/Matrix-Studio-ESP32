#include "LiveData.h"
#include "LiveState.h"
#include <cassert>
#include <iostream>
int main(){
  DynamicJsonDocument doc(16384);
  auto parse=[&](const char* text){assert(!deserializeJson(doc,text));return doc.as<JsonVariantConst>();};
  LiveData::Weather w;
  assert(LiveData::weather(parse(R"({"current_units":{"temperature_2m":"°C","weather_code":"wmo code"},"current":{"temperature_2m":18.6,"weather_code":3}})"),w));
  assert(w.temperature>18 && w.code==3);
  assert(LiveData::weather(parse(R"({"current":{"temperature_2m":0,"weather_code":0}})"),w));assert(w.temperature==0);
  assert(LiveData::weather(parse(R"({"current":{"temperature_2m":-12.5,"weather_code":71}})"),w));
  assert(!LiveData::weather(parse(R"({"current":{"temperature_2m":"°C","weather_code":0}})"),w));
  assert(!LiveData::weather(parse(R"({"current":{"temperature_2m":null,"weather_code":0}})"),w));
  assert(!LiveData::weather(parse(R"({"current":{"temperature_2m":1,"weather_code":4}})"),w));
  assert(!LiveData::weather(parse(R"({"current":{"temperature_2m":1}})"),w));
  LiveData::Youtube y;
  assert(LiveData::youtube(parse(R"({
    "items": [{"snippet": {"title": "Cosmic Hippo Sounds"},
    "statistics": {"subscriberCount": "1234", "viewCount": "9876543210", "videoCount": "0", "hiddenSubscriberCount": false}}]
  })"),y));assert(y.subs==1234 && y.views==9876543210ULL && y.videos==0);
  assert(LiveData::youtube(parse(R"({"items":[{"snippet":{"title":"X"},"statistics":{"hiddenSubscriberCount":true,"viewCount":"0","videoCount":"1"}}]})"),y));assert(y.hidden);
  assert(!LiveData::youtube(parse(R"({"items":[]})"),y));
  assert(!LiveData::youtube(parse(R"({"items":[{"snippet":{"title":"X"}}]})"),y));
  assert(!LiveData::youtube(parse(R"({"items":[{"snippet":{"title":"X"},"statistics":{"subscriberCount":"oops","viewCount":"0","videoCount":"0"}}]})"),y));
  assert(!LiveData::youtube(parse(R"({"items":[{"snippet":{"title":"X"},"statistics":{"subscriberCount":"0","viewCount":"-1","videoCount":"0"}}]})"),y));
  assert(!LiveData::youtube(parse(R"({"items":[{"snippet":{"title":"X"},"statistics":{"subscriberCount":"0","viewCount":"18446744073709551616","videoCount":"0"}}]})"),y));
  assert(std::string(LiveData::apiError(403,parse(R"({"error":{"errors":[{"reason":"quotaExceeded"}]}})"),true)).find("Quota")!=std::string::npos);
  assert(std::string(LiveData::apiError(403,parse("{}"),true)).find("key")!=std::string::npos);
  assert(std::string(LiveData::apiError(400,parse(R"({"error":{"errors":[{"reason":"invalidParameter"}]}})"),true)).find("parameters")!=std::string::npos);
  LiveData::Place p;
  assert(LiveData::place(parse(R"({"results":[{"name":"Paris","latitude":48.8,"longitude":2.3}]})"),false,p));
  assert(!LiveData::place(parse(R"({"results":[]})"),false,p));
  assert(LiveData::place(parse(R"({"status":"success","city":"Paris","lat":48.8,"lon":2.3})"),true,p));
  assert(!LiveData::place(parse(R"({"status":"success","city":"Paris","lat":148.8,"lon":2.3})"),true,p));
  using namespace LiveState;
  Appearance a,b;a.palette=4;assert(effective(2,a)==4 && effective(2,b)==2);assert(effective(5,a)==4 && effective(5,b)==5);
  assert(migrateTheme(0)==0 && migrateTheme(1)==1 && migrateTheme(2)==2 && migrateTheme(9)==2);
  assert(restoredMode(true,0)==0 && restoredMode(true,3)==3 && restoredMode(false,0)==1 && restoredMode(true,99)==1);
  Service s;assert(s.due(0));s.start(0);assert(!s.due(5000));
  s.invalidate();assert(!s.finish(0,true,10,""));assert(!s.valid && s.due(10));
  s.start(20);assert(s.finish(1,true,30,""));assert(s.valid && !s.due(31));
  assert(std::string(s.state(true,true,31))=="ready");assert(std::string(s.state(false,true,31))=="stale");
  assert(s.due(600030));s.start(600030);s.finish(1,false,600050,"failed");assert(!s.due(660049));assert(s.due(660050));
  s.invalidate();assert(s.due(600100));
  Service wrap;wrap.start(0xfffffff0);wrap.finish(0,true,0xfffffff0,"");assert(!wrap.due(16));assert(wrap.due(600000));
  std::cout<<"Firmware parsing, inheritance, migration, scheduling, reconnect invalidation and stale-result tests passed.\n";
}
