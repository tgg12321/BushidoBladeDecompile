# Evidence bank — func_80061C00

## s1 (recon, 2026-07-20)

- **Baseline:** canonical verdict C (93 insns), honest floor `sandbox --disable all` = **2**. One regfix rule (`reorder 6,3,4,5 @ 3-6`) papers over the whole gap.
- **The entire distance-2 diff is ONE displaced instruction:** `move v1,a1` (the arg1 param copy). Target compiler-body order: `move s2,a0; move v1,a1; move s1,a2; lui; addiu; sw gp; li; beq` (pure param/RTL order). Our build sinks the copy to between `addiu` and `sw gp` (2 alignment edits = distance 2).
- **Sibling** func_80061D74 (COMPLETED-C, similarity 0.83) avoids the copy entirely — it stores `sh a1,0x1A(sp)` early (no arg2 clamp branch, single block). Not transplantable: our function's clamp branch pushes all array stores into block 2, forcing the a1→v1 copy to live across block 0.
- **Root mechanism (from cc1 -dS dump + sched.c source):** GCC 2.7.2 sched.c is a BACKWARD list scheduler. `adjust_priority` (sched.c:2534) boosts "birthing" insns — `birthing_insn_p` = SET of a REG that is live in `bb_live_regs` with `reg_n_sets[] == 1` — to `LAUNCH_PRIORITY` (0x7f000001, sched.c:187). Boosted insns are picked early in backward order = **placed at the block bottom**. Our arg1 copy `(set (reg:HI 73) (reg:HI a1))` is single-set + live-into-block-2 → boosted → sinks. The arg2 copy (pseudo 75) escapes because `arg2 = 0;` in the clamp arm makes it double-set. Dump extract: `tmp/grind/func_80061C00/s1/sched_block0_extract.txt` (jump 23's LOG_LINKS `(anti 8, anti 19, 10, 22)`; T-2 ready shows insns 8/22 at 0x7f000001).
- **Confirmed positively:** BOTH multi-set probes (arg1 reused as pointer holder; arg1 reused as sp20[1] repack temp) moved the copy to EXACT target slot 3. The boost is the whole story for the displacement.
- **But value-reuse cascades:** with arg1 multi-set, (a) the merged pseudo allocates $a3 not $v1; (b) whichever temp arg1 absorbs loses ITS single-set boost, shifting the post-call schedule (pointer `lui v1` hoists into arg1's live range, $v1/$a0/$a1 rotate). Scores: 28 / 21 / 12 vs baseline 2.
- **cse/flow delete no-op self-assigns before sched1** (probe: `arg1 = arg1;` in clamp arm → byte-identical output). A dead store to the param CANNOT create the needed second set — the dead-store-FAKE route is mechanically closed for this diff.
- **K&R-style definition:** byte-identical output. Param-copy RTL unchanged. Dead axis.
- Post-sched1 order is additionally polished by sched2 (moved the copy up one slot, after reload where the boost is disabled) — final build order `lui,addiu,move v1,sw` vs sched1 RTL order `lui/addiu,sw,move`. Any future lever analysis must reason about BOTH passes.
- Caller: single C caller `src/code6cac_b.c:3170` uses an implicit declaration with only 2 args — the definition's param types/spelling are free to vary without caller codegen impact.

- [s1] Honest floor 2 (93/93 insns), verdict C; the single regfix rule 'reorder 6,3,4,5 @ 3-6' covers exactly this displacement

- [s1] Target prologue body is pure RTL/param order (move s2,a0; move v1,a1; move s1,a2; lui; addiu; sw gp; li; beq); build sinks only the a1 copy

- [s1] sched dump: jump 23 LOG_LINKS (anti 8, anti 19, 10, 22); insns 8/22/17 boosted to 0x7f000001; arg2 pseudo 75 double-set (clamp) hence unboosted — explains why only arg1's copy moves

- [s1] Defeating the boost (any multi-set spelling) reproducibly places the copy at target slot 3 — the displacement mechanism is fully understood

- [s1] cse/flow delete no-op self-assigns before sched1: reg_n_sets cannot be inflated by dead stores

- [s1] Sibling func_80061D74 shape (early sh a1) not transplantable: the arg2 clamp branch forces the copy

- [s1] Single C caller (code6cac_b.c:3170) is implicit-decl 2-arg: definition spelling free to vary

- [s1] sched2 (post-reload, boost disabled) additionally lifts the copy one slot; final order shaped by both passes
