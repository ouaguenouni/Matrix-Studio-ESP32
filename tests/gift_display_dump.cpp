#include "GiftDisplay.h"
#include <iostream>
struct Canvas {
  uint32_t pixels[2048]={};
  void rect(int x,int y,int w,int h,uint32_t rgb){for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx)if(x+dx>=0&&x+dx<64&&y+dy>=0&&y+dy<32)pixels[(y+dy)*64+x+dx]=rgb;}
};
int main(){
 for(int scene=0;scene<6;++scene)for(int palette=0;palette<6;++palette)for(int motion=0;motion<2;++motion)for(int scenario=0;scenario<5;++scenario){
  Gift::Frame f;f.scene=scene;f.palette=palette;f.motion=motion;f.tick=scenario*7000;f.age=32;f.name="Inge";f.message="ADHD future millionaire";f.left=scenario*600000;f.total=3000000;f.paused=scenario==1;
  Canvas c;Gift::draw(c,f);uint32_t hash=2166136261u;for(auto rgb:c.pixels){hash^=rgb;hash*=16777619u;}
  std::cout<<scene<<","<<palette<<","<<motion<<","<<scenario<<":"<<hash<<"\n";
 }
}
