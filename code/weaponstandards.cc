// last weapon - sword, cutlass



// weap_name, damage, speed, size (1-10), length (inches), weapongroup, attacktypes, uses

// NOTE ON DAMAGE AND SPEED: damage*speed should be no greater than 10000, and should only
//  come to 10000 'perfected' combat weapons. everything else should be less.
// NOTE ON WEAPON SIZE: basically average sized onehanded weapons (short swords, hand axes)
//   are roughly 4, longer onehanders are 5 and 6, smaller would be 3, daggers would be 2,
//   and really small stuff like darts and such would be 1. smaller two handers such as
//   will be 7, 'normal' twohanders will be 8, stuff like greatswords and claymores would be 9.
//   really fucking big stuff will be 10 reserve this stuff for like, hand held balistas and
//   the like.
// basically here is why: i want to eventually do a size/strength restriction on weapons,
//  so a two hander for a hobbit would be one hander for human, etc.
//     yet another breakdown cause i want to get my point across.
//     1 - teeny tiny stuff         (needles, darts, throwing stars)
//     2 - small weapons            (daggers, knives, spoons)
//     3 - smallish/average weapons (longer daggers, short swords)
//     4 - largish/average weapons  (hand axes, long swords)
//     5 - large weapons            (broad swords, warhammer)
//     6 - 1h/2h edge (for humans)  (bastard swords, large clubs)
//     7 - normal 2handed weapons   (quarterstaffs, generic 2h sword)
//     8 - very large 2h weapons    (claymore, greatsword, spear)
//     9 - huge weapons             (polearms, boulders)
//    10 - disgustingly large       (hand held balistas, small cars, etc)
//
// length is obvious, just the reach of the weapon (for bows/thrown, this is still melee reach)
// attack type is an array of 4 spellNumT attack types, which will be randomly chosen
// uses is a bitvector that determines how the weapon can be used, see weaponstandards.h for
// a breakdown.

const struct weapon_type_nums[200] =
{


  // two handed



  // long blades



  // short blades



  // staves



  // axes
  {"adze", 80, 80, 4, 48, WCLASS_AXES, {TYPE_SLASH, TYPE_SLASH, TYPE_CLEAVE, TYPE_CLEAVE},
   USE_NONE},
  {"battle axe", 125, 80, 6, WCLASS_AXES, {TYPE_SLASH, TYPE_SLICE, TYPE_CLEAVE, TYPE_CLEAVE},
   USE_NONE},
  {"throwing axe", 125, 80, 4, WCLASS_AXES, {TYPE_SLASH, TYPE_SLICE, TYPE_CLEAVE, TYPE_CLEAVE},
   USE_THROWING},



  // polearms



  // hammers



  // clubs



  // bows



  // crossbows



  // spears



  // lances



  // nets



  // oddball
}

