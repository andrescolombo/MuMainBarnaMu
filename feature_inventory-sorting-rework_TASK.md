# Feature: Inventory Sorting Rework

## Goal
Rework the existing inventory arrange/rearrange logic in the personal inventory
so it is simpler, deterministic, and provably safe. The button already exists
and sends individual item moves through the existing move-equipment protocol;
the rework only touches client-side planning, validation, and rollback.

## Scope
- Branch: `feature/inventory-sorting-rework` (off `mixed_client`).
- Only files inside `src/source/UI/NewUI/Inventory/NewUIMyInventory.{cpp,h}`.
- Do **not** touch tooltip, graphics, config, networking, or protocol code.
- Do **not** rotate items (client does not support rotation).
- Do **not** merge upstream/friend branches.

## Files / Functions to Inspect
- `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`
  - `IsAllowedArrangeItemSize` — current 1..4 size guard.
  - `IsBetterArrangePlacement` / `FindBestArrangeTarget` — heuristic scoring
    (column lanes / odd column) that we are replacing with a clean first-fit.
  - `ComputeArrangeTargets` — sort + placement entry.
  - `GenerateArrangeMoves` — two-pass placement with blocker eviction.
  - `BuildInventoryRearrangeMoves` — top-level builder; gathers items, calls
    Compute + Generate, validates, returns moves.
  - `RequestInventoryRearrange` / `ProcessInventoryRearrange` /
    `ExecuteNextInventoryRearrangeMove` — wiring with the network move command.
- `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h`
  - `GetNumberOfColumn` / `GetNumberOfRow` / `GetItem` / `GetIndexByItem` —
    grid query API used by the planner.

## Implementation Steps
1. Replace the scored placement heuristic with a deterministic top-left
   first-fit scan: scan rows top→bottom, columns left→right; place at the first
   cell where the item fits. No "column lane" / "odd column" scoring.
2. Use a strict total order for the size sort:
   `area DESC, height DESC, width DESC, sourceIndex ASC`. `sourceIndex` is the
   inventory linear slot and is a stable tiebreaker (more stable than the
   runtime `Key`).
3. Add a dedicated `ValidateArrangeLayout` helper that, after planning,
   reconstructs an occupancy grid from the planned targets and verifies:
   - every item is inside the grid,
   - no two planned cells overlap,
   - every item ended up with a target,
   - total occupied cell count equals the sum of areas.
   If validation fails, `BuildInventoryRearrangeMoves` returns `false` and the
   caller leaves the inventory untouched.
4. Treat an already-sorted inventory (no moves) as a success that simply does
   nothing — do not flag it as failure.
5. Keep the existing two-pass move generator (direct placement first, then
   evict blockers to a temporary slot). It already preserves item identity
   through the live network round-trip via `ReplaceArrangeMoveItemKey`.
6. Keep the per-move runtime guard in `ProcessInventoryRearrange`: if the item
   that just arrived at `targetIndex` is missing or not where expected, abort
   the remaining queue.

## Validation Rules
- All items have width/height in [1..4].
- All source indices are in the inventory grid (>= MAX_EQUIPMENT). Any item in
  an equipment slot aborts the build.
- The planned target rectangle for every item is within `[0..columnCount) x
  [0..rowCount)`.
- Planned rectangles never overlap.
- Move sequence ends with every item at its planned target.
- If `ComputeArrangeTargets`, the layout validation, or `GenerateArrangeMoves`
  fails, no network move is ever issued.

## Test / Checklist (manual, in-client)
- [ ] Empty inventory → button does nothing, no crash, no console error.
- [ ] One small item, already in slot 0 → no moves (button is a no-op).
- [ ] Mixed sizes (1x1, 1x2, 2x2, 2x3, 2x4 wing) → packs without overlap.
- [ ] Fully packed inventory → reorders cleanly OR aborts; never corrupts.
- [ ] Click arrange twice in a row → second click is a no-op (idempotent).
- [ ] Items with excellent/ancient/socket/luck/skill options keep all data
      after rearrange (verify via tooltip).
- [ ] Open trade/storage/shop, then attempt arrange → button is blocked.
- [ ] Pickup an item, then attempt arrange → button is blocked.

## Known Risks / Assumptions
- The rework keeps the client-network move flow intact. Any server-side
  rejection mid-sequence is still surfaced by `ProcessInventoryRearrange`
  aborting the queue.
- First-fit is provably deterministic but is not guaranteed to be
  area-optimal. If the inventory cannot be packed by first-fit but could be
  packed by a more advanced algorithm, the rework will refuse rather than
  produce an unsafe layout. That is the conservative tradeoff the task
  requires ("if sorting cannot produce a valid layout, abort").
- Item rotation is intentionally not implemented (client lacks support).
- Wing-sized items (2x4 / 4x2) are supported via the existing 1..4 size guard.

## Final Status
- Done in code on branch `feature/inventory-sorting-rework`. Not committed.
- Touched files:
  - `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp` (+88, -86)
- Summary of changes:
  - Removed `ArrangePlacementScore` and `IsBetterArrangePlacement`.
  - Replaced `FindBestArrangeTarget` with `FindFirstFitArrangeTarget` — a
    deterministic top-left first-fit scan.
  - Strengthened the size sort tiebreaker (`area, height, width, sourceIndex,
    key`) so identical-sized items have a stable ordering across runs.
  - Hoisted size bounds into `ARRANGE_MIN_ITEM_SIZE` / `ARRANGE_MAX_ITEM_SIZE`.
  - Added `ValidateArrangeLayout` (bounds + overlap + cell-count proof) and
    `IsArrangeLayoutNoop`, both invoked by `BuildInventoryRearrangeMoves`
    before any move is queued.
- Behaviour:
  - If validation fails, the build returns `false` and no network move is
    sent, leaving the inventory untouched.
  - If the inventory is already arranged, the build returns `false` (treated
    as a no-op — the existing button click handler simply clears moves).
- Manual in-client validation (Test / Checklist) is up to Andres — no build,
  no test, no push performed per project policy.
