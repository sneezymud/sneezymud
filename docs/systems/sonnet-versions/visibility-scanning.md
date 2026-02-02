---
title: Visibility and Scanning
category: important
keywords: [eyeSight, visibility, canSee, lookRoom, doScan, infravision, AFF_BLIND, AFF_TRUE_SIGHT, pitchBlackDark, SKILL_SEARCH]
related: [room-environment.md, affects-system.md, skill-combat.md]
primary_symbols:
  functions: [eyeSight, visibility, canSeeMe, lookRoom, doScan, listExits, doConsider, clearpath, list_in_heap]
  classes: [TBeing, TRoom, TObj]
  files: [code/code/misc/utility.cc, code/code/cmd/cmd_look.cc, code/code/misc/range.cc, code/code/misc/show.cc]
---

# Visibility and Scanning

## Overview

Visibility operates as a bidirectional comparison system where observers possess an eyeSight value representing visual acuity while targets have a visibility value representing concealment difficulty. An observer successfully perceives a target when their eyeSight meets or exceeds the target's visibility threshold.

The system integrates with environmental conditions, character states, equipment properties, and racial abilities to determine what characters can perceive in their surroundings. This affects room observation, distant scanning, inventory examination, and combat assessment.

Core visibility functions reside in utility.cc, while specific applications appear in cmd_look.cc for room observation, range.cc for scanning, show.cc for object display, and cmd_consider.cc for combat evaluation.

## Patterns

### Vision Calculation Workflow

The observer's visual capability combines base modifiers with environmental factors. The visionBonus attribute serves as the character-specific foundation, augmented by racial bonuses such as elven sight enhancements. Magical effects like AFF_TRUE_SIGHT or AFF_CLARITY provide substantial bonuses. Room illumination contributes based on time of day and weather, while indoor locations incur penalties. Precipitation reduces visibility through rain, snow, and lightning storm modifiers.

Target concealment builds from the canBeSeen baseline, increased by hiding skills and environmental advantages. Characters benefit from home terrain familiarity and background matching. Equipment with shadowy properties contributes proportionally to body coverage. Forest sectors and rainfall increase concealment, while snow and lightning reduce it.

### Room Observation Sequence

When characters observe rooms through lookRoom, the system first transmits GMCP data to the client, then renders the automap if enabled. Room names display followed by descriptions unless PLR_BRIEF suppresses them. Weather and ground conditions receive textual description. Exit listing follows through listExits with color coding based on door states and sector types. Tracking information updates for hunting characters. Finally, list_thing_in_room enumerates visible beings and objects.

Darkness detection occurs early in the observation flow. When pitchBlackDark returns true for insufficient illumination and the observer lacks true sight, clarity, or immortal status, lookDark handles the restricted display. This limited view still reveals beings visible through infravision or personal glow effects, along with any glowing objects.

### Scan Range Mechanics

Distance scanning calculates maximum range from a base value reduced by terrain thickness, enhanced by vision bonuses and racial line-of-sight modifiers. Weather conditions further adjust this range, with snow imposing the heaviest penalty and clear conditions providing bonuses.

The clearpath check validates each step along the scan direction, rejecting paths through closed doors or invalid exits. For each accessible room within range, beings undergo canSeeMe evaluation. The system accumulates visible beings until crowd hindrance thresholds activate, stopping further scanning when too many beings obstruct the view.

### Combat Assessment Flow

The consider command branches into self-evaluation and monster comparison modes. Self-assessment examines armor effectiveness by comparing worn protection against recommended values for the character's level. Visibility and noise calculations derive from the character's current state and equipment.

Monster consideration computes level differences between the observer and target, translating numerical gaps into qualitative descriptions. Characters with adventuring lore skills gain additional information based on creature type. Skill learning thresholds unlock progressively detailed statistics including health estimates, armor class, attack counts, and damage projections. Trophy tracking integration modifies displayed experience values based on previous encounters with that monster type.

### Inventory and Equipment Display

Inventory examination requires either non-blind status or compensating magical vision through true sight or clarity. The list_in_heap function groups similar items using isSimilar comparisons, displaying aggregated counts. Optional filtering applies when arguments specify item names. Characters above level 10 receive capacity indicators showing volume and weight percentages.

Equipment display iterates through all wear slots from MIN_WEAR to MAX_WEAR, skipping duplicate entries for paired items like gloves or boots. Tattoos integrated into empty slots appear in the listing. The damaged filter option restricts display to items with current structural points below maximum values.

## Reference

### Vision Formulas

Observer eyeSight combines visionBonus, racial modifiers, spell bonuses, light level, and weather penalties. True sight or clarity grants +25. Racial bonuses vary by species. Indoor locations suffer light reduction. Precipitation imposes incremental penalties: rain -1, snow -2, lightning -1.

Target visibility sums canBeSeen, hide bonuses, equipment modifiers, and environment factors. Hide affection adds 5 plus half character level when not fighting. Home terrain and background matching each contribute +5. Shadowy equipment scales with coverage percentage. Forest terrain adds +2. Rain increases concealment by +1 while snow decreases it by -2.

### Scan Distance Ranges

The base formula 15 minus terrain thickness plus visionBonus divided by 10 plus racial line-of-sight determines maximum scan range. Weather adjustments: snow -3, rain -2, cloudy or fog -1, clear +1.

Descriptive text varies by distance: adjacent rooms show "immediately," nearby at 2 rooms, short ways at 3, not too far at 4, a ways at 5, quite a ways at 6-7, way off at 8-9, far at 10-11, way way off at 12-14, real far at 15-17, very far at 18-19, on the horizon at 20 or greater.

### canSeeMe Check Sequence

The core visibility validation proceeds through ordered stages. Immortal invisibility levels prevent lower-level observers from detection. Immortal observers bypass most restrictions except higher invisibility. Self-observation always succeeds. Invisibility affections or shadow walk in dim lighting block detection. True sight or clarity override all subsequent checks. Blindness without compensating magic fails. Sanctuary glow forces visibility. Personal light sources make carriers visible. Room always-lit flags bypass illumination requirements. Infravision provides bonuses against warm-blooded creatures. Final comparison tests eyeSight exceeding visibility.

### Exit Display Color Coding

Normal open exits display in purple for both brief and verbose modes. Open doors use bold variants in brief mode and blue in verbose mode. Closed doors receive asterisk prefixes in brief mode and red coloring for immortal observers. Sector types override with specific colors: fire in red, air in cyan, water in blue.

### Secret Door Detection Mechanics

Passive detection during exit listing grants characters with SKILL_SEARCH a probability-based hint. Base chance derives from search skill value, enhanced by racial bonuses: elven +25, gnome adds perception plus half level, dwarf receives half level plus 10 when indoors. Success on a 1000-sided roll displays "You suspect something out of the ordinary here."

Active searching through the search task costs 3 movement per direction. The system examines all 10 cardinal and vertical directions sequentially, skipping directions with visible exits and ceiling when room lacks height. Learning opportunities occur every 3 directions. Revelation requires skill success, EXIT_SECRET flag, EXIT_CLOSED flag, non-empty keyword, and keyword not matching "_unique_door_".

### Consider Level Difference Messages

The monster comparison calculates target real level minus observer maximum level. Results map to qualitative assessments: -15 or less suggests tying hands behind back, -10 or less questions bothering, -6 or less warns against strain, -3 or less calls it cake, -2 or less favors odds, -1 or less grants slight advantage, 0 declares fairness, 1 or less minimizes apparent toughness, 2 or less recommends finger crossing, 3 or less adds hopeful crossing, 6 or less demands good planning, 10 or less suggests bringing friends, 15 or less questions army size, 30 or less promises victory without being hit, above 30 identifies better suicide methods.

### Lore Information Thresholds

Skill learning above 5 reveals estimated maximum health point ratios. Learning above 20 exposes estimated armor class. Learning above 40 shows estimated attack counts. Learning above 60 displays estimated damage per attack. All estimates apply GetApprox variance based on skill proficiency.

## Implementation

### eyeSight Function Logic

Implementation in utility.cc begins with visionBonus retrieval through getVision. Race-specific bonuses accumulate via getRacialVisionBonus. Magical sight enhancements check for AFF_TRUE_SIGHT or AFF_CLARITY flags, adding 25 when present. Room light calculation invokes getRoomLight, halved for indoor sectors. Weather penalties examine current conditions, subtracting values for rain, snow, and lightning. The final sum represents total visual capability.

### visibility Function Logic

Target concealment starts with canBeSeen value access. Hide affection contributes when AFF_HIDE is set and the character is not fighting, adding 5 plus half the character's level. Terrain familiarity checks compare room sector against character native terrain, granting +5 for matches. Background matching verifies sector color against character base color for another +5. Equipment iteration examines each worn item for ITEM_SHADOWY flags, accumulating coverage-weighted bonuses. Environmental modifiers check room sector for forest type and weather for precipitation effects.

### canSeeMe Implementation Structure

The function receives observer and target pointers, validating both before proceeding. Immortal invisibility comparison returns false when target has higher invisibility level than observer. Immortal observer status bypasses to success except for higher invisibility. Self-reference comparison succeeds immediately. Invisibility detection combines AFF_INVISIBLE checking with shadow walk in dim light verification. True sight or clarity presence forces success return. Blindness without compensating magic forces failure. Sanctuary detection via AFF_SANCTUARY permits visibility. Light carrying detection through hasLight succeeds. Room flag verification for ROOM_ALWAYS_LIT bypasses further checks. Infravision application calculates bonuses via getInfravision against warm-blooded targets. Final comparison returns eyeSight exceeding visibility.

### Scan Direction Traversal

The doScan implementation in range.cc initializes with direction parsing, handling both specific directions and all-directions scanning. Movement cost deduction applies 10 for omnidirectional scans, 2 for single direction. Range calculation follows the formula incorporating terrain, vision, and weather. The clearpath loop validates each successive room, breaking on closed doors or invalid exits. Being enumeration at each valid distance calls canSeeMe for each room occupant. Crowd counting increments with each visible being, breaking when exceeding 5 plus visionBonus divided by 3. Distance descriptive text selection uses the room count to index appropriate messages.

### list_thing_in_room Processing

Room contents iteration separates beings from objects, processing each category through distinct display paths. Being visibility filters through canSee before including in the visible list. Object visibility applies similar filtering. The showObjectsToChar function groups objects via list_in_heap for consolidated display. Being display calls showMultTo for each visible character or creature.

### list_in_heap Grouping Algorithm

Implementation in show.cc creates a temporary item list, iterating through all provided objects. For each object, isSimilar comparisons check against already-grouped items. Matches increment the group count. New unique items create new groups. Display generation formats item names with bracketed counts for quantities exceeding one. The show_all parameter triggers recursive descent into container contents.

### Consider Self-Assessment

The self-consider path begins with armor difference calculation, subtracting current armor from suggestArmor recommendations. Threshold comparisons map differences to qualitative descriptions ranging from laughably pathetic through incredibly good. Visibility invocation provides concealment value for assessment description. Noise calculation similarly derives loudness metrics.

### Consider Monster Evaluation

Monster comparison retrieves target real level including spell effects, subtracts observer maximum level. Level difference maps through threshold comparisons to message selection. Lore skill checking iterates through creature-type-specific skills: SKILL_CONS_ANIMAL for animals, SKILL_CONS_VEGGIE for vegetables, SKILL_CONS_DEMON for demons, SKILL_CONS_REPTILE for reptiles, SKILL_CONS_UNDEAD for undead, SKILL_CONS_GIANT for giants, SKILL_CONS_PEOPLE for humanoids, SKILL_CONS_OTHER for monsters. Skill value determines information detail level through threshold checks. Trophy integration queries count via getCount, modifying experience display based on encounter history.

### Exit Color Application

The listExits function in info.cc iterates through MAX_DIR directions, checking dir_option existence. Normal open exits format with purple ANSI codes. Door detection examines door_type for non-DOOR_NONE values. Open door status applies bold variants or blue coloring. Closed door status prefixes asterisks or applies red for immortals. Sector type overrides check room flags for fire, air, and water, applying corresponding color codes.

### Search Task Execution

Task initialization in task_search.cc establishes direction iteration from NORTH through DOWN, resetting to NORTH after completing the cycle. Each iteration checks existing exits via canSeeThruDoor, skipping directions with visible access. Ceiling searches require isFlying or climbing skill checks. Movement deduction applies before skill attempt. Skill check invocation uses bSuccess with SKILL_SEARCH. Success combined with EXIT_SECRET, EXIT_CLOSED, valid keyword, and non-unique door triggers revelation through sendTo messaging and EXIT_SECRET flag removal.

### Darkness Handling

The pitchBlackDark check in TRoom compares light level against zero threshold. True result triggers lookDark invocation in doLook. The lookDark implementation sends "It is very dark in here..." message, then filters room contents for infravision-visible beings and glowing objects. Infravision visibility checks warm-blooded creature types. Glow detection examines ITEM_GLOW object flags and being sanctuary effects.

### Door Transparency Evaluation

The canSeeThruDoor function in doors.cc performs ordered condition checks. EXIT_CAVED_IN immediately returns false. EXIT_DESTROYED returns true. Open state check succeeds when EXIT_CLOSED is not set. Door type evaluation permits DOOR_NONE, DOOR_PORTCULLIS, DOOR_GRATE, and DOOR_SCREEN. All other closed door types fail transparency.

## Troubleshooting

### Invisibility Not Working

Verify the target has AFF_INVISIBLE set and the observer lacks AFF_TRUE_SIGHT or AFF_CLARITY. Check for sanctuary glow overriding invisibility through AFF_SANCTUARY. Confirm the character is not carrying light sources detectable through hasLight. Examine shadow walk conditions requiring dim lighting to function.

### Scan Showing Nothing

Validate clearpath returns true for the scanned direction by checking closed door states. Confirm eyeSight exceeds visibility for intended targets through manual calculation. Verify crowd hindrance has not activated by counting previously displayed beings. Check maximum range calculation against actual room distance, considering terrain thickness and weather penalties.

### Secret Door Detection Failing

Confirm the exit has both EXIT_SECRET and EXIT_CLOSED flags set. Verify keyword is non-empty and not "_unique_door_" which blocks search revelation. Check skill value meets success thresholds through bSuccess probability. Ensure sufficient movement points remain for the 3-point cost per direction.

### Consider Information Missing

Validate the character possesses appropriate lore skill for the creature type being examined. Check skill learning level exceeds information thresholds: 5 for health, 20 for armor, 40 for attacks, 60 for damage. Confirm the target is not a special proc creature bypassing normal consider mechanics.

### Darkness Not Blocking Vision

Verify the room light level is zero or negative through getRoomLight. Check for ROOM_ALWAYS_LIT flag overriding darkness. Confirm the observer is not immortal. Examine for AFF_TRUE_SIGHT or AFF_CLARITY granting magical vision. Validate pitchBlackDark conditions including all flag checks and vision bonus state.

### Infravision Not Revealing Targets

Confirm the target is warm-blooded by checking race and creature type. Verify the observer has infravision capability through racial traits or equipment. Check that lighting conditions permit infravision bonuses to apply. Examine canBeSeen thresholds to ensure infravision bonus provides sufficient eyeSight increase.

### Equipment Display Showing Duplicates

Check for paired item handling logic properly detecting items worn on symmetric positions. Verify wear slot iteration skips appropriate duplicates for gloves, boots, and other paired equipment. Confirm equipment tracking correctly identifies primary versus secondary worn locations.

### Exit Colors Not Displaying

Validate ANSI color code support in the client connection. Check descriptor color mode settings through color preferences. Verify exit type determination correctly identifies door states and sector types. Examine color code application in listExits for proper ANSI sequence formatting.
