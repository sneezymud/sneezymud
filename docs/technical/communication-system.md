---
title: Communication System
description: Player communication channels including say, tell, shout, grouptell with garble transformations, soundproof rooms, ignore system, and predefined socials.
keywords: [doSay, doTell, doShout, doGrouptell, doWhisper, doAsk, doSign, doEmote, doCommune, canSpeak, applySoundproof, ROOM_SILENCE, TOG_IS_MUTE, AFF_SILENT, isDumbAnimal, garble, ignoreList, socialMessg, bootSocialMessages]
category: Understanding Systems

last_updated: 2026-01-29
source_files: [code/code/misc/talk.cc, code/code/misc/actions.cc, code/code/misc/immortal.cc, code/code/misc/garble.cc, code/code/sys/sound.cc, code/code/sys/connect.h, code/code/misc/parse.cc]
related:
  - command-implementation.md
  - monster-ai-behavior.md
  - group-party.md
  - affects-system.md
---

# Communication System

This document covers player communication channels, socials, and the restrictions that govern who can speak and who can hear.

**Key concepts:**
- Multiple channel types with different ranges and restrictions
- Garble system that transforms speech based on conditions
- Soundproof rooms that block local verbal communication
- Ignore system for filtering unwanted messages
- Predefined social actions with customizable targets

---

## Communication Channels

### Channel Overview

| Channel | Range | Command | Position Required | Restrictions |
|---------|-------|---------|-------------------|--------------|
| Say | Room | `say`, `'` | Resting | Soundproof, mute, silent, dumb animal |
| Tell | Global | `tell` | Resting | Mute, silent, dumb animal, target conditions |
| Whisper | Room | `whisper` | Resting | Soundproof, mute, dumb animal |
| Ask | Room | `ask` | Resting | Soundproof, mute, dumb animal |
| Shout | Global | `shout` | Resting | Soundproof, mute, silent, charm, level 2+, 15 move |
| Group Tell | Group | `gt`, `gtell` | Resting | Soundproof, mute, silent, AFF_GROUP required |
| Sign | Room | `sign` | Standing | Requires hands, SKILL_SIGN to understand |
| Emote | Room | `emote`, `:`, `,` | Resting | Soundproof, mute, drunk level check |
| Commune | Gods | `commune` | Any | POWER_WIZNET required |

**Source:** Command registration in `code/code/misc/parse.cc`

### Say Command

Local room communication. All awake characters in the room receive the message.

```cpp
int TBeing::doSay(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc`

**Validation checks:**
1. `applySoundproof()` - Blocks if room has ROOM_SILENCE flag
2. `hasQuestBit(TOG_IS_MUTE)` - Character trait blocking speech
3. `isDumbAnimal()` - Race/form cannot speak
4. `isAffected(AFF_SILENT)` - Magical silence effect

**Message flow:**
1. Speaker sees green-colored "You say" message
2. Each awake PC in room receives garbled message (individual garbles applied)
3. Mob AI responses trigger via `checkResponses(this, 0, garbleRoom, CMD_SAY)`

**Shortcuts:** `'` is aliased to `say` (CMD_SAY2)

### Tell Command

Global private messaging to any online player.

```cpp
int TBeing::doTell(const sstring& name, const sstring& message, bool visible);
```

**Source:** `code/code/misc/talk.cc`

**Validation checks:**
1. `AFF_SILENT` effect check
2. `applySoundproof()` for non-immortals
3. `isDumbAnimal()` check
4. Pet/charm/thrall cannot tell
5. Target existence via `findTellTarget()`
6. `PLR_GODNOSHOUT` flag (god-imposed communication ban)
7. `AUTO_NOTELL` autobit on target (allows reply only to last teller)
8. `TOG_IS_MUTE` quest bit
9. Target position (sleeping, stunned blocks)
10. Target descriptor state (editing, mailing, bugging)
11. Target soundproof room (for non-immortals)

**Special behaviors:**
- Sets `desc->last_teller` for reply command
- Sets `desc->last_told` for retelling
- Logs to `tellhistory` database table
- Shows AFK warning if target is away
- Immortals can find alternate characters on same account

**Reply command:** `reply <message>` uses `desc->last_teller`

### Shout Command

Global broadcast to all awake players not blocking shouts.

```cpp
void TBeing::doShout(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc`

**Restrictions:**
- Minimum level 2 required
- Costs 15 movement points
- Adds 0.5 combat round wait
- Charmed characters cannot shout
- Can be globally disabled via `toggleInfo[TOG_SHOUTING]`
- Blocked by `AUTO_NOSHOUT` autobit on receivers
- Blocked by `PLR_GODNOSHOUT` player flag

**Distribution:** `Descriptor::sendShout()` iterates all connected descriptors, checking:
- `CON_PLYNG` connection state
- Not sleeping
- Not ignoring sender
- Not set `AUTO_NOSHOUT` (unless immortal shouting or receiver is a mob)

**Twitter integration:** Shouts are posted via `twitterShout(getName(), garbled)`

### Group Tell Command

Communication to all group members regardless of location.

```cpp
void TBeing::doGrouptell(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc`

**Requirements:**
- Must have `AFF_GROUP` flag set
- Recipients must also have `AFF_GROUP` and share same master

**Distribution:**
1. Send to group leader (if not self and has `AFF_GROUP`)
2. Iterate leader's followers list
3. Each follower with `AFF_GROUP` receives individually garbled message
4. Skip recipients who ignore sender or are in soundproof rooms

### Whisper Command

Private room-local communication with eavesdropping potential.

```cpp
int TBeing::doWhisper(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc`

**Special mechanic:** Characters with `SKILL_SPY` active can overhear whispers if:
- Their level >= speaker's level
- Neither speaker, recipient, nor spy is immortal

### Sign Command

Silent visual communication requiring the SKILL_SIGN skill.

```cpp
int TBeing::doSign(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc`

**Requirements:**
- Must have hands (not transformed arms/hands)
- Neither arm hurt
- Not holding items in either hand
- Not fighting

**Reception:** Only characters with `doesKnowSkill(SKILL_SIGN)` see the message. Others see "makes funny motions with hands" (unless speaker is a thief).

### Emote Command

Free-form action descriptions visible to the room.

```cpp
int TBeing::doEmote(const sstring& argument);
```

**Source:** `code/code/misc/immortal.cc`

**Validation:**
1. `checkSoundproof()` blocks
2. Must be awake (or be a PC)
3. `PLR_GODNOSHOUT` blocks
4. `TOG_IS_MUTE` blocks
5. Drunk level check: Cannot emote if `getCond(DRUNK) > plotStat(STAT_CURRENT, STAT_CON, 0, 9, 6)`

**Format:** `<name> <emote text>` - The emote text is appended to the character's name.

**Shortcuts:** `:` and `,` are aliased to emote (CMD_EMOTE2, CMD_EMOTE3)

### Commune Command

Immortal-only communication channel (wiznet).

```cpp
void TBeing::doCommune(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc`

**Requirements:** `POWER_WIZNET` wizard power (or switched immortal with power)

**Level targeting:** `commune @<level> <message>` sends only to gods at or above specified level.

---

## Communication Restrictions

### Soundproof Rooms

Rooms with `ROOM_SILENCE` flag block all verbal communication.

```cpp
int TThing::checkSoundproof() const {
    return roomp->isRoomFlag(ROOM_SILENCE);
}

int TBeing::applySoundproof() const {
    if (checkSoundproof() && !isImmortal()) {
        sendTo("You are in a silence zone, you can't make a sound!\n\r");
        return TRUE;
    }
    return FALSE;
}
```

**Source:** `code/code/sys/sound.cc`

**Affected commands:** say, shout, grouptell, whisper, ask, emote

**Immune:** Immortals bypass soundproof checks, tell works globally (not room-local)

### Mute Condition

The `TOG_IS_MUTE` quest bit represents a permanent character trait (damaged throatbox).

**Affected commands:** say, shout, grouptell, tell, whisper, ask, emote, order, faction messages

**Partial bypass:** Mute characters CAN tell to immortal PCs

### Silent Effect

The `AFF_SILENT` affect flag represents magical silence.

**Affected commands:** say, shout, grouptell, tell, whisper, ask

**Visual feedback:** Silent characters attempting to speak trigger:
```cpp
act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
```

### Dumb Animal Check

Some race/polymorph forms cannot speak.

```cpp
if (isDumbAnimal()) {
    sendTo("You are a dumb animal; you can't talk!\n\r");
    return FALSE;
}
```

**Affected commands:** say, shout, grouptell, tell, whisper, ask

### Position Requirements

| Position | Communication Available |
|----------|------------------------|
| Dead | None |
| Stunned | None |
| Sleeping | None (cannot receive tells) |
| Resting | say, tell, shout, grouptell, whisper, ask, emote |
| Standing | sign |

### canSpeak() Aggregate Check

```cpp
bool TBeing::canSpeak() {
    if (isAffected(AFF_SILENT)) return FALSE;
    if (checkSoundproof()) return FALSE;
    if (isDumbAnimal()) return FALSE;
    if (hasQuestBit(TOG_IS_MUTE)) return FALSE;

    // Disease checks
    for (aff = affected; aff; aff = aff->next) {
        if (aff->type == AFFECT_DISEASE) {
            if (aff->modifier == DISEASE_GARROTTE) return FALSE;
            if (aff->modifier == DISEASE_SUFFOCATE) return FALSE;
            if (aff->modifier == DISEASE_DROWNING) return FALSE;
        }
    }
    return TRUE;
}
```

**Source:** `code/code/misc/talk.cc`

---

## Ignore System

Players can ignore other players to filter their communications.

### ignoreList Class

```cpp
class ignoreList {
    bool isIgnored(Descriptor* desc);
    bool isIgnored(const sstring ignored);
    static bool isMailIgnored(Descriptor* desc, const sstring ignored);

    bool add(Descriptor* desc);
    bool add(const sstring name);
    bool add(const TAccount& acct);
    bool addAccount(const sstring name);
    bool remove(Descriptor* desc);
    bool remove(const sstring name);
    bool removeAccount(const sstring name);
};
```

**Source:** `code/code/sys/connect.h`

### Ignored Communications

The ignore check `desc->ignored.isIgnored(sender_desc)` filters:
- Say messages
- Tell messages (silent failure - sender sees success)
- Whisper messages
- Shout messages
- Grouptell messages
- Emote messages
- Social actions

### NO_TELL Toggle

The `AUTO_NOTELL` autobit blocks incoming tells except from the last person you told to:

```cpp
if (!isImmortal() && vict->desc &&
    IS_SET(vict->desc->autobits, AUTO_NOTELL) &&
    strcmp(vict->desc->last_told, this->name.c_str()) != 0) {
    // Tell blocked
}
```

### NO_SHOUT Toggle

The `AUTO_NOSHOUT` autobit blocks receiving shouts (toggleable via `toggle noshout`).

---

## Garble System

Speech is transformed based on various conditions before delivery.

### Garble Types

| Type | Effect | Scope |
|------|--------|-------|
| `TYPE_DRUNK` | Slurs words | Everyone + self |
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

**Source:** `code/code/misc/garble.cc`

### Garble Scopes

- `SCOPE_EVERYONE` - Same transformation for all recipients
- `SCOPE_INDIVIDUAL` - Different transformation per recipient
- `SCOPE_EVERYONEANDSELF` - Includes speaker in transformation

### Speech Types

Different channels apply different garble flags:

| Speech Type | Channels |
|-------------|----------|
| `SPEECH_SAY` | say |
| `SPEECH_ASK` | ask |
| `SPEECH_WHISPER` | whisper |
| `SPEECH_SHOUT` | shout |
| `SPEECH_TELL` | tell |
| `SPEECH_GROUPTELL` | gtell |
| `SPEECH_SIGN` | sign |
| `SPEECH_EMOTE` | emote |

**Source:** `code/code/misc/garble.cc`

---

## Social/Emote System

### Predefined Socials

Socials are loaded from `lib/actions` file at boot via `bootSocialMessages()`.

**Source:** `code/code/misc/actions.cc`

### socialMessg Structure

```cpp
class socialMessg {
    bool hide;                    // Hide from room if target present
    positionTypeT minPos;         // Minimum position for target

    const char* char_no_arg;      // Message to actor (no target)
    const char* others_no_arg;    // Message to room (no target)

    const char* char_found;       // Message to actor (target found)
    const char* others_found;     // Message to room (target found)
    const char* vict_found;       // Message to target

    const char* not_found;        // Message when target not found

    const char* char_auto;        // Message to actor (self-target)
    const char* others_auto;      // Message to room (self-target)
};
```

**Source:** `code/code/misc/actions.cc`

### doAction() Implementation

```cpp
int TBeing::doAction(const sstring& argument, cmdTypeT cmd);
```

**Source:** `code/code/misc/actions.cc`

**Restrictions while fighting/riding:**
- **Allowed (facial only):** wink, boggle, nod, smile, grin, laugh, etc.
- **Allowed while riding only:** hug, comfort, pat, wave, point, etc.
- **Blocked while fighting:** dance, wiggle, bow, curtsey, etc.

### Social Sounds

Some socials trigger MSP sounds:

| Social | Sound |
|--------|-------|
| yawn | SOUND_YAWN_1 to SOUND_YAWN_4 (random) |
| giggle | SOUND_GIGGLE |
| burp | SOUND_BURP |
| clap | SOUND_CLAP |
| fart | SOUND_FART |
| sneeze | SOUND_SNEEZE |
| cackle | SOUND_CACKLE |
| scream | SOUND_SCREAM |
| chortle | SOUND_DM_LAUGH |
| disagree | SOUND_DISAGREE |
| woo | SOUND_YAHOO |

**Source:** `code/code/misc/actions.cc`

### Available Socials

From `lib/help/socials`:

**Primary socials:** applaud, beg, bleed, blush, bounce, bow, burp, cackle, chuckle, clap, comfort, cough, cringe, cry, cuddle, curtsey, dance, daydream, fart, flip, fondle, french, frown, fume, gasp, giggle, glare, grin, groan, grope, grovel, growl, hiccup, hop, hug, yawn, kiss, laugh, lick, love, moan, nibble, nod, nudge, pat, peer, point, poke, ponder, pout, preen, puke, punch, purr, ruffle, scream, shake, shiver, shrug, sigh, sing, slap, smirk, smile, snap, snarl, sneeze, snicker, sniff, snore, snuggle, spank, spit, squeeze, stare, steam, strut, sulk, tackle, taunt, thank, toast, twiddle, wave, whine, whistle, wiggle, wink, worship

**Extended socials:** insult, nuzzle, accuse, comb, massage, tickle, yodel, think, blink, highfive, bonk, scold, drool, rip, stretch, pimp, belittle, piledrive, tap, pinch, bite, moon, whap, beam, chortle, scratch, flipoff, cheer, woo, grumble, apologize, agree, disagree, spam, arch, roll, faint, greet, tip, bop, jump, whimper, sneer, moo, boggle, snort, tango, roar, flex, tug, cross, howl, grunt, wedgie, scuff, noogie, brandish, trip, duck, beckon, wince, hum, razz, gag, avert, salute, pet, grimace

### Social AI Responses

Socials trigger mob AI via `aiSocialSwitch()`:

```cpp
rc = tmp->aiSocialSwitch(this, vict, cmd, TARGET_TYPE);
```

Target types: `TARGET_NONE`, `TARGET_SELF`, `TARGET_MOB`, `TARGET_OTHER`

---

## GMCP Integration

Communication commands send GMCP messages for client integration:

```cpp
sstring gmcp = format("comm.channel { \"chan\": \"%s\", \"msg\": \"%s\", \"player\": \"%s\" }") %
    channel % message % player;
desc->sendGmcp(gmcp, false);
```

**Channels:**
- `say`, `mobsay` - Local say
- `tell`, `mobtell` - Private tells
- `yell` - Shouts
- `gtell` - Group tells
- `wiz` - Wiznet/commune

---

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/talk.cc` | doSay, doShout, doGrouptell, doTell, doWhisper, doAsk, doSign, doReply, canSpeak |
| `code/code/misc/actions.cc` | bootSocialMessages, doAction, socialMessg class |
| `code/code/misc/immortal.cc` | doEmote, doCommune |
| `code/code/misc/garble.cc` | Garble types, speech transformations |
| `code/code/sys/sound.cc` | checkSoundproof, applySoundproof |
| `code/code/sys/connect.h` | ignoreList class, AUTO_NOSHOUT, AUTO_NOTELL |
| `code/code/misc/parse.cc` | Command registration (CMD_SAY, CMD_TELL, etc.) |

---

## Common Gotchas

### Ignoring Descriptor Checks

```cpp
// WRONG: Assuming vict->desc exists
vict->desc->clientf(...);

// CORRECT: Check first
if (vict->desc && (vict->desc->m_bIsClient || IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
    vict->desc->clientf(...);
```

### Not Checking Ignore Before Sending

```cpp
// WRONG: Sending without ignore check
act(message, TRUE, this, 0, vict, TO_VICT);

// CORRECT: Check ignore first
if (!vict->desc || !vict->desc->ignored.isIgnored(desc))
    act(message, TRUE, this, 0, vict, TO_VICT);
```

### Forgetting Position Checks

Recipients who are sleeping or stunned should not receive most communications. Check `getPosition() > POSITION_SLEEPING` for awake recipients.

### Soundproof vs Tell

Tell is NOT blocked by soundproof rooms (it's global). Only local communications (say, whisper, ask, emote) check soundproof.

### DELETE Flag Returns

Communication commands can return `DELETE_THIS` when mob responses trigger character death:

```cpp
rc = tmons->checkResponses(this, 0, garbleRoom, CMD_SAY);
if (IS_SET_DELETE(rc, DELETE_THIS))
    delete tmons;
if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;  // Speaker died from mob response
```

---

## Related Documentation

- [Command Implementation](command-implementation.md) - Command dispatch and handler patterns
- [Mob AI](mob-ai.md) - AI responses to player communication
- [Group Party](group-party.md) - Group tell and AFF_GROUP requirements
- [Affects System](affects-system.md) - AFF_SILENT and other communication affects
