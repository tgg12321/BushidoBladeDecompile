# Hypotheses — func_80086130 (src/main.c)

## Session 1 (2026-08-17, recon)

### H1 — CONFIRMED (this is the whole function)
**Statement.** func_80086130 is the matched sibling `func_80086014`
(src/main.c:967) with a `* 129` channel scaling added; writing the sibling's C
shape verbatim and adding the scaling closes it.
**Mechanism.** Both functions are members of the same libsnd-style per-voice
volume-setter family writing the same 16-byte-strided attribute table at
`D_80102A78` and the same dirty-flag byte array `D_800F65E0`. Their asm skeletons
are instruction-for-instruction identical (empty -8/+8 frame, `andi`/`sltiu 0x18`
guard with a copy in the delay slot, `j` to a shared epilogue with `li v0,-1` in
its delay slot, `sll/sra/sll 4` offset, `sh` to +2 then +0, `lbu`/`ori 3`/`sb`
flag update). The only delta is `sll r,v,7 ; addu r,r,v` on each stored value.
**Probe.** Replaced the cheat body with the sibling's C plus `* 129`, then
sandboxed.
**Result.** 26 → 21 with build_insns dropping 39 → 35, exactly matching
target_insns. Every remaining diff was register choice / rotation, no
count mismatch.
**Verdict.** CONFIRMED.

### H2 — CONFIRMED (the closing lever)
**Statement.** Typing the two scaled channel values as `s16` named locals — the
element type of the destination array — rather than as `s32` locals or as inline
expressions at the store sites, reproduces the target's evaluation order and
closes the function.
**Mechanism.** cc1's first-pass scheduler (sched1). With `s32` or inline
spellings the x-channel multiply is left half-formed across the address
arithmetic: `sll v1,a0,0x7` issues before the `lbu` and `addu v1,v1,a0` is held
back to fill the `lbu`'s load-delay slot. That consumes the slot, so the
`return 0` materialisation (`move v0,zero`) has no late home and is hoisted to
the top of the taken branch, rotating ~13 instructions against the target. With
`s16` locals both multiplies complete before the address arithmetic, leaving
`move v0,zero` as the only instruction available for the delay slot — which is
exactly where the target (and the matched sibling) puts it.
**Probe.** 12-variant hand-authored sweep, `tmp/grind/func_80086130/s1/sweep.py`,
each variant scored with `sandbox --disable all`.
**Result.** `s16` intermediates = 1. Every other spelling landed in the 21-26
band: inline mults 21, `s32` intermediates 25, `s32` with `vy` declared first 25,
`s32 vx` + inline y 21, named index 21, `s32` params with casts 21, pointer walk
24/32 insns, `(y<<7)+y` 23/32, `D_80102A7A[]` extern 22/33, flag-first 24,
inverted guard 26.
**Verdict.** CONFIRMED.

### H3 — CONFIRMED (the residual 1 was not real)
**Statement.** The last unit of sandbox distance at the `s16` spelling is a
scorer artifact from named-symbol LO16 addend handling, not a byte difference,
so the full build will match the oracle.
**Mechanism.** `engine/score.py` masks section-relative HI16/LO16 relocation
addends but deliberately leaves named-symbol ones unmasked. The reference object
(built from the old cheat body) spells the +2 store `sh v1,%lo(D_80102A7A)($at)`;
this C spells it `sh v1,%lo(D_80102A78)+2($at)`. `%hi` is 0x1010 for both and
`0x2A78 + 2 == 0x2A7A`, so both assemble to the same linked word.
**Probe.** `tmp/grind/func_80086130/s1/diffit.py` to isolate the single differing
instruction, then a full `engine build` + SHA1 comparison.
**Result.** diffit showed exactly one differing entry, index 20, and it was
precisely the predicted addend spelling. Full build SHA1
`62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, MATCH (twice).
**Verdict.** CONFIRMED.

### H4 — KILLED
**Statement.** Spelling the +2 store through the separate `D_80102A7A` extern
would silence the scoring artifact at no codegen cost.
**Mechanism.** Two distinct array symbols are non-aliasing to GCC's alias
analysis, so it is free to merge the two `lui`/`addu at` address computations
and to reorder the stores.
**Probe.** Sandboxed `D_80102A7A[idx * 8] = vy;` both with inline multiplies and
in combination with the winning `s16` intermediates.
**Result.** 22 with 33 build_insns (inline) and 22 with 32 build_insns (`s16`
intermediates) — GCC collapsed three instructions out of the function. Strictly
worse than the 1 it was meant to fix.
**Verdict.** KILLED. Keep the single-symbol `D_80102A78[idx * 8 + 1]` spelling,
which is also what the matched sibling uses.

### H5 — KILLED
**Statement.** The empty 8-byte frame (`addiu sp,sp,-8` / `addiu sp,sp,8` with
nothing stored) is a phantom-frame-slot residual needing the
[[phantom-slot-frame-lever]] treatment.
**Mechanism.** n/a — the premise was wrong.
**Probe.** Read the already-matched sibling `asm/funcs/func_80086014.s`.
**Result.** The matched sibling emits the identical empty -8/+8 frame from plain
C with no locals and no frame lever, and every one of the 12 swept variants
reproduced it for free. The frame is just what GCC 2.7.2 emits for this shape;
it was never a residual.
**Verdict.** KILLED before it cost anything.

## Frontier
Empty — the function is closed. Bytes proven on main this session (sandbox
distance 0, full-build SHA1 == oracle, zero regfix/asmfix rules, zero register
pins, zero inline asm). Awaiting the layer-1 cheat-reviewer and the Judge; the
self-vet is in `self_vet.md` and claims no sanctioned family, because the diff
adds only two live named locals and DELETES five register pins, one gratuitous
`volatile`, and a statement-per-instruction asm transliteration.
