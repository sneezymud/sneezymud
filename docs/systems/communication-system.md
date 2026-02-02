---
title: Communication System
category: gameplay
created_by_model: opus
keywords: [doSay, doTell, doShout, doGrouptell, doWhisper, doSign, doEmote, doCommune, canSpeak, applySoundproof, ignoreList, socialMessg, garble, ROOM_SILENCE, TOG_IS_MUTE, AFF_SILENT]
related: [group-party.md, affects-system.md, command-implementation.md]
primary_symbols:
  functions: [doSay, doTell, doShout, doGrouptell, doWhisper, doAsk, doSign, doEmote, doCommune, doAction, canSpeak, applySoundproof, checkSoundproof, bootSocialMessages, checkResponses]
  classes: [ignoreList, socialMessg, TBeing, Descriptor]
  files: [code/code/misc/talk.cc, code/code/misc/actions.cc, code/code/misc/immortal.cc, code/code/misc/garble.cc, code/code/sys/sound.cc, code/code/sys/connect.h]
---

## Overview

How does a player's spoken message reach others in the room? How do magical silence effects prevent communication? How do speech impediments and drunkenness affect dialogue?

The communication system provides multiple channels for player interaction: local room chat, global private messaging, group coordination, and immortal administration. Each channel has distinct range, restrictions, and transformation rules.

Messages flow through a validation pipeline before reaching recipients. The system checks room properties (soundproof zones), character state (muted, silenced, dumb animal), position requirements, and social permissions. Recipients can further filter messages through an ignore system.

Speech undergoes garble transformations based on speaker conditions (drunk, underwater, ghostly) and receiver settings (profanity filters). These transformations apply individually per recipient, allowing the same message to appear differently to different observers.

Predefined socials provide structured emotive actions with contextual messages for actor, target, and observers. These trigger mob AI responses and optional sound effects.

---

## Patterns

### Always Check Soundproof Before Local Communication

Call `applySoundproof()` before say, whisper, ask, grouptell, and emote. This checks `ROOM_SILENCE` flag and sends the appropriate failure message. Immortals bypass this check automatically.

Tell is global and does not check soundproof on the sender's room. However, non-immortal tell recipients in soundproof rooms cannot receive.

### Always Check canSpeak() for Aggregate Validation

The `canSpeak()` method checks all speech-blocking conditions in one call: AFF_SILENT, soundproof room, dumb animal form, TOG_IS_MUTE quest bit, and speech-blocking diseases (garrotte, suffocate, drowning). Use this when you need a comprehensive check.

### Always Check Descriptor Before Sending Client Data

Never assume `vict->desc` exists. Mobs do not have descriptors. Before sending GMCP messages or client-specific data, verify the descriptor exists and check client capability flags.

### Always Check Ignore Before Delivering Messages

Call `desc->ignored.isIgnored(sender_desc)` before delivering say, tell, whisper, shout, grouptell, emote, and social messages. Tell failures are silent; the sender sees success even when ignored.

### Always Check Position for Recipients

Recipients who are sleeping or stunned should not receive most communications. Verify `getPosition() > POSITION_SLEEPING` before delivery. Tells explicitly fail if the target is sleeping or stunned.

### Always Handle DELETE Flags from Mob Responses

Communication commands can trigger mob AI via `checkResponses()`. This can return DELETE_THIS or DELETE_VICT if the mob response kills someone. Check return values and propagate deletion flags appropriately.

### Never Bypass Mute Check for Non-Immortal Targets

Mute characters (TOG_IS_MUTE) can only tell to immortal PCs. This is a deliberate carve-out. Do not extend this bypass to other channels or targets.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doSay()` | function | Local room speech |
| `doTell()` | function | Global private messaging |
| `doShout()` | function | Global broadcast |
| `doGrouptell()` | function | Group-only messaging |
| `doWhisper()` | function | Private room-local with eavesdrop potential |
| `doAsk()` | function | Room-local question |
| `doSign()` | function | Silent visual communication |
| `doEmote()` | function | Free-form action descriptions |
| `doCommune()` | function | Immortal wiznet channel |
| `doAction()` | function | Predefined social execution |
| `canSpeak()` | method | Aggregate speech validation |
| `applySoundproof()` | method | Soundproof room check with message |
| `checkSoundproof()` | method | Soundproof room check (bool only) |
| `bootSocialMessages()` | function | Load socials from lib/actions |
| `ignoreList` | class | Player ignore list management |
| `socialMessg` | class | Social action message templates |

### Channel Properties

| Channel | Range | Position | Key Restrictions |
|---------|-------|----------|------------------|
| Say | Room | Resting | Soundproof, mute, silent, dumb animal |
| Tell | Global | Resting | Mute, silent, dumb animal, target state |
| Whisper | Room | Resting | Soundproof, mute, dumb animal |
| Ask | Room | Resting | Soundproof, mute, dumb animal |
| Shout | Global | Resting | Soundproof, mute, silent, charm, level 2+, 15 move |
| Group Tell | Group | Resting | Soundproof, mute, silent, AFF_GROUP required |
| Sign | Room | Standing | Requires hands, SKILL_SIGN to understand |
| Emote | Room | Resting | Soundproof, mute, drunk level check |
| Commune | Gods | Any | POWER_WIZNET required |

### Command Shortcuts

| Shortcut | Command |
|----------|---------|
| `'` | say |
| `:` | emote |
| `,` | emote |
| `gt` | grouptell |
| `reply` | tell to last_teller |

### Communication Blocking Conditions

| Condition | Affected Channels | Notes |
|-----------|-------------------|-------|
| `ROOM_SILENCE` | say, whisper, ask, grouptell, emote, shout | Immortals bypass |
| `TOG_IS_MUTE` | All except tell to immortals | Quest bit (permanent) |
| `AFF_SILENT` | say, shout, grouptell, tell, whisper, ask | Magical effect |
| `isDumbAnimal()` | say, shout, grouptell, tell, whisper, ask | Race/polymorph form |
| `PLR_GODNOSHOUT` | tell, shout, emote | God-imposed ban |
| `AUTO_NOTELL` | tell (except from last_told) | Player toggle |
| `AUTO_NOSHOUT` | shout reception | Player toggle |

### Garble Types

| Type | Effect | Scope |
|------|--------|-------|
| `TYPE_DRUNK` | Slurred words | Everyone + self |
| `TYPE_GHOST` | Ghostly speech | Everyone |
| `TYPE_WAHWAH` | Crying sounds | Everyone |
| `TYPE_BLAHBLAH` | Whining sounds | Shout only |
| `TYPE_GLUBGLUB` | Underwater garble | Everyone |
| `TYPE_PG13IN` | Profanity filter (receiving) | Individual |
| `TYPE_PG13OUT` | Profanity filter (sending) | Everyone |
| `TYPE_SIGN` | Sign language distortion | Everyone |
| `TYPE_PIRATE` | Pirate accent | Everyone |
| `TYPE_VAMPIRE` | Vampire speech patterns | Everyone |
| `TYPE_IGOR` | Igor-style lisp | Everyone |
| `TYPE_IRISH` | Irish accent | Everyone |
| `TYPE_TROLLTALK` | Troll dialect | Individual |
| `TYPE_FROGTALK` | Frog speech | Individual |
| `TYPE_BIRDTALK` | Bird chirps | Individual |

### GMCP Channel Names

| GMCP Channel | Game Channels |
|--------------|---------------|
| `say`, `mobsay` | Local say |
| `tell`, `mobtell` | Private tells |
| `yell` | Shouts |
| `gtell` | Group tells |
| `wiz` | Wiznet/commune |

### Key Files

| File | Contents |
|------|----------|
| `talk.cc` | doSay, doShout, doGrouptell, doTell, doWhisper, doAsk, doSign, doReply, canSpeak |
| `actions.cc` | bootSocialMessages, doAction, socialMessg class |
| `immortal.cc` | doEmote, doCommune |
| `garble.cc` | Garble types, speech transformations |
| `sound.cc` | checkSoundproof, applySoundproof |
| `connect.h` | ignoreList class, AUTO_NOSHOUT, AUTO_NOTELL |

---

## Implementation

### Message Flow Architecture

Local communication (say, whisper, ask, emote) follows a consistent pattern: validate speaker conditions, apply speaker-side garble transformations, iterate room occupants, apply recipient-side garbles individually, deliver to qualifying recipients, and trigger mob AI responses.

Global communication (tell, shout) iterates all connected descriptors. Tell validates a single target; shout broadcasts to all non-blocking recipients. Both respect the ignore system and position requirements.

Group communication iterates the group leader's follower list. Recipients must have AFF_GROUP set and share the same master. Messages are individually garbled per recipient.

### Tell Target Resolution

`findTellTarget()` resolves tell recipients through multiple strategies: exact name match among visible characters, then account-based alternate character lookup for immortals. This allows immortals to reach players on different characters within the same account.

The system tracks `desc->last_teller` for reply and `desc->last_told` for retelling. Tells are logged to the `tellhistory` database table.

### Shout Distribution

`Descriptor::sendShout()` broadcasts to all descriptors in `CON_PLYNG` connection state. Recipients must be awake, not ignoring the sender, and not have AUTO_NOSHOUT set (unless the shouter is immortal or recipient is a mob).

Shouts cost 15 movement points, add 0.5 combat round wait, require level 2+, and cannot be performed while charmed. Global shout toggle `toggleInfo[TOG_SHOUTING]` can disable the channel entirely.

### Whisper Eavesdropping

Characters with active SKILL_SPY can intercept whispers if their level equals or exceeds the speaker's level. Neither speaker, recipient, nor spy can be immortal. The spy sees the full whispered message directed to the original target.

### Sign Language Reception

Sign requires hands (not transformed arms/hands), neither arm hurt, no items held, and not fighting. Only characters with SKILL_SIGN see the actual message. Others see a generic "makes funny motions with hands" message, unless the signer is a thief (stealth class exemption).

### Emote Drunk Check

Emotes validate against a drunk threshold calculated from the character's constitution: `getCond(DRUNK) > plotStat(STAT_CURRENT, STAT_CON, 0, 9, 6)`. Characters too drunk to emote receive a slurred failure message.

### Commune Level Targeting

Commune supports level-targeted messaging via `@<level>` prefix: `commune @60 <message>` sends only to gods at or above level 60. Without a level prefix, all POWER_WIZNET holders receive the message.

### Social Action System

Socials are loaded from `lib/actions` at boot via `bootSocialMessages()`. Each `socialMessg` contains message templates for: no target (actor/room), target found (actor/room/victim), target not found, and self-target (actor/room).

`doAction()` parses target, validates position and combat restrictions, selects appropriate message templates, and sends via `act()`. Some socials are facial-only (allowed while fighting/riding), others require standing and no combat.

### Social AI Integration

Socials trigger mob AI via `aiSocialSwitch()` with target type flags: TARGET_NONE, TARGET_SELF, TARGET_MOB, TARGET_OTHER. Mob responses can trigger combat or other interactions.

### Social Sounds

Specific socials trigger MSP sound effects: yawn (random from 4), giggle, burp, clap, fart, sneeze, cackle, scream, chortle, disagree, woo. Sound selection is hardcoded in `doAction()`.

### Ignore System

The `ignoreList` class manages per-player ignore lists. It supports ignoring by descriptor, character name, or entire account. The `isMailIgnored()` static method extends blocking to the mail system.

Ignored communications fail silently for the sender (they see success) but never reach the target. This applies to say, tell, whisper, shout, grouptell, emote, and socials.

---

## Troubleshooting

### Message Appears to Send But Target Never Receives

**Symptom:** Sender sees normal output, target gets nothing.

**Likely cause:** Target has sender on ignore list, or target is in a soundproof room (for tell to non-immortal), or target has AUTO_NOTELL set and sender is not last_told.

**Diagnostic approach:** Check target's ignore list, room flags, and autobits. Verify sender identity matches last_told if AUTO_NOTELL is set.

**Fix:** This is usually intentional behavior. Inform sender about AUTO_NOTELL status if appropriate.

### Tell Fails with "Couldn't find anyone by that name"

**Symptom:** Tell to valid online player fails with not-found message.

**Likely cause:** Target is linkdead, editing/mailing/bugging, sleeping, or invisible to sender without immortal powers.

**Diagnostic approach:** Check target connection state, position, and visibility flags.

**Fix:** Wait for target to return to playing state. Use immortal powers if appropriate to bypass visibility.

### Soundproof Room Doesn't Block Communication

**Symptom:** Characters can communicate in ROOM_SILENCE flagged room.

**Likely cause:** Speaker is immortal (bypasses check), or using tell (global, not room-local), or room flag not properly set.

**Diagnostic approach:** Verify speaker mortality, communication channel type, and room flags via stat command.

**Fix:** Confirm room has ROOM_SILENCE flag. Tell is intentionally not blocked by sender's soundproof room.

### Mob AI Does Not Respond to Speech

**Symptom:** Mob with response triggers ignores say commands.

**Likely cause:** Speech garbled beyond recognition, mob not awake, or checkResponses return value ignored causing crash before response.

**Diagnostic approach:** Check mob position, verify garble transformations, examine checkResponses trigger patterns.

**Fix:** Ensure mob response patterns account for garble transformations. Verify DELETE flag handling after checkResponses.

### Sign Language Visible to Non-Signers

**Symptom:** Characters without SKILL_SIGN see actual signed content.

**Likely cause:** Skill check bypassed, or signer is thief class (intentional exemption).

**Diagnostic approach:** Check receiver's skill list and signer's class.

**Fix:** Thief class exemption is intentional. For other cases, verify SKILL_SIGN check in doSign delivery loop.

### Garble Not Applied

**Symptom:** Drunk/ghost/underwater speech appears normal.

**Likely cause:** Garble condition not detected, or garble scope excludes recipient.

**Diagnostic approach:** Check speaker condition flags (drunk level, AFF_GHOST, underwater sector). Verify garble type scope (SCOPE_INDIVIDUAL vs SCOPE_EVERYONE).

**Fix:** Ensure condition flags are properly set. Some garbles only apply to specific speech types.
