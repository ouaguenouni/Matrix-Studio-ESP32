#include "GiftState.h"
#include <cassert>
#include <iostream>
int main(){
  using namespace Gift;
  assert(validDate(10,9));assert(validDate(2,29));assert(validDate(0,0));assert(!validDate(2,30));assert(!validDate(0,9));assert(!validDate(13,1));
  assert(night(23*60,22*60,8*60));assert(night(7*60,22*60,8*60));assert(!night(8*60,22*60,8*60));assert(!night(12*60,22*60,8*60));
  assert(night(13*60,12*60,14*60));assert(!night(15*60,12*60,14*60));assert(!night(10,0,0));
  Timer t;t.start(1000,25);assert(t.left(61000)==1440000);t.pause(61000);assert(t.left(80000)==1440000);t.resume(80000);assert(t.left(81000)==1439000);assert(t.tick(1520000));assert(!t.tick(1520001));
  t.start(0xfffffff0,1);assert(t.left(16)==59968);t.cancel();assert(!t.running && t.left(40)==0);
  Button b;assert(!b.update(true,0));assert(!b.update(true,50));assert(!b.update(false,100));assert(b.update(false,145)==1);assert(!b.update(false,200));
  assert(!b.update(true,300));assert(!b.update(true,350));assert(b.update(true,1250)==2);assert(!b.update(true,1300));assert(!b.update(false,1400));assert(!b.update(false,1450));
  assert(studioMode(0,15,true)==1);assert(studioMode(15000,15,true)==4);assert(studioMode(30000,15,true)==2);assert(studioMode(60000,15,true)==3);assert(studioMode(60000,15,false)==1);
  std::cout<<"Gift scheduling, timer pause/resume/rollover, night windows, rotation and short/long BOOT tests passed.\n";
}
