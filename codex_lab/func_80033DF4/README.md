# `func_80033DF4` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-33df4/src/code6cac_b.c`
- Split asm: `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-33df4/asm/funcs/func_80033DF4.s`
- Address: `0x80033DF4`
- Size: `0x1f0`
- Source state: `matched in live TU with localized asmfix`

## Lane

- Classification: `source-shape -> localized asmfix`
- Why:
  - the semantic logic was straightforward and the main blocker was register/base-pointer choreography in the non-`0x64` tail path
  - a register-shaped C baseline dropped quickly from a broad mismatch into one compact else-block cluster
  - the clean finish was a small asmfix replacement of that tail block, promoted with instruction anchors instead of isolated labels

## Hot Regions

1. Non-`0x64` path from `D_800A38E2 = state + 1` through the final `D_8010277D` store
2. Live promotion needed stable instruction anchors so the anonymous epilogue label stayed intact

## Workflow

1. Confirm the clean semantic baseline.
2. Shape the function around the target register flow before escalating.
3. Use asmfix once the remaining mismatch is one compact block.

## Notes

- First handwritten baseline: `73` differing words, `-12` bytes.
- Register-shaped rewrite with pinned variables: `41` differing words, `-4` bytes.
- One follow-up rewrite regressed badly to `115` differing words; reverting quickly was the right call.
- The winning asmfix replaced only the else-path body and used instruction anchors in the live TU:
  - start anchor: `addu $3,$3,1`
  - end anchor: original final `sb $3,D_8010277D`
- Promotion details:
  - cast `D_8010277C` to `(s8)` in live C so the compiler emits `lb` instead of `lbu`
  - store `D_800A384C` via `*((u8 *)&D_800A384C)` because this TU declares it as `s32`
- Verification:
  - `wsl python3 tools/check_func.py codex_lab/func_80033DF4/permuter/base.c func_80033DF4` -> `OK`
  - `wsl make check` -> `build/bb2.exe: OK` and `OK: bb2 matches!`
