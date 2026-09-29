# Hypothesis ledger - func_8005C8A8

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
