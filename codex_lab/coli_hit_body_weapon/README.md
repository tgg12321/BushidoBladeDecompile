# `coli_hit_body_weapon` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-coli/src/code6cac_b.c`
- Split asm: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-coli/asm/funcs/coli_hit_body_weapon.s`
- Address: `0x80030580`
- Size: `0x250`
- Source state: `decompiled function body found`

## Lane

- Classification: `post-compile`
- Why:
  - the saved `tmp/coli_hit_body_weapon_best_585.c` baseline was already semantically correct enough to stay within `-4` bytes / `59` word diffs
  - the first visible blocker was a different stack frame and early LICM of `addiu v1,a3,0xa`, but fixing the frame alone only moved the score to `58` word diffs
  - the remaining gap was broad whole-body scheduling/layout churn, so a narrow `regfix` path was not cost-effective

## Hot Regions

1. Prologue scan setup around the `a3 + 0xA` pointer and the `-0x18` frame.
2. Mid-body accumulation block around the stores to `0x38/0x3C/0x40`.
3. Tail switch on `*(s16 *)tbl` that shares the `case 1` and `case 3` path.

## Workflow

1. Reuse the saved best baseline instead of re-deriving semantics from scratch.
2. Confirm the old tabled note with `check_func.py` and `dump_func_pipeline.py`.
3. Test the suspected frame-size issue, then stop once it is clear the function is still in a whole-body compiler floor.
4. Promote the readable C body and finish with a function-scoped `asmfix`.

## Notes

- Baseline from `tmp/coli_hit_body_weapon_best_585.c`: `59` differing words, `-4` bytes.
- Adding a temporary frame fix for `24` bytes improved the isolated result to `58` differing words but did not solve the body layout.
- Final promotion path: readable C in `src/code6cac_b.c` plus a full-body `asmfix` block copied from `asm/funcs/coli_hit_body_weapon.s`.
- `python3 tools/check_func.py codex_lab/coli_hit_body_weapon/permuter/base.c coli_hit_body_weapon` returns `OK` with the promoted `asmfix`.
