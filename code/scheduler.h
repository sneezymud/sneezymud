#ifndef __SCHEDULER_H
#define __SCHEDULER_H


class TScheduler {


  add(const TScheduleJob &);
};


class TScheduleJob {
  int pulse;
  loopTypeT loop_t;
};


class jobRoomSpec : public TScheduleJob {

  void exec(){ call_room_spec(); }

  jobRoomSpec(loopTypeT, int);

};


TScheduler scheduler;

scheduler.add(jobRoomSpec(TARG_LOOP, 1));

scheduler.add(jobBurning(TARG_OBJS, PULSE_SPEC_PROCS));

#endif
