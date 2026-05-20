# Copilot & ChatGPT Guidelines (.github/copilot-instructions.md)

This file contains instructions for ChatGPT and GitHub Copilot extensions working in the MuOnline S6 Client repository.

## 1. CRITICAL CONSTRAINTS (NO EXCEPTIONS)
* **DO NOT BUILD**: AI must **NEVER** run or propose running any build, compile, or configuration commands (e.g., `cmake`, `msbuild`, `dotnet`, `make`). The user compiles manually.
* **DO NOT TEST**: AI must **NEVER** run or propose running any automated testing suites (e.g., `ctest`, `dotnet test`).
* **DO NOT PUSH**: AI must **NEVER** execute `git push`.
* **ASK IF UNSURE**: If requirements are ambiguous, stop and ask the user directly instead of guessing.

## 2. STRICT CODING RULES
- **Exit Early**: Put guard conditions at the top of functions (`if (bad) return;`). Avoid deep nesting.
- **Short Functions**: Max 40 lines. Extract logical blocks on touch.
- **No Magic Numbers**: Give numbers, colors, times named constants (`constexpr` / `const` / `enum`).
- **No Heap Allocations on Hot Paths**: In functions called per-frame (Inside rendering loops, character updates, and vertex generation), **do not allocate memory on the heap** (avoid constructing `std::vector`, `std::string` or using `new`).
- **One Class per File**: Separate C++ classes into matching `.h`/`.cpp` files. One public type per C# `.cs` file.

## 3. CORE REFERENCES
- Refer to [AGENTS.md](file:///c:/Users/Andres/MuMain/MuMain/AGENTS.md), [LLM.md](file:///c:/Users/Andres/MuMain/MuMain/LLM.md), and [docs/CODING_RULES.md](file:///c:/Users/Andres/MuMain/MuMain/docs/CODING_RULES.md) for full architecture and styling rules.
