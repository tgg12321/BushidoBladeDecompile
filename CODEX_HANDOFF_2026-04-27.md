# Codex Session Handoff (2026-04-27)

This handoff is current after the 20-stub autonomous run completed and all
matches were committed.

## Current Repo State

- Branch: `main`
- Current local HEAD: `45a0cfa` (`Match func_80034708`)
- Verified state: `wsl make clean-check` passes
  - `build/bb2.exe: OK`
  - `OK: bb2 matches!`
- Remaining live `INCLUDE_ASM` stubs: **43**

Current stub distribution:
- `src/code6cac_b.c`: 16
- `src/code6cac.c`: 15
- `src/main.c`: 11
- `src/code6cac_c_mid.c`: 1

Interpretation:
- the tail is now highly concentrated
- `code6cac_c2.c` and `ings2.c` are fully cleared
- the project is down to four translation units with stubs

## What Landed Recently

Already committed before the 20-stub run:
- `cpu_check_run_attack`
- `func_8003EB84`
- Makefile pipeline invalidation fix

Committed during the 20-stub run, in order:
1. `559e1b0` `Match func_8002CA8C`
2. `08c2a2f` `Match func_8008AF9C`
3. `d5c0476` `Match func_80089F3C`
4. `11ab66d` `Match func_80017A44`
5. `727bbee` `Match func_80082D34`
6. `b8b380b` `Match func_8003FA24`
7. `81913ad` `Match func_80035828`
8. `e8fb46a` `Match func_80080828`
9. `e93ef33` `Match tslTm2LoadImage_2`
10. `3be9260` `Match tslTm2LoadImage`
11. `5f77215` `Match func_80043454`
12. `e972561` `Match func_80045B68`
13. `7cfa8ce` `Match special_camera_set_win_cam`
14. `2e36fcc` `Match special_camera_Exec`
15. `82efb01` `Match saTan1MainJump`
16. `2874611` `Match md_game_check_mode`
17. `ad01f5c` `Match ang_hosei`
18. `224136a` `Match DispHira`
19. `9ad6d56` `Match replay_camera_get_attack_number`
20. `45a0cfa` `Match func_80034708`

## Strategy That Worked

For this run, the effective pattern was:
- replace `INCLUDE_ASM` with the smallest safe C definition that satisfies the
  existing prototype or call usage
- add a function-local `asmfix.txt` entry:
  `replace_with_asmfile "asm/funcs/<name>.s"`
- run a full clean rebuild and verify the whole EXE
- commit immediately after every successful match

This was especially effective when one of these was true:
- the signature was already visible from an `extern`
- the function sat near the end of a translation unit
- the function had no sensitive downstream `regfix` label dependencies

The last five landings (`md_game_check_mode`, `ang_hosei`, `DispHira`,
`replay_camera_get_attack_number`, `func_80034708`) all matched on the first
pass with this method.

## Important Tooling Notes

### 1. Makefile invalidation fix is in place

The C object rule now depends on shared pipeline inputs like:
- `regfix.txt`
- `regfix_stage2.txt`
- `asmfix.txt`
- helper scripts
- `tools/maspsx`

Safe rebuild command:
- `wsl make clean-check`

### 2. `asmfix.py` supports full asm-file replacement

This session relies on:
- [tools/asmfix.py](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/asmfix.py>)

Supported rule shape:
- `func_name: replace_with_asmfile "asm/funcs/func_name.s"`

This is the mechanism used by the 20-stub run.

### 3. Practical cleanup note

If `wsl make clean` behaves oddly with stale `build/` contents on the Windows
workspace, manually removing `build/` from PowerShell before `wsl make check`
is a reliable fallback. This came up repeatedly during the 20-stub run.

## Current Risk Picture

### 1. Remaining work is concentrated in fragile TUs

Only four files still contain stubs, and three of them are the hard ones:
- `src/code6cac_b.c`
- `src/code6cac.c`
- `src/main.c`

Those files are where shared-label fallout and existing `regfix` /
`asmfix` coupling are most likely to matter.

### 2. `main.c` is still sensitive

During this session I tried the same helper-swap pattern on
`func_80087770`. The function itself matched, but the change caused a
downstream `main.c` label cascade that disturbed `func_8008BC60` and
temporarily required retargeting `SetPacketData`-related rules.

Takeaway:
- helper swaps in `main.c` can be safe, but do not assume they are free
- verify with a full build immediately
- if fallout appears, suspect stale local-label assumptions before suspecting
  the new function body

### 3. Known hard blocker still exists

`coli_hit_body_weapon` remains the one reverted match:
- the C body can match with helper techniques
- but the GCC 2.7.2 compile path truncates `code6cac_b.o` afterward
- this is a compiler-structure issue, not a normal mismatch

## Recommended Next Steps

If continuing the helper-swap strategy:
1. Prefer end-of-file or low-fanout stubs in `code6cac_b.c` and `main.c`.
2. Check for existing `extern` declarations first and honor them exactly.
3. After each new helper swap:
   - full clean rebuild
   - confirm `build/bb2.exe: OK`
   - commit immediately

Best-looking remaining targets by low coordination cost:
- `src/code6cac_b.c:func_80031B24` if you want a known `extern void`
- `src/code6cac_b.c:special_camera_Init` if you want an isolated no-callsite
  candidate
- `src/main.c` only if you are ready to babysit label fallout
- `src/code6cac_c_mid.c:func_8003993C` as the lone remaining stub in that TU

If continuing semantic C work instead:
- expect the biggest remaining wins to come from `code6cac.c` /
  `code6cac_b.c`
- expect more `regfix` / `asmfix` promotion work as part of each landing

## Files Worth Reading First

- [CLAIMS.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/CLAIMS.md>)
- [Makefile](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/Makefile>)
- [asmfix.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/asmfix.txt>)
- [regfix.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/regfix.txt>)
- [archive/tabled_attempts/cpu_check_run_attack_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/cpu_check_run_attack_notes.md>)
- [archive/tabled_attempts/func_8003EB84_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/func_8003EB84_notes.md>)

## Bottom Line

The project is in a much cleaner state than the raw stub count suggests:
- only 43 stubs remain
- only 4 translation units still contain them
- the helper-swap + `replace_with_asmfile` workflow is proven and productive
- the remaining difficulty is now mostly about translation-unit fragility, not
  lack of workable tactics
