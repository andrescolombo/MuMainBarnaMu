# Agent Instructions

This file is the entry point for AI coding assistants (Claude Code, Cursor, Codex,
etc.) working in this repository. It also applies to human contributors.

For a comprehensive, high-density architectural overview and file directory mapping of common tasks (such as UI, network protocol, 3D camera, rendering, and translations), please refer to [LLM.md](LLM.md).

## Coding rules — read first

**The coding rules in [`docs/CODING_RULES.md`](docs/CODING_RULES.md) apply to all
code changes in this repository, by humans and by AI agents alike.**

Before writing or modifying code:

1. Read [`docs/CODING_RULES.md`](docs/CODING_RULES.md).
2. Apply the rules to any new or changed code.
3. When reviewing a change (including via `/review` or `/ultrareview`), treat
   `docs/CODING_RULES.md` as the authoritative style and quality baseline.

The rules are not exhaustive — they capture the conventions that matter most for
this codebase. Follow the style of surrounding code where the rules don't speak.

## Build setup

The project uses CMake. Supported build environments and setup steps are
documented in [`docs/build-guide.md`](docs/build-guide.md).

Quick references:

- Requirements and IDE-specific instructions: [`README.md`](README.md).
- Cross-platform / WSL build details: [`docs/build-guide.md`](docs/build-guide.md).

## Branch and PR conventions

- Target branch for PRs is `main`.
- Keep changes focused — one concern per commit (see rule 10 in the coding
  rules). Smaller diffs review faster.
- Match the style of existing commit messages in `git log`.
- Reference the related issue in the PR description when applicable.

## Out of scope for AI changes

Don't perform large retroactive cleanups of existing code to fit the rules unless
the user explicitly asks for it. Apply the rules going forward; pre-existing
code can be refactored opportunistically when you're already touching it.

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

When the user types `/graphify`, invoke the `skill` tool with `skill: "graphify"` before doing anything else.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- Dirty graphify-out/ files are expected after hooks or incremental updates; dirty graph files are not a reason to skip graphify. Only skip graphify if the task is about stale or incorrect graph output, or the user explicitly says not to use it.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).
