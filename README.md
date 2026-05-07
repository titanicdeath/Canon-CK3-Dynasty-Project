# Canon CK3 Dynasty Project

A fast C++ parser for Crusader Kings III save files, designed to extract a
specific dynasty's complete family tree (descendants and their spouses) from
a full uncompressed gamestate file.

## Status

**Working prototype.** Successfully parses ~300 MB / 18 million-line save
files in ~2.5 seconds, including dynasty tree extraction. Output to JSON
and the GUI layer are next.

## Performance (reference)

On a representative save (`gamestate`, ~281 MB, 18,135,539 lines):

| Metric                          | Value             |
|---------------------------------|-------------------|
| Wall time (avg of 5 runs)       | 2.47 seconds      |
| Throughput                      | ~120 MB/sec       |
| Character blocks identified     | 181,447           |
| Lines after filtering           | 7,669,165         |
| Dynasty subtree extracted       | 8,655 characters  |
| Peak memory                     | ~810 MB           |

## Build

Requires a C++17 compiler with Windows SDK headers (uses `windows.h` and
`psapi.h` for memory instrumentation).

```powershell
.\scripts\run.ps1          # build and run
.\scripts\run.ps1 -Time    # build, run 5 times, report timing
```

## Project layout

```
src/        - parser source (parser-V5.cpp + memory/string utilities)
include/    - headers (memory_utill.hpp, string_utill.hpp, json.hpp)
scripts/    - build/run/tooling scripts (PowerShell + Python)
tests/      - test fixtures (sample blocks, prior parser output)
legacy/     - historical iterations (v1 through v4 + experiments)
save_game/  - drop your gamestate file here (gitignored)
```

## Use

1. Place an uncompressed CK3 save file at `save_game/gamestate`.
   (To uncompress: rename a save from `.ck3` to `.zip`, extract,
   the `gamestate` file is inside.)
2. Edit the hardcoded `founderID` in `src/parser-V5.cpp`
   (currently `37676`) to your dynasty founder's character ID.
   You can find this in-game with the console enabled, or by
   grepping the gamestate file.
3. Run `.\scripts\run.ps1`.

## See also

- `NOTES.md` - current state, known issues, roadmap.
- `legacy/README.md` - history of prior iterations.
