---
title: Communication System
category: important
keywords: [communication, channels, garble, soundproof, ignore, socials, tell, say, shout]
related: [command-implementation.md, monster-ai-behavior.md, group-party.md, affects-system.md]
primary_symbols:
  functions: [doSay, doTell, doShout, doGrouptell, doWhisper, doAsk, doSign, doEmote, doCommune, canSpeak, applySoundproof, doAction, bootSocialMessages]
  classes: [ignoreList, socialMessg]
  files: [code/code/misc/talk.cc, code/code/misc/actions.cc, code/code/misc/garble.cc, code/code/sys/sound.cc]
---

## Overview

The communication system provides multiple channels for player interaction, each with distinct ranges, restrictions, and behaviors. Speech can be transformed by garble effects, blocked by soundproof zones, filtered through ignore lists, and intercepted by mob AI response systems.

Communication divides into verbal channels (say, tell, shout, grouptell, whisper, ask), non-verbal channels (sign, emote), immortal channels (commune), and predefined socials. Each channel validates speaker capabilities, applies environmental restrictions, transforms messages through garble filters, and delivers to appropriate recipients.

The system enforces position requirements, mute conditions, silence effects, soundproof zones, and recipient filtering. Messages trigger mob AI responses and integrate with GMCP for client notifications.

## Patterns

### Channel Implementation Pattern

Communication commands follow a consistent validation and distribution flow implemented in talk.cc. Each command checks speaker eligibility through canSpeak or channel-specific restrictions, applies garble transformations based on speaker and recipient conditions, filters recipients by ignore lists and environmental factors, then delivers messages with appropriate formatting and side effects.

The say command checks applySoundproof for ROOM_SILENCE flags, validates speaker with canSpeak checks, garbles the message individually for each recipient, delivers to all awake room occupants, and triggers mob AI responses via checkResponses. The tell command validates both sender and receiver conditions, checks target availability and descriptor state, records conversation history in last_teller and last_told fields, logs to the tellhistory database table, and handles account-based character lookup for immortals.

Group communication requires AFF_GROUP flags on both sender and all recipients sharing the same master pointer. Distribution iterates the leader's followers list, applying individual garble transformations and skipping soundproof or ignoring recipients.

### Restriction Layering Pattern

Communication restrictions stack from multiple sources checked in sequence. The canSpeak function aggregates AFF_SILENT affects, ROOM_SILENCE room flags, isDumbAnimal race checks, TOG_IS_MUTE quest bits, and disease effects like DISEASE_GARROTTE. Commands check position requirements before delivery and filter out sleeping, stunned, or editing recipients.

Soundproof rooms block local verbal communication (say, whisper, ask, emote, grouptell, shout) for non-immortals through applySoundproof. The ROOM_SILENCE flag prevents sound generation at the source, while individual commands may also block delivery to recipients in silent zones. Tell bypasses soundproof checks as global communication.

The mute condition blocks most verbal communication but allows tells specifically to immortal recipients. Silent affects trigger visual feedback showing the character pointing at their mouth without sound.

### Garble Transformation Pattern

Speech transforms through multiple garble types applied at different scopes. Some garbles affect everyone receiving the message identically (SCOPE_EVERYONE for drunk, ghost, underwater), some apply individually per recipient (SCOPE_INDIVIDUAL for language comprehension, profanity filters), and some include the speaker themselves (SCOPE_EVERYONEANDSELF).

Each channel applies garble transformations before delivery. Say and grouptell garble messages individually for each recipient in garbleRoom or garbleGroupTell, allowing different recipients to perceive the same utterance differently based on their language skills or filters. Shout applies TYPE_BLAHBLAH whining sounds only to shouts, while TYPE_GLUBGLUB underwater garble affects all verbal communication.

Profanity filtering operates bidirectionally with TYPE_PG13OUT transforming outgoing speech from speakers with the filter enabled and TYPE_PG13IN transforming incoming speech for recipients with the filter enabled.

### Ignore and Privacy Pattern

The ignoreList class provides character and account-level filtering stored on descriptors. Commands check ignored.isIgnored before delivering messages, silently dropping communications from blocked senders. Tell commands succeed from the sender's perspective even when ignored, preventing senders from detecting ignore status.

The AUTO_NOTELL autobit implements selective tell blocking, accepting only from the last person the character told to. This allows reply chains while blocking unsolicited tells. The AUTO_NOSHOUT autobit filters broadcast shouts globally.

Whisper implements eavesdropping through SKILL_SPY, allowing skilled characters to overhear private conversations if their level matches or exceeds the speaker's level and no participant is immortal.

### Social Action Pattern

Predefined socials loaded from lib/actions provide structured emotes with actor, target, and observer messaging variants. Each social defines messages for no-target, found-target, self-target, and not-found cases. The hide flag controls room visibility when targets are present, and minPos enforces target position requirements.

Socials check fighting and riding states to allow facial expressions while blocking physical actions. Some socials trigger MSP sound effects for audio feedback. All socials invoke mob AI through aiSocialSwitch with target type classification.

The doAction implementation looks up social definitions by command type, validates restrictions, resolves targets, selects appropriate message variants, and broadcasts to room occupants with act formatting.

## Reference

### Communication Channels

**doSay** - Room-local verbal communication requiring speaker to pass canSpeak validation and not be in soundproof room. Each awake recipient receives individually garbled message. Mob AI responds via checkResponses. Alias: apostrophe.

**doTell** - Global private messaging checking AFF_SILENT, applySoundproof for non-immortals, isDumbAnimal, pet/charm status, PLR_GODNOSHOUT flag, target's AUTO_NOTELL autobit, TOG_IS_MUTE, target position (blocks sleeping/stunned), target descriptor state (blocks editing/mailing), and target soundproof for non-immortals. Records last_teller and last_told for reply. Logs to tellhistory table. Shows AFK warnings. Immortals resolve alternate characters on same account.

**doShout** - Global broadcast requiring level 2 or higher, costing 15 movement points, adding 0.5 combat round wait. Blocked for charmed characters, by PLR_GODNOSHOUT flag, filtered by AUTO_NOSHOUT on recipients. Posts to Twitter via twitterShout. Global toggle via toggleInfo[TOG_SHOUTING]. Distributed through sendShout iterating CON_PLYNG descriptors checking awake status and ignore lists.

**doGrouptell** - Group broadcast requiring AFF_GROUP on sender and all recipients sharing same master. Distributes to leader if not self, then iterates followers with AFF_GROUP, applying individual garble transformations, skipping ignored or soundproof recipients.

**doWhisper** - Private room communication allowing SKILL_SPY eavesdropping when spy level equals or exceeds speaker level and no immortals participate.

**doAsk** - Room-local questioning with same restrictions as say.

**doSign** - Silent visual communication requiring hands (not transformed arms/hands), neither arm hurt, no held items, not fighting, standing position. Recipients without SKILL_SIGN see generic hand motion message. Thieves hide their signing attempts from unskilled observers.

**doEmote** - Free-form action descriptions blocked by checkSoundproof, requiring awake or PC status, blocked by PLR_GODNOSHOUT and TOG_IS_MUTE, checking drunk level against constitution. Format: character name followed by emote text. Aliases: colon, comma.

**doCommune** - Immortal channel requiring POWER_WIZNET. Level targeting via at-sign prefix sends only to gods at or above specified level.

**doReply** - Responds to last_teller stored on descriptor. Wraps doTell with cached target.

### Restriction Functions

**canSpeak** - Aggregates AFF_SILENT affect, checkSoundproof, isDumbAnimal, TOG_IS_MUTE quest bit, and disease checks for DISEASE_GARROTTE, DISEASE_SUFFOCATE, DISEASE_DROWNING. Returns false if any restriction applies.

**applySoundproof** - Checks ROOM_SILENCE room flag, sends failure message to non-immortals, returns true to block communication. Immortals bypass check.

**checkSoundproof** - Tests ROOM_SILENCE flag on current room without immortal bypass or messaging.

**isDumbAnimal** - Race or polymorph form check preventing verbal communication.

### Ignore System

**ignoreList::isIgnored(Descriptor*)** - Checks descriptor against ignore list.

**ignoreList::isIgnored(sstring)** - Checks character name against ignore list.

**ignoreList::isMailIgnored(Descriptor*, sstring)** - Static mail-specific ignore check.

**ignoreList::add(Descriptor*)** - Adds descriptor's character to ignore list.

**ignoreList::add(sstring)** - Adds character by name to ignore list.

**ignoreList::add(TAccount&)** - Adds entire account to ignore list.

**ignoreList::addAccount(sstring)** - Adds account by name to ignore list.

**ignoreList::remove(Descriptor*)** - Removes descriptor's character from ignore list.

**ignoreList::remove(sstring)** - Removes character by name from ignore list.

**ignoreList::removeAccount(sstring)** - Removes account from ignore list.

### Garble Types

**TYPE_DRUNK** - Slurs words, applies to everyone including self.

**TYPE_GHOST** - Ghostly speech transformation for everyone.

**TYPE_WAHWAH** - Crying sounds for everyone.

**TYPE_BLAHBLAH** - Whining sounds applied only to shouts.

**TYPE_GLUBGLUB** - Underwater garble for everyone.

**TYPE_PG13IN** - Profanity filter for receiving individual.

**TYPE_PG13OUT** - Profanity filter applied to sender's output for everyone.

**TYPE_SIGN** - Sign language distortion for everyone.

**TYPE_PIRATE** - Pirate accent for everyone.

**TYPE_VAMPIRE** - Vampire speech patterns for everyone.

**TYPE_IGOR** - Igor-style lisp for everyone.

**TYPE_IRISH** - Irish accent for everyone.

**TYPE_TROLLTALK** - Troll dialect comprehension per individual.

**TYPE_FROGTALK** - Frog speech comprehension per individual.

**TYPE_BIRDTALK** - Bird chirps comprehension per individual.

### Speech Types

**SPEECH_SAY** - Say command garble context.

**SPEECH_ASK** - Ask command garble context.

**SPEECH_WHISPER** - Whisper command garble context.

**SPEECH_SHOUT** - Shout command garble context.

**SPEECH_TELL** - Tell command garble context.

**SPEECH_GROUPTELL** - Grouptell command garble context.

**SPEECH_SIGN** - Sign command garble context.

**SPEECH_EMOTE** - Emote command garble context.

### Social Structure

**socialMessg::hide** - Hides social from room observers when target is present.

**socialMessg::minPos** - Minimum position required for target.

**socialMessg::char_no_arg** - Message to actor when no target specified.

**socialMessg::others_no_arg** - Message to room when no target specified.

**socialMessg::char_found** - Message to actor when target found.

**socialMessg::others_found** - Message to room when target found.

**socialMessg::vict_found** - Message to target.

**socialMessg::not_found** - Message when target not found.

**socialMessg::char_auto** - Message to actor for self-targeting.

**socialMessg::others_auto** - Message to room for self-targeting.

### Social Actions

**bootSocialMessages** - Loads social definitions from lib/actions file at boot.

**doAction** - Executes social command by looking up definition, validating restrictions (fighting allows facial expressions only, riding allows subset), resolving targets, selecting message variants, broadcasting to room, triggering MSP sounds for yawn, giggle, burp, clap, fart, sneeze, cackle, scream, chortle, disagree, woo, and invoking mob AI via aiSocialSwitch.

**aiSocialSwitch** - Mob AI response to social actions with TARGET_NONE, TARGET_SELF, TARGET_MOB, or TARGET_OTHER classification.

### GMCP Integration

Communication commands send comm.channel GMCP messages containing channel name (say, mobsay, tell, mobtell, yell, gtell, wiz), message text, and player name. Delivered via sendGmcp to client descriptors for notification handling.

### Position Requirements

Dead and stunned positions block all communication. Sleeping blocks sending and receiving most communication including tells. Resting allows say, tell, shout, grouptell, whisper, ask, emote. Standing required for sign command.

### Flags and Toggles

**ROOM_SILENCE** - Room flag blocking local verbal communication for non-immortals.

**AFF_SILENT** - Affect flag imposing magical silence with visual feedback.

**TOG_IS_MUTE** - Quest bit representing permanent throat damage.

**PLR_GODNOSHOUT** - Player flag imposing god-level communication ban.

**AUTO_NOTELL** - Autobit blocking tells except from last told recipient.

**AUTO_NOSHOUT** - Autobit filtering incoming shouts.

**AFF_GROUP** - Affect flag required for grouptell sender and recipients.

## Implementation

### Message Distribution Architecture

Communication commands implement three distribution patterns: room broadcast, group iteration, and global descriptor scan. Room broadcast delivers to all awake occupants in the current room by iterating room->people. Group iteration walks the master's followers list checking AFF_GROUP flags. Global distribution scans Descriptor::descriptor_list checking connection state, position, and autobit filters.

Each pattern applies garble transformations at different points. Room broadcast garbles individually per recipient in the delivery loop. Group iteration garbles per follower. Global distribution garbles once before scanning or per recipient depending on garble scope.

The act function handles message formatting with actor, target, and observer variants, substituting name, pronoun, and object references. Communication commands construct act format strings with color codes and call act with TO_ROOM, TO_VICT, or TO_CHAR flags.

### Garble Transformation Pipeline

The garble system transforms messages through a pipeline of active garble types. Each speech type (SPEECH_SAY, SPEECH_SHOUT, etc.) maps to a set of applicable garble transformations. The speaker's conditions (drunk level, underwater, polymorphed) and recipient's conditions (language skills, profanity filters) determine which garbles apply.

Garble functions take the original message, speaker, recipient, and speech type, returning the transformed message. SCOPE_EVERYONE garbles apply once with the result sent to all recipients. SCOPE_INDIVIDUAL garbles apply per recipient with different transformations for each. SCOPE_EVERYONEANDSELF garbles apply to everyone including what the speaker sees themselves saying.

The drunk garble checks getCond(DRUNK) and applies progressive slurring. The language garbles check skill levels for comprehension. The profanity filter scans for banned words and replaces with sanitized versions.

### Ignore List Storage

The ignoreList class stores ignored character names and account IDs in parallel structures. Character-level ignores match descriptor->character->getName(). Account-level ignores match descriptor->account.name. Both checks apply when filtering communications.

Static ignored lists persist on descriptors, loaded from database on login and saved on logout. The isMailIgnored static method provides mail-specific filtering without requiring a descriptor instance.

Adding ignores immediately affects subsequent communication attempts. Removing ignores requires exact name or account match. Account ignores block all characters on the account.

### Social Definition Storage

Social definitions load from lib/actions into a static array indexed by command type. Each social command (CMD_APPLAUD, CMD_KISS, etc.) maps to an array index containing the socialMessg structure.

The file format uses multi-line entries with tildes terminating each message string. The parser reads command name, hide flag, minimum position, then the eight message variants in order. Missing entries default to null.

At runtime, doAction indexes the array by command minus CMD_APPLAUD offset to retrieve the definition. The hide flag controls whether observers see the action when targeting another character. Position validation ensures targets meet minimum position requirements.

### Spy Eavesdropping Mechanism

Whisper command iterates room occupants checking for active SKILL_SPY. The spy check compares skill level to speaker level, rejecting immortal participants, and delivers the whispered message to qualifying spies. Spies see a special format indicating eavesdropped content.

The implementation walks room->people, skips speaker and target, checks spy skill activation, validates level requirements, and sends the whispered message with eavesdrop formatting. Spies must be awake and not ignoring the speaker.

### Tell History Logging

Tell commands log to the tellhistory database table with timestamp, sender name, receiver name, and message content. The logging occurs after successful delivery, capturing the garbled message text as delivered.

Database fields: time (timestamp), teller (sender name), tellee (receiver name), message (garbled text). Immortals can query tell history for investigation purposes. The table supports account-based queries for alternate character detection.

### Descriptor State Filtering

Tell delivery checks descriptor state flags to prevent interrupting editing sessions, mail composition, or bug reporting. The checks examine desc->connected for CON_PLYNG state and desc->str for active string editing. Recipients in incompatible states receive queued or dropped messages.

The descriptor field prompt_d.type indicates prompt mode. Client prompts may bypass some restrictions. The m_bIsClient flag enables client-specific message formats through clientf instead of sendTo.

### Mob AI Response Triggering

Say command invokes checkResponses on all room mobs, passing the speaker, garbled message, and CMD_SAY command type. Mob AI handlers receive the command and can react with speech, actions, or combat initiation. The return value can include DELETE_THIS or DELETE_VICT flags.

Response processing must cache next pointers before invoking AI to handle mob deletion. The doSay implementation checks for DELETE_VICT indicating speaker death from mob reaction and returns DELETE_THIS to propagate to caller.

### Soundproof Room Mechanics

The ROOM_SILENCE flag set on rooms prevents sound generation and transmission. Local verbal commands call applySoundproof early in execution, failing before message construction. The flag affects say, whisper, ask, emote, grouptell in sender's room, and shout despite being global.

Tell bypasses soundproof checks as conceptual direct mental communication. Immortals bypass all soundproof restrictions. The checkSoundproof function provides boolean check without messaging for conditional logic.

## Troubleshooting

### Messages Not Reaching Recipients

Check ignore lists first by verifying ignored.isIgnored returns false for the sender. Confirm recipient position is not sleeping or stunned for most communications. Verify recipient descriptor state is CON_PLYNG and not editing or mailing. Check AUTO_NOTELL autobit on recipient and ensure sender is last_told. For grouptell, verify both sender and recipient have AFF_GROUP set and share the same master pointer.

### Soundproof Blocking Unintended Commands

ROOM_SILENCE blocks local verbal communication but not tell. If tell appears blocked, check recipient's room soundproof status separately. For grouptell, sender's soundproof blocks transmission even though group members are remote. Immortal status bypasses all soundproof checks.

### Garble Not Applying

Verify speech type matches channel expectations. Check garble scope - SCOPE_INDIVIDUAL garbles apply per recipient while SCOPE_EVERYONE garbles apply once. Confirm speaker conditions trigger garble (drunk level, underwater, polymorph). For language garbles, check recipient skill levels for comprehension.

### Social Restrictions During Combat

Fighting blocks physical socials but allows facial expressions. The allowed list includes wink, boggle, nod, smile, grin, laugh, frown, and similar facial actions. Physical socials like dance, wiggle, bow, curtsey fail during combat. Riding allows intermediate set including hug, comfort, pat, wave, point.

### Spy Not Overhearing Whispers

Spy eavesdropping requires active SKILL_SPY, spy level equal or greater than speaker level, and no immortal participants. Check spy skill activation status. Verify level comparison. Ensure speaker, target, and spy are not immortal.

### Tell History Not Logging

Database logging occurs after successful delivery. Check database connectivity and tellhistory table existence. Verify message was not blocked by ignore, AUTO_NOTELL, or descriptor state. Check transaction commit if logging wrapped in transaction.

### DELETE Flag Propagation

Communication commands return DELETE flags from mob AI responses. If speaker dies from mob reaction to say, checkResponses returns DELETE_VICT which doSay translates to DELETE_THIS for return to caller. Caller must check IS_SET_DELETE and handle speaker deletion. Missing checks cause use-after-free when continuing execution after death.

### Mute Condition Bypasses

Mute blocks most communication but allows tells specifically to immortal recipients. Check recipient isImmortal status. Verify TOG_IS_MUTE quest bit set correctly. Confirm command checks mute condition before delivery.

### Group Communication Failing

Grouptell requires AFF_GROUP on sender and all recipients, shared master pointer, and absence of soundproof or ignore blocks. Check each condition independently. Verify master pointer consistency across group. Confirm AFF_GROUP flag set, not just follower relationship.

### GMCP Messages Not Sending

GMCP integration requires client support and enabled GMCP protocol negotiation. Check descriptor GMCP state. Verify sendGmcp call format matches comm.channel structure. Confirm client parsing GMCP messages correctly.
