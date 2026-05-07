# NOTES.md

Project state, known issues, and what to do next.
This is the working notebook - update freely.

## Origin

Started March 23, 2025 as a personal project to extract dynasty trees
from CK3 save files. Hit a flow-state development streak through early
April 2025, then burnout. Picked up again May 2026 with the migration
to this Canon repo.

The V3 (predecessor) repo lives at:
`D:\Final Week Drive Recovery\Centralized CK3 project 2026\(V3)_CK3_Dynasty_Project\`

That folder is left untouched as a backup. Migrate fixes here, not there.

## Current state (V5)

The parser works end-to-end: load -> filter to character blocks ->
build numeric-id-keyed map -> recursive descendant + spouse walk
from a hardcoded founder ID -> result map. No JSON output yet.
No GUI yet. Founder ID is hardcoded.

### Known issues / cleanup targets

These are real but deferred. Listed roughly in order of how much they
matter, not how easy they are.

#### Architecture / correctness

- [ ] `blockMapping` materializes every character block as `vector<string>`
      (full copies). This holds the buffer alive in memory and is the
      single biggest reason peak memory is ~810 MB. The map should hold
      `string_view` ranges into a kept-alive buffer, OR (better) parse
      directly into typed structs and free the buffer.
- [ ] Founder ID is hardcoded (`int founderID = 37676;` in
      `pointerPlayOptimized`). Make it a parameter / CLI arg.
- [ ] No JSON output yet. The `nlohmann/json` header is vendored in
      `include/json.hpp` ready to use.

#### Performance

- [ ] `blockMapping` uses `std::regex` per line. A manual scan
      (`isdigit` + check for `={`) would be 5-10x faster on the hot path.
- [ ] `extractIdsFromLine` also uses regex. Same fix applies.
- [ ] `std::map<int, ...>` should be `std::unordered_map<int, ...>` -
      we don't need ordered iteration over character IDs.

#### Code hygiene (low stakes, easy wins)

- [ ] Unused `ltrim` lambda in `filterCharacterBlocks` (line ~109).
- [ ] Unused `buffer` parameter in `filterCharacterBlocks`.
- [ ] `memoryLogging` has a control path with no return value
      (compiler warning).
- [ ] `int lineNumBefore = allLines.size();` truncates - use `size_t`.
- [ ] `firstNameCheck` reimplements `std::string_view::starts_with`
      (available since C++20).
- [ ] Header guards in `.cpp` files (memory_utill.cpp, string_utill.cpp)
      are unusual - residue from when these were `#include`d directly.
- [ ] `using namespace std;` at file scope in `.cpp` and `.hpp` files.

## Roadmap (rough)

### Near-term

1. Fix the warnings and quick-win hygiene items above.
2. Replace `std::map` with `std::unordered_map`.
3. Replace regex calls with manual scanning.
4. Make founder ID a CLI argument.
5. Add JSON output for the dynasty map.

### Medium-term

6. Replace `vector<string>` block storage with typed `Character` structs.
   This unlocks freeing the buffer mid-parse and drops peak memory
   significantly.
7. Add a CLI flag system (`--founder-id`, `--input`, `--output`,
   `--quiet`, etc.).
8. Write a real GUI layer. No experience here yet - candidates to
   evaluate include Qt, Dear ImGui, and just rendering to HTML and
   serving from a small embedded server.

### Long-term / speculative

9. Generalize the parser core into a Layer 1 / Layer 2 / Layer 3
   architecture (generic Clausewitz block extractor + per-game schemas
   + per-game extractors). Would let the same core handle Stellaris,
   EU4, etc. See chat history for the full sketch.

## Decisions made

- **Repo name "Canon"** vs "V4" or "V5" intentionally - this is the
  reference implementation going forward, not another iteration to be
  superseded. Future cleanup happens in this repo.
- **Old V3 repo not deleted.** Insurance until this repo has been
  worked in for a few weeks.
- **Folder names: `src/`, `include/`, `scripts/`, `tests/`, `legacy/`.**
  Standard C++ project naming. The old `source/` / `library/` /
  `source-experimental/` naming was idiosyncratic.
- **Legacy files renamed with `v1-`, `v2-`, `v3-`, `v4-`, `experimental-`
  prefixes.** Alphabetical sort = chronological project history.
- **Source files NOT renamed.** `parser-V5.cpp`, `memory_utill.cpp`,
  `string_utill.cpp` keep their existing names (typos and all) to
  avoid any risk of breaking the build during migration. Renames are
  a separate decision for later.
