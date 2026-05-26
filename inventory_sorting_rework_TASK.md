# Inventory Sorting Rework — TASK

## Goal
Rework the inventory rearrange/sort feature so it:
- Plans **multi-step** move sequences (blocker displacement), not only direct moves.
- Snapshots current inventory state and only applies the plan if the new
  layout is materially better.
- Validates the plan end-to-end before any network move is issued.
- Preserves item metadata exactly (serial, options, durability, level, luck,
  skill, excellent/ancient/socket data, stack count).
- Is deterministic: same input → same move sequence.
- Lives in a clean `UI::Inventory::Sorting` namespace, without `NewUI` prefix
  on new helpers.

## Branch
- `feature/inventory-sorting-rework` (off `mixed_client`). Already pushed
  through iter 3 (`b979264bb`). This rework supersedes the previous helpers
  in the anonymous namespace of `NewUIMyInventory.cpp`.

## Investigation Checklist
- [ ] `src/source/Core/Globals/_struct.h` — `ITEM` struct (key, x, y, Type,
      durability, options, stack/quantity).
- [ ] `src/source/Core/Globals/_define.h` — `MAX_EQUIPMENT`, inventory size
      constants (column/row counts, MAX_INVENTORY_*).
- [ ] `src/source/Core/Globals/_enum.h` — `STORAGE_TYPE`, item attribute
      enums.
- [ ] `ItemAttribute[type]` — confirm `Width` / `Height` are the canonical
      item dimensions.
- [ ] `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h` —
      `GetNumberOfColumn`, `GetNumberOfRow`, `GetItem`, `GetIndexByItem`,
      `FindItem(linearPos)`, `FindItemByKey`, `RemoveItem`,
      `CreatePickedItem`, `BackupPickedItem`.
- [ ] `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp` — existing
      `InventoryRearrangeMove`, `BuildInventoryRearrangeMoves`,
      `ProcessInventoryRearrange`, `ExecuteNextInventoryRearrangeMove`,
      and the helpers in the anonymous namespace (iter 1-3).

## Algorithm Design Checklist
- [ ] Snapshot: capture every item's key, source (x, y), width, height.
      Reject items in equipment slots or with unsupported sizes upfront.
- [ ] Plan target layout with phased first-fit:
      A. wide items (width > 1) — row-major,
      B. tall items (width = 1, height > 1) — column-major,
      C. 1x1 fillers — row-major.
      Inside each phase, sort `area DESC, height DESC, width DESC,
      sourceIndex ASC, key ASC`.
- [ ] Layout validator: bounds + non-overlap + cell-count proof.
- [ ] Layout scorer: lower is better. Score = weighted sum of
      `bottomMostRow`, `rightMostColumn`, hole count, fragmentation
      penalty, future-1x3-fit penalty. Same scorer for current and proposed
      layouts.
- [ ] No-op gate: if `score(proposed) >= score(current)`, bail.
- [ ] Move generator: two-phase with cascading blocker eviction. Items
      already at target stay. Items whose target is reachable move first.
      If nothing moves and items remain, evict any blocker into a temp
      slot that does not stomp another item's planned target. Bounded
      iteration cap.
- [ ] Move-sequence simulator: replay the planned moves against a virtual
      grid seeded from the current snapshot. Every intermediate move must
      be in bounds, hit only empty cells (or cells owned by the moving
      item), and the final state must match every planned target.
- [ ] Rollback: any failure (sort, layout, score, generate, simulate) →
      return false, no network move issued, inventory untouched.

## Implementation Checklist
- [ ] New header `InventorySortingPlanner.h` exposing
      `UI::Inventory::Sorting::Planner` plus a small POD `Item`/`Move`
      contract.
- [ ] New implementation `InventorySortingPlanner.cpp` holding all the
      private helpers (no longer inside `NewUIMyInventory.cpp`'s anonymous
      namespace).
- [ ] `NewUIMyInventory::BuildInventoryRearrangeMoves` is reduced to:
      gather items, call `Planner::Plan`, copy moves out.
- [ ] No heap allocation in render/tick paths; planner allocations only
      happen on button click.
- [ ] All functions ≤ 40 lines; early-exit guards; named constants.
- [ ] Coordinate convention documented in the header: `x` = column,
      `y` = row, 0-based, item at `(x, y)` of size `(w, h)` occupies
      cells `[x..x+w) × [y..y+h)`.

## Validation Checklist
- [ ] Source items all exist when the plan is applied (`FindItemByKey`).
- [ ] Each source coordinate matches the snapshot (or the move is aborted
      safely — already in `ProcessInventoryRearrange`).
- [ ] All target positions are in bounds.
- [ ] No target collision.
- [ ] Final item count equals initial item count.
- [ ] Item metadata is preserved (we never reconstruct items; we only emit
      slot-index moves through the existing protocol).

## Manual Test Cases
- [ ] No-op case: arrange on a layout that is already as good as the plan
      makes 0 moves.
- [ ] Blocker displacement: 1x3 at (7,4) with column 0 rows 2-4 occupied by
      1x2 + 1x1 blockers ends up at (0,2) after the blockers relocate.
- [ ] Full inventory with no valid better plan → 0 moves.
- [ ] Inventory of only 1x1s → packs to top-left without overlap.
- [ ] Mixed 1x1, 1x2, 1x3, 2x2, 2x3 → wide items top-left, tall items
      column-major, 1x1 fillers in remaining cells.
- [ ] Blockers exist but cannot be relocated → plan aborts, 0 moves.
- [ ] Blockers could be relocated but score does not improve → 0 moves.
- [ ] Stacked consumables (apples, healing potions) → stacks preserved
      (the move protocol does not unstack).
- [ ] Click arrange, then click again → second click is a no-op.
- [ ] Open trade/storage/shop, then attempt arrange → button is blocked.

## Known Risks / Assumptions
- First-fit is deterministic but not area-optimal. If a layout can only be
  packed by a more advanced solver, the planner refuses rather than emit an
  unsafe sequence.
- Item rotation is intentionally not supported.
- Stack count and item metadata travel with the move protocol; the client
  never reconstructs an item, so durability/options/etc. are preserved by
  construction.
- The scorer is heuristic. The no-op gate uses a strict `<` comparison so
  ties leave the inventory untouched.
- The move generator currently treats blockers one level deep; the simulator
  catches any case where a cascading move sequence diverges from the plan
  and the build is rejected.

## Diagnostic Pass (current iteration)

### Problem summary
Four 2x3 bow items in an 8x8 inventory. Expected: tight 4-wide x 3-tall block
at rows 0-2, occupying (0,0), (2,0), (4,0), (6,0). Observed: three 2x3s top-
left, one 2x3 stranded in the lower/right area, with 1x1 / 1x3 items occupying
the top-right region that should belong to the fourth bow.

### Hypotheses (ordered most-likely-first)
1. Snapshot / metadata: the stray bow does not enter the planner as 2x3.
2. Sort: the fourth 2x3 sorts after 1x3 / 1x1 items.
3. Pass 1: even with correct sort, first-fit does not place the fourth 2x3 at (6,0).
4. Pass 2 / Pass 3 structural regression: a later pass moves a multi-cell item.
5. Scoring: a fragmented multi-cell layout is being accepted because
   ScoreLayout does not penalise shape fragmentation.
6. Move emission: in-memory target is correct but the realised UI state diverges.

### Diagnostic logs added (this iteration)
All logs go through `g_ConsoleDebug->Write(MCD_NORMAL, ...)` and are tagged
with one of:

- `[ArrangeItem] key=... type=... level=... w=... h=... area=... source=(...,...) srcIdx=... gk=...`
  One per item, emitted at the top of `Plan()`. Confirms hypothesis 1.
- `[ArrangeSortOrder] rank=N key=... type=... level=... w=... h=... area=... gk=... srcIdx=...`
  One per item after the canonical sort, first convergence iter only. Confirms hypothesis 2.
- `[ArrangePass1] rank=N key=... w=... h=... mode=0/1/2 placed=(...,...)`
  One per non-filler placement, first convergence iter only. Confirms hypothesis 3.
  `mode=0` = wide row-major, `mode=1` = tall column-major, `mode=2` = filler.
- `[ArrangeError] pass=PassN key=... w=... h=... oldTarget=(...) newTarget=(...)`
  Emitted by `CheckStructuralInvariant` if any item with width>1 OR height>1
  has a different target in pass 2 or pass 3 compared to pass 1. Confirms
  hypothesis 4.
- `[ArrangeScore] iter=N compact=... holes=... adjMis=... moves=... largestRect=...`
  `[ArrangeScore] iter=N group1x1=... shapeFrag=... (weight=...)`
  Per convergence iteration. The `shapeFrag` metric is logged but
  `kShapeFragmentationWeight = 0` so it is NOT yet applied to ScoreLayout.

### Structural invariant
Pass 2 and Pass 3 must not change `targetX/Y` for any item where
`width > 1 || height > 1`. `CheckStructuralInvariant(pass1, passN)` walks
both layouts and logs `[ArrangeError]` for every violation. The current
`BuildPass2Layout` / `BuildPass3Layout` only modify filler items, so the
invariant should hold; the check is a safety net.

### Expected canonical 8x8 layout for four 2x3 items only
```
(0,0)(0,0) (2,0)(2,0) (4,0)(4,0) (6,0)(6,0)
(0,0)(0,0) (2,0)(2,0) (4,0)(4,0) (6,0)(6,0)
(0,0)(0,0) (2,0)(2,0) (4,0)(4,0) (6,0)(6,0)
. . . . . . . .
. . . . . . . .
. . . . . . . .
. . . . . . . .
. . . . . . . .
```

### Conditional next step (NOT applied yet)
If `[ArrangeItem]` / `[ArrangeSortOrder]` / `[ArrangePass1]` /
`[ArrangeError]` all look correct but the final layout is still fragmented,
flip `kShapeFragmentationWeight` from 0 to 40 (or 80) to penalise stranded
multi-cell items. The metric is already being computed and logged.

### Verification checklist
- [ ] All four bow items log as `w=2 h=3`.
- [ ] Sort places all four bows at ranks 0..3 (before any 1x3 / 1x1).
- [ ] Pass 1 places them at (0,0), (2,0), (4,0), (6,0).
- [ ] No `[ArrangeError]` lines emitted.
- [ ] If 1..3 pass and the layout is still wrong, ship a follow-up that
      sets `kShapeFragmentationWeight = 40`.

## Final Status
- 3-pass planner extracted into `UI::Inventory::Sorting` and wired into
  `CNewUIMyInventory::BuildInventoryRearrangeMoves`. Pending in-client test.
- New files:
  - `src/source/UI/NewUI/Inventory/InventorySortingPlanner.h`
  - `src/source/UI/NewUI/Inventory/InventorySortingPlanner.cpp`
- `NewUIMyInventory.cpp`: anonymous-namespace planner helpers removed (-628
  lines); the call site now hands a `PlannerItem` vector to the new module
  and copies `PlannerMove`s back into the existing `InventoryRearrangeMove`
  vector that drives the network protocol.
- `ITEM` group key (used by pass 3) is a deterministic mix of Type, Level,
  OptionType, OptionLevel — so apples line up next to apples, healing
  potions next to healing potions, etc., without disturbing larger items.
- Scoring weights (`W_COMPACTNESS=1`, `W_HOLE=60`, `W_ADJACENCY=3`,
  `W_LARGEST_RECT=4`, `W_MOVES=1`) and thresholds
  (`acceptanceThreshold=5`, `passProgressionThreshold=1`) are named
  constants and easy to tune.
- The planner enforces every check from the validation list before issuing
  any move:
  - source items must fit in the grid,
  - bounds + non-overlap layout validation,
  - move-sequence simulator replays the plan from the snapshot and asserts
    the final state matches every planned target,
  - if a later pass produces an unrealizable sequence, the planner falls
    back to pass 1 (always realizable),
  - if no pass beats the current layout by `acceptanceThreshold`, the
    planner returns no plan and the inventory is untouched.
- Manual in-client verification (see Test cases above) is up to Andres —
  no build, no test, no push performed automatically.
