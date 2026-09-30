# Hypothesis ledger - func_8005C8A8

## OPEN (floor 33, 2026-09-30): the `size` 0x4F0 stack slot. Owner Q45 option B REFUSED the cancellation spelling
(`size = (s32)tile + 0x4F0 - arg2` and every cancel-against-a-second-name variant). See evidence.md s3c + s3d. Look
for another source form that keeps the once-set constant out of update_equiv_regs' REG_EQUIV rewrite.

## LEAD (orchestrator, 2026-09-30; not yet spent)
The target's `li/sw` at entry + `lw` at the return is a reload SPILL of a pseudo live across the whole body
(slot 0x70 in pseudo order between mode_off 0x68 and the xpos orphan 0x78), not a declared stack local. So
look for a source value GCC keeps live the whole body with no REG_EQUIV constant that is genuinely NOT a
constant in the C: a size derived from real prim-pointer arithmetic, a count of emitted prims times a sizeof,
or a value computed from arguments that happen to be the same at every call site -- provided it is not a
cancellation of one value against itself or a second name for it (owner Q45). Callers for the latter check:
src/code6cac_c2.c:748, src/code6cac_tu2.c:2082, src/ings.c:468/470 (arg2 = D_800A38B4 at every site).

## s4 (2026-09-30, laneC): KILLED volatile / s16 / one-member struct / one-element array / `return 0x4F0;`
(probes/s4/scores.txt). The slot needs a once-set pseudo with no REG_EQUIV constant, i.e. a set whose constant
cse cannot see (evidence.md s4: cse.c:6918-6934, local-alloc.c:1019-1032, reload1.c:567-586/2381-2385).
SOTN precedent search for such a spelling in matched PS1 code: negative (evidence.md s4).

## s3 RESOLVED (floor 0): the hole is a combine-orphan `(use)` of the dead sign-extension temp of an always-zero
s16 local read once after the case-0 label (Q27 (A)); see evidence.md s3. H1 (a)/(c) and H2 are moot; H1 (b)'s
family (combine orphans) was right, via the known-zero fold rather than a stale reg_n_refs.

## Was open (s2 frontier, floor 30 = only the 8-byte frame hole at sp+0x78)
H1. The hole is a MEM pseudo X (created after `size`, before the final-loop temps) whose every reference vanishes in
    reload. Mechanisms in tools/gcc-2.7.2/reload1.c that leave allocated-but-unreferenced frame space:
    (a) delete_output_reload "forget we had a stack slot": X LOCAL (one block, one death), set and single use with
        no label/jump between, the use served by the inherited reload reg;
    (b) REG_UNUSED output of a MEM pseudo (no store emitted) when X's uses were removed after flow (combine's stale
        reg_n_refs case, combine.c header note) so X still has n_refs>0 at global time;
    (c) setup_save_areas slot for a caller-saved hard reg that a later spill made unnecessary (needs X's slot AFTER
        the initial pass: contradicted by 0x78 < 0x80(ot*4) < 0x88((s16)k) unless those two were kicked out of
        different hard regs).
    Only visible t0-computed-and-consumed value in the target: `mflo t0; sll v0,t0,3` (icon product j*count). Fits
    (a) if the product pseudo (pref LO_REG, else GR_REGS; global because CLASS_LIKELY_SPILLED) were MEM, but LO has
    no competitor in this function. Unresolved.
H2. The target's value that nv2 approximates (a pseudo equal to mode near the final frame block) comes from a
    construct cse cannot see through, and its final-tile read is mode's own slot. Untried: the frame section as a
    separate block scope / helper with real work taking mode, a copy of mode read from memory, a different spelling
    of the final dim tile.

## Killed (s2): see evidence.md "Hole investigation" for the measured list.
