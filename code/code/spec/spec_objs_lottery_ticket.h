#ifndef __SPEC_OBJS_LOTTERY_TICKET_H
#define __SPEC_OBJS_LOTTERY_TICKET_H

const int NUM_LOTTERY_PRIZES=10;

const int OBJ_LOTTERY_TICKET = 2372;

struct LotteryPrizes {
  const sstring name;
  int vnum;
  int odds;
};

extern LotteryPrizes prizes[NUM_LOTTERY_PRIZES];

#endif
