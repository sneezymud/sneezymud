#pragma once

class TBeing;

// Chain entry point for backstab→stab combo. Runs the full stab
// (preconditions + attack) without charging the stab move cost — the
// chained stab is a follow-on to backstab, not an independent action.
int stabChain(TBeing* thief, TBeing* victim);
