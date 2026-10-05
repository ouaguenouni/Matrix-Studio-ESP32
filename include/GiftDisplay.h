#pragma once
#include "Display.h"
namespace Gift {
struct Frame {
  int scene=0; // 0 skyline, 1 wireframe, 2 birthday, 3 message, 4 focus, 5 complete
  int palette=2, age=32;
  uint32_t tick=0,left=0,total=1;
  bool paused=false,motion=true;
  std::string name="Inge",message="MAKE SOMETHING BEAUTIFUL";
};
template<class Canvas> void draw(Canvas& c,const Frame& m) {
  Display::Painter<Canvas> p(c);const auto* palette=Display::palettes[m.palette];
  const auto fg=palette[1],accent=palette[2];p.rect(0,0,64,32,palette[0]);
  Display::Model ticker;ticker.tick=m.tick;ticker.motion=m.motion?1:0;
  const uint32_t tick=m.motion?m.tick:0;
  if(m.scene==0){
    const int heights[]={9,15,11,20,13,17,10,14};
    for(int i=0;i<8;++i){int x=i*8,h=heights[i];p.rect(x,25-h,6,1,accent);p.rect(x,25-h,1,h,accent);p.rect(x+5,25-h,1,h,accent);
      for(int row=0;row<(h-3)/4;++row)if(((i+row+int(tick/1300))%4)!=0)p.rect(x+2,28-h+row*4,2,1,fg);}
    p.rect(0,25,64,1,accent);p.ticker(m.name+" / AFTER HOURS",27,fg,ticker);
  } else if(m.scene==1){
    // A perspective grid, animated as a slow architectural scan.
    for(int i=0;i<6;++i){int y=8+i*4;int inset=12-i*2;p.rect(inset,y,64-inset*2,1,accent);}
    for(int i=0;i<7;++i)for(int y=8;y<28;++y){int x=32+((i-3)*(y+4))/3;if(x>=0&&x<64)p.rect(x,y,1,1,accent);}
    int scan=8+int(tick/350)%20;p.rect(0,scan,64,1,fg);p.centred("AFTER HOURS",1,1,fg);
  } else if(m.scene==2){
    if(m.tick<6500){
      p.centred("FOR "+Display::label(m.name),0,1,fg);
      p.rect(19,20,26,8,accent);p.rect(17,27,30,2,fg);p.rect(19,19,26,2,fg);
      for(int i=0;i<3;++i){int x=24+i*7;p.rect(x,13,1,6,fg);p.rect(x,10-int((tick/250+i)%2),1,2,accent);}
      for(int i=0;i<6;++i)p.rect((i*13+3)%64,7+int((tick/240+i*3)%18),1,1,accent);
    } else if(m.tick<13000){p.centred("HAPPY BIRTHDAY",2,1,accent);p.centred(std::to_string(m.age),12,3,fg);}
    else if(m.tick<19000){p.ticker(m.name,6,fg,ticker);p.centred("MAKE SOME NOISE",22,1,accent);}
    else {p.centred("FOR YOU",4,1,accent);ticker.tick=m.tick-19000;p.ticker(m.message,15,fg,ticker);p.rect(28,27,8,1,accent);}
  } else if(m.scene==3){
    p.centred("HEY "+Display::label(m.name),2,1,accent);p.ticker(m.message,14,fg,ticker);
    const int x=5+int(tick/160)%54;p.rect(x,27,3,1,accent);
  } else if(m.scene==4){
    p.centred(m.paused?"PAUSED":"IN THE FLOW",1,1,accent);
    char time[12];unsigned seconds=(m.left+999)/1000;snprintf(time,sizeof(time),"%02u:%02u",seconds/60,seconds%60);
    p.centred(time,10,Display::width(time,3)<=60?3:2,fg);p.rect(2,29,60,1,0x18202a);
    p.rect(2,29,m.total?int((uint64_t(m.left)*60)/m.total):0,1,accent);
  } else {p.centred("NICE WORK",5,1,accent);p.centred("BREATHE",16,2,fg);}
}
}
