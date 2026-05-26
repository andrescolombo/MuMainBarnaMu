#include "stdafx.h"
#include "UI/NewUI/Inventory/InventorySortingPlanner.h"
#include "Core/Utilities/Log/muConsoleDebug.h"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

namespace UI
{
namespace Inventory
{
namespace Sorting
{

namespace
{
    constexpr int MIN_ITEM_SIZE = 1;
    constexpr int MAX_ITEM_SIZE = 4;
    constexpr int MAX_PLAN_PASSES = 3;

    // Scoring weights (lower score => better layout). Tuned so an internal hole
    // is roughly as bad as several missed adjacencies.
    constexpr int W_COMPACTNESS = 1;
    constexpr int W_HOLE = 60;
    constexpr int W_ADJACENCY = 3;
    constexpr int W_LARGEST_RECT = 4;
    constexpr int W_MOVES = 1;

    // 1x1 grouping-score weights (separate component used by the
    // "accept-on-1x1-cleanup" path; lower => better grouping).
    constexpr int W_GROUP_SEPARATION = 1;       // Manhattan distance penalty per same-group pair
    constexpr int W_GROUP_ADJACENCY_REWARD = 4; // bonus per orthogonal same-group pair

    // Flip to false to silence planner diagnostics without removing call sites.
    constexpr bool kPlannerDebugLogging = true;
    // Per-item logs (snapshot, sort order, per-placement). Noisy; gated also
    // to the first convergence iteration only at the call site.
    constexpr bool kPlannerDebugVerbose = true;
    // Diagnostic-only: logged in [ArrangeScore] but NOT applied to ScoreLayout
    // until logs prove the earlier pipeline stages are correct.
    constexpr int kShapeFragmentationWeight = 0;

    // Maximum in-memory planning iterations. Each iteration feeds the previous
    // iteration's target layout back as the next iteration's source so the
    // sort tiebreaker (sourceIndex) can stabilise without the user clicking
    // arrange multiple times.
    constexpr int kMaxArrangeConvergenceIterations = 8;

    enum Phase
    {
        PHASE_WIDE = 0,
        PHASE_TALL = 1,
        PHASE_FILLER = 2,
    };

    struct WorkItem
    {
        DWORD key;
        int sourceIndex;
        int sourceX;
        int sourceY;
        int targetX;
        int targetY;
        int width;
        int height;
        int groupKey;
        Phase phase;
    };

    // Forward declarations: the per-stage logging helpers live next to
    // PlannerLog further down, but BuildPass1Layout (above them in the file)
    // needs to call them.
    void LogSortedRank(int rank, const WorkItem& item);
    void LogPass1Placement(int rank, const WorkItem& item);

    struct Layout
    {
        std::vector<WorkItem> items;
    };

    int LinearIndex(int x, int y, int columnCount)
    {
        return y * columnCount + x;
    }

    Phase ClassifyPhase(int width, int height)
    {
        if (width > 1)
        {
            return PHASE_WIDE;
        }
        if (height > 1)
        {
            return PHASE_TALL;
        }
        return PHASE_FILLER;
    }

    bool ItemAtTarget(const WorkItem& item)
    {
        return item.sourceX == item.targetX && item.sourceY == item.targetY;
    }

    bool FitsInBounds(int x, int y, int width, int height, int columnCount, int rowCount)
    {
        return x >= 0 && y >= 0 && x + width <= columnCount && y + height <= rowCount;
    }

    void StampOccupancy(std::vector<DWORD>& grid, int columnCount, int x, int y, int width, int height, DWORD value)
    {
        for (int dy = 0; dy < height; ++dy)
        {
            for (int dx = 0; dx < width; ++dx)
            {
                grid[LinearIndex(x + dx, y + dy, columnCount)] = value;
            }
        }
    }

    bool RegionMatches(const std::vector<DWORD>& grid, int columnCount, int x, int y, int width, int height, DWORD owner)
    {
        for (int dy = 0; dy < height; ++dy)
        {
            for (int dx = 0; dx < width; ++dx)
            {
                const DWORD cell = grid[LinearIndex(x + dx, y + dy, columnCount)];
                if (cell != 0 && cell != owner)
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool RegionEmpty(const std::vector<bool>& grid, int columnCount, int x, int y, int width, int height)
    {
        for (int dy = 0; dy < height; ++dy)
        {
            for (int dx = 0; dx < width; ++dx)
            {
                if (grid[LinearIndex(x + dx, y + dy, columnCount)])
                {
                    return false;
                }
            }
        }
        return true;
    }

    // Higher value = placed earlier. The canonical priority is height-first so a
    // 1x4 spear and a 2x4 wing are placed before any height-3 item, and the
    // height-3 group (2x3 + 1x3) is placed before any height-2 item.
    int GetArrangeHeightPriority(const WorkItem& item)
    {
        return item.height;
    }

    int GetArrangeAreaPriority(const WorkItem& item)
    {
        return item.width * item.height;
    }

    bool CompareItemsForCanonicalArrange(const WorkItem& left, const WorkItem& right)
    {
        const int lh = GetArrangeHeightPriority(left);
        const int rh = GetArrangeHeightPriority(right);
        if (lh != rh) return lh > rh;

        const int la = GetArrangeAreaPriority(left);
        const int ra = GetArrangeAreaPriority(right);
        if (la != ra) return la > ra;

        if (left.width != right.width) return left.width > right.width;
        if (left.groupKey != right.groupKey) return left.groupKey < right.groupKey;
        if (left.sourceIndex != right.sourceIndex) return left.sourceIndex < right.sourceIndex;
        return left.key < right.key;
    }

    bool FindRowMajorFit(const std::vector<bool>& grid, int columnCount, int rowCount, int width, int height, int& outX, int& outY)
    {
        const int maxRow = rowCount - height;
        const int maxColumn = columnCount - width;
        for (int row = 0; row <= maxRow; ++row)
        {
            for (int column = 0; column <= maxColumn; ++column)
            {
                if (RegionEmpty(grid, columnCount, column, row, width, height))
                {
                    outX = column;
                    outY = row;
                    return true;
                }
            }
        }
        return false;
    }

    bool FindColumnMajorFit(const std::vector<bool>& grid, int columnCount, int rowCount, int width, int height, int& outX, int& outY)
    {
        const int maxRow = rowCount - height;
        const int maxColumn = columnCount - width;
        for (int column = 0; column <= maxColumn; ++column)
        {
            for (int row = 0; row <= maxRow; ++row)
            {
                if (RegionEmpty(grid, columnCount, column, row, width, height))
                {
                    outX = column;
                    outY = row;
                    return true;
                }
            }
        }
        return false;
    }

    bool PlaceForPhase(std::vector<bool>& grid, int columnCount, int rowCount, WorkItem& item)
    {
        int x = 0;
        int y = 0;
        const bool found = item.phase == PHASE_TALL
            ? FindColumnMajorFit(grid, columnCount, rowCount, item.width, item.height, x, y)
            : FindRowMajorFit(grid, columnCount, rowCount, item.width, item.height, x, y);
        if (!found)
        {
            return false;
        }
        item.targetX = x;
        item.targetY = y;
        for (int dy = 0; dy < item.height; ++dy)
        {
            for (int dx = 0; dx < item.width; ++dx)
            {
                grid[LinearIndex(x + dx, y + dy, columnCount)] = true;
            }
        }
        return true;
    }

    // Pass 1: place wide and tall items only; 1x1 fillers handled by pass 2+.
    bool BuildPass1Layout(Layout& layout, int columnCount, int rowCount, bool verbose = false)
    {
        std::sort(layout.items.begin(), layout.items.end(), CompareItemsForCanonicalArrange);
        if (verbose)
        {
            for (size_t i = 0; i < layout.items.size(); ++i) LogSortedRank(static_cast<int>(i), layout.items[i]);
        }
        std::vector<bool> grid(columnCount * rowCount, false);
        int rank = 0;
        for (WorkItem& item : layout.items)
        {
            if (item.phase == PHASE_FILLER)
            {
                ++rank;
                continue;
            }
            if (!PlaceForPhase(grid, columnCount, rowCount, item))
            {
                return false;
            }
            if (verbose) LogPass1Placement(rank, item);
            ++rank;
        }
        // Tentatively assign 1x1s top-left so the layout is complete even if
        // pass 2 is not adopted; this keeps the scorer comparable across passes.
        for (WorkItem& item : layout.items)
        {
            if (item.phase != PHASE_FILLER)
            {
                continue;
            }
            if (!PlaceForPhase(grid, columnCount, rowCount, item))
            {
                return false;
            }
        }
        return true;
    }

    // Detect 1x1 cells that are surrounded by non-filler items (or grid borders)
    // on at least three sides; these are the "holes" pass 2 targets first.
    bool IsHoleCell(const std::vector<DWORD>& grid, int columnCount, int rowCount, int x, int y)
    {
        const std::array<std::pair<int, int>, 4> deltas{ { {1, 0}, {-1, 0}, {0, 1}, {0, -1} } };
        int closed = 0;
        for (const auto& d : deltas)
        {
            const int nx = x + d.first;
            const int ny = y + d.second;
            if (nx < 0 || ny < 0 || nx >= columnCount || ny >= rowCount)
            {
                ++closed;
                continue;
            }
            if (grid[LinearIndex(nx, ny, columnCount)] != 0)
            {
                ++closed;
            }
        }
        return closed >= 3;
    }

    void StampLayoutGrid(std::vector<DWORD>& grid, int columnCount, const Layout& layout, bool onlyNonFillers)
    {
        std::fill(grid.begin(), grid.end(), DWORD{ 0 });
        for (const WorkItem& item : layout.items)
        {
            if (onlyNonFillers && item.phase == PHASE_FILLER)
            {
                continue;
            }
            StampOccupancy(grid, columnCount, item.targetX, item.targetY, item.width, item.height, item.key);
        }
    }

    // Pass 2: re-place 1x1 items, preferring internal holes first then row-major
    // fill. Operates on a copy of the pass 1 layout.
    bool BuildPass2Layout(const Layout& base, Layout& out, int columnCount, int rowCount)
    {
        out = base;
        std::vector<DWORD> grid(columnCount * rowCount, 0);
        StampLayoutGrid(grid, columnCount, out, /*onlyNonFillers*/ true);

        std::vector<int> holeIndices;
        holeIndices.reserve(static_cast<size_t>(columnCount) * static_cast<size_t>(rowCount));
        for (int y = 0; y < rowCount; ++y)
        {
            for (int x = 0; x < columnCount; ++x)
            {
                if (grid[LinearIndex(x, y, columnCount)] != 0)
                {
                    continue;
                }
                if (IsHoleCell(grid, columnCount, rowCount, x, y))
                {
                    holeIndices.push_back(LinearIndex(x, y, columnCount));
                }
            }
        }

        std::vector<WorkItem*> fillers;
        fillers.reserve(out.items.size());
        for (WorkItem& item : out.items)
        {
            if (item.phase == PHASE_FILLER)
            {
                fillers.push_back(&item);
            }
        }
        std::sort(fillers.begin(), fillers.end(), [](const WorkItem* a, const WorkItem* b)
        {
            if (a->sourceIndex != b->sourceIndex) return a->sourceIndex < b->sourceIndex;
            return a->key < b->key;
        });

        size_t fillerCursor = 0;
        for (int holeLinear : holeIndices)
        {
            if (fillerCursor >= fillers.size()) break;
            const int hx = holeLinear % columnCount;
            const int hy = holeLinear / columnCount;
            WorkItem* item = fillers[fillerCursor++];
            item->targetX = hx;
            item->targetY = hy;
            grid[LinearIndex(hx, hy, columnCount)] = item->key;
        }

        // Remaining fillers: row-major fit into whatever empty cells are left.
        for (; fillerCursor < fillers.size(); ++fillerCursor)
        {
            WorkItem* item = fillers[fillerCursor];
            int x = 0;
            int y = 0;
            bool placed = false;
            for (int row = 0; row < rowCount && !placed; ++row)
            {
                for (int column = 0; column < columnCount && !placed; ++column)
                {
                    if (grid[LinearIndex(column, row, columnCount)] == 0)
                    {
                        x = column;
                        y = row;
                        placed = true;
                    }
                }
            }
            if (!placed)
            {
                return false;
            }
            item->targetX = x;
            item->targetY = y;
            grid[LinearIndex(x, y, columnCount)] = item->key;
        }
        return true;
    }

    // Pass 3: keep pass 2 cells, reassign which 1x1 lands in each cell so items
    // with the same groupKey end up adjacent in row-major traversal order.
    bool BuildPass3Layout(const Layout& base, Layout& out, int columnCount, int rowCount)
    {
        out = base;

        struct Cell { int x; int y; };
        std::vector<Cell> fillerCells;
        std::vector<WorkItem*> fillers;
        fillerCells.reserve(out.items.size());
        fillers.reserve(out.items.size());
        for (WorkItem& item : out.items)
        {
            if (item.phase != PHASE_FILLER) continue;
            fillerCells.push_back({ item.targetX, item.targetY });
            fillers.push_back(&item);
        }

        std::sort(fillerCells.begin(), fillerCells.end(), [columnCount](const Cell& a, const Cell& b)
        {
            return LinearIndex(a.x, a.y, columnCount) < LinearIndex(b.x, b.y, columnCount);
        });

        std::sort(fillers.begin(), fillers.end(), [](const WorkItem* a, const WorkItem* b)
        {
            if (a->groupKey != b->groupKey) return a->groupKey < b->groupKey;
            if (a->sourceIndex != b->sourceIndex) return a->sourceIndex < b->sourceIndex;
            return a->key < b->key;
        });

        if (fillers.size() != fillerCells.size())
        {
            return false;
        }
        for (size_t i = 0; i < fillers.size(); ++i)
        {
            fillers[i]->targetX = fillerCells[i].x;
            fillers[i]->targetY = fillerCells[i].y;
        }
        (void)rowCount;
        return true;
    }

    bool ValidateLayout(const Layout& layout, int columnCount, int rowCount)
    {
        if (columnCount <= 0 || rowCount <= 0) return false;
        std::vector<bool> grid(columnCount * rowCount, false);
        int expected = 0;
        int occupied = 0;
        for (const WorkItem& item : layout.items)
        {
            if (item.width < MIN_ITEM_SIZE || item.width > MAX_ITEM_SIZE) return false;
            if (item.height < MIN_ITEM_SIZE || item.height > MAX_ITEM_SIZE) return false;
            if (!FitsInBounds(item.targetX, item.targetY, item.width, item.height, columnCount, rowCount)) return false;
            expected += item.width * item.height;
            for (int dy = 0; dy < item.height; ++dy)
            {
                for (int dx = 0; dx < item.width; ++dx)
                {
                    const int index = LinearIndex(item.targetX + dx, item.targetY + dy, columnCount);
                    if (grid[index]) return false;
                    grid[index] = true;
                    ++occupied;
                }
            }
        }
        return expected == occupied;
    }

    int CountHolesAndCompactness(const Layout& layout, int columnCount, int rowCount, int& compactness)
    {
        std::vector<DWORD> grid(columnCount * rowCount, 0);
        StampLayoutGrid(grid, columnCount, layout, /*onlyNonFillers*/ false);

        compactness = 0;
        int bottomMostUsed = 0;
        for (const WorkItem& item : layout.items)
        {
            const int bottom = item.targetY + item.height;
            compactness += bottom;
            if (bottom > bottomMostUsed) bottomMostUsed = bottom;
        }

        int holes = 0;
        for (int y = 0; y < bottomMostUsed; ++y)
        {
            for (int x = 0; x < columnCount; ++x)
            {
                if (grid[LinearIndex(x, y, columnCount)] != 0) continue;
                if (IsHoleCell(grid, columnCount, rowCount, x, y)) ++holes;
            }
        }
        return holes;
    }

    int ComputeAdjacencyMismatch(const Layout& layout, int columnCount, int rowCount)
    {
        std::vector<int> groupGrid(columnCount * rowCount, -1);
        std::vector<bool> isFiller(columnCount * rowCount, false);
        for (const WorkItem& item : layout.items)
        {
            if (item.phase != PHASE_FILLER) continue;
            const int idx = LinearIndex(item.targetX, item.targetY, columnCount);
            groupGrid[idx] = item.groupKey;
            isFiller[idx] = true;
        }

        const std::array<std::pair<int, int>, 4> deltas{ { {1, 0}, {-1, 0}, {0, 1}, {0, -1} } };
        int mismatches = 0;
        for (int y = 0; y < rowCount; ++y)
        {
            for (int x = 0; x < columnCount; ++x)
            {
                const int idx = LinearIndex(x, y, columnCount);
                if (!isFiller[idx]) continue;
                for (const auto& d : deltas)
                {
                    const int nx = x + d.first;
                    const int ny = y + d.second;
                    if (nx < 0 || ny < 0 || nx >= columnCount || ny >= rowCount) continue;
                    const int nidx = LinearIndex(nx, ny, columnCount);
                    if (!isFiller[nidx]) continue;
                    if (groupGrid[nidx] != groupGrid[idx]) ++mismatches;
                }
            }
        }
        return mismatches / 2;
    }

    // Independent score that reflects ONLY 1x1 grouping quality. Lower is better.
    // Used to gate cosmetic "1x1 cleanup" plans that do not move large items and
    // therefore barely shift the main total score.
    int ComputeOneByOneGroupingScore(const Layout& layout, int columnCount, int rowCount)
    {
        struct Pos { int x; int y; int groupKey; };
        std::vector<Pos> fillers;
        fillers.reserve(layout.items.size());
        for (const WorkItem& item : layout.items)
        {
            if (item.phase != PHASE_FILLER) continue;
            fillers.push_back({ item.targetX, item.targetY, item.groupKey });
        }
        if (fillers.size() < 2) return 0;

        int separation = 0;
        int adjacencyMatches = 0;
        for (size_t i = 0; i < fillers.size(); ++i)
        {
            for (size_t j = i + 1; j < fillers.size(); ++j)
            {
                if (fillers[i].groupKey != fillers[j].groupKey) continue;
                const int dx = fillers[i].x - fillers[j].x;
                const int dy = fillers[i].y - fillers[j].y;
                const int manhattan = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
                separation += manhattan;
                if (manhattan == 1) ++adjacencyMatches;
            }
        }
        (void)columnCount;
        (void)rowCount;
        return separation * W_GROUP_SEPARATION - adjacencyMatches * W_GROUP_ADJACENCY_REWARD;
    }

    int CountSameGroupAdjacencies(const Layout& layout, int columnCount, int rowCount)
    {
        std::vector<int> groupGrid(columnCount * rowCount, -1);
        for (const WorkItem& item : layout.items)
        {
            if (item.phase != PHASE_FILLER) continue;
            groupGrid[LinearIndex(item.targetX, item.targetY, columnCount)] = item.groupKey;
        }
        int matches = 0;
        for (int y = 0; y < rowCount; ++y)
        {
            for (int x = 0; x < columnCount; ++x)
            {
                const int g = groupGrid[LinearIndex(x, y, columnCount)];
                if (g < 0) continue;
                if (x + 1 < columnCount && groupGrid[LinearIndex(x + 1, y, columnCount)] == g) ++matches;
                if (y + 1 < rowCount && groupGrid[LinearIndex(x, y + 1, columnCount)] == g) ++matches;
            }
        }
        return matches;
    }

    void PlannerLog(const wchar_t* fmt, int a = 0, int b = 0, int c = 0, int d = 0, int e = 0, int f = 0)
    {
        if (!kPlannerDebugLogging) return;
        if (g_ConsoleDebug == nullptr) return;
        g_ConsoleDebug->Write(MCD_NORMAL, fmt, a, b, c, d, e, f);
    }

    void LogPlannerItem(const PlannerItem& item)
    {
        if (!kPlannerDebugLogging || !kPlannerDebugVerbose || g_ConsoleDebug == nullptr) return;
        const int type = (item.groupKey >> 16) & 0xFFFF;
        const int level = item.groupKey & 0xFFFF;
        const int area = item.width * item.height;
        g_ConsoleDebug->Write(MCD_NORMAL,
            L"[ArrangeItem] key=%08X type=%d level=%d w=%d h=%d area=%d source=(%d,%d) srcIdx=%d gk=%d",
            static_cast<int>(item.key), type, level, item.width, item.height, area,
            item.sourceX, item.sourceY, item.sourceIndex, item.groupKey);
    }

    void LogSortedRank(int rank, const WorkItem& item)
    {
        if (!kPlannerDebugLogging || !kPlannerDebugVerbose || g_ConsoleDebug == nullptr) return;
        const int type = (item.groupKey >> 16) & 0xFFFF;
        const int level = item.groupKey & 0xFFFF;
        const int area = item.width * item.height;
        g_ConsoleDebug->Write(MCD_NORMAL,
            L"[ArrangeSortOrder] rank=%d key=%08X type=%d level=%d w=%d h=%d area=%d gk=%d srcIdx=%d",
            rank, static_cast<int>(item.key), type, level, item.width, item.height, area,
            item.groupKey, item.sourceIndex);
    }

    void LogPass1Placement(int rank, const WorkItem& item)
    {
        if (!kPlannerDebugLogging || !kPlannerDebugVerbose || g_ConsoleDebug == nullptr) return;
        // mode: 0=wide(row-major), 1=tall(column-major), 2=filler(row-major)
        const int mode = static_cast<int>(item.phase);
        g_ConsoleDebug->Write(MCD_NORMAL,
            L"[ArrangePass1] rank=%d key=%08X w=%d h=%d mode=%d placed=(%d,%d)",
            rank, static_cast<int>(item.key), item.width, item.height, mode,
            item.targetX, item.targetY);
    }

    void LogStructuralViolation(const wchar_t* pass, const WorkItem& item, int newX, int newY)
    {
        if (!kPlannerDebugLogging || g_ConsoleDebug == nullptr) return;
        g_ConsoleDebug->Write(MCD_NORMAL,
            L"[ArrangeError] pass=%s key=%08X w=%d h=%d oldTarget=(%d,%d) newTarget=(%d,%d)",
            pass, static_cast<int>(item.key), item.width, item.height,
            item.targetX, item.targetY, newX, newY);
    }

    // Diagnostic: number of pairs (a, b) where a precedes b in row-major
    // traversal yet a is strictly shorter than b. A canonical height-first
    // layout has zero violations; the messier the layout, the higher the
    // count. Cheap to compute (items in a 8x8 inventory are <= 64).
    int CountHeightOrderViolations(const Layout& layout, int columnCount)
    {
        int violations = 0;
        const size_t n = layout.items.size();
        for (size_t i = 0; i < n; ++i)
        {
            const WorkItem& a = layout.items[i];
            const int posA = LinearIndex(a.targetX, a.targetY, columnCount);
            for (size_t j = i + 1; j < n; ++j)
            {
                const WorkItem& b = layout.items[j];
                const int posB = LinearIndex(b.targetX, b.targetY, columnCount);
                if (posA < posB && a.height < b.height) ++violations;
                else if (posB < posA && b.height < a.height) ++violations;
            }
        }
        return violations;
    }

    // Sum over shape groups (width, height) of (boundingBoxArea - occupiedArea).
    // Zero when all instances of a shape sit in a tight contiguous block; grows
    // when one instance gets stranded away from its siblings.
    int ComputeShapeFragmentation(const Layout& layout)
    {
        // Items in a 8x8 inventory are <= 64, simple O(n^2) grouping is fine.
        struct ShapeBucket { int w; int h; int n; int minX; int maxX; int minY; int maxY; };
        std::vector<ShapeBucket> buckets;
        buckets.reserve(layout.items.size());
        for (const WorkItem& item : layout.items)
        {
            ShapeBucket* found = nullptr;
            for (ShapeBucket& b : buckets)
            {
                if (b.w == item.width && b.h == item.height) { found = &b; break; }
            }
            const int right = item.targetX + item.width;
            const int bottom = item.targetY + item.height;
            if (found == nullptr)
            {
                buckets.push_back({ item.width, item.height, 1,
                    item.targetX, right, item.targetY, bottom });
            }
            else
            {
                ++found->n;
                if (item.targetX < found->minX) found->minX = item.targetX;
                if (right > found->maxX) found->maxX = right;
                if (item.targetY < found->minY) found->minY = item.targetY;
                if (bottom > found->maxY) found->maxY = bottom;
            }
        }
        int fragmentation = 0;
        for (const ShapeBucket& b : buckets)
        {
            if (b.n < 2) continue;
            const int boundingArea = (b.maxX - b.minX) * (b.maxY - b.minY);
            const int occupiedArea = b.n * b.w * b.h;
            const int diff = boundingArea - occupiedArea;
            if (diff > 0) fragmentation += diff;
        }
        return fragmentation;
    }

    int ComputeLargestEmptyRectangle(const Layout& layout, int columnCount, int rowCount)
    {
        std::vector<bool> occupied(columnCount * rowCount, false);
        for (const WorkItem& item : layout.items)
        {
            for (int dy = 0; dy < item.height; ++dy)
            {
                for (int dx = 0; dx < item.width; ++dx)
                {
                    occupied[LinearIndex(item.targetX + dx, item.targetY + dy, columnCount)] = true;
                }
            }
        }

        std::vector<int> heights(columnCount, 0);
        int best = 0;
        for (int y = 0; y < rowCount; ++y)
        {
            for (int x = 0; x < columnCount; ++x)
            {
                heights[x] = occupied[LinearIndex(x, y, columnCount)] ? 0 : heights[x] + 1;
            }
            // Largest-rectangle-in-histogram for this row.
            std::vector<int> stack;
            stack.reserve(static_cast<size_t>(columnCount) + 1);
            for (int x = 0; x <= columnCount; ++x)
            {
                const int h = x == columnCount ? 0 : heights[x];
                while (!stack.empty() && heights[stack.back()] > h)
                {
                    const int top = stack.back();
                    stack.pop_back();
                    const int width = stack.empty() ? x : x - stack.back() - 1;
                    const int area = heights[top] * width;
                    if (area > best) best = area;
                }
                stack.push_back(x);
            }
        }
        return best;
    }

    int ScoreLayout(const Layout& layout, int columnCount, int rowCount, int moveCount)
    {
        int compactness = 0;
        const int holes = CountHolesAndCompactness(layout, columnCount, rowCount, compactness);
        const int mismatches = ComputeAdjacencyMismatch(layout, columnCount, rowCount);
        const int largestRect = ComputeLargestEmptyRectangle(layout, columnCount, rowCount);
        return compactness * W_COMPACTNESS
            + holes * W_HOLE
            + mismatches * W_ADJACENCY
            - largestRect * W_LARGEST_RECT
            + moveCount * W_MOVES;
    }

    int CountMovesNeeded(const Layout& layout)
    {
        int moves = 0;
        for (const WorkItem& item : layout.items)
        {
            if (!ItemAtTarget(item)) ++moves;
        }
        return moves;
    }

    WorkItem* FindByKey(std::vector<WorkItem>& items, DWORD key)
    {
        for (WorkItem& item : items)
        {
            if (item.key == key) return &item;
        }
        return nullptr;
    }

    DWORD FindTargetBlocker(const std::vector<DWORD>& grid, int columnCount, const WorkItem& item)
    {
        for (int dy = 0; dy < item.height; ++dy)
        {
            for (int dx = 0; dx < item.width; ++dx)
            {
                const DWORD cell = grid[LinearIndex(item.targetX + dx, item.targetY + dy, columnCount)];
                if (cell != 0 && cell != item.key) return cell;
            }
        }
        return 0;
    }

    bool FindTemporarySlot(const std::vector<DWORD>& grid, const std::vector<bool>& planned, int columnCount, int rowCount, const WorkItem& item, int& outX, int& outY)
    {
        const int maxRow = rowCount - item.height;
        const int maxColumn = columnCount - item.width;
        for (int pass = 0; pass < 2; ++pass)
        {
            const bool avoidPlanned = pass == 0;
            for (int row = maxRow; row >= 0; --row)
            {
                for (int column = maxColumn; column >= 0; --column)
                {
                    if (column == item.sourceX && row == item.sourceY) continue;
                    bool ok = true;
                    for (int dy = 0; dy < item.height && ok; ++dy)
                    {
                        for (int dx = 0; dx < item.width && ok; ++dx)
                        {
                            const int index = LinearIndex(column + dx, row + dy, columnCount);
                            if (avoidPlanned && planned[index]) { ok = false; break; }
                            const DWORD cell = grid[index];
                            if (cell != 0 && cell != item.key) { ok = false; break; }
                        }
                    }
                    if (ok)
                    {
                        outX = column;
                        outY = row;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void EmitMove(std::vector<PlannerMove>& moves, const PlannerSettings& settings, WorkItem& item, int newX, int newY, int columnCount, std::vector<DWORD>& grid)
    {
        StampOccupancy(grid, columnCount, item.sourceX, item.sourceY, item.width, item.height, 0);
        item.sourceX = newX;
        item.sourceY = newY;
        StampOccupancy(grid, columnCount, item.sourceX, item.sourceY, item.width, item.height, item.key);
        moves.push_back({ item.key, LinearIndex(newX, newY, columnCount) + settings.equipmentOffset });
    }

    bool GenerateMoves(Layout workingLayout, int columnCount, int rowCount, const PlannerSettings& settings, std::vector<PlannerMove>& outMoves)
    {
        std::vector<WorkItem>& items = workingLayout.items;
        std::vector<DWORD> grid(columnCount * rowCount, 0);
        std::vector<bool> planned(columnCount * rowCount, false);
        for (const WorkItem& item : items)
        {
            StampOccupancy(grid, columnCount, item.sourceX, item.sourceY, item.width, item.height, item.key);
            for (int dy = 0; dy < item.height; ++dy)
            {
                for (int dx = 0; dx < item.width; ++dx)
                {
                    planned[LinearIndex(item.targetX + dx, item.targetY + dy, columnCount)] = true;
                }
            }
        }

        const size_t safetyCap = items.size() * static_cast<size_t>(columnCount) * static_cast<size_t>(rowCount) + 1;
        while (true)
        {
            if (outMoves.size() > safetyCap) return false;

            bool pending = false;
            bool moved = false;
            for (WorkItem& item : items)
            {
                if (ItemAtTarget(item)) continue;
                pending = true;
                if (!RegionMatches(grid, columnCount, item.targetX, item.targetY, item.width, item.height, item.key)) continue;
                EmitMove(outMoves, settings, item, item.targetX, item.targetY, columnCount, grid);
                moved = true;
                break;
            }
            if (!pending) return true;
            if (moved) continue;

            // No direct move possible: evict one blocker into a temp slot.
            for (WorkItem& item : items)
            {
                if (ItemAtTarget(item)) continue;
                const DWORD blockerKey = FindTargetBlocker(grid, columnCount, item);
                if (blockerKey == 0) continue;
                WorkItem* pBlocker = FindByKey(items, blockerKey);
                if (pBlocker == nullptr) continue;
                int tx = 0;
                int ty = 0;
                if (!FindTemporarySlot(grid, planned, columnCount, rowCount, *pBlocker, tx, ty)) continue;
                EmitMove(outMoves, settings, *pBlocker, tx, ty, columnCount, grid);
                moved = true;
                break;
            }
            if (!moved) return false;
        }
    }

    bool SimulateMoves(const Layout& plannedLayout, int columnCount, int rowCount, const PlannerSettings& settings, const std::vector<PlannerMove>& moves)
    {
        if (columnCount <= 0 || rowCount <= 0) return false;
        struct SimItem { DWORD key; int x; int y; int tx; int ty; int width; int height; };
        std::vector<SimItem> sim;
        sim.reserve(plannedLayout.items.size());
        std::vector<DWORD> grid(columnCount * rowCount, 0);
        for (const WorkItem& item : plannedLayout.items)
        {
            if (!FitsInBounds(item.sourceX, item.sourceY, item.width, item.height, columnCount, rowCount)) return false;
            for (int dy = 0; dy < item.height; ++dy)
            {
                for (int dx = 0; dx < item.width; ++dx)
                {
                    const int index = LinearIndex(item.sourceX + dx, item.sourceY + dy, columnCount);
                    if (grid[index] != 0) return false;
                    grid[index] = item.key;
                }
            }
            sim.push_back({ item.key, item.sourceX, item.sourceY, item.targetX, item.targetY, item.width, item.height });
        }

        for (const PlannerMove& move : moves)
        {
            SimItem* pItem = nullptr;
            for (SimItem& candidate : sim)
            {
                if (candidate.key == move.itemKey) { pItem = &candidate; break; }
            }
            if (pItem == nullptr) return false;
            const int linear = move.targetIndex - settings.equipmentOffset;
            if (linear < 0 || linear >= columnCount * rowCount) return false;
            const int tx = linear % columnCount;
            const int ty = linear / columnCount;
            if (!FitsInBounds(tx, ty, pItem->width, pItem->height, columnCount, rowCount)) return false;
            for (int dy = 0; dy < pItem->height; ++dy)
            {
                for (int dx = 0; dx < pItem->width; ++dx)
                {
                    const DWORD cell = grid[LinearIndex(tx + dx, ty + dy, columnCount)];
                    if (cell != 0 && cell != pItem->key) return false;
                }
            }
            StampOccupancy(grid, columnCount, pItem->x, pItem->y, pItem->width, pItem->height, 0);
            pItem->x = tx;
            pItem->y = ty;
            StampOccupancy(grid, columnCount, pItem->x, pItem->y, pItem->width, pItem->height, pItem->key);
        }

        for (const SimItem& s : sim)
        {
            if (s.x != s.tx || s.y != s.ty) return false;
        }
        return true;
    }

    struct PipelineRun
    {
        Layout best;
        int structuralScore;    // ScoreLayout(best, moves=0)
        int groupScore;         // ComputeOneByOneGroupingScore(best)
        int adoptedPasses;      // 1..MAX_PLAN_PASSES
    };

    // Walks two layouts in lockstep (same item set, same order) and logs an
    // [ArrangeError] line for every structural item (width>1 OR height>1)
    // whose target differs between the two. Returns the number of violations.
    int CheckStructuralInvariant(const Layout& reference, const Layout& candidate, const wchar_t* passName)
    {
        int violations = 0;
        for (const WorkItem& refItem : reference.items)
        {
            if (refItem.width <= 1 && refItem.height <= 1) continue;
            for (const WorkItem& candItem : candidate.items)
            {
                if (candItem.key != refItem.key) continue;
                if (candItem.targetX != refItem.targetX || candItem.targetY != refItem.targetY)
                {
                    LogStructuralViolation(passName, refItem, candItem.targetX, candItem.targetY);
                    ++violations;
                }
                break;
            }
        }
        return violations;
    }

    // Runs the existing pass1/2/3 pipeline against a single "current" layout
    // and returns the best target found. Does not touch the original snapshot
    // or emit moves; the caller is responsible for chaining iterations.
    bool RunPassPipeline(const Layout& currentLayout, int columnCount, int rowCount, const PlannerSettings& settings, PipelineRun& out, int iterIndex, bool verbose)
    {
        Layout pass1 = currentLayout;
        if (!BuildPass1Layout(pass1, columnCount, rowCount, verbose))
        {
            PlannerLog(L"[ArrangeSort] iter=%d pass1 build failed", iterIndex);
            return false;
        }
        if (!ValidateLayout(pass1, columnCount, rowCount))
        {
            PlannerLog(L"[ArrangeSort] iter=%d pass1 validate failed", iterIndex);
            return false;
        }
        int pass1Score = ScoreLayout(pass1, columnCount, rowCount, CountMovesNeeded(pass1));
        int pass1Group = ComputeOneByOneGroupingScore(pass1, columnCount, rowCount);

        Layout best = pass1;
        int bestScore = pass1Score;
        int bestGroup = pass1Group;
        int adopted = 1;

        Layout pass2;
        if (adopted < MAX_PLAN_PASSES && BuildPass2Layout(best, pass2, columnCount, rowCount)
            && ValidateLayout(pass2, columnCount, rowCount))
        {
            const int viol = CheckStructuralInvariant(pass1, pass2, L"Pass2");
            (void)viol;
            const int score = ScoreLayout(pass2, columnCount, rowCount, CountMovesNeeded(pass2));
            const int group = ComputeOneByOneGroupingScore(pass2, columnCount, rowCount);
            const bool ok = (score < bestScore - settings.passProgressionThreshold)
                || (group < bestGroup && score <= bestScore);
            if (ok)
            {
                best = pass2;
                bestScore = score;
                bestGroup = group;
                ++adopted;
            }
        }

        Layout pass3;
        if (adopted < MAX_PLAN_PASSES && BuildPass3Layout(best, pass3, columnCount, rowCount)
            && ValidateLayout(pass3, columnCount, rowCount))
        {
            const int viol = CheckStructuralInvariant(pass1, pass3, L"Pass3");
            (void)viol;
            const int score = ScoreLayout(pass3, columnCount, rowCount, CountMovesNeeded(pass3));
            const int group = ComputeOneByOneGroupingScore(pass3, columnCount, rowCount);
            const bool ok = (score < bestScore - settings.passProgressionThreshold)
                || (group < bestGroup && score <= bestScore);
            if (ok)
            {
                best = pass3;
                bestScore = score;
                bestGroup = group;
                ++adopted;
            }
        }

        out.best = std::move(best);
        out.structuralScore = ScoreLayout(out.best, columnCount, rowCount, /*moves*/ 0);
        out.groupScore = bestGroup;
        out.adoptedPasses = adopted;
        return true;
    }

    Layout BuildCurrentLayout(const std::vector<PlannerItem>& items)
    {
        Layout layout;
        layout.items.reserve(items.size());
        for (const PlannerItem& src : items)
        {
            WorkItem w;
            w.key = src.key;
            w.sourceIndex = src.sourceIndex;
            w.sourceX = src.sourceX;
            w.sourceY = src.sourceY;
            w.targetX = src.sourceX;
            w.targetY = src.sourceY;
            w.width = src.width;
            w.height = src.height;
            w.groupKey = src.groupKey;
            w.phase = ClassifyPhase(src.width, src.height);
            layout.items.push_back(w);
        }
        return layout;
    }

    bool TryPlan(const Layout& candidate, int columnCount, int rowCount, const PlannerSettings& settings, std::vector<PlannerMove>& outMoves)
    {
        outMoves.clear();
        if (!ValidateLayout(candidate, columnCount, rowCount)) return false;
        if (!GenerateMoves(candidate, columnCount, rowCount, settings, outMoves)) return false;
        if (outMoves.empty()) return false;
        if (!SimulateMoves(candidate, columnCount, rowCount, settings, outMoves)) return false;
        return true;
    }
}

bool IsSupportedItemSize(int width, int height)
{
    return width >= MIN_ITEM_SIZE && width <= MAX_ITEM_SIZE
        && height >= MIN_ITEM_SIZE && height <= MAX_ITEM_SIZE;
}

PlannerResult Plan(const std::vector<PlannerItem>& items, int columnCount, int rowCount, const PlannerSettings& settings)
{
    PlannerResult result;
    result.hasPlan = false;

    if (columnCount <= 0 || rowCount <= 0) return result;
    for (const PlannerItem& it : items)
    {
        if (!IsSupportedItemSize(it.width, it.height)) return result;
        if (!FitsInBounds(it.sourceX, it.sourceY, it.width, it.height, columnCount, rowCount)) return result;
    }

    // Snapshot dump: one [ArrangeItem] line per item so we can confirm every
    // metadata field the planner is actually working with.
    for (const PlannerItem& it : items) LogPlannerItem(it);

    Layout original = BuildCurrentLayout(items);
    if (!ValidateLayout(original, columnCount, rowCount)) return result;
    const int originalScore = ScoreLayout(original, columnCount, rowCount, /*moves*/ 0);
    const int originalGroup = ComputeOneByOneGroupingScore(original, columnCount, rowCount);
    const int originalViol = CountHeightOrderViolations(original, columnCount);
    const int originalFrag = ComputeShapeFragmentation(original);
    PlannerLog(L"[ArrangeSort] start score=%d group=%d viol=%d frag=%d", originalScore, originalGroup, originalViol, originalFrag);

    // Rolling virtual state: starts at the real snapshot, then each accepted
    // iteration replaces its sourceX/Y with the previous candidate's targetX/Y.
    // This is what lets the sort tiebreaker (sourceIndex) converge in memory
    // instead of forcing the user to click arrange multiple times.
    Layout virtualState = original;
    int virtualStruct = originalScore;
    int virtualGroup = originalGroup;

    // Per-item final target tracked across iterations. Initialised to the
    // original positions so a do-nothing convergence ends with no moves.
    std::vector<std::pair<DWORD, std::pair<int, int>>> finalTargets;
    finalTargets.reserve(original.items.size());
    for (const WorkItem& it : original.items)
    {
        finalTargets.push_back({ it.key, { it.targetX, it.targetY } });
    }

    int converged = -1;
    for (int iter = 0; iter < kMaxArrangeConvergenceIterations; ++iter)
    {
        PipelineRun run;
        // Only the first iteration emits the noisy per-item logs; later
        // iterations would just repeat the same data.
        const bool verbose = (iter == 0);
        if (!RunPassPipeline(virtualState, columnCount, rowCount, settings, run, iter, verbose))
        {
            PlannerLog(L"[ArrangeSort] iter=%d pipeline aborted", iter);
            break;
        }

        const int candStruct = run.structuralScore;
        const int candGroup = run.groupScore;
        const int candViol = CountHeightOrderViolations(run.best, columnCount);
        const int candFrag = ComputeShapeFragmentation(run.best);
        const int candMoves = CountMovesNeeded(run.best);
        // Score breakdown for the candidate layout (moves=0 component view).
        int candCompactness = 0;
        const int candHoles = CountHolesAndCompactness(run.best, columnCount, rowCount, candCompactness);
        const int candMismatches = ComputeAdjacencyMismatch(run.best, columnCount, rowCount);
        const int candLargest = ComputeLargestEmptyRectangle(run.best, columnCount, rowCount);
        const bool improvesStruct = candStruct < virtualStruct - settings.passProgressionThreshold;
        const bool improvesGroupNoReg = candGroup < virtualGroup && candStruct <= virtualStruct;
        const bool adopt = improvesStruct || improvesGroupNoReg;

        PlannerLog(L"[ArrangeSort] iter=%d virtStruct=%d candStruct=%d candGroup=%d viol=%d adopt=%d",
            iter, virtualStruct, candStruct, candGroup, candViol, adopt ? 1 : 0);
        PlannerLog(L"[ArrangeScore] iter=%d compact=%d holes=%d adjMis=%d moves=%d largestRect=%d",
            iter, candCompactness, candHoles, candMismatches, candMoves, candLargest);
        PlannerLog(L"[ArrangeScore] iter=%d group1x1=%d shapeFrag=%d (weight=%d)",
            iter, candGroup, candFrag, kShapeFragmentationWeight);

        if (!adopt)
        {
            converged = iter;
            break;
        }

        // Update finalTargets with this iteration's target positions.
        for (const WorkItem& cand : run.best.items)
        {
            for (auto& entry : finalTargets)
            {
                if (entry.first == cand.key)
                {
                    entry.second.first = cand.targetX;
                    entry.second.second = cand.targetY;
                    break;
                }
            }
        }

        // Roll virtualState forward: source = previous target.
        virtualState = run.best;
        for (WorkItem& it : virtualState.items)
        {
            it.sourceX = it.targetX;
            it.sourceY = it.targetY;
            it.sourceIndex = LinearIndex(it.targetX, it.targetY, columnCount);
        }
        virtualStruct = candStruct;
        virtualGroup = candGroup;
    }
    if (converged < 0) PlannerLog(L"[ArrangeSort] convergence cap reached");

    // Build the delivery layout: original sources, converged targets.
    Layout delivery = original;
    for (WorkItem& dit : delivery.items)
    {
        for (const auto& entry : finalTargets)
        {
            if (entry.first == dit.key)
            {
                dit.targetX = entry.second.first;
                dit.targetY = entry.second.second;
                break;
            }
        }
    }

    if (!ValidateLayout(delivery, columnCount, rowCount))
    {
        PlannerLog(L"[ArrangeSort] delivery validate failed");
        return result;
    }

    const int deliveryStruct = ScoreLayout(delivery, columnCount, rowCount, /*moves*/ 0);
    const int deliveryGroup = ComputeOneByOneGroupingScore(delivery, columnCount, rowCount);
    const int deliveryViol = CountHeightOrderViolations(delivery, columnCount);
    const bool acceptByStruct = deliveryStruct < originalScore - settings.acceptanceThreshold;
    const bool acceptByGroup = deliveryGroup < originalGroup - settings.oneByOneCleanupThreshold
        && deliveryStruct <= originalScore;
    PlannerLog(L"[ArrangeSort] final struct=%d group=%d viol=%d byStruct=%d byGroup=%d",
        deliveryStruct, deliveryGroup, deliveryViol,
        acceptByStruct ? 1 : 0, acceptByGroup ? 1 : 0);
    if (!acceptByStruct && !acceptByGroup)
    {
        PlannerLog(L"[ArrangeSort] no plan adopted");
        return result;
    }

    std::vector<PlannerMove> moves;
    if (!TryPlan(delivery, columnCount, rowCount, settings, moves))
    {
        PlannerLog(L"[ArrangeSort] delivery plan unrealizable");
        return result;
    }

    int oneByOneMoves = 0;
    for (const PlannerMove& m : moves)
    {
        for (const WorkItem& w : delivery.items)
        {
            if (w.key != m.itemKey) continue;
            if (w.width == 1 && w.height == 1) ++oneByOneMoves;
            break;
        }
    }
    PlannerLog(L"[ArrangeSort] emitting moves=%d (1x1=%d)", static_cast<int>(moves.size()), oneByOneMoves);

    result.hasPlan = true;
    result.moves = std::move(moves);
    return result;
}

} // namespace Sorting
} // namespace Inventory
} // namespace UI
