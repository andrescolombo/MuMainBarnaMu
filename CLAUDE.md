# Claude Guidelines (CLAUDE.md)

This file contains instructions specifically for Claude-based agents (like Claude Code, Claude in Cursor, and other Claude extensions).

## Build Commands
> [!CRITICAL]
> **DO NOT BUILD**: AI must **NEVER** run any build, compile, or configuration commands (e.g., `cmake`, `msbuild`, `dotnet`, `make`). Compilation is handled exclusively and manually by the user.

## Test Commands
> [!CRITICAL]
> **DO NOT TEST**: AI must **NEVER** run any automated testing suites (e.g., `ctest`, `dotnet test`). Testing is handled exclusively and manually by the user.

## Git Commands
> [!CRITICAL]
> **DO NOT PUSH**: AI must **NEVER** run `git push` without specific "push" command from user. Remote pushes are strictly out of scope without authorization.

## Code Style & Conventions
- **Exit Early**: Place guard conditions at the top of functions (`if (bad) return;`). Avoid deep nesting.
- **Max Function Length**: Keep functions short (max 40 lines). Extract helper functions on touch.
- **No Magic Numbers**: Give numbers, colors, times, and array offsets named constants (`constexpr` / `const` / `enum`).
- **No Heap Allocations in Hot Paths**: Inside rendering loops or per-frame character updates, do NOT allocate memory on the heap (avoid `new`, `std::vector`, or `std::string` instantiation).
- **Core Architecture References**: Refer to [AGENTS.md](file:///c:/Users/Andres/MuMain/MuMain/AGENTS.md), [LLM.md](file:///c:/Users/Andres/MuMain/MuMain/LLM.md), and [docs/CODING_RULES.md](file:///c:/Users/Andres/MuMain/MuMain/docs/CODING_RULES.md) for full architecture and styling rules.

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).

<!-- gitnexus:start -->
# GitNexus — Code Intelligence

This project is indexed by GitNexus as **MuMain** (31905 symbols, 74114 relationships, 101 execution flows). Use the GitNexus MCP tools to understand code, assess impact, and navigate safely.

> If any GitNexus tool warns the index is stale, run `npx gitnexus analyze` in terminal first.

## Always Do

- **MUST run impact analysis before editing any symbol.** Before modifying a function, class, or method, run `gitnexus_impact({target: "symbolName", direction: "upstream"})` and report the blast radius (direct callers, affected processes, risk level) to the user.
- **MUST run `gitnexus_detect_changes()` before committing** to verify your changes only affect expected symbols and execution flows.
- **MUST warn the user** if impact analysis returns HIGH or CRITICAL risk before proceeding with edits.
- When exploring unfamiliar code, use `gitnexus_query({query: "concept"})` to find execution flows instead of grepping. It returns process-grouped results ranked by relevance.
- When you need full context on a specific symbol — callers, callees, which execution flows it participates in — use `gitnexus_context({name: "symbolName"})`.

## Never Do

- NEVER edit a function, class, or method without first running `gitnexus_impact` on it.
- NEVER ignore HIGH or CRITICAL risk warnings from impact analysis.
- NEVER rename symbols with find-and-replace — use `gitnexus_rename` which understands the call graph.
- NEVER commit changes without running `gitnexus_detect_changes()` to check affected scope.

## Resources

| Resource | Use for |
|----------|---------|
| `gitnexus://repo/MuMain/context` | Codebase overview, check index freshness |
| `gitnexus://repo/MuMain/clusters` | All functional areas |
| `gitnexus://repo/MuMain/processes` | All execution flows |
| `gitnexus://repo/MuMain/process/{name}` | Step-by-step execution trace |

## CLI

| Task | Read this skill file |
|------|---------------------|
| Understand architecture / "How does X work?" | `.claude/skills/gitnexus/gitnexus-exploring/SKILL.md` |
| Blast radius / "What breaks if I change X?" | `.claude/skills/gitnexus/gitnexus-impact-analysis/SKILL.md` |
| Trace bugs / "Why is X failing?" | `.claude/skills/gitnexus/gitnexus-debugging/SKILL.md` |
| Rename / extract / split / refactor | `.claude/skills/gitnexus/gitnexus-refactoring/SKILL.md` |
| Tools, resources, schema reference | `.claude/skills/gitnexus/gitnexus-guide/SKILL.md` |
| Index, status, clean, wiki CLI commands | `.claude/skills/gitnexus/gitnexus-cli/SKILL.md` |

<!-- gitnexus:end -->
