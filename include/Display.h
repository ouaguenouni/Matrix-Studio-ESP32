#pragma once
#include "DisplayData.h"
#include <string>
#include <cstdio>
#include <cstring>
#include <algorithm>
namespace Display {
struct Model {
  int mode = 1, palette = 2, layout = 0, motion = 1;
  uint32_t tick = 0;
  int hour = -1, minute = 0, second = 0, day = 1, month = 1;
  bool valid = false, stale = false;
  int temperature = 0, code = 0;
  std::string city, title, subs = "--", views = "--", videos = "--", state = "loading";
};
inline const char* kind(int code) {
  if (code == 0) return "SUN";
  if (code <= 3) return "CLOUD";
  if (code == 45 || code == 48) return "FOG";
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return "RAIN";
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "SNOW";
  return "STORM";
}
inline const Glyph& glyph(unsigned code) {
  for (const auto& g : font) if (g.code == code) return g;
  for (const auto& g : font) if (g.code == '?') return g;
  return font[0];
}
// Pixel labels use ASCII, transliterating common Latin accents from UTF-8.
inline std::string label(const std::string& input) {
  std::string out;
  for (size_t i = 0; i < input.size(); ++i) {
    unsigned char c = input[i];
    if (c < 128) { out += char(c >= 'a' && c <= 'z' ? c - 32 : c); continue; }
    unsigned cp = 0; unsigned count = 0;
    if ((c & 0xe0) == 0xc0) { cp=c&31; count=1; }
    else if ((c&0xf0)==0xe0) {cp=c&15;count=2;}
    else if ((c&0xf8)==0xf0) {cp=c&7;count=3;}
    for (unsigned j=0;j<count && i+1<input.size();++j) cp=(cp<<6)|(input[++i]&63);
    if (cp>=0xe0 && cp<=0xfe) cp-=32;
    if (cp>=0xc0 && cp<=0xc5) out+='A';
    else if(cp==0xc7) out+='C';
    else if(cp>=0xc8 && cp<=0xcb) out+='E';
    else if(cp>=0xcc && cp<=0xcf) out+='I';
    else if(cp==0xd1) out+='N';
    else if(cp>=0xd2 && cp<=0xd6) out+='O';
    else if(cp>=0xd9 && cp<=0xdc) out+='U';
    else if(cp==0xdd) out+='Y';
    else out+='?';
  }
  return out;
}
inline int width(const std::string& text, int scale=1) {
  int w=0; for (unsigned char c : text) w+=(glyph(c).width+1)*scale;
  return text.empty()?0:w-scale;
}
template<class Canvas> class Painter {
  Canvas& c;
public:
  explicit Painter(Canvas& canvas):c(canvas){}
  void rect(int x,int y,int w,int h,uint32_t colour) { c.rect(x,y,w,h,colour); }
  void text(const std::string& t,int x,int y,int scale,uint32_t colour) {
    for (unsigned char ch : t) {
      const auto& g=glyph(ch);
      for(int row=0;row<5;++row) for(int col=0;col<g.width;++col)
        if(g.rows[row] & (1<<(g.width-1-col))) rect(x+col*scale,y+row*scale,scale,scale,colour);
      x+=(g.width+1)*scale;
    }
  }
  void centred(const std::string& t,int y,int scale,uint32_t colour) { text(t,(64-width(t,scale))/2,y,scale,colour); }
  void ticker(const std::string& input,int y,uint32_t colour,const Model& m) {
    const std::string t=label(input); const int w=width(t);
    if(w<=60) {centred(t,y,1,colour); return;}
    if(m.motion==0) {text(t.substr(0,15),2,y,1,colour);return;}
    // Hold at the start, then scroll pixel-by-pixel, followed by a short gap.
    int offset=int(m.tick/(m.motion==2?110:180))%(w+20+12)-12;
    offset=std::max(0,offset);
    text(t,2-offset,y,1,colour);text(t,2-offset+w+20,y,1,colour);
  }
  void icon(const char* name,int x,int y,int motion,uint32_t tick,uint32_t fg,uint32_t accent) {
    const int p=motion?int(tick/(motion==2?150:300))%8:0;
    if(!strcmp(name,"SUN")) {
      rect(x+5,y+5,7,7,accent);rect(x+7,y+1,3,2,accent);rect(x+7,y+14,3,2,accent);
      rect(x+1,y+7,2,3,accent);rect(x+14,y+7,2,3,accent);
      if(motion==2 && p%2) {rect(x+2,y+2,2,2,accent);rect(x+13,y+13,2,2,accent);} return;
    }
    if(!strcmp(name,"FOG")) {for(int r=0;r<3;++r)rect(x+2+(p+r)%2,y+4+r*4,12,1,fg);return;}
    rect(x+5,y+3,7,5,fg);rect(x+2,y+6,14,5,fg);
    if(!strcmp(name,"RAIN")||!strcmp(name,"STORM")) {
      for(int r=0;r<3;++r)rect(x+3+r*5,y+12+(p+r*2)%4,1,2,accent);
      if(!strcmp(name,"STORM")){rect(x+9,y+10,2,4,accent);rect(x+7,y+13,3,2,accent);}
    } else if(!strcmp(name,"SNOW")) {for(int r=0;r<3;++r)rect(x+3+r*5,y+12+(p+r)%4,2,2,accent);}
  }
  void render(const Model& m) {
    const auto* palette=palettes[m.palette];const uint32_t fg=palette[1],accent=palette[2];
    rect(0,0,64,32,palette[0]);
    if(m.mode==1) {
      char time[8]="--:--";
      if(m.hour>=0)snprintf(time,sizeof(time),"%02d:%02d",m.hour,m.minute);
      const int y=m.layout?3:8;centred(time,y,3,fg);
      if(m.hour>=0 && m.motion && m.second%2)rect((64-width(time,3))/2+24,y,3,15,palette[0]);
      if(m.layout) {char date[8];snprintf(date,sizeof(date),"%02d/%02d",m.day,m.month);centred(m.hour<0?"SYNC":date,25,1,accent);}
      else {rect(2,28,60,1,0x18202a);if(m.hour>=0)rect(2,28,(m.second+1),1,accent);}
      if(m.motion==2 && m.hour>=0)rect((m.tick/120)%64,0,1,1,accent);
      return;
    }
    if(!m.valid) {
      centred(m.mode==2?"WEATHER":"YOUTUBE",3,1,accent);
      centred("--",11,2,fg);
      const char* status=m.state=="unconfigured"?"ADD KEY":m.state=="error"?"CHECK WEB":"LOADING";
      centred(status,25,1,accent);return;
    }
    if(m.mode==2) {
      char temp[8];snprintf(temp,sizeof(temp),"%d",m.temperature);
      if(!m.layout) {
        icon(kind(m.code),1,2,m.motion,m.tick,fg,accent);
        int scale=width(temp,3)<=35?3:2;const int w=width(temp,scale);
        text(temp,22+(36-w)/2,3,scale,fg);rect(60,3,2,2,accent);
      } else {
        centred(temp,1,3,fg);centred(kind(m.code),19,1,accent);
      }
      ticker(m.city,26,fg,m);
    } else {
      const int slide=m.layout?0:int(m.tick/5000)%3;
      const std::string value=slide==0?m.subs:slide==1?m.views:m.videos;
      const char* names[]={"SUBSCRIBERS","VIEWS","VIDEOS"};
      centred(names[slide],1,1,accent);
      const int scale=width(value,3)<=60?3:2;
      centred(value,10,scale,fg);ticker(m.title,27,accent,m);
      if(!m.layout)for(int i=0;i<3;++i)rect(26+i*5,7,3,1,i==slide?accent:0x18202a);
    }
    if(m.stale)rect(62,0,2,2,0xffb040);
  }
};
}
