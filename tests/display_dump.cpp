#include "Display.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <cstring>
struct Canvas {
  uint32_t pixels[2048]={};
  void rect(int x,int y,int w,int h,uint32_t colour){for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx)if(x+dx>=0&&x+dx<64&&y+dy>=0&&y+dy<32)pixels[(y+dy)*64+x+dx]=colour;}
};
int main(){
  for(int mode=1;mode<=3;++mode)for(int layout=0;layout<2;++layout)for(int motion=0;motion<3;++motion)for(int palette=0;palette<6;++palette)for(int scenario=0;scenario<9;++scenario){
    Display::Model m;m.mode=mode;m.layout=layout;m.motion=motion;m.palette=palette;m.tick=12570;
    m.hour=scenario==8?-1:9;m.minute=5;m.second=31;m.day=4;m.month=10;
    m.valid=scenario<6;m.stale=scenario==5;m.state=scenario==6?"loading":scenario==7?"error":"unconfigured";
    m.temperature=scenario==0?-99:scenario==1?0:24;m.code=scenario==0?71:scenario==1?0:scenario==2?45:scenario==3?61:scenario==4?95:3;
    m.city="Saint-Étienne";m.title="Cosmic Hippo Sounds";m.subs=scenario==0?"--":"12.3K";m.views="123.4M";m.videos="0";
    Canvas c;Display::Painter<Canvas>(c).render(m);
    uint32_t hash=2166136261u;for(auto rgb:c.pixels){hash^=rgb;hash*=16777619u;}
    std::cout<<mode<<","<<layout<<","<<motion<<","<<palette<<","<<scenario<<":"<<hash<<"\n";
  }
}
