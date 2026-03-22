#pragma once

class TBeing;

// Chain entry point for backstab→stab combo. Runs the full stab
// (preconditions + attack) but skips move cost and skilllag since
// the calling skill owns those.
int stabChain(TBeing* thief, TBeing* victim);
