# BUG_PARTY_REQUEST - Mu Helper Party Request Mode

## Current Status

**Resolved — pending final verification.**

Fixed in commit `c5c7203d` ("feat(muhelper): implement local party/social request modes & align
config logic with Sven"). The root cause was that the request-mode fields lived inside
`MUHelper::ConfigData`, the same struct used to build the server `MuHelperSaveData` packet. Moving
them out of `ConfigData` into client-only state restored the original Play/Save behavior.

Andres rebuilt the client in Visual Studio 2022 after the commit and the Play button starts Mu Helper
again. One hardening fix remains (uninitialized member defaults — see "Remaining Fix Plan").

The original repro (now passing):

1. Open Mu Helper.
2. Set Party request handling to `On`.
3. Save the Mu Helper settings.
4. Press Play.
5. Mu Helper starts.

The client is built and tested by Andres with Visual Studio 2022.

See "Root Cause and Resolution" below for the full analysis; the historical investigation notes are
kept further down for context.

## Root Cause and Resolution

### What we found

The investigation (reading the committed baseline vs. the working tree) narrowed the regression to a
single architectural mistake, not the request handlers and not the UI:

- `MuHelperData.cpp` (the `ConfigDataSerDe::Serialize` / `Deserialize` that build the server
  `MuHelperSaveData` packet) was **byte-identical** to the last working commit — `git diff` showed
  only a blank line removed. The request modes were never written into the packet.
- In the working baseline, the old `bAutoAcceptFriend` / `bAutoAcceptGuild` booleans also lived in
  `ConfigData` but were **never serialized** — they were already effectively client-local.
- The feature replaced those booleans with `byFriendRequestMode` / `byGuildRequestMode` /
  `byPartyRequestMode` **inside `ConfigData`**, and added save-path logic
  (`HasServerConfigChanged()`, `_LastServerConfig`, `_HasLastServerConfig`) in
  `CNewUIMuHelper::SaveConfig()`.

The problem: `ConfigData` is the server save/load model. Mixing client-only request state into it
coupled the local request settings to the Mu Helper packet/start path. Keeping that state in the
shared struct is what made the Save/Play flow fragile and is the documented "high risk" called out in
the original Working Theory below.

### The fix that worked (commit `c5c7203d`)

Request modes were **decoupled from `ConfigData`** and moved into client-only state:

- Removed `byFriendRequestMode` / `byGuildRequestMode` / `byPartyRequestMode` from
  `MUHelper::ConfigData` (`src/source/MUHelper/MuHelperData.h`). `ConfigData` now ends at
  `bUseSelfDefense`, so `Serialize`/`Deserialize` and the `MuHelperSaveData` packet are identical to
  the known-good baseline.
- Added member variables on the UI class instead
  (`src/source/UI/NewUI/NewUIMuHelper.h`): `m_byFriendRequestMode`, `m_byGuildRequestMode`,
  `m_byPartyRequestMode`.
- `LoadRequestModesFromConfig()` / `SaveRequestModesToConfig()` now read/write those members against
  `GameConfig` (local INI), with no `ConfigData` involvement.
- `ApplyRequestModeBoxStates()` and `ApplyFriend/Guild/PartyRequestMode()` read/write the member
  variables.
- The incoming-request handlers in `WSclient.cpp` (`HandlePartyRequestByMuHelperMode`,
  `HandleFriendRequestByMuHelperMode`, `HandleGuildRequestByMuHelperMode`) read `GameConfig`
  directly — unchanged and correct.

Note: the `SaveConfig()` skip-logic (`HasServerConfigChanged` / `_LastServerConfig` /
`g_MuHelper.Load` branch, and the `<cstring>` include) was **kept** and is now correct and harmless.
With the modes out of `ConfigData`, that comparison only ever sends the server packet when a real
server-relevant field changed, and never fires for a request-mode-only change. It is not dead code.

### What was tested

- Static review of the full Save/Play path: `Toggle()` → `TriggerStart()` →
  `SendMuHelperStatusChangeRequest(0)` → server → `ReceiveMuHelperStatusUpdate()` → `Start()`. This
  path does not depend on the config save, confirming the regression was in the save-side coupling.
- Confirmed `MuHelperData.cpp` serialization is unchanged vs. the working baseline (server packet not
  corrupted by current code).
- Confirmed the new `Send*Response` functions used by the receive handlers exist with matching
  signatures (build is not broken by missing symbols).
- Andres rebuilt in Visual Studio 2022 after `c5c7203d`; the original repro (Party `On` → Save →
  Play) now starts Mu Helper.

### Versions / commits

- **`c5c7203d`** — "feat(muhelper): implement local party/social request modes & align config logic
  with Sven". The fix commit. Decouples request modes from `ConfigData`, keeps them in `GameConfig` +
  UI member state. Built and tested locally; **not pushed**.

### Remaining Fix Plan (hardening — uninitialized members)

One latent issue remains. The new member variables in `src/source/UI/NewUI/NewUIMuHelper.h`
(`m_byFriendRequestMode`, `m_byGuildRequestMode`, `m_byPartyRequestMode`) are declared **without
initializers**, and the `CNewUIMuHelper` constructor does not set them. They only get a defined value
once `LoadSavedConfig()` (server config arrives) or `Reset()` runs.

`ApplyRequestModeBoxStates()` reads all three, and each `ApplyFriend/Guild/PartyRequestMode()` reads
the other two while setting only one. If a radio is clicked before the server config arrives, the
other boxes render from indeterminate bytes (cosmetic, not a crash) — but it is undefined behavior and
should be fixed.

**Plan:**

1. Add inline initializers in `src/source/UI/NewUI/NewUIMuHelper.h` (the `m_by*RequestMode`
   declarations):
   ```cpp
   BYTE m_byFriendRequestMode = MUHelper::REQUEST_HANDLING_SHOW;
   BYTE m_byGuildRequestMode  = MUHelper::REQUEST_HANDLING_SHOW;
   BYTE m_byPartyRequestMode  = MUHelper::REQUEST_HANDLING_SHOW;
   ```
   The header already includes `MUHelper/MuHelper.h` → `MuHelperData.h`, so the enum is in scope.
2. Purely additive; cannot change behavior for paths that already set the members. It only removes the
   indeterminate-byte window before the first `LoadSavedConfig()` / `Reset()`.
3. Do **not** remove the `SaveConfig` skip-logic / `HasServerConfigChanged` / `_LastServerConfig` /
   `<cstring>` — they are in active use and correct.

### Verification checklist (manual, VS2022)

1. Rebuild in Visual Studio 2022.
2. Original repro: Mu Helper → Other Settings → Party `On` → Save → Play. Confirm it starts.
3. Re-test each mode after the decoupling:
   - Party `Off` → incoming party invite auto-rejected, no popup.
   - Party `Auto` → auto-accepted, no popup.
   - Party `On` → normal popup.
   - Same matrix for Friend and Guild.
4. Confirm modes persist across a client restart (they live in `GameConfig` INI).

## Intended Behavior

The Other Settings tab should handle friend, guild member, and party requests with the same three
modes:

- `On`: show the normal request popup, but do not auto accept.
- `Off`: block or reject the request automatically.
- `Auto`: accept the request automatically.

These options are local client behavior. They should not require a server edit and should not change
the server-side Mu Helper configuration packet.

## What We Were Trying to Implement

The original Mu Helper Other Settings tab had simple auto-accept checkboxes for friend and guild
member requests. The goal was to replace that binary behavior with an explicit three-mode request
policy, then extend the same policy to party invites.

The feature was meant to work like this:

- Friend requests, guild member requests, and party invites each get their own mode.
- `On` keeps the original client behavior: show the normal confirmation popup and let the player
  decide manually.
- `Off` automatically rejects the request before the popup is shown.
- `Auto` automatically accepts the request before the popup is shown.
- The setting is local to the client, because request popups are client-side behavior.
- The feature must not require game server changes.
- The feature must not change the server-owned Mu Helper hunting, obtaining, skill, or potion
  configuration packet.

## How It Was Supposed to Work

The intended implementation was split into three layers.

First, the Mu Helper UI would expose three radio-style choices per request type in the Other
Settings tab:

- Friend: `On`, `Off`, `Auto`
- Guild: `On`, `Off`, `Auto`
- Party: `On`, `Off`, `Auto`

Second, the selected modes would be saved in local client config through `GameConfig`, not through
the Mu Helper server packet. That means closing and reopening the client should remember the modes,
but pressing Save in Mu Helper should not send these local-only request choices to the server.

Third, the network receive handlers would read the local mode when an invite/request packet arrives:

- In `On` mode, do nothing special and continue into the original message box path.
- In `Off` mode, send the existing reject response packet immediately and skip the message box.
- In `Auto` mode, send the existing accept response packet immediately and skip the message box.

For party invites specifically, the intended path was:

1. `ReceiveParty()` receives the party invite packet and stores `PartyKey`.
2. `HandlePartyRequestByMuHelperMode(PartyKey)` reads `GameConfig::GetPartyRequestMode()`.
3. If the mode is `Off`, the client calls `SendPartyInviteResponse(false, PartyKey)` and returns.
4. If the mode is `Auto`, the client calls `SendPartyInviteResponse(true, PartyKey)` and returns.
5. If the mode is `On`, the client continues to the original `CPartyMsgBoxLayout` popup.

This design should have made party request handling independent from starting or stopping Mu Helper.
The current bug is that the UI/save side still appears to interfere with the Mu Helper Play flow.

## Observed Behavior

- The UI can display the request mode choices.
- The choices can be saved and loaded from local config.
- Friend, guild, and party request handlers were added client-side.
- After the party request option was added, saving the Mu Helper settings and pressing Play still
  does not start Mu Helper.

## Historical Investigation Notes (pre-fix)

> The sections below describe the state **before** commit `c5c7203d` and the dead-end attempts that
> led to the fix. They are kept for context. Where they say request modes live in `ConfigData` or
> that "Play still does not start", that is the pre-fix situation — superseded by "Root Cause and
> Resolution" above.

## Files Modified or Touched

### `src/source/UI/NewUI/NewUIMuHelper.cpp`

Main Mu Helper UI changes.

Touched code:

- Added request mode checkbox IDs for friend, guild, and party.
- Added request mode layout constants and labels.
- Added local helpers:
  - `ClampRequestMode()`
  - `LoadRequestModesFromConfig()`
  - `SaveRequestModesToConfig()`
  - `HasServerConfigChanged()`
- Added `_LastServerConfig` and `_HasLastServerConfig` to avoid sending a server save when only
  local request modes changed.
- Updated `CNewUIMuHelper::InitCheckBox()` to create three radio-style choices for each request
  type.
- Updated `CNewUIMuHelper::InitText()` to render `Friend`, `Guild`, and `Party` labels.
- Updated `CNewUIMuHelper::ApplyConfigFromCheckbox()` to route the new checkbox IDs.
- Updated `CNewUIMuHelper::Reset()` to default all request modes to `On`.
- Updated `CNewUIMuHelper::LoadSavedConfig()` to load local request modes after receiving the
  server Mu Helper config.
- Updated `CNewUIMuHelper::ApplyConfig()` to apply request mode checkbox states.
- Updated `CNewUIMuHelper::SaveConfig()` to save request modes locally through `GameConfig`.
- Added:
  - `CNewUIMuHelper::ApplyRequestModeBoxStates()`
  - `CNewUIMuHelper::ApplyFriendRequestMode()`
  - `CNewUIMuHelper::ApplyGuildRequestMode()`
  - `CNewUIMuHelper::ApplyPartyRequestMode()`

Outcome:

- The UI side works well enough to save and restore the choices.
- The current failure is that Mu Helper Play still does not start after saving.

### `src/source/UI/NewUI/NewUIMuHelper.h`

Added declarations for the new request mode UI helpers:

- `ApplyRequestModeBoxStates()`
- `ApplyFriendRequestMode()`
- `ApplyGuildRequestMode()`
- `ApplyPartyRequestMode()`

### `src/source/MUHelper/MuHelperData.h`

Changed request mode data in `MUHelper::ConfigData`.

Touched code:

- Added `ERequestHandlingMode`:
  - `REQUEST_HANDLING_SHOW`
  - `REQUEST_HANDLING_BLOCK`
  - `REQUEST_HANDLING_AUTO`
- Replaced the previous friend/guild auto-accept booleans with:
  - `byFriendRequestMode`
  - `byGuildRequestMode`
  - `byPartyRequestMode`

Outcome:

- This made the UI model support three states instead of a single auto-accept checkbox.
- Risk remains because these fields live inside `ConfigData`, which is also used for server Mu Helper
  save/load logic.

### `src/source/MUHelper/MuHelperData.cpp`

Touched during earlier attempts.

Attempted code:

- Tried serializing request modes into `PRECEIVE_MUHELPER_DATA::_UnusedPadding`.
- Later removed that serialization because it broke Mu Helper start.

Current code:

- Request modes are not serialized into the server Mu Helper packet.
- The current diff only removes an empty line.

Outcome:

- Writing to `_UnusedPadding` was a bad path. The server appears to expect those bytes to stay clear,
  or at least the saved blob is not safe to change there.

### `src/source/Data/GameConfig/GameConfigConstants.h`

Added local config keys:

- `CfgSectionMuHelper`
- `CfgKeyFriendRequestMode`
- `CfgKeyGuildRequestMode`
- `CfgKeyPartyRequestMode`
- `CfgDefaultRequestMode`

Outcome:

- Local config now has a place to save request modes outside the server Mu Helper packet.

### `src/source/Data/GameConfig/GameConfig.h`

Added local config accessors and fields:

- `GetFriendRequestMode()`
- `GetGuildRequestMode()`
- `GetPartyRequestMode()`
- `SetFriendRequestMode()`
- `SetGuildRequestMode()`
- `SetPartyRequestMode()`
- `m_friendRequestMode`
- `m_guildRequestMode`
- `m_partyRequestMode`

Outcome:

- Local save/load works for the three request modes.

### `src/source/Data/GameConfig/GameConfig.cpp`

Updated config load/save.

Touched code:

- `GameConfig::Load()` reads the three Mu Helper request modes.
- `GameConfig::Save()` writes the three Mu Helper request modes.
- Added the three request mode setters.

Outcome:

- The selected request mode survives closing and reopening the Mu Helper UI.

### `src/source/Network/Server/WSclient.cpp`

Added request handling behavior for incoming social packets.

Touched code:

- Added `Data/GameConfig/GameConfig.h` include.
- Added `HandlePartyRequestByMuHelperMode()`.
- Updated `ReceiveParty()` to reject, accept, or show the party request based on local mode.
- Added `SOCIAL_REQUEST_REJECTED` and `SOCIAL_REQUEST_ACCEPTED`.
- Added `HandleFriendRequestByMuHelperMode()`.
- Added `HandleGuildRequestByMuHelperMode()`.
- Updated `ReceiveGuild()` to reject, accept, or show the guild request based on local mode.
- Updated `ReceiveRequestAcceptAddFriend()` to reject, accept, or show the friend request based on
  local mode.

Outcome:

- This is where the client-side behavior should happen.
- It should be independent from starting Mu Helper, but the Play regression still happens after the
  UI/save changes.

### `src/source/MUHelper/MuHelper.cpp`

Touched during earlier repair attempts, then reverted.

Attempted code:

- Temporarily loaded/saved request modes from `GameConfig` inside `CMuHelper::Load()` and
  `CMuHelper::Save()`.
- Temporarily changed `CMuHelper::TriggerStart()` to call `Save(m_config)` before sending
  `SendMuHelperStatusChangeRequest(0)`.

Outcome:

- Saving right before Play did not fix the issue and likely interfered with starting.
- These changes were reverted.
- Current `CMuHelper::TriggerStart()` is back to only sending the status change request when the
  hero is not in a safe zone.

### `HOTKEYS.md`

Added a user-facing note that Mu Helper request handling supports:

- `On`
- `Off`
- `Auto`

Outcome:

- Documentation was updated, but the feature is not stable yet because Play still fails after save.

## Attempt History

### Attempt 1: Store modes in the Mu Helper server packet

Changed friend/guild auto-accept from booleans to three-state modes and tried storing those modes in
`PRECEIVE_MUHELPER_DATA::_UnusedPadding`.

Outcome:

- Mu Helper Play stopped working.
- Conclusion: `_UnusedPadding` must not be used for client-only settings.

### Attempt 2: Move modes to local GameConfig

Removed request mode serialization from the server packet and saved the modes to local config
instead.

Outcome:

- Choices could be saved and loaded locally.
- Incoming requests still did not block or auto accept until network receive handlers were added.

### Attempt 3: Add friend and guild request handlers

Added client-side handling in `WSclient.cpp` for friend and guild requests.

Outcome:

- Friend/guild requests now have code paths for show, block, and auto accept.
- This did not address party invites yet.

### Attempt 4: Add party request mode

Added Party to the Mu Helper UI, local config, temporary config data, and `ReceiveParty()`.

Outcome:

- Party mode saves locally.
- User reported that setting Party to `On`, saving, then pressing Play still does not start Mu
  Helper.

### Attempt 5: Save clean config before Play

Temporarily changed `CMuHelper::TriggerStart()` to save the Mu Helper config before sending the
start request.

Outcome:

- Did not fix the problem.
- The change was removed because it could race or interfere with the normal Play/start flow.

### Attempt 6: Avoid server save when only local request modes change

Added serialized server config comparison in `CNewUIMuHelper::SaveConfig()`:

- Save request modes through `GameConfig`.
- Compare the server-relevant serialized Mu Helper config before calling `g_MuHelper.Save()`.
- If only local request modes changed, call `g_MuHelper.Load(_TempConfig)` instead of sending a
  server save.

Outcome:

- Intended to avoid touching the server Mu Helper packet for local-only request settings.
- User still reports that Play does not start, so this is not confirmed as a full fix.

## Current Working Theory

The request modes are client-only settings, but they were added through `ConfigData`, which is also
used by the Mu Helper server save/load path. That creates a high risk of accidentally touching the
server helper packet or changing the save/start sequence.

The party `On` mode should behave like the original client behavior, so if `On` plus Save breaks
Play, the likely trigger is the Mu Helper save path, not the party request handler itself.

There may also be a stale server-side Mu Helper config blob from the earlier `_UnusedPadding`
attempt. Since the server cannot be edited directly, any cleanup must happen safely from the client
without blocking or racing the Play request.

## Known Bad Paths

- Do not write request modes into `PRECEIVE_MUHELPER_DATA::_UnusedPadding`.
- Do not save Mu Helper server config from `CMuHelper::TriggerStart()` immediately before Play.
- Do not require server-side edits for this feature.

## Suggested Next Debug Steps

1. Add temporary logging around `CNewUIMuHelper::SaveConfig()` to confirm whether `g_MuHelper.Save()`
   is called after changing only request modes.
2. Add temporary logging around `CMuHelper::TriggerStart()` to confirm
   `SendMuHelperStatusChangeRequest(0)` is sent when Play is pressed.
3. Add temporary logging in `ReceiveMuHelperStatusUpdate()` to confirm whether the server answers the
   Play request.
4. Dump or compare the serialized `PRECEIVE_MUHELPER_DATA` bytes before and after saving only request
   mode changes.
5. Consider removing `byFriendRequestMode`, `byGuildRequestMode`, and `byPartyRequestMode` from
   `ConfigData` completely. Store these modes only in `GameConfig` or UI-local state so they cannot
   affect the server Mu Helper config path by accident.
6. After Play works again, retest each request mode:
   - Friend: `On`, `Off`, `Auto`
   - Guild: `On`, `Off`, `Auto`
   - Party: `On`, `Off`, `Auto`
