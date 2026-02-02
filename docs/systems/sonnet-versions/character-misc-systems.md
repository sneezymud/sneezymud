---
title: Character Miscellaneous Systems
category: understanding
keywords: [alignment, factionData, reconcileHurt, reconcileHelp, pets, charm, thrall, followers, isPet, doOrder, tooManyFollowers, orphan pets, language, garble, SKILL_COMMON, getGarbles, garbleFunction, getLanguageChance, racial speech]
related: [affects-system.md, character-foundation.md, faction-system.md, monster-ai-behavior.md, spell-skill-framework.md]
primary_symbols:
  functions: [reconcileHurt, reconcileHelp, isPet, doOrder, tooManyFollowers, garble, getGarbles, getLanguageChance, petSave, doRetrainPet, restorePetToPc]
  classes: [factionData, affectedData, GarbleData]
  files: [code/code/misc/faction.cc, code/code/misc/pets.cc, code/code/misc/garble.cc, code/code/misc/offense.cc, code/code/misc/utility.cc, code/code/disc/disc_mage_spirit.cc]
---

# Character Miscellaneous Systems

## Overview

Three interconnected character subsystems govern moral positioning, follower relationships, and inter-species communication. The alignment system tracks moral and ethical standing on two independent axes, influencing deity relations and equipment compatibility. The pet and charm system manages followers created through spells, purchases, or special mechanics, enforcing ownership rules and command restrictions. The language system models speech comprehension between races through garbling transformations based on skill proficiency and racial phonetics.

All three systems share a common pattern of graduated effects rather than binary states. Alignment shifts occur incrementally through repeated actions. Charm types exist on a spectrum from full autonomy to absolute obedience. Language comprehension combines multiple skill checks and statistical modifiers to produce probabilistic garbling. These systems operate primarily through affect flags, numeric tracking in the factionData structure, and transformation functions applied to outgoing speech.

## Patterns

### Alignment Tracking and Modification

Alignment uses a dual-axis system stored in the factionData structure attached to each being. The align_ge field tracks good versus evil positioning, while align_lc tracks lawful versus chaotic. Both axes use the same -1000 to +1000 scale, divided into three bands. Values from -1000 to -350 represent evil or chaotic positioning, -349 to 349 represent neutral, and 350 to 1000 represent good or lawful.

Alignment shifts occur through reconcileHelp and reconcileHurt functions. These accept a target being and an amplitude modifier that determines shift magnitude. Hurting is implemented as negative helping, creating symmetry in the shift calculation logic. Combat damage triggers small shifts with amplitude 0.005, while killing blows trigger larger shifts at 0.03. Spell casting uses discipline-specific alignment modifiers defined in each spell's spellInfo structure.

The shift direction depends on both the action and the target's current alignment. Hurting good beings shifts the actor toward evil, while hurting evil beings shifts toward good. This creates natural alignment drift based on behavior patterns. Repeated combat against good mobs gradually shifts a character evil, while hunting evil creatures shifts them good.

Alignment restrictions gate access to class features, deity favors, and equipment. Deikhan classes require specific alignment bands. Alignment deities reward adherence and punish deviation based on faction percentage calculations. Items can have ITEM_ANTI_GOOD or ITEM_ANTI_EVIL flags preventing use by aligned characters. Some spells only affect targets of specific alignments.

### Charm and Follower Control

Three distinct follower types share the AFF_CHARM flag but differ in obedience and self-preservation. Thralls created by animate dead or golem construction have no self-preservation and obey all orders unconditionally. Charms created by ensorcer spell have limited self-preservation and high obedience, though they may resist obviously suicidal commands. Pets purchased from shops or tamed by rangers have full self-preservation and only obey safe commands.

Priority rules govern stacked charm effects. When multiple charm-type affects exist on a single mob, thrall status overrides all others, charm overrides pet, and pet serves as the base state. The isPet function checks these in priority order, first testing for AFFECT_THRALL, then AFFECT_CHARM, then AFFECT_PET.

Follower limits scale with level and charisma. The maximum follower count equals level plus charisma adjustment divided by twenty. Different follower types consume different amounts of this budget. Zombies consume one plus level divided by ten, charms consume two plus level divided by ten, and pets consume one plus level divided by seven. This creates natural specialization where high-level casters can maintain one powerful follower or several weak ones.

The order command enables follower control but enforces multiple restrictions. Charmed beings cannot give orders themselves to prevent chain control scenarios. Commands fail in ROOM_NO_ORDER flagged rooms. Pets refuse orders filtered by orderDenyCheck, which blocks combat and suicidal commands. Mounts have ego checks that may cause bucking when ordered.

### Orphan Pet Lifecycle

When a master logs out or dies, their pets transition to orphaned status. The charm affect is replaced with AFFECT_ORPHAN_PET having duration of eighty mud hours. During this window, the original owner can reclaim the pet by logging back in and reestablishing the follower relationship. Rangers can also retrain orphaned pets using the retrain command, either for themselves or for others.

Retraining has a twenty percent failure chance representing the pet's resistance to new ownership. Failure causes the AFFECT_ORPHAN_PET to be removed, leaving the mob fully wild. Success transfers ownership by creating new charm affects pointing to the new master. Named pets with ACT_STRINGS_CHANGED flag are saved to the database with their owner ID, experience, level, and customized strings.

### Language Comprehension and Garbling

Speech between beings of different races undergoes garbling transformations based on language skills and racial phonetics. Each race has associated language skills like SKILL_FISHBURBLE or SKILL_GNOLL_JARGON. When a speaker's race garble differs from the listener's, and the listener has intelligence below 180, racial garbles apply.

The garble function processes speech through multiple transformation passes. The getGarbles function first determines which garble types should apply based on race differences, condition states like drunkenness or being underwater, and player preferences like PG13 mode. Each active garble is applied in sequence if it matches the current speech type and scope.

Language comprehension combines speaker and listener skills through getLanguageChance. The listener's language skill provides the base understanding, reduced to ninety percent if they pass the skill check. Perception stat adds zero to sixteen bonus points for accent comprehension. The speaker's SKILL_COMMON provides eighty percent of its value as a clarity bonus. Intelligence modifies the final chance from -10 to +10. The result represents the percentage chance of garbling each word, where zero indicates perfect understanding and one hundred indicates complete incomprehension.

Races without native garbles automatically receive maximum SKILL_COMMON during character creation. Races with garbles must learn Common through practice to communicate clearly with others. High intelligence at 180 or above completely bypasses racial language barriers. Immortals are exempt from all garbling when receiving speech.

### Charm Spell Mechanics

The ensorcer charm spell has multiple failure conditions checked in sequence. The spell automatically fails if the victim has charm immunity, if the victim's level exceeds the caster's level, if the mob already hates the caster, if the power differential is too high, or if the victim makes a luck save. Failure causes the mob to become hostile toward the caster.

Duration scales with caster level at three times level times updates per mud hour, modified by the caster's duration modification for the spell. This base duration is then reduced by the victim's charm immunity percentage. Critical success doubles or triples duration, while luck saves halve it. The affect stores the caster's name in the be field for owner tracking.

## Reference

### Alignment Ranges and Categories

The good-evil axis divides into three bands. Evil ranges from -1000 to -350, neutral from -349 to 349, and good from 350 to 1000. The lawful-chaotic axis uses identical thresholds with chaotic from -1000 to -350, neutral from -349 to 349, and lawful from 350 to 1000.

### Alignment Shift Triggers and Magnitudes

Standard combat damage uses amplitude 0.005. Killing blows use amplitude 0.03. Spell casting uses spell-specific alignMod values typically ranging from 0.01 to 0.05. Positive alignMod values shift toward good and lawful when cast on enemies, while negative values shift toward evil and chaotic.

### Follower Type Characteristics

Thralls use AFFECT_THRALL, have no self-preservation, obey all orders, and are created by animate dead or golems. Charms use AFFECT_CHARM with spell SPELL_ENSORCER, have limited self-preservation, resist beguiling commands, and are created by ensorcer spell. Pets use AFFECT_PET, have full self-preservation, obey only safe commands, and come from pet shops or ranger taming.

### Follower Weight by Type

Zombies consume one plus level divided by ten follower slots. Charms consume two plus level divided by ten slots. Pets consume one plus level divided by seven slots. Maximum followers equals level plus charisma adjustment divided by twenty.

### Order Command Restrictions

Orders fail if the commander is charmed. Orders fail in rooms flagged ROOM_NO_ORDER. Pets refuse commands matching orderDenyCheck patterns, primarily combat and suicidal actions. Mounts have ego checks that may reject orders with bucking.

### Orphan Pet Timing

Orphaned pets receive AFFECT_ORPHAN_PET with duration eighty mud hours. During this window, original owners can reclaim automatically. Rangers can retrain with twenty percent failure chance. After expiration, the mob becomes fully wild.

### Language Skills by Race

SKILL_COMMON (719) is universal speech. SKILL_SIGN (576) is sign language. SKILL_FISHBURBLE (718) is fishman. SKILL_GNOLL_JARGON (713) is gnoll. SKILL_TROGLODYTE_PIDGIN (714) is troglodyte. SKILL_TROLLISH (715) is troll. SKILL_BULLYWUGCROAK (716) is bullywug and frogman. SKILL_AVIAN (717) is aarakocra. SKILL_GUTTER_CANT (712) is goblin and orc.

### Garble Types and Effects

TYPE_SIGN applies when sign skill is below maximum, causing sign language errors. TYPE_DRUNK applies when drunkenness reaches nine or higher, producing slurred speech. TYPE_GLUBGLUB applies underwater without waterbreath, replacing all speech with "Glub glub glub." TYPE_PG13IN filters incoming profanity for players with PG13 mode enabled. TYPE_PG13OUT filters outgoing profanity to recipients with PG13 mode. TYPE_FISHTALK applies fishman accent. TYPE_TROLLTALK applies Klingon-like troll accent. TYPE_FROGTALK applies soft consonant frogman accent. TYPE_BIRDTALK applies bird squawk aarakocra accent. TYPE_GUTTER applies Cockney goblin-orc accent. TYPE_TROGTALK applies hyphenated troglodyte accent. TYPE_LOLCATS applies internet speak gnoll accent.

### Speech Type Flags

SPEECH_SAY covers say and sayto commands. SPEECH_ASK covers ask. SPEECH_WHISPER covers whisper. SPEECH_SHOUT covers shout. SPEECH_TELL covers tell. SPEECH_GROUPTELL covers gtell. SPEECH_COMMUNE covers commune. SPEECH_SIGN covers sign language. SPEECH_WRITE covers writing. SPEECH_EMOTE covers emote speech. SPEECH_ROOMDESC applies drunk effects to room descriptions.

### Ensorcer Failure Conditions

Charm fails if victim has charm immunity on WEAR_BODY slot. Charm fails if victim level exceeds caster level. Charm fails if mob already hates caster according to Hates check. Charm fails if caster lacks sufficient power for spell versus victim. Charm fails if victim succeeds on luck save modified by spell luck modifier.

### Comprehension Formula Components

Base understanding equals listener language skill times ninety percent if skill check succeeds, otherwise zero. Perception adds plotStat of current perception from zero to sixteen with baseline eight. Speaker common skill adds eighty percent of SKILL_COMMON value if known and check succeeds. Intelligence adds plotStat of current intelligence from -10 to +10 with baseline zero. Final garble chance is clamped between zero and one hundred.

## Implementation

### Alignment Storage and Access

The factionData structure embeds directly in TBeing and contains align_ge and align_lc integer fields. Access occurs through direct field references on the being's faction data member. No getter or setter wrappers exist for alignment values.

### Reconcile Functions for Alignment Shifts

The reconcileHelp function accepts a victim pointer and amplitude modifier. It calculates alignment shift based on the victim's current alignment, the amplitude, and various scaling factors. The reconcileHurt wrapper simply calls reconcileHelp with negated amplitude, creating the symmetry between helping and harming.

Combat damage triggers call reconcileHurt from the damage application code path with amplitude 0.005. Killing blows call it from death processing with amplitude 0.03. Spell casting calls it from discipline implementations using the spell's alignMod field.

### Charm Affect Application

The ensorcer spell creates an affectedData structure with type AFFECT_CHARM and modifier AFF_CHARM. Duration calculation multiplies three times level times updates per mud hour, applies duration modification from the caster's skill level, then reduces by the victim's charm immunity percentage. Critical success multiplier ranges from two to three based on critical degree. Luck save reduction halves duration.

The be field in the affect stores the caster's name as a character pointer allocated with strdup. This enables ownership tracking after the caster disconnects. The level field stores the spell level used for duration and power calculations.

### Follower Counting and Limits

The tooManyFollowers function iterates the followers linked list counting pets, charms, and thralls. Each follower type adds a weighted value to the count based on the follower's level. The function compares total weight against maximum followers derived from level and charisma.

For new followers, the function adds the prospective follower's weight to the current total. If the sum exceeds maximum, it returns true to block the addition. Otherwise it returns false to allow the new follower.

### Order Processing and Validation

The doOrder function first checks if the commander is charmed, blocking charmed beings from giving orders. It parses the argument to extract target identifier and command string. For each potential target in the room or "all followers", it validates the follower relationship by checking master pointer equality and AFF_CHARM flag.

For pets specifically, it calls orderDenyCheck on the command buffer to filter combat and suicidal orders. Thralls and charms skip this check and accept all orders. The command is then queued into the follower's command queue for execution on their next action pulse.

### Orphan Creation and Restoration

When stopFollower is called on a charmed mob and the master is disconnecting or dead, the charm affects are replaced with AFFECT_ORPHAN_PET. The affect has type AFFECT_ORPHAN_PET, level zero, and duration eighty mud hours. All original charm affects are removed to prevent ownership conflicts.

The restorePetToPc function checks for AFFECT_ORPHAN_PET on the target mob. If found and the character is the original owner or a ranger using retrain, it performs a twenty percent rejection check. On success, it removes AFFECT_ORPHAN_PET and reapplies the original charm affects with the new master.

### Pet Persistence and Database Storage

The petSave function checks for ACT_STRINGS_CHANGED flag indicating a named pet. If set, it inserts a row into the pet table with player ID, mob vnum, customized name, experience, and level. Loading occurs during character login by querying the pet table and spawning mobs that match the owner ID.

### Garble Application Pipeline

The getGarbles function builds a bitmask of active garble types. It checks racial garble differences between speaker and listener, verifying listener intelligence is below 180. It adds condition-based garbles for drunkenness, underwater state, and PG13 mode. The result is a combined flag set representing all applicable transformations.

The garble function receives this flag set and the original speech string. It iterates from zero to TYPE_MAX checking each bit position. For each active garble, it verifies the garble's scope matches the requested scope and the garble's speech flags include the current speech type. Matching garbles call their transformation function, passing speaker, listener, current string, and speech type. Each transformation returns a modified string that becomes input to the next transformation.

### Language Comprehension Calculation

The getLanguageChance function first retrieves the listener's language skill value and performs a skill check. Success allows ninety percent of skill value to contribute to understanding. It then adds plotStat for perception ranging from zero to sixteen.

If the speaker knows SKILL_COMMON and succeeds a skill check, it adds eighty percent of the Common skill value. The speaker's intelligence plotStat from -10 to +10 is added. The final sum is subtracted from one hundred to get garble percentage, then clamped to the zero to one hundred range.

### Common Skill Initialization

During player creation and race assignment, the initialization code checks if the selected race has zero garbles. If true, it sets both natural and current skill values for SKILL_COMMON to MAX_SKILL_LEARNEDNESS. This auto-maxes the Common skill for races without native accents. Races with garbles start at zero and must learn through practice.

### Racial Garble Transformation Examples

The garble_fishtalk function demonstrates typical racial transformation logic. It iterates the input string character by character. At word boundaries determined by whitespace, it has a chance to insert "blub" or "burble" based on random rolls. Consonant clusters may be modified with aquatic phonemes. The transformed string accumulates in a buffer and is returned.

Other racial garbles follow similar patterns with race-specific phonetic rules. Troll garbles add guttural sounds, frogman garbles soften consonants, aarakocra garbles add squawks, goblin garbles apply Cockney accent transformations, troglodyte garbles hyphenate words, and gnoll garbles convert to internet lolcat speak.

## Troubleshooting

### Alignment Not Shifting as Expected

Verify reconcileHurt or reconcileHelp is being called from the action code path. Check that the amplitude value is non-zero and appropriately scaled. Confirm the victim's alignment is not at the extreme end of the axis, which may reduce shift magnitude. Examine faction percentage calculations if deity rewards are not triggering.

### Followers Not Obeying Orders

Confirm the AFF_CHARM flag is set on the follower and master pointer equals the commander. For pets, check if orderDenyCheck is blocking the command as combat-related. Verify the room does not have ROOM_NO_ORDER flag. Check if the commander itself is charmed, which blocks order giving.

### Charm Spell Failing Unexpectedly

Verify victim level does not exceed caster level. Check if victim has charm immunity on WEAR_BODY slot. Examine if the mob has an existing hate entry for the caster. Confirm the power differential check is passing using isNotPowerful. Check if victim is making luck saves frequently.

### Too Many Followers Error

Calculate total follower weight by summing each follower's contribution based on type and level. Compare against maximum followers from level plus charisma divided by twenty. High-level followers consume more slots, potentially blocking additional followers even when count appears low.

### Orphan Pet Not Reclaiming

Verify AFFECT_ORPHAN_PET still exists and has not expired past eighty mud hour duration. Check that the original owner ID matches the current character. For ranger retraining, confirm the target has the orphan affect. Check if the twenty percent rejection roll succeeded.

### Speech Completely Garbled

Check if listener intelligence is below 180, which bypasses racial garbles. Verify speaker and listener races have different garble flags. Examine active condition garbles like underwater or drunkenness. For complete garbling, check if TYPE_GLUBGLUB is active from underwater state without waterbreath.

### Language Skill Not Improving Understanding

Confirm listener has non-zero skill value in the racial language skill. Verify skill checks are succeeding using bSuccess for the language. Check that speaker's SKILL_COMMON is contributing if they know the skill. Examine perception and intelligence stats for modifier contribution.

### Named Pet Not Persisting

Verify ACT_STRINGS_CHANGED flag is set on the mob, which triggers database save. Check that petSave is being called during appropriate save points. Examine database for the pet table entry with matching owner ID and mob vnum. Confirm load code is querying and spawning pets during login.
