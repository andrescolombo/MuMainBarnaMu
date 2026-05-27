# Mu Helper Session Status Task

## UI
- [x] Add a compact two-column floating Mu Helper session panel.
- [x] Use a transparent frame with no border.
- [x] Keep rendering separate from stat tracking.
- [x] Support drag movement and close behavior through the existing NewUI manager.
- [x] Reuse fixed-size text buffers in the refresh/render path.
- [x] Trim visible metrics to the confirmed in-game set only.

## Data tracking
- [x] Add `GameLogic::Helper::SessionStats` as the dedicated session tracker.
- [x] Reset all counters when Mu Helper starts.
- [x] Stop accumulating time when Mu Helper stops.
- [x] Derive session time from active helper milliseconds.
- [x] Use the current map name as Current Spot.
- [ ] TODO: Replace map-only Current Spot with coordinate/spot naming if a named spot source is added.

## Damage tracking
- [x] Track Total Damage.
- [x] Track Normal Damage.
- [x] Track Bonus Damage.
- [x] Add Elemental Damage and Elemental DPS fields.
- [x] Track Average DPS, Peak DPS, Normal DPS, Bonus DPS, and Elemental DPS.
- [x] Use a named rolling DPS window constant.
- [ ] TODO: Elemental damage currently displays `N/A`; the inspected client damage packet exposes damage type flags but no reliable elemental channel.
- [ ] TODO: Bonus damage classification is isolated in `ClassifyCombatDamage` and is based on perfect/excellent/critical/double/combo flags. It may not include every server-side item-option bonus source.

## Economy tracking
- [x] Track EXP gained from EXP/death packets.
- [x] Track EXP / hour from active session time.
- [x] Track Zen picked from Zen pickup events.
- [x] Track Zen / hour from active session time.
- [x] Track jewel-type pickups only.
- [x] Do not track generic items picked.

## Helper-state tracking
- [x] Track kills from the safest client-side monster death/EXP events.
- [x] Track kills / min.
- [x] Track average kill time.
- [x] Track HP potion requests from Mu Helper potion use.
- [x] Track MP potion requests while Mu Helper is active.
- [x] Track idle, stuck, and no-target time with conservative thresholds.
- [x] Track last warning for low durability, no mana, no potions, and disconnected.
- [x] Track last death reason as monster, PK, disconnected, or unknown.
- [ ] TODO: Old EXP packets do not carry an explicit killer id; kill attribution there may include party/shared EXP cases.
- [ ] TODO: MP potion usage is counted from client use requests while Mu Helper is active, not from a server consumption confirmation.
- [ ] TODO: No arrows/bolts and inventory-full warnings need a dedicated helper warning event/source before they can be set reliably.

## Translations
- [x] Add visible label keys to `src/source/Data/Translation/i18n.h`.
- [x] Add English strings to `src/bin/Translations/en/game.json`.

## Testing
- [x] Attempt a Debug build with the existing `out/build/windows-x86` CMake tree.
- [x] Compile the new session tracker, helper/network hooks, NewUISystem integration, and HUD panel object.
- [ ] Full Debug build is still blocked by existing `ZzzOpenglUtil.cpp` `CheckGLError` compile errors.
- [ ] Run a helper session in-game and confirm the panel appears on start.
- [ ] Confirm counters reset on a new helper session.
- [ ] Confirm the panel can be dragged and closed.

## Known limitations
- The requested "Instant Hunting Log" implementation was not found in the inspected source tree, so the panel uses the existing transparent HUD/system-log style instead.
- Current Spot uses the current map name only.
- Elemental damage cannot be reliably classified with the currently inspected client-side damage event.
- Bonus damage is approximate and isolated for future correction.
- Stuck, idle, and no-target time are conservative client-side estimates.
- Hidden/removed UI metrics may still be tracked internally so the event hooks remain simple and reusable.
