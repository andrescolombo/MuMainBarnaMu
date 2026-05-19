# Changelog

## Change Index

1. `f27e4718` - Configure local server defaults
2. `94ec2b9b` - Inventory tooltip comparison + RMB equip fix
3. `4c77a87f` - Fix equipped-item tooltip comparisons
4. `0a43333b` - Show inventory equip-status backgrounds
5. `a062845f` - Dim equip-status backgrounds
6. `38508767` - Try inventory rearrange button
7. `5bbcbf9c` - Compute inventory arrange plan
8. `fc9b8b1a` - Keep arrange item keys in sync
9. `e4a932dd` - Support wing size in inventory arrange
10. `3f4f3062` - Correct minimap click movement direction
11. `9b21610f` - Minimap click movement + docs update
12. `b2039e99` - Spawn markers, Mu Helper hotkeys, arrange expansion
13. `4ebe0c3d` - Admin minimap right-click teleport
14. `b63a3f3a` - Area-skill PvP desync fix + diagnostics
15. `8f85fd3e` - Triple Shot switch break + spawn PK init fix
16. `5582c064` - Repo cleanup for essential tracked files
## Unreleased

- Elf party buffs now work through Mu Helper automation (`src/source/MUHelper/MuHelper.cpp`, `src/source/Engine/Object/ZzzInterface.cpp`, `BUG_ELF_BUFF.md`):
  - Symptom: manually casting Elf support buffs worked, but Mu Helper automation only played the cast animation and sent packets like `SendRequestMagic(27 65535)` / `SendRequestMagic(28 65535)`, so no party member received Defense or Attack buff.
  - Root cause: Mu Helper wrote raw skill IDs into `g_MovementSkill.m_iSkill` even though the downstream magic path expects a learned skill-list index. After that was fixed, the Elf PvP target gate still cleared friendly party targets to `-1`, causing `UseSkillElf()` to send `0xffff` (`65535`) as the target key.
  - Fix: Mu Helper now resolves the skill-list index before executing skills. Elf support skills preserve valid player targets before applying attack-only PvP gates, and `UseSkillElf()` falls back to `SelectedCharacter` for friendly support casts so automated party buffs send the real party member key.

- Elf and Summoner area skills now require CTRL like every other class (`src/source/Engine/Object/ZzzInterface.cpp`, marker `[BUG_CTRL_PVP]`):
  - Symptom: on PvP-allowed maps (Arena, Lorencia, Noria, etc.) with two neutral players (no guild, no party, no PK, no duel), Elf `Multi Shot` / `Triple Shot` and Summoner `Lightning Shock` damaged the target without `CTRL` being held. Dark Knight, Magic Gladiator, Dark Wizard, and Rage Fighter area skills correctly required `CTRL` — only Elf and Summoner bypassed the PvP rule, breaking the standard MU Online expectation that neutral players must explicitly opt into PvP via `CTRL`.
  - Root cause: `AttackRagefighter`, the wall-recovery branch for DK/MG/Dark Lord, both `AT_SKILL_PLASMA_STORM_FENRIR` paths, and the end of `AttackWizard` all gated `g_MovementSkill.m_iTarget = SelectedCharacter` behind `if (CheckAttack())`, forcing `-1` on neutral players (no CTRL) so downstream area skills shipped `TargetKey = 0xFFFF` and the server applied no PvP damage. Two paths were missing this gate: `AttackElf()` set `m_iTarget = SelectedCharacter` unconditionally (Elf bug), and a second unconditional assignment inside `AttackWizard` (Summoner Lightning Shock / Alice skills) did the same.
  - Fix: added the same `if (CheckAttack()) m_iTarget = SelectedCharacter; else m_iTarget = -1;` gate in three places — `AttackElf()` (cursor-target block), the Summoner/Alice dispatch in `AttackWizard` (the second `m_iTarget` assignment), and the Elf branch of the wall-collision recovery path which previously passed through to `SkillElf()` (Triple Shot) without setting `m_iTarget` and thus inherited stale state from prior actions. Elf and Summoner now match every other class: hold `CTRL` to PvP on a normal map, or the area skill resolves to ground-only damage that does nothing to players.
  - Not changed: `getTargetCharacterKey()` itself, all skill send sites, and friendly-only paths (e.g. Teleport Ally, Recovery) that need an unconditional target.

- PvP target state preservation on viewport-create (`src/source/Network/Server/WSclient.cpp`, marker `[BUG_CTRL_PVP]`):
  - Symptom: on first login (or after relog / character switch), area skills like Elf Triple Shot, Multi Shot, and Summoner Lightning Shock could not hit other players without holding `CTRL`. Going out and back into viewport (or another relog after the players moved) restored normal behavior.
  - Root cause: `ReceiveCreatePlayerViewportExtended` calls `CreateCharacter()` → `CreateCharacterPointer()` which unconditionally resets `GuildMarkIndex`, `GuildStatus`, `GuildType`, `GuildRelationShip`, `GuildTeam`, `GuildMasterKillCount`, `EtcPart`, `m_byGensInfluence`, and `CtlCode`. If `ReceiveGuildIDViewport` / Gens packets arrived before the viewport-create for an already-visible player, that populated state was wiped and never resent, so `CheckAttack()` and `getTargetCharacterKey()` saw all neutral defaults and required `CTRL` to recognize the target.
  - Fix: snapshot the previous values for the same `Key` *before* `CreateCharacter`, then restore them after — but only when the new lookup resolves to the same slot (`Index == ExistingIndex && c->Key == Key`) to avoid copying stale state onto a reused slot. `PK` is intentionally not restored because the viewport packet always carries it fresh in `RotationAndHeroState & 0xf`. The same pattern already exists for transformed characters in `ReceiveCreateTransformViewport`.
  - This is a client-side workaround. The proper long-term fix is server-side (OpenMU): bundle `GuildIDViewport` / Gens influence into — or send them immediately after — the viewport-create packet so the client never needs to keep a snapshot. See `BUG_CTRL_PVP.md` for the full analysis, test logs, and diagnostic logging hooks.
  - Known limitation: if a player legitimately changes guild / Gens faction / guild-war team while in your viewport and the server does not re-send the guild packet, the stale state will persist until they leave and re-enter viewport.

- PvP area-skill target diagnostics (`src/source/Engine/Object/ZzzInterface.cpp`):
  - Added plain-text debug logging beside `Main.exe` in `AreaSkillTarget.log` for Triple Shot, Multi Shot, Lightning Shock, and Decay target packets so first-login/relog PvP misses can be checked against the selected target, sent target key, CTRL state, PK, guild, party, and duel state.
  - Hooked the trace at the shared area-skill packet sender so variant skill paths are covered consistently.
  - Added `PvpSelectionTrace.log` for selected character changes and `PvpAttackDecision.log` for `CheckAttack()` / `getTargetCharacterKey()` decision reasons.
  - Corrected a player-kind check in the target-key helper from `Object.Type` to `Object.Kind`, matching the rest of the client-side attack checks.

- PvP viewport-state diagnostics (`src/source/Network/Server/WSclient.cpp`):
  - Added debug-only `PvpViewportState.log` output beside `Main.exe` for player viewport creation, PK updates, and guild viewport updates.
  - Logs the target player's PK, guild team/relation/status/type/mark, Gens influence, party, duel, object kind/type, and world so relog-order PvP state desyncs can be compared.

- BMD invalid bone diagnostics (`src/source/Render/Models/ZzzBMD.cpp`):
  - Added debug-only reporting before the existing `"Bone number error"` assertion in `BMD::TransformByObjectBone`.
  - Logs the requested bone index, model bone count, model name, object type/kind, and caller address to both Visual Studio debug output and `MuError.log`.
  - This makes the runtime assertion actionable by identifying which model/effect is asking for an out-of-range bone.

- Admin minimap right-click teleport (`src/source/UI/NewUI/HUD/NewUIMiniMap.cpp`):
  - Right-clicking anywhere on the open minimap now teleports the character instantly to that tile, but only for admin/GM accounts (`CTLCODE_08OPERATOR` or `CTLCODE_20OPERATOR`). Normal players are unaffected.
  - Uses `SendInstantMoveRequest` to let the server handle the position change.
  - Shows the golden ring move-target effect and plays the click sound for feedback.
  - Admin state is latched on first detection and persists across death/respawn for the session, because `CtlCode` is reset to 0 on character re-initialization but is never re-sent by the server on respawn. The latch also checks `eBuff_GMEffect` as a secondary signal.
  - Added `#include "Network/Server/WSclient.h"` for `SocketClient`.

- Mu Helper hotkeys (`src/source/UI/NewUI/HUD/NewUIHotKey.cpp`):
  - `Z` — opens/closes the Mu Helper configuration panel (was already bound; behavior unchanged).
  - `F8` — starts/stops (play/pause) the Mu Helper without opening the panel (new binding).
  - Added `#include "MUHelper/MuHelper.h"` to `NewUIHotKey.cpp` to access `MUHelper::g_MuHelper.Toggle()`.

- Added an experimental inventory rearrange button that supports all item sizes from `1x1` to `4x4` (16 combinations), computes one fixed compact 8x8 layout, prefers width-aligned lanes for wide items, refreshes the client-only item key after each accepted move, and uses temporary empty slots to keep moving items one at a time after a single click.

- Dimmed inventory equip-status slot overlays:
  - Red slot background still marks items for a different class.
  - Yellow slot background still marks items with missing stat requirements.
  - Overlay alpha was reduced from `0.45f` to `0.12f` to make the color much less saturated.
  - Changed lines: `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp:23`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp:176`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp:180`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp:185`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp:1701`.

### Commit-by-commit summary (since `origin/main`)

1. `f27e4718` (2026-05-18) - Configure local server defaults
   - Added local setup guidance (`REQUIREMENTS.md`) and updated client defaults/config constants for local server connectivity.
   - Files: `REQUIREMENTS.md`, `src/bin/config.ini.template`, `src/source/Data/GameConfig/GameConfigConstants.h`, `src/source/Scenes/SceneCore.cpp`.

2. `94ec2b9b` (2026-05-18) - Inventory tooltip comparison + RMB equip fix
   - Added side-by-side equip tooltip comparison behavior and fixed right-click equip flow.
   - Files: `.gitignore`, `inventory_fixes.md`, `report.md`, `src/CMakeLists.txt`, `src/source/Engine/Object/ZzzInfomation.cpp`, `src/source/Engine/Object/ZzzInventory.cpp`, `src/source/Engine/Object/ZzzInventory.h`, `src/source/Render/Effects/ZzzEffect.cpp`, `src/source/UI/NewUI/Inventory/NewUIInventoryActionController.cpp`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`.

3. `4c77a87f` (2026-05-18) - Fix equipped-item tooltip comparisons
   - Corrected comparison logic/details for equipped inventory tooltips.
   - Files: `inventory_fixes.md`, `src/source/Engine/Object/ZzzInventory.cpp`, `src/source/Engine/Object/ZzzInventory.h`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`.

4. `0a43333b` (2026-05-18) - Show inventory equip-status backgrounds
   - Added visual slot state overlays to indicate equipment usability/status.
   - Files: `inventory_fixes.md`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h`.

5. `a062845f` (2026-05-18) - Dim equip-status backgrounds
   - Reduced overlay intensity/alpha for clearer item readability.
   - Files: `CHANGELOG.md`, `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`.

6. `38508767` (2026-05-19) - Try inventory rearrange button
   - Added initial UI entry point/button wiring for experimental inventory rearrange.
   - Files: `CHANGELOG.md`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.h`.

7. `5bbcbf9c` (2026-05-19) - Compute inventory arrange plan
   - Implemented arrangement planning logic for packing items before applying moves.
   - Files: `CHANGELOG.md`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.h`.

8. `fc9b8b1a` (2026-05-19) - Keep arrange item keys in sync
   - Fixed client-side key synchronization after rearrange moves.
   - Files: `CHANGELOG.md`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`.

9. `e4a932dd` (2026-05-19) - Support wing size in inventory arrange
   - Extended arrange logic to handle wing dimensions correctly.
   - Files: `CHANGELOG.md`, `inventory_fixes.md`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`.

10. `3f4f3062` (2026-05-19) - Correct minimap click movement direction
    - Fixed movement vector/direction handling for minimap click-to-move.
    - Files: `src/source/UI/NewUI/HUD/NewUIMiniMap.cpp`.

11. `9b21610f` (2026-05-19) - Minimap click movement + docs update
    - Added minimap click movement functionality and documented map navigation behavior.
    - Files: `.graphifyignore`, `AGENTS.md`, `LLM.md`, `docs/map_navigation_movement.md`, `src/source/UI/NewUI/HUD/NewUIMiniMap.cpp`, `src/source/UI/NewUI/HUD/NewUIMiniMap.h`.

12. `b2039e99` (2026-05-19) - Spawn markers, Mu Helper hotkeys, arrange expansion
    - Added minimap spawn markers, new Mu Helper hotkey behavior, and expanded arrange support coverage.
    - Files: `CHANGELOG.md`, `HOTKEYS.md`, `TASKS.md`, `src/source/UI/NewUI/HUD/NewUIHotKey.cpp`, `src/source/UI/NewUI/HUD/NewUIMiniMap.cpp`, `src/source/UI/NewUI/HUD/NewUIMiniMap.h`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`.

13. `4ebe0c3d` (2026-05-19) - Admin minimap right-click teleport
    - Added GM/admin-only minimap right-click instant teleport support.
    - Files: `CHANGELOG.md`, `src/source/UI/NewUI/HUD/NewUIMiniMap.cpp`.

14. `b63a3f3a` (2026-05-19) - Area-skill PvP desync fix + diagnostics
    - Fixed client-side area-skill PvP targeting desync behavior and added diagnostics/docs.
    - Files: `BUG_CTRL_PVP.md`, `CHANGELOG.md`, `changes.md`, `docs/SERVER_FIX_BUG_CTRL_PVP.md`, `newui_Bt_arrange.ozt`, `newui_Bt_arrange.tga`, `newui_Bt_arrange_big.ozt`, `newui_Bt_arrange_big.tga`, `newui_Bt_mix.OZT`, `newui_repair_00.OZT`, `newui_repair_00.tga`, `skill_analysis.md`, `src/source/Engine/Object/ZzzInterface.cpp`, `src/source/Network/Server/WSclient.cpp`, `src/source/Render/Models/ZzzBMD.cpp`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`, `src/source/UI/NewUI/Inventory/NewUIMyInventory.h`, `src/source/UI/NewUI/NewUIMuHelper.cpp`.

15. `8f85fd3e` (2026-05-19) - Triple Shot switch break + spawn PK init fix
    - Fixed missing `break` in Triple Shot handling and initialized viewport spawn PK state.
    - Files: `src/source/Engine/Object/ZzzCharacter.cpp`, `src/source/Network/Server/WSclient.cpp`.

16. `5582c064` (2026-05-19) - Repo cleanup for essential tracked files
    - Removed non-essential root-level analysis/assets from tracking and tightened ignore rules.
    - Files: `.gitignore`, `BUG_CTRL_PVP.md`, `changes.md`, `inventory_fixes.md`, `newui_Bt_arrange.ozt`, `newui_Bt_arrange.tga`, `newui_Bt_arrange_big.ozt`, `newui_Bt_arrange_big.tga`, `newui_Bt_mix.OZT`, `newui_repair_00.OZT`, `newui_repair_00.tga`, `report.md`, `skill_analysis.md`.
