#pragma once

#include <windows.h>
#include <vector>

namespace UI
{
namespace Inventory
{
namespace Sorting
{

// Coordinate convention:
//   x = column (left -> right), y = row (top -> bottom), 0-based.
//   An item at (x, y) of size (width, height) occupies the cells
//   [x .. x + width) X [y .. y + height).

struct PlannerItem
{
    DWORD key;          // unique runtime item key
    int sourceIndex;    // linear inventory slot (relative to inventory grid)
    int sourceX;        // current column
    int sourceY;        // current row
    int width;          // item attribute width (1..MAX_ITEM_SIZE)
    int height;         // item attribute height (1..MAX_ITEM_SIZE)
    int groupKey;       // deterministic identity used to cluster 1x1 items
};

struct PlannerMove
{
    DWORD itemKey;
    int targetIndex;    // protocol-ready: linear slot + equipment offset
};

struct PlannerSettings
{
    int equipmentOffset;            // MAX_EQUIPMENT (added to linear inventory index)
    int acceptanceThreshold;        // minimum score gain to apply a plan
    int passProgressionThreshold;   // minimum score gain to adopt a later pass
};

struct PlannerResult
{
    bool hasPlan;                       // false => no moves should be issued
    std::vector<PlannerMove> moves;     // empty when hasPlan is false
};

// Run the 3-pass planner. Returns a fully validated move sequence or an empty,
// has-no-plan result. Never mutates the caller's snapshot.
PlannerResult Plan(const std::vector<PlannerItem>& items,
                   int columnCount,
                   int rowCount,
                   const PlannerSettings& settings);

// Item-size guard re-exported so the caller can reject items before snapshotting.
bool IsSupportedItemSize(int width, int height);

} // namespace Sorting
} // namespace Inventory
} // namespace UI
