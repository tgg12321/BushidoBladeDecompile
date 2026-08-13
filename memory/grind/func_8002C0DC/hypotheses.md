# Hypothesis ledger — func_8002C0DC

## Session 1 (2026-08-13, modality: recon)

### H1 — CONFIRMED, closes the function
**Statement.** The honest 23-point gap is caused by ONE surplus live value: a
second induction variable that loop.c derives for the clustered `+0xD8` / `+0xE0`
field reads in the two-iteration loop. Removing that surplus IV removes the
fourth callee-save, the oversized frame, the downstream register rotation and
the stray load-delay nop all at once.

**Mechanism.** The HEAD body hand-walked the array (`var_s0 += 0x44C;`) with a
separate counter, i.e. two hand-written induction variables. GCC 2.7.2's loop.c
`strength_reduce` then treats the two clustered address expressions as reducible
and `combine_givs` merges them onto a NEW derived register (base `$s0 + 0xE0`,
consumed at `-8(...)` and `0(...)`) instead of onto the existing base pointer.
The extra live pointer needs a fourth callee-save (`$s3`), which raises the frame
from 0x20 to 0x28, adds the save/restore pair and the giv's init + increment, and
rotates every downstream register role by one slot. Writing the loop with a
single basic induction variable (`i`, kept live by the `i < 2` bound test) makes
all three element references (+0x00, +0xD8, +0xE0) combine onto ONE derived
pointer — target's `$s0`.

**Probe.** Replaced the pinned walking-pointer `do { } while` with
`for (i = 0; i < 2; i++) { u8 *e = (u8 *)&D_80101EC8 + i * 0x44C; ... }`, deleting
all three `register asm("sN")` pins; then `sandbox func_8002C0DC --disable all`
and an instruction-level objdump comparison against `asm/funcs/func_8002C0DC.s`.

**Result.** Score **23 -> 0** on the first probe. `build_insns` 89 -> 84 ==
`target_insns` 84. Objdump comparison: all 84 instructions identical (opcodes,
registers, immediates, offsets, delay slots, frame 0x20, callee-saves s0/s1/s2);
only link-time relocation fields read as zero. Confirmed not a masked zero.

**Verdict: CONFIRMED.**

### H2 — dissolved before it was probed (recorded so nobody re-runs it)
**Statement.** The load-delay `nop` ahead of the `bnez` on `D_800A376E`, and the
12-rule post-loop register-rename `subst` block, are separate defects needing
their own levers (a scheduling lever for the nop, an RA lever for the renames).

**Why it is dead.** Both were downstream symptoms of H1's surplus induction
variable, not independent defects. They vanished with the H1 fix and were never
attacked directly. A future session facing a similar residual shape should check
the callee-save count and frame size against target BEFORE spending probes on
individual scheduling/RA residuals — a single surplus live value can present as
several unrelated-looking diffs.

**Verdict: KILLED (as an independent hypothesis).**

## Modality ladder status
The function closed in the FIRST session, in `recon` modality. No permuter
campaign, no forensics, no rederive, no synthesis was needed or run. Nothing on
the ladder is spent; nothing on the ladder is owed.
