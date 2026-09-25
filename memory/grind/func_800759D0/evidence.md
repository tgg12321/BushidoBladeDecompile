# func_800759D0 — evidence (manual lane, 2026-09-25)

Character-select grid renderer, select-screen case 2 of func_80077374 (the draw half of
func_80075F80). Same descriptor (S_80074488) and draw calls as the case-3 sibling
func_8007636C (COMPLETED-C d844de59a) and func_800753D8.

Floor: 364 (INCLUDE_ASM, no prior C) -> 66 (first transcription) -> 39 -> 32 -> **25**
(candidate.c, per-site locals, the honest floor under current rules) -> 0 with ONE
function-scope `q` reused at 4 sites, which layer-2 FAILED 2026-09-25 (multi-write carrier,
Ruling 5 1(b)/1(f), extension (A), Ruling 6): rejected/function-scope-q-multiwrite-0.c.
Owner question: docs/grind/borderline.md 2026-09-25 "one role, differing constant offsets".
All measured with `sandbox --disable all --diff`. The q-step below describes the rejected 0.

## What closed it (each step measured)
- Loop 2 highlight test written `if (i != f3C) {sp40 = 0} else {sp40 = 1; sp18 += ...}` with
  `q = s.sp18 + 0x24` computed before it and `s.sp1C = q; s.sp1C += ...` after: 66 -> 39.
- D_8009BCF8[i] read as `(D_8009BCF8 + i)->unk0` (pointer arithmetic, address formed as its
  own value) at all three sites: the address `sym + i*2` is then a pseudo, cse reuses it for
  the second read (target `t0`) and loop.c hoists the symbol into `$s6`, which the target
  uses for all three reads. `D_8009BCF8[i].unk0` folds the symbol into each MEM
  (`lui at; addu at; lbu %lo`): 41 at the third site alone, 44 with it at all sites.
- Grid lookup `((u8 *)D_8009BCF8)[index]` with `index` a local, exactly func_80075F80's
  (main) spelling: at-form lbu as target. Inline index 36; the struct-index spelling
  `D_8009BCF8[row*5+col + arg1*10].unk0` 67.
- Head order `s.sp18 = table[0];` before the sp30/sp34 stores: 32 -> 25.
- ONE function-scope `q` for the sprite-image pointer written at the head (+0xC) and in
  loops 1 and 2 (+0x24) (and loop 3, byte-neutral there): 25 -> 0. Same construct as the
  sibling func_8007636C on main (function-scope `q`, +0xC / +0x24 writes).
  Mechanism: local-alloc only takes a pseudo whose every reference is in one basic block;
  a q referenced in several blocks goes to global.c, which seats it in `$a1` at every site
  as the target does (head `addiu a1,v1,12`, loop 1 `addiu a1,a2,36`, loop 2/3
  `addiu a1,v1,36`), and that also re-seats the neighbouring loop-1 values
  (table value a2, state v0, arg1*4 a3, cell address t0).
- `s32 zero` constant-holder (FAKE, named-local-fake-exception) for func_8006E480's 2nd arg
  (target `$fp`): literal 0 = 30/364 at 362 insns.

## Ablations of the final form (all from candidate.c, one change each)
| change | score |
|---|---|
| per-site q0..q3 (one local per write) — = candidate.c (honest floor) | 25 |
| head without q (`s.sp1C = s.sp18 + 0xC`) | 3 |
| loop 1 without q | 22 |
| loop 2 block-local q | 30 |
| loop 3 block-local q | 0 (byte-neutral; kept on the one q for one role) |
| literal 0 instead of `zero` — rejected/literal-zero-no-holder-30.c | 30 |
| third cell read as `D_8009BCF8[i].unk0` | 41 |
| `rec = &D_8009BCF8[i]` held across the func_8007352C call | 123 |
| `(&D_8009BCF8[i])->unk0` at all three sites | 0 (same tree as `+ i`) |

## Tools
tmp/f759d0/gen.py (variant generator), tmp/f759d0/rtl.py (splice + cc1 -da, per-function
dumps), tmp/f759d0/hk.py (scored hunks). tmp is gitignored.
