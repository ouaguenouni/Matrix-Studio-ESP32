#pragma once
#include <cstdint>
namespace Gift {
inline bool validDate(unsigned month,unsigned day) {
  static const unsigned days[]={0,31,29,31,30,31,30,31,31,30,31,30,31};
  return (month==0 && day==0) || (month>=1 && month<=12 && day>=1 && day<=days[month]);
}
inline bool night(unsigned minute,unsigned start,unsigned end) {
  return start<end ? minute>=start && minute<end : start>end && (minute>=start || minute<end);
}
struct Timer {
  bool running=false,paused=false,complete=false;
  uint32_t started=0,remaining=0,total=0;
  void start(uint32_t now,unsigned minutes){running=true;paused=false;complete=false;started=now;remaining=total=minutes*60000U;}
  uint32_t left(uint32_t now) const {return !running || paused?remaining:uint32_t(now-started)>=remaining?0:remaining-uint32_t(now-started);}
  void pause(uint32_t now){if(running&&!paused){remaining=left(now);paused=true;}}
  void resume(uint32_t now){if(running&&paused){started=now;paused=false;}}
  void cancel(){running=paused=complete=false;remaining=0;}
  bool tick(uint32_t now){if(running&&!paused&&left(now)==0){running=false;complete=true;remaining=0;return true;}return false;}
};
// Debounced short press on release; one long event per press, never both.
struct Button {
  bool raw=false,stable=false,longSent=false;uint32_t changed=0,down=0;
  int update(bool pressed,uint32_t now){
    if(raw!=pressed){raw=pressed;changed=now;}
    if(raw!=stable && uint32_t(now-changed)>=40){stable=raw;
      if(stable){down=now;longSent=false;}
      else {if(!longSent)return 1;}
    }
    if(stable&&raw&&!longSent&&uint32_t(now-down)>=900){longSent=true;return 2;}
    return 0;
  }
};
inline int studioMode(uint32_t elapsed,unsigned seconds,bool youtube){
  const int steps[]={1,4,2,4,3,4};int i=(elapsed/(seconds*1000U))%(youtube?6:4);return steps[i];
}
}
