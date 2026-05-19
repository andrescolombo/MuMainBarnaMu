# Inventory fixes — full change log

## 1. Right-click equip with expanded inventory open

**File:** `src/source/UI/NewUI/Inventory/NewUIInventoryActionController.cpp`  
**Function:** `HandleInventoryRightClickActions`

**Problem:** When the extended inventory panel was open (**K**), right-click on any item hit the
`g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT)` early-return guard and always ran
`TryTransferBetweenInventorySections`, skipping the equip path entirely.

**Fix:** Removed the early-return guard and moved the extension-transfer block to run *after*
`TryEquipItem`. The full RMB priority chain is now:

1. Consume (potions, scrolls, etc.)
2. **Equip** — `TryEquipItem`
3. Transfer main ↔ extension bags — only if extension UI is visible and equip did not apply
4. Drop — main inventory only, extension closed

Right-click on the equipment panel (unequip) is unchanged.

---

## 2. Side-by-side equipped-item comparison tooltip

**Files:** `NewUIInventoryCtrl.cpp`, `ZzzInventory.cpp`, `ZzzInventory.h`

### 2a. New helper infrastructure (`ZzzInventory.cpp` anonymous namespace)

| Helper | Purpose |
|--------|---------|
| `MeasureTipTextListWidth(TextNum, Tab)` | Measures pixel width of the current TextList content (scaled by `g_fScreenRate_x`) without rendering |
| `ComputeTipLeftFromCenter(sx, fWidth)` | Converts center-X anchor to clamped left-edge |
| `RenderTipTextListAtLeft(leftX, sy, TextNum, Tab)` | Renders tooltip box with explicit left anchor |
| `GetItemAttackRangeForTooltip(ip, min, max)` | Reads `DamageMin`/`DamageMax` plus Jewel-of-Harmony bonus for compare coloring |
| `GetItemDefenseForTooltip(ip)` | Reads `Defense` plus Jewel-of-Harmony bonus |
| `ApplyTooltipStatCompareColor(textIndex, hoverVal, equippedVal)` | Sets green (higher) / red (lower) on a TextList entry |
| `g_pTooltipCompareEquipped` | File-scope pointer to the equipped item used during TextList build |

### 2b. Signature change (`ZzzInventory.h` / `ZzzInventory.cpp`)

```cpp
// Before
void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell,
                    int Inventype = 0, bool bItemTextListBoxUse = false);

// After
void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell,
                    int Inventype = 0, bool bItemTextListBoxUse = false,
                    ITEM* pCompareEquipped = nullptr,
                    bool bAnchorLeft = false,
                    float* pOutWidth = nullptr);
```

- `pCompareEquipped` — equipped item to show as a side-by-side tooltip; `nullptr` = no compare
- `bAnchorLeft` — when true, `sx` is treated as a left edge (used for recursive calls); prevents double-reset of globals and infinite recursion
- `pOutWidth` — when non-null, performs a measure-only pass: builds TextList, stores width, returns without rendering

### 2c. Layout — compare tooltip is placed to the LEFT

`RenderItemInfo` end section now:

1. Measures hovered-item tooltip width (measure-only pass)
2. Computes `tipLeft` from the center anchor
3. If a compare item is set: measures the equipped tooltip width (measure-only recursive call), renders the main tooltip, then renders the equipped tooltip at `tipLeft − equippedWidth − 8 px`
4. Resets `g_tooltipFrameColor` to `NORMAL` before rendering the compare tooltip so it always has a plain black border

### 2d. Green / red stat comparison coloring

Applied to the **hovered item tooltip only** (compare tooltip keeps default colors):

| Stat | Compared against |
|------|-----------------|
| Attack max (+ harmony) | Equipped attack max (+ harmony) |
| Defense (+ harmony) | Equipped defense (+ harmony) |
| Magic defense | Equipped magic defense |
| Successful blocking | Equipped successful blocking |

Green = hovered value is higher; red = hovered value is lower; unchanged when equal or not comparable.

### 2e. Tooltip frame color (`ZzzInventory.h` / `ZzzInventory.cpp`)

Added enum and setter:

```cpp
enum TOOLTIP_FRAME_COLOR { TOOLTIP_FRAME_NORMAL = 0, TOOLTIP_FRAME_RED, TOOLTIP_FRAME_YELLOW };
void SetTooltipFrameColor(TOOLTIP_FRAME_COLOR color);
```

`g_tooltipFrameColor` is declared as a `static` file-scope variable before `RenderTipTextList`
(it was originally in the anonymous namespace at line ~2120, causing C2065 because
`RenderTipTextList` at line ~301 used it before that namespace was reached).

`RenderTipTextList` draws the dark background first, then draws the border in the frame color:

| Value | Border color |
|-------|-------------|
| `TOOLTIP_FRAME_NORMAL` | Black (0, 0, 0) |
| `TOOLTIP_FRAME_RED` | Red (1.0, 0.15, 0.15) |
| `TOOLTIP_FRAME_YELLOW` | Yellow (1.0, 0.85, 0.0) |

### 2f. Equip-status helpers (`NewUIInventoryCtrl.cpp` anonymous namespace)

```
IsClassCompatibleForItem(pItem)  — class + step-class check only
HasSufficientStatsForItem(pItem) — all stat/level requirements
GetEquipStatus(pItem)            — returns EquipStatus enum:
                                   None / ClassMismatch / StatInsufficient / CanEquip
IsEquipableIgnoreStats(slot, pItem) — class+slot only, no stats
                                      (used so stat-insufficient items still show compare)
GetEquippedCompareSlot(pHoverItem)  — returns equipment slot index or -1
```

`GetEquippedCompareSlot` logic:
1. Tries `IsEquipable` (full check) on the natural slot
2. Falls back to left-hand weapon / left ring slot where applicable
3. If full check fails, retries with `IsEquipableIgnoreStats` so yellow-frame items still get a compare tooltip
4. Returns -1 if no slot matches or that slot is empty

### 2g. Call site (`NewUIInventoryCtrl.cpp` — `RenderItemToolTip`)

```cpp
if (m_ToolTipType == TOOLTIP_TYPE_INVENTORY)
{
    ITEM* pEquippedCompare = nullptr;
    if (m_StorageType == STORAGE_TYPE::INVENTORY)
    {
        const int equipSlot = GetEquippedCompareSlot(m_pToolTipItem);
        if (equipSlot != -1)
            pEquippedCompare = &CharacterMachine->Equipment[equipSlot];

        const EquipStatus status = GetEquipStatus(m_pToolTipItem);
        if (status == EquipStatus::ClassMismatch)
            SetTooltipFrameColor(TOOLTIP_FRAME_RED);
        else if (status == EquipStatus::StatInsufficient)
            SetTooltipFrameColor(TOOLTIP_FRAME_YELLOW);
        else
            SetTooltipFrameColor(TOOLTIP_FRAME_NORMAL);
    }
    RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false, 0, false, pEquippedCompare);
}
```

Frame color and compare tooltip are only applied when `STORAGE_TYPE::INVENTORY` and
`TOOLTIP_TYPE_INVENTORY`. Vault, trade, mix, shop, repair, and NPC contexts are unchanged.

**Added include:** `#include "Character/CharacterManager.h"` (was missing; caused C2065 on
`gCharacterManager` and cascading "const object must be initialized" errors for `byFirstClass` /
`byStepClass`).

---

## 3. Bug fixes (pre-existing, surfaced during build)

### 3a. `ZzzInfomation.cpp` — C4477 format-string mismatch

`PrintItem` passed `ItemValue(&ip)` (`int64_t`) to `%8d` (`int`).  
Fix: cast to `(int)` in both `fwprintf` calls. This is a debug/info dump function; truncation is acceptable.

### 3b. `ZzzEffect.cpp` — buffer overrun in `CreateEffect`

`arv3PosProcess` was declared with 3 elements but written at index [3] (a 4-point bezier
interpolation path):

```cpp
// Before
vec3_t arv3PosProcess[3];   // valid indices: 0, 1, 2
...
VectorCopy(v3PosTargetModify, arv3PosProcess[3]);  // out-of-bounds
arv3PosProcess[3][0] = ...;
arv3PosProcess[3][1] = ...;
arv3PosProcess[3][2] = ...;

// After
vec3_t arv3PosProcess[4];   // valid indices: 0, 1, 2, 3
```

---

## Files changed

| File | Change |
|------|--------|
| `NewUIInventoryActionController.cpp` | RMB priority: equip before ext-bag transfer |
| `NewUIInventoryCtrl.cpp` | Equip-status helpers, frame color call, dual tooltip render; added `CharacterManager.h` include |
| `ZzzInventory.cpp` | Anonymous namespace helpers, `g_tooltipFrameColor` hoisted to file scope, colored border in `RenderTipTextList`, full `RenderItemInfo` rewrite for measure-mode + LEFT compare placement + stat compare coloring |
| `ZzzInventory.h` | `TOOLTIP_FRAME_COLOR` enum + `SetTooltipFrameColor`, updated `RenderItemInfo` signature |
| `ZzzInfomation.cpp` | Cast `ItemValue()` to `int` (C4477) |
| `ZzzEffect.cpp` | `arv3PosProcess` size 3 → 4 (buffer overrun) |

---

## Testing

1. Right-click armor/weapon with extension panel **closed** → equips.
2. Open extension (**K**), right-click same item → still equips (not moved to other bag).
3. Hover an item your class **cannot wear** → red border, no compare tooltip.
4. Hover an item your class can wear but **stats are too low** → yellow border, compare tooltip visible to the left.
5. Hover an equippable item with **sufficient stats** → normal black border, compare tooltip to the left.
6. Hover item while equipment slot is **empty** → single tooltip, appropriate frame color.
7. Hover **potion / jewel** → single tooltip, no frame coloring.
8. Hover item in vault / trade / shop → no compare, no frame coloring.

---

## Rollback note: item-slot background experiment

Requested follow-up: move the class/stat warning color from the tooltip frame to the inventory item background/frame itself.

Attempted approach:

- Added a helper in `NewUIInventoryCtrl.cpp` to choose the slot background color during `CNewUIInventoryCtrl::Render`.
- Red slot background for `EquipStatus::ClassMismatch`.
- Yellow slot background for `EquipStatus::StatInsufficient`.
- Removed the `SetTooltipFrameColor(...)` calls from `RenderItemToolTip`.

Result:

- The edit compiled.
- The client no longer loaded/reached the server screen.
- The experiment was rolled back.

Current state after rollback:

- Tooltip frame coloring is restored:
  - red tooltip frame for class mismatch
  - yellow tooltip frame for insufficient stats
- Inventory item background rendering is back to the previous durability/trade-warning behavior.

Next attempt should avoid adding equip-status evaluation directly inside the per-square inventory render path until the startup/server-screen failure is understood.

---

## Follow-up: equipped compare tooltip progress

Implemented next steps:

- Compare tooltips now stay aligned to the same top Y position as the hovered item tooltip.
- Hovered inventory items can compare against up to two equipped items when the target item type supports paired slots:
  - right-hand and left-hand weapon slots
  - right-ring and left-ring slots
- The hovered tooltip is rendered before compare tooltip measurement, preventing the shared `TextList` buffer from being overwritten by the equipped item. This fixes the case where hovering a sword while an axe is equipped showed axe text in both tooltip boxes.

Current behavior:

- Hovered item tooltip remains the main tooltip.
- Equipped item tooltip(s) render to the left of the hovered tooltip.
- Tooltip frame coloring remains restored after the item-slot background experiment rollback.

---

## Follow-up: always-on equip-status background

Implemented after the hover-only test:

- Class/stat equip-status coloring now renders for every visible item in the inventory control, not only while hovering.
- The logic stays scoped to player inventory and keeps tooltip frames normal.
- The implementation is functionally acceptable, but the visual result is not final; the next experiment should try a red glow instead of a flat red/yellow background fill.

Status: pushed as an intermediate visual iteration so it can be compared or reverted later.

---

## Follow-up: Arrange Items button logic

This documents the current inventory arrange flow for future fixes.

### Entry point

The arrange button lives in `CNewUIMyInventory` and calls:

```cpp
RequestInventoryRearrange()
```

The button only starts arranging when `CanUseInventoryRearrange()` allows it:

- inventory control exists
- no item is currently picked by the cursor
- no item move request is already in progress (`EquipmentItem == false`)
- blocked interfaces are closed, including NPC shop, trade, storage, mix inventory, my shop,
  lucky item windows, and purchase shop

### Supported grid and item sizes

The target grid is the main inventory grid, currently `8x8`.

Supported item sizes:

- `1x1`
- `1x3`
- `2x2`
- `2x3`
- `2x4`
- `4x3`

Items are never rotated. A `1x3` item remains width `1`, height `3`; a `2x4` item remains width `2`,
height `4`; a wing-sized `4x3` item remains width `4`, height `3`.

If any inventory item has an unsupported size, the arrange request is cancelled before moving
anything.

### Phase 1: compute the compact target layout

`BuildInventoryRearrangeMoves()` first reads the current inventory items into a temporary planning
list. The real inventory is not changed during this phase.

Each planned item stores:

- current source cell (`sourceX`, `sourceY`)
- target cell (`targetX`, `targetY`)
- item width and height
- the current client item key, used as a temporary planning identity

Target placement is deterministic:

1. Sort items by descending area.
2. Break ties by taller height first.
3. Break ties by wider width first.
4. Break remaining ties by item key.

For each item, the planner scans every valid position and chooses the best placement. Wide items
prefer columns aligned to their own width. Two-wide items pack naturally into columns `0-1`, `2-3`,
`4-5`, and `6-7`; four-wide wings prefer `0-3` and `4-7` instead of drifting into offset gaps.

The placement score then prefers:

1. smaller occupied bottom edge
2. smaller bounding area
3. smaller right edge
4. smaller row
5. smaller column

This creates one fixed compact layout before any real item move is sent.

### Phase 2: generate a safe one-by-one move list

`GenerateArrangeMoves()` simulates the inventory with a temporary occupancy grid. It does not mutate
the real inventory.

The move generator loops until every simulated item reaches its target:

1. If an item's target rectangle is currently empty, queue a move directly to that target.
2. If no direct target move is possible, find the item blocking another item's target.
3. Move that blocker into a temporary empty rectangle.
4. Repeat until all target positions are reachable.

Temporary slots are searched from bottom-right toward top-left. The first pass avoids final target
cells, which helps preserve the compact layout. If no such temporary slot exists, the second pass
allows a currently empty target cell as a temporary buffer.

A guard stops planning if the simulated move list grows beyond `itemCount * columnCount * rowCount`.
If the full move sequence cannot be planned, the arrange request is cancelled before real movement
starts.

### Phase 3: execute through the normal server move path

After a full valid move list exists, the client executes exactly one move at a time through the same
path as manual item movement:

1. Find the item for the front queued move.
2. Create a picked item with `CNewUIInventoryCtrl::CreatePickedItem(...)`.
3. Remove it locally from the inventory control.
4. Send `SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, sourceIndex, pickedItem,
   STORAGE_TYPE::INVENTORY, targetIndex)`.
5. Wait for the server response before sending the next move.

The server remains authoritative. If a move is rejected or the expected destination item cannot be
found after the response, the arrange queue is cleared and no more automatic moves are sent.

### Important identity detail

The arrange system does not check item names for movement.

The planner initially uses `ITEM::Key` because the inventory occupancy grid already stores keys.
However, `ITEM::Key` is marked as client-only in `Core/Globals/_struct.h`, and `CreateItem(...)`
generates a new key when the server response recreates the item in the destination slot.

That caused the previous loop bug: after one successful move, queued moves could still point at the
old client key and start following the wrong object.

Current fix:

1. After each accepted move, `ProcessInventoryRearrange()` reads the item now sitting at
   `move.targetIndex`.
2. It gets that item's new `ITEM::Key`.
3. It replaces the old key in all remaining queued moves with the new key.

This keeps the queued move list attached to the same logical item even though the client-side item
object/key changes after every server-confirmed move.

### Current behavior summary

- One button click computes a final compact target layout first.
- Real movement only starts after the full simulated move list is valid.
- Items move one by one through normal server-authoritative item move requests.
- The cursor-held item state is used as the movement buffer.
- Temporary inventory slots are used only when a target position is blocked.
- Item metadata is preserved because the server move path is reused; the arrange code does not
  duplicate, recreate, or edit item stats/options/sockets directly.
