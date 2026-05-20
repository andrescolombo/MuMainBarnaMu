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
> **DO NOT PUSH**: AI must **NEVER** run `git push`. Remote pushes are strictly out of scope.

## Code Style & Conventions
- **Exit Early**: Place guard conditions at the top of functions (`if (bad) return;`). Avoid deep nesting.
- **Max Function Length**: Keep functions short (max 40 lines). Extract helper functions on touch.
- **No Magic Numbers**: Give numbers, colors, times, and array offsets named constants (`constexpr` / `const` / `enum`).
- **No Heap Allocations in Hot Paths**: Inside rendering loops or per-frame character updates, do NOT allocate memory on the heap (avoid `new`, `std::vector`, or `std::string` instantiation).
- **Core Architecture References**: Refer to [AGENTS.md](file:///c:/Users/Andres/MuMain/MuMain/AGENTS.md), [LLM.md](file:///c:/Users/Andres/MuMain/MuMain/LLM.md), and [docs/CODING_RULES.md](file:///c:/Users/Andres/MuMain/MuMain/docs/CODING_RULES.md) for full architecture and styling rules.
