# Bug Report: PvP Filter Failure in Area Spells (Target Key Fall-through)

**File:** `src/source/Engine/Object/ZzzInterface.cpp`
**Function:** `getTargetCharacterKey(CHARACTER* c, int selected)`
**Location:** Around line 1951
**Severity:** High. Affects party combat and friendly-fire behavior.

## Problem Description

In the C++ client (MuMain), `getTargetCharacterKey` decides whether the player selected under the mouse cursor is a valid attack target. It considers PK state, guild wars, duels, and whether the `CTRL` key is pressed.

Currently, if none of the PvP conditions match, for example when targeting a clean player in a normal map without holding `CTRL`, the function has a fall-through logic issue at the end:

```cpp
    // ... Duel, Gens, Guild, PK, and Ctrl checks ...

    if (gMapManager.IsCursedTemple())
    {
        if (g_CursedTemple->IsPartyMember(selected))
            return -1;
        return sc->Key;
    }

    return sc->Key; // Critical fall-through
}
```

Returning `sc->Key` instead of `-1` makes the client treat an allied player as a valid target and acquire that player's network key.

## Network and Combat Impact

When a player, such as an Elf, Summoner, or Wizard, casts a large area spell like Multi Shot or Lightning Shock on an ally:

1. The client sends a network packet of type **`0x1E` (`SendRequestMagicContinue` / area skill)** to the server.
2. Because `getTargetCharacterKey` returned the ally's key, the packet is sent as a **targeted attack** (`TargetKey = sc->Key`).
3. In older official clients, the function returned `-1`, forcing the client to send the skill as a **ground/no-target attack** (`TargetKey = 0xffff`).

When the server receives an explicit target, it may treat the packet as a client-validated attack and apply direct damage to an allied player, bypassing some normal area-protection rules.

Combined with party desynchronization after relog on the OpenMU server, this can allow allies to damage each other without holding `CTRL`.

## Proposed Client-Side Fix From Original Theory

To restore classic ("vanilla") client behavior, the function should avoid returning the character key by default.

**Replace the final return:**

```diff
-    return sc->Key;
+    return -1;
```

That would make the target resolve to no target (`0xffff`) when the player is not holding `CTRL`, allowing the server to apply its own party-protection rules for untargeted area attacks.

Important: later testing showed this may not be the correct fix for the current relog-order issue. See the sections below.

## Follow-up 2026-05-19: Opposite Case on First Login

The current tested case is the opposite of the initial report:

- First login: the friend/caster cannot hit the player with area skills unless holding `CTRL`.
- After relog: the same caster can hit with those skills without `CTRL`.

Reported skills:

- Elf: Triple Shot / Multi Shot.
- Summoner: Lightning Shock.

### Current Hypothesis

The area-skill flow uses `SendRequestMagicContinue`, which sends a `TargetKey` to the server. For the reviewed skills, that `TargetKey` normally comes from `getTargetCharacterKey(...)` using `g_MovementSkill.m_iTarget`.

If, on first login, the client does not yet have synchronized PvP/guild/party/duel/PK data for the target, `CheckAttack()` or `getTargetCharacterKey(...)` can block the target and the packet can be sent with:

```text
TargetKey = 0xffff
```

That would explain why the skill does not hit without `CTRL` on first login, but works after relog: relog can refresh or reorder viewport, guild, PK, party, or duel state packets.

## Changes Made in `ZzzInterface.cpp`

1. Added debug-only diagnostics for PvP area skills.

   Added helpers near the start of the file:

   - `ShouldReportAreaSkillTarget(int skill)`
   - `WriteAreaSkillTargetLog(const char* message)`
   - `ReportAreaSkillTargetState(int skill, int selected, WORD targetKey)`

   The log covers:

   - `AT_SKILL_TRIPLE_SHOT`
   - `AT_SKILL_TRIPLE_SHOT_STR`
   - `AT_SKILL_TRIPLE_SHOT_MASTERY`
   - `AT_SKILL_MULTI_SHOT`
   - `AT_SKILL_LIGHTNING_SHOCK`
   - `AT_SKILL_LIGHTNING_SHOCK_STR`

2. Moved the diagnostic to the shared `SendRequestMagicContinue(...)` path.

   Reason: all those skills eventually send the area packet through that function, so the trace does not depend on one specific Elf or Summoner branch.

3. Wrote the log as plain text beside `Main.exe`.

   Expected file:

   ```text
   AreaSkillTarget.log
   ```

   Local debug path:

   ```text
   C:\Users\Andres\MuMain\MuMain\out\build\windows-x86\src\Debug\AreaSkillTarget.log
   ```

   Example format:

   ```text
   [AreaSkillTarget] skill=123 selected=5 sentTargetKey=65535 checkAttack=0 ctrl=0 world=0 targetKey=42 targetKind=1 targetType=1163 targetPK=0 targetGuildTeam=0 targetGuildRelation=0 targetGuildMark=-1 targetDead=0 party=0 duelEnemy=0
   ```

   Key values:

   - `sentTargetKey=65535`: the client did not send a real target.
   - `checkAttack=0`: the client thinks the target is not attackable.
   - `ctrl=1`: the caster was holding `CTRL`.
   - `targetPK`, `targetGuildTeam`, `targetGuildRelation`, `party`, `duelEnemy`: state used to decide whether the target is valid.

4. Fixed a real player-check typo.

   Before:

   ```cpp
   sc->Object.Type == KIND_PLAYER
   ```

   After:

   ```cpp
   sc->Object.Kind == KIND_PLAYER
   ```

   `Type` is the object/model id. `Kind` is the field that contains `KIND_PLAYER`, matching the rest of the attack checks in this file.

## First Test Result

`AreaSkillTarget.log` did not appear on the local client during the test. The local `Main.exe` was confirmed to be running from:

```text
C:\Users\Andres\MuMain\MuMain\out\build\windows-x86\src\Debug\Main.exe
```

However, the log is generated on the client that casts the skill. If the friend casts Triple Shot, Multi Shot, or Lightning Shock, the friend needs to run the instrumented `Main.exe` to produce the log.

It was also found that `Main.exe` was open and blocked the final relink:

```text
LINK : fatal error LNK1168: cannot open src\Debug\Main.exe for writing
```

Timestamps at that moment:

```text
ZzzInterface.cpp      2026-05-19 16:11:49
ZzzInterface.cpp.obj  2026-05-19 16:12:09
Main.exe              2026-05-19 15:59:12
```

Conclusion: the `.obj` had the latest changes, but the `.exe` could not be updated until the game was closed.

## Next Check

1. Close all `Main.exe` processes.
2. Rebuild `windows-x86-debug`.
3. Run the new `Main.exe` on the caster client.
4. Reproduce:

   - First login without `CTRL`.
   - First login with `CTRL`.
   - Relog without `CTRL`.

5. Compare the `[AreaSkillTarget]` lines.

If first login shows `sentTargetKey=65535` or `checkAttack=0`, the problem is in the client target/attack decision. If it sends a real `sentTargetKey` but still does no damage, the cause is server-side or in validation after the packet is sent.

## Follow-up 2026-05-19: Decay Test and Relog Order

Another test was done with `Decay` on both sides:

- The Mage used `Decay`.
- The local player also used `Decay`.

Observed result:

1. Mage freshly logged in:
   - Mage can hit the local player with `Decay`.
   - Local player can hit the Mage with `Decay`.

2. Mage relogs:
   - Mage can no longer hit the local player with `Decay`.
   - Local player can still hit the Mage with `Decay`.

3. Local player relogs:
   - Local player can no longer hit the Mage with `Decay`.

### Interpretation

The failure follows the client that just relogged. It does not appear to depend only on class or on one Elf/Summoner-specific skill, because it also reproduces with `Decay`.

This changes the main hypothesis:

- Earlier it looked like a specific issue with area skills such as Triple Shot, Multi Shot, or Lightning Shock.
- Now it looks like a PvP/target state issue after relog.

The client that just entered probably does not rebuild or receive some state needed to consider an already-online player attackable, for example:

- PK state.
- Guild relation / guild team.
- Party state.
- Duel state.
- Viewport player creation/update order.

Code points to review after this test:

- `ReceiveCreatePlayerViewportExtended(...)`
- `ReceiveGuildIDViewport(...)`
- `ReceivePK(...)`
- party / duel / guild relation packets
- `CheckAttack()`
- `getTargetCharacterKey(...)`

Important warning: the earlier proposal to change the final `return sc->Key` to `return -1` may not fix this specific case. For the current bug, where the freshly relogged client cannot hit without `CTRL`, returning fewer target keys may make the symptom worse. First confirm whether the relogged client is losing state or whether the server rejects the attack even when the client sends a valid target.

## Follow-up 2026-05-19: Extra Logging Changes

Logging was expanded to cover the `Decay` test and viewport/network state.

### `AreaSkillTarget.log`

The area-skill log now also covers:

- `AT_SKILL_DECAY`
- `AT_SKILL_DECAY_STR`

This allows comparing packets sent when both players use `Decay`.

### `PvpViewportState.log`

Added a second plain-text log beside `Main.exe`:

```text
PvpViewportState.log
```

This log is written from:

- `ReceiveCreatePlayerViewportExtended(...)`
- `ReceivePK(...)`
- `ReceiveGuildIDViewport(...)`

Example format:

```text
[PvpViewportState] event=create-player-viewport key=123 index=4 isHero=0 world=0 pk=0 guildTeam=0 guildRelation=0 guildStatus=-1 guildType=0 guildMark=-1 gens=0 dead=0 objectKind=1 objectType=1163 party=0 duelEnemy=0
```

Key values to compare after relog:

- `event=create-player-viewport`: initial state when the player enters the viewport.
- `event=pk-update`: whether a PK update arrives afterward.
- `event=guild-viewport`: whether a guild/relation/team update arrives afterward.
- `pk`, `guildTeam`, `guildRelation`, `guildMark`, `party`, `duelEnemy`: data used by `CheckAttack()` and `getTargetCharacterKey(...)`.

Compare these two clients:

1. The client that just relogged and cannot hit.
2. The client that did not relog and can still hit.

If the relogged client shows default values or missing `pk-update` / `guild-viewport` events, the problem is viewport/network state synchronization. If the values are correct but `AreaSkillTarget.log` shows `checkAttack=0` or `sentTargetKey=65535`, the problem is client target/attack logic. If the packet goes out with a valid target and still does no damage, validation is probably server-side.

## Follow-up 2026-05-19: Logs With Decay and Elf/Multi Shot

After expanding the logs, another sequence was tested:

- Initial login: nobody could hit anybody with `Decay`.
- Then the player switched to Elf.
- With Elf, the player could hit.

### `AreaSkillTarget.log`

Most `Decay` casts were logged like this:

```text
[AreaSkillTarget] skill=38 selected=-1 sentTargetKey=65535 checkAttack=0 ctrl=0 world=3 targetKey=-1 targetKind=-1 targetType=-1 targetPK=-1 targetGuildTeam=-1 targetGuildRelation=-1 targetGuildMark=-1 targetDead=-1 party=0 duelEnemy=0
```

Interpretation:

- `skill=38` is `AT_SKILL_DECAY`.
- `selected=-1` means the client had no selected character target.
- `sentTargetKey=65535` equals `0xffff`, meaning no real target.
- `checkAttack=0` confirms the client does not consider any target attackable at that moment.

Conclusion for `Decay`: the immediate problem is that the client casts it as a ground/no-target skill. If the server requires a valid `TargetKey` to apply PvP area damage, it will not deal damage.

Then Elf/Multi Shot lines appeared:

```text
[AreaSkillTarget] skill=235 selected=2 sentTargetKey=1590 checkAttack=0 ctrl=0 world=3 targetKey=1590 targetKind=1 targetType=1162 targetPK=0 targetGuildTeam=0 targetGuildRelation=0 targetGuildMark=-1 targetDead=0 party=0 duelEnemy=1
```

Interpretation:

- `skill=235` is `AT_SKILL_MULTI_SHOT`.
- `selected=2` means the client did have a player selected.
- `sentTargetKey=1590` means the packet was sent with a real target.
- `duelEnemy=1` means the client recognizes that target as a duel enemy.

Conclusion for Elf/Multi Shot: when the client manages to select the target and send `TargetKey`, the hit works.

### `PvpViewportState.log`

The log mostly showed:

```text
event=create-player-viewport
```

No `pk-update` or `guild-viewport` appeared in that sample.

Observed states:

```text
guildTeam=0
guildRelation=0
guildStatus=255
guildMark=-1
```

It was also observed that `duelEnemy` changes between entries in the same type of flow:

```text
duelEnemy=1
duelEnemy=0
```

Interpretation:

- The client is creating/recreating viewport players.
- There is no evidence in that test that PK/guild updates arrive afterward.
- Duel/target state can differ depending on login, relog, or character-switch order.
- This reinforces that the problem follows the client that just entered or changed state.

## Consolidated Current Hypothesis

The new client appears to lose or fail to rebuild information needed for target/PvP when it receives `ReceiveCreatePlayerViewportExtended(...)`, relogs, or changes character.

Additional reproduction clue:

- The player who logs in first appears to become immune to the other player's PvP area-skill damage.
- After the other player relogs, the immunity/attackability relationship changes with login order.
- This reinforces that the bug is tied to viewport/login-order state, not to one specific class or damage formula.
- Area-skill PvP damage appears to depend on cursor target acquisition for several skills: if the cursor is not over the other player, the client often sends no target key and PvP damage does not apply.

Dark Wizard first-login observations:

| Skill | Cursor not over player | Cursor over player | Notes |
|-------|-------------------------|--------------------|-------|
| Decay | Hits | Hits | Can apply PvP damage with or without cursor target in this scenario. |
| Frost | Hits | Hits | Can apply PvP damage with or without cursor target in this scenario. |
| Nova | Does not hit | Hits | Appears to require cursor target / selected player for PvP damage. |
| Inferno | Does not hit | Does not hit | Does not apply PvP damage in this scenario. |

This suggests the skills do not all share the same PvP target rules. Some behave like true area damage, some require a selected target key, and some may be blocked by skill-specific server/client rules.

The old client may work because:

1. It preserves/caches guild/duel/PK/target information during viewport recreation.
2. Or it selects PvP targets more permissively for area skills and lets the server validate.

The commented backup/restore block in `ReceiveCreatePlayerViewportExtended(...)` points to the first possibility.

## About the Commented Backup/Restore Block

In `ReceiveCreatePlayerViewportExtended(...)`, the client receives a packet to create or update a visible player. The function calls:

```cpp
CHARACTER* c = CreateCharacter(Key, MODEL_PLAYER, Data->PositionX, Data->PositionY, 0);
```

`CreateCharacter(...)` can call `CreateCharacterPointer(...)`, which resets character fields to default values.

Fields that can be lost during that reset:

```text
GuildMarkIndex
GuildStatus
GuildType
GuildRelationShip
GuildTeam
EtcPart
CtlCode
PK
```

The backup/restore idea is:

1. Before `CreateCharacter(...)`, check whether a live character with the same `Key` already existed.
2. Save social/PvP fields that the current packet does not fully provide.
3. Call `CreateCharacter(...)`, which may reset the object.
4. Restore those fields on the same character if it is confirmed to still be the same `Key`.

This can help if packet order is:

```text
PvP/guild/duel state arrives first
viewport-create arrives later
CreateCharacter resets the state
client loses the ability to select/attack without CTRL
```

### Why the Forum Patch Should Not Be Applied Literally

The forum patch suggests directly uncommenting old blocks. In this codebase, copying it as-is is not safe because:

- The backup block uses `Index` before `Index` exists in the current function.
- It can preserve stale information if the player changed guild, party, duel, or PK state.
- It can restore state onto a reused slot if the same `Key` is not validated.
- It only covers some guild/social fields, not necessarily all state that affects `CheckAttack()`.

## Suggested Client-Side Fix

### Recommended Fix 1: Safe Restore for the Same `Key`

Implement a controlled backup/restore in `ReceiveCreatePlayerViewportExtended(...)`.

Proposed rules:

- Calculate `ExistingIndex = FindCharacterIndex(Key)` before calling `CreateCharacter(...)`.
- Only back up if `ExistingIndex != MAX_CHARACTERS_CLIENT`.
- Only save fields from the character with the same `Key`.
- After `CreateCharacter(...)`, calculate `Index = FindCharacterIndex(Key)`.
- Restore only if:

```text
ExistingIndex != MAX_CHARACTERS_CLIENT
Index != MAX_CHARACTERS_CLIENT
c->Key == Key
CharactersClient[ExistingIndex].Key == Key
```

Candidate fields to restore:

```text
GuildMarkIndex
GuildStatus
GuildType
GuildRelationShip
GuildMasterKillCount
EtcPart
CtlCode only for Hero
```

Fields that need caution:

- `PK`: the viewport packet carries PK in `Data->RotationAndHeroState & 0xf`, so the packet value should generally be used instead of restoring an old value.
- `GuildTeam`: recalculated with `GuildTeam(c)` when the guild viewport packet arrives. If that packet never arrives, restoring it may help, but it can also become stale. Log first.
- `duelEnemy`: comes from `g_DuelMgr`, not directly from the `CHARACTER` struct. If it is missing, duel packets need to be reviewed.

This fix is conservative: it preserves state only when the same player already existed and the create/update resets it.

### Possible Fix 2: Fallback Targeting for PvP Area Skills

If, after the safe restore, `Decay` still logs:

```text
selected=-1
sentTargetKey=65535
```

then review the selection flow for area skills. One option is: for PvP area skills such as `Decay`, if `SelectedCharacter == -1` but the mouse is over a valid player, the client should try to resolve the target before sending `SendRequestMagicContinue(...)`.

This must be done carefully to avoid accidental friendly fire. Priority should be:

1. Resolve a real selection under the cursor.
2. Validate it with existing duel/guild/PK/CTRL rules.
3. Send `TargetKey` only when the target is valid.

### Fix Not Recommended Yet

Do not change the final:

```cpp
return sc->Key;
```

to:

```cpp
return -1;
```

yet.

Reason: the current symptom is that after relog the client cannot hit without `CTRL` because it does not send a valid target or does not select a target at all. Returning fewer keys may make that case worse.

## Recommended Test Plan

1. First test with clean logs:

```text
AreaSkillTarget.log
PvpViewportState.log
PvpSelectionTrace.log
PvpAttackDecision.log
```

2. Compare three moments:

- Clean login with both players.
- Caster relog.
- Character switch to Elf.

3. If the safe restore is implemented, repeat the same test.

Improvement indicators:

- `Decay` no longer logs `selected=-1`.
- `sentTargetKey` changes from `65535` to the real target key when appropriate.
- `duelEnemy`, `guildRelation`, `guildTeam`, or equivalent state is not lost after recreate.
- Hits work without depending on switching to Elf or relogging again.

## Follow-up 2026-05-19: Full Client Trace Set

Additional debug-only traces were added because the issue now depends on login order, cursor position, skill type, and selection state.

New logs beside `Main.exe`:

```text
PvpSelectionTrace.log
PvpAttackDecision.log
```

`PvpSelectionTrace.log` records selected-character changes at the end of `SelectObjects()`. It includes:

- selected character index
- selected NPC/item/operate ids
- current skill
- world
- whether the mouse is on UI
- CTRL state
- selected target key/kind/type/PK/dead state
- party and duel-enemy state

`PvpAttackDecision.log` records decision reasons from:

- `CheckAttack()`
- `getTargetCharacterKey(...)`

It includes:

- decision result
- reason string
- selected target
- target key/kind/type/PK
- guild team/relation/mark
- party state
- duel enabled / duel enemy state
- CTRL/ALT state
- guild-war and soccer flags

The purpose is to answer the remaining open questions:

1. Does Decay fail because the mouse pick never selects the player?
2. Does the client select the player, then `CheckAttack()` rejects it?
3. Does `getTargetCharacterKey(...)` reject it because duel/guild/party state is stale?
4. Does login order change the selection result or only the attack decision?
