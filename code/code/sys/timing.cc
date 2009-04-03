#include "timing.h"

double TTiming::to_secs(struct timeval tv){
  return tv.tv_sec + ((tv.tv_usec)/1000000.0);
}

void TTiming::start(){
  gettimeofday(&tv_start, &tz);
}

void TTiming::end(){
  gettimeofday(&tv_end, &tz);
}

double TTiming::getStart(){
  return to_secs(tv_start);
}

double TTiming::getEnd(){
  return to_secs(tv_end);
}

double TTiming::getElapsed(){
  struct timeval tmp;
  gettimeofday(&tmp, &tz);

  return (to_secs(tmp)-to_secs(tv_start));
}

double TTiming::getElapsedReset(){
  end();
  double t=(to_secs(tv_end)-to_secs(tv_start));
  start();
  return t;
}

TTiming::TTiming(){
  tv_start.tv_sec=tv_start.tv_usec=0;
  tv_end.tv_sec=tv_end.tv_usec=0;
}

