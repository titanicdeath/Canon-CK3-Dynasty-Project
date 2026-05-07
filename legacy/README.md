# legacy/

Historical iterations of the CK3 dynasty parser, kept for reference.
None of this is current. The working code lives in `src/`.

## Files

| File                                      | Era | Notes                                                |
|-------------------------------------------|-----|------------------------------------------------------|
| v1-character_parser.cpp                   | v1  | First attempt. Naive line-by-line parsing.           |
| v1-character-block-example.txt            | v1  | Sample CK3 character block (id 37676).               |
| v1-characters.jsonl                       | v1  | Output from v1 parser run.                           |
| v1-compile.bat                            | v1  | Build script.                                        |
| v2-version-2_parser.cpp                   | v2  | Second iteration.                                    |
| v2-new_parser.cpp                         | v2  | Branch of v2.                                        |
| v2-own_parser.cpp                         | v2  | Hand-written branch of v2.                           |
| v2-new_compile.bat / v2-own_compile.bat   | v2  | Build scripts.                                       |
| v2-New_characters.jsonl                   | v2  | Empty output file from v2 era.                       |
| v3-prototype_parser.cpp                   | v3  | Third iteration. Introduced string_view buffering.   |
| v4-prototype_parser.cpp                   | v4  | Pre-V5 working state. Direct ancestor of src/V5.     |
| experimental-referencing-map.cpp          | exp | Early map-based block lookup experiment.             |
| experimental-total_memory.cpp / .bat      | exp | Standalone memory-tracking experiments.              |
| experimental-Untitled-2.cpp               | exp | Stray experimental file from VS Code.                |

## Why keep these?

Two reasons:

1. They show the evolution of the parser - the architectural decisions
   in V5 were not obvious at v1, and seeing the path matters.
2. The pointerLexicon / dictionary-encoding ideas in the experimental
   files might be worth revisiting if the project ever needs to handle
   uncompressed saves larger than RAM.
