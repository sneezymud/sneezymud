#pragma once

#include "sstring.h"

const int NUM_LOTTERY_PRIZES = 10;

const int OBJ_LOTTERY_TICKET = 2372;

struct LotteryPrizes {
    const sstring name;
    int vnum;
    int odds;
};

extern LotteryPrizes prizes[NUM_LOTTERY_PRIZES];
