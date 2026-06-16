# func_8001F938 (code6cac.c) — WIP, NEEDS-DECISION, dual-typed-read question

## TL;DR (2026-06-15)
Comprehensive 41-variant sweep this session. Discovered the pure-C lever set
that drops the honest sandbox floor from HEAD's 11 to **2** (with build_insns
108, off by 1 from target's 107) and to **4** with matched insn count. The
floor-2 candidate (saved in `rejected/dual-type-probe-load.c`) was **FAILED by
the adversarial cheat-reviewer** — the deciding construct is an unconditional
dual-typed read of the SAME address (`s16 probe` + `u16 v` at +0x270), judged
as codegen-steering with no semantic purpose. HEAD's pure-C body ALSO contains
a dual-typed read at +0x270, but in a guarded-ternary form. **Whether that
construct (already in HEAD) is sanctioned is the open question this session
surfaces.**

## What works
Two pure-C levers reduce the honest distance meaningfully:

1. **kind-split** — `u32 kind_full = lhu; u32 kind = kind_full & 0xFFFFU;`
   provokes target's `lhu $a1, 0x6A($a0); andi $v1, $a1, 0xFFFF; ...` shape
   (target's redundant andi after lhu). Drops 5 distance vs HEAD when applied.
   Reviewer noted this as codegen-steering but secondary; treated as dual-use
   with split-init-accumulation.

2. **preload + conditional override + uniform shift** —
   `s32 v = *((u16*)(+0x270)); if (probe>=4) v=3; t = v<<16;` provokes target's
   `addiu $v1,$zero,3; sll $v0,$v1,16` else-branch shape (preventing GCC's
   lui-fold of `3 << 16`). Reuses `v` across both arms so the constant doesn't
   fold. Drops another 2-3 distance.

Combined: floor 2 with bi=108.

## The reviewer FAIL
Reviewer's deciding evidence:
> `s32 probe = *((s16 *)(arg0 + 0x270)); s32 v = *((u16 *)(arg0 + 0x270));` —
> Both load the same address with different sign types. `probe` is used only
> for the `>= 4` branch guard; `v` is used for the subsequent shift arithmetic.
> A single variable with one load would produce identical program semantics.
> The split exists to produce distinct RTL nodes (signed vs. unsigned memory
> access) to influence GCC's instruction selection. Not on the SOTN-accepted
> list. Test 1 (semantic purpose) and Test 2 (human-programmer) both fail.

Verdict: FAIL. Saved to `rejected/dual-type-probe-load.c`.

## The wrinkle — HEAD already has it
HEAD's pure-C body for the saturating ternary is:
```c
s32 probe = *((s16 *)(arg0 + 0x270));
s32 raw_or_3 = (probe < 4) ? (s32)*((u16 *)(arg0 + 0x270)) : 3;
idx = ((raw_or_3 << 16) >> 15);
```
This is **also** a dual-typed read at +0x270 — just in a guarded-ternary form
(the `(u16)` read only executes when `probe < 4`). The reviewer flagged the
unconditional split-into-separate-variables form but did not opine on the
HEAD guarded form. If the construct is fundamentally a cheat, HEAD's pure-C
body is ALSO suspect — the 13 regfix rules just paper over GCC's actual emission
of that source. If the construct is sanctioned (since it's already in main
and this function was "retired" with it 2026-05-14), then the v36 form can be
shipped as candidate.c with the 13 rules retired.

## Levers tried that DIDN'T help

- **Single-read variant** (`s32 v = *((s16*)(+0x270)); if (v >= 4) v = 3; t =
  v<<16;`) — scored 8, bi=104. Removes too many insns; target really has both
  the lh and the lhu.
- **Volatile-qualified u16 read** — scored 5. Volatile changes load semantics
  and emits extra work elsewhere.
- **Two-pointer alias** (`const s16 *p_s; const u16 *p_u; *p_s; *p_u;`) —
  same score as the single-pointer form. GCC's 2.7.2 CSE still merges.
- **Memory union at +0x270** — scored 4 (no better than v30); union didn't
  unlock additional savings.
- **Shift inside each if/else branch** with const else (no preload) — scored
  4 or 5; the `else` branch's `3 << 16` always folds to `lui`.

## Resume steps
1. **READ THIS FIRST.** The deciding question is the dual-typed read.
2. If user sanctions the dual-typed-read pattern for this function (it's
   already in HEAD), apply `rejected/dual-type-probe-load.c` to
   `src/code6cac.c`'s `func_8001F938`, retire the 13 regfix rules with `retire
   func_8001F938`, and verify oracle. The candidate has bi=108 (1 extra insn:
   the `andi $v1,$v0,0xFFFF` that materializes u16 from the s16-loaded $v0
   because GCC CSE'd the two reads). That extra insn is the LAST remaining
   gap; without it bytes would match. New lever idea: try a memory clobber
   that forces a second load — but that itself may be a cheat.
3. If user rejects the dual-typed read, this function is structurally not
   pure-C-closable under GCC 2.7.2 (CSE merges the two reads) and warrants
   either parking-with-evidence or canonical-asm authorization for the ternary
   region.

## Pointers
- `rejected/dual-type-probe-load.c` — the floor-2 form + full reviewer rationale
- `.claude/rules/inline-asm-policy.md`, `.claude/rules/codegen-technique-index.md`
- `memory/feedback/split-init-accumulation-sanctioned.md` — the closest
  sanctioned analogue
- 27-commit history: `git log --all -- src/code6cac.c | grep F938` (and the
  2026-05-14 "retire ... un-pin stabilizer" commits that left these rules).
