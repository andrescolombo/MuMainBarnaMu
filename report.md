# Inventory items & tooltips — change report

Brief summary of client changes made in this session (inventory input and hover tooltips).

---

## 1. Right-click equip with expanded inventory open

**File:** `src/source/UI/NewUI/Inventory/NewUIInventoryActionController.cpp`  
**Function:** `HandleInventoryRightClickActions`

**Problem:** When the expanded inventory panel was open (**K**), right-click on an item always ran **bag-to-bag transfer** (`TryTransferBetweenInventorySections`) and never reached **equip** (`TryEquipItem`).

**Change:** RMB handling order on inventory items is now:

1. Consume (potions, scrolls, etc.)
2. **Equip** (`TryEquipItem`)
3. Transfer main ↔ extension bags (only if extension UI is visible and equip did not apply)
4. Drop (main inventory only, extension closed)

**Unchanged:** Right-click on the **equipment panel** still **unequips** to inventory. Other guards (repair mode, trade, shop, etc.) are unchanged.

---

## 2. Side-by-side equip comparison tooltips

**Files:** `NewUIInventoryCtrl.cpp`, `ZzzInventory.cpp` / `ZzzInventory.h`  
**Functions:** `RenderItemToolTip`, `RenderItemInfo`, `GetEquippedCompareSlot`, `GetEquipStatus`

**Feature:** Hovering an equippable item in **player inventory** (main or extension grid) shows:

- The hovered item tooltip (stats colored **green** if higher than equipped, **red** if lower — damage max, defense, magic defense, block)
- The equipped item tooltip placed **immediately to the left** (8px gap), not at screen center

**Tooltip frame color on the hovered item:**

| Situation | Frame color |
|-----------|-------------|
| Class/step mismatch — item can never be worn by this character | **Red** |
| Class matches, but current stats are too low | **Yellow** |
| Fully equippable right now | No frame (normal black border) |
| Non-equipment item (potion, jewel, etc.) | No frame |

**Compare tooltip shown when:**

- Storage type is `STORAGE_TYPE::INVENTORY`
- Tooltip mode is `TOOLTIP_TYPE_INVENTORY` (not repair / NPC shop / etc.)
- The hovered item's class + slot is compatible (even if stats are too low — stat-insufficient items still show the comparison)
- That equipment slot is not empty

**Not in scope:** Vault, trade, mix, shop grids; empty slots; non-equipment items; hover on equipment panel slots.

---

## Files touched

| File | Change |
|------|--------|
| `NewUIInventoryActionController.cpp` | RMB priority: equip before ext-bag transfer |
| `NewUIInventoryCtrl.cpp` | Dual tooltip render + `GetEquippedCompareSlot` / equip-status frame color; added missing `CharacterManager.h` include |
| `ZzzInventory.cpp` | Compare tooltip LEFT placement, measure-mode, `g_tooltipFrameColor` hoisted to file scope before `RenderTipTextList` |
| `ZzzInventory.h` | `TOOLTIP_FRAME_COLOR` enum + `SetTooltipFrameColor` declaration, updated `RenderItemInfo` signature |
| `ZzzInfomation.cpp` | Cast `ItemValue()` result to `int` to silence C4477 format-string warnings |
| `ZzzEffect.cpp` | `arv3PosProcess` array size 3→4 (index [3] was out-of-bounds buffer overrun) |

---

## Testing suggestions

1. Open inventory, right-click armor/weapon with extension panel **closed** → equips.
2. Open extension (**K**), right-click same → still equips (not only moves between bags).
3. Hover equippable item while wearing something in that slot → two tooltips side by side.
4. Hover item with empty slot → single tooltip only.
5. Hover potion/jewel → single tooltip only.


1. Tooltips side by side (not screen center)
The compare tooltip was using a fixed +220px center offset, so RenderTipTextList often clamped it to the middle of the screen.

Now: RenderItemInfo lays out both tooltips in one pass:

Measures the hovered tooltip width
Draws it at the normal inventory anchor
Draws the equipped tooltip at left + width + 8px (left-aligned chain)
2. Green / red stat comparison
On the hovered item tooltip only:

Stat	Comparison
Attack (max)
vs equipped max (incl. harmony)
Defense
vs equipped defense
Magic defense
vs equipped
Successful blocking
vs equipped
Green = hovered value is higher
Red = hovered value is lower
Unchanged color when equal or not comparable
The equipped tooltip keeps normal colors.

Files changed
ZzzInventory.cpp / ZzzInventory.h — layout + compare coloring
NewUIInventoryCtrl.cpp — single RenderItemInfo(..., pEquippedCompare) call
Rebuild and hover an equippable item in your bag while wearing something in that slot. You should see both tooltips adjacent to the inventory item, with green/red on the new item’s combat stats.

If you want more lines compared (e.g. required stats, excellent options, attack speed), say which ones and we can extend it.