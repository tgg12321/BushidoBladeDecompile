# `func_80017848` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-17848/src/ings.c`
- Split asm: `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-17848/asm/funcs/func_80017848.s`
- Address: `0x80017848`
- Size: `0x1fc`
- Source state: `live C body in src/ings.c with asmfix promotion`

## Lane

- Classification: `post-compile`
- Why:
  - The semantic C is straightforward and stable.
  - The remaining blocker was the whole-body register/save pattern around the two link-scan loops and the `math_Distance3D` call.
  - A function-scoped asmfix matches cleanly without further source churn.

## Hot Regions

1. Prologue/save layout and the early active-slot guard.
2. Both link-scan loops, where GCC keeps choosing a different carried base/count pattern.
3. The `math_Distance3D` call and the post-call `s0/s1` reuse for edge-slot updates.

## Workflow

1. Confirm the clean semantic baseline in `permuter/base.c`.
2. Check isolated output with `python3 tools/check_func.py codex_lab/func_80017848/permuter/base.c func_80017848`.
3. Promote the same readable body into `src/ings.c`.
4. Finish with a function-scoped full-body `asmfix` once it is clear the blocker is whole-body compiler behavior.

## Notes

- First readable baseline: `100` differing words, `0x1bc` vs expected `0x1fc`.
- Register-shaped live/lab baseline with pinned args and slot pointers: `99` differing words, still too far for a small regfix.
- The old tabled note was correct in spirit: GCC would not hold the same `s0/s1` pattern across `math_Distance3D`.
- Practical finish: keep the readable C and promote with a full-body `asmfix`.
- Function-level verification passes on the live source: `python3 tools/check_func.py src/ings.c func_80017848` -> `OK`.
- Repo-wide `make check` could not be used as a trustworthy gate in this session because both the active main checkout and this isolated worktree currently have unrelated nonmatching baseline noise outside `func_80017848`.

