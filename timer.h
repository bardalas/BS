#ifndef TIMER
#define TIMER

struct Timer{
  unsigned long target_s;
  unsigned long current_s;
  unsigned long last_s;
};

//Timer initialize
//timer:a pointer to Timer struct instance
//target_s: target time for trigger.
void initTimer( struct Timer* timer, unsigned long target_s){ //initialize Timer instance so we need -> pointer notation
  timer->target_s=target_s; //
  timer->current_s=0;
  timer->last_s=0;
}

//Update the called timer structure
//Proggress the ms counter
//Return false if not reached the trigger limit. If it did then reset the counter and return true
boolean updateTimer(struct Timer* timer){
  timer->current_s=getSysTime();
  //TODO: build this with micros();
  //TODO: seek and use AVR hardware timer interupts
  if (timer->current_s-timer->last_s>=timer->target_s){
    timer->last_s=timer->current_s;
    return true;
  }
  return false;
}
#endif
