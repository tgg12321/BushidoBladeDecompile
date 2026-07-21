# Evidence bank — func_80033550

## s1 (2026-07-20, recon)

- **Baseline:** canonical verdict C (34 insns, distance 4). Honest floor measured
  THIS session: **4** with the pin-free do-while form (candidate.c), **5** without
  the wrap. HEAD's old form carried a `register s32 * asm("a3")` pin (cheat,
  stripped by sandbox → its 4 was the same wrap effect, the pin bought nothing).
- **The whole residual is ONE RA choice:** arg0's pointer pseudo (72) is homed in
  $a1 by our build, $a3 by target. 4 diffs = `move a1,a0` (entry) + the base reg
  of the three `lw`s. Everything else — loop, sb, idx chain, w-value regs
  (w0=$v1, w1=$a0, w2=$a1), store pattern — matches exactly.
- **RTL dump facts** (tmp/grind/func_80033550/s1/probe.i.greg, clean form):
  - Only 2 global pseudos: 72 = arg0-copy (→ a1), 73 = i (→ v1).
  - `72 conflicts: 73 + hard {2 (v0: idx/temps), 3 (v1: w0), 4 (a0: w1), 29}`.
    First free in alloc order = **5 (a1)**.
  - w2 (pseudo 76, local-alloc) ALSO sits in a1: born exactly at ptr's death
    (its own `lw a1,8(a1)`), so no conflict — death/birth adjacency lets them
    share. **Any geometry whose last ptr use is w2's load leaves a1 shareable**,
    so first-free can never skip to a3 without TWO extra conflicts (a1 AND a2
    occupants overlapping ptr's live range).
  - Nothing occupies a2 in target's emitted code either — so the original's RA
    input must have had pseudo(s)/hard-reg pressure invisible in the final
    bytes (no-op-move-deleted copies, or a structure we haven't found).
- **Scheduling residual (solved):** without the wrap, sched1 hoists the three
  lw's above the final `sll v0,v0,2` (+1 reordering → 5). The single-level
  do-while(0) around `idx = new_var; w0 = arg0[0];` fences it. This is NOW
  SANCTIONED: do-while-zero-exception was rewritten 2026-07-06 (owner ruling —
  ANY codegen effect, FAKE annotation required). The 2026-06-07 park predates
  that ruling and its "not sanctioned" note is obsolete.
- **Prior sweep (park b07abaa5, 2026-06-07):** 27+ structural variants all ≥5
  (loop forms, idx spellings, statement orders, pointer pre-computes, types);
  reads-first form = 10. Do not re-run these.
- **Sibling precedent:** func_8001CD68 (COMPLETED-C, code6cac.c) also copies
  a0→a3 naturally — but there a0/a1/a2 are all consumed by real conflicting
  values (div magic constants, global reload, minutes). Confirms the mechanism
  needed; our function lacks visible occupants.
- **Probe kills this session:** i*3-array-index spelling → byte-identical
  (combine canonicalizes). `register` keyword on the alias → still 4 AND the
  engine's cheat-stripper counts the register-alias as strippable
  (cheat_asm_stripped 369→370) — avoid that spelling.
- Artifacts: tmp/grind/func_80033550/s1/{probe.i.greg,probe.i.lreg,probe.i.sched,
  dump.sh,diffit.sh} (full -da dump set present).

- [s1] canonical: verdict C, 34 insns, pure-C target

- [s1] sandbox floors this session: 5 (clean, no wrap), 4 (pin-free FAKE-annotated do-while wrap) — 4 is now a ZERO-cheat floor, unlike the pinned HEAD form

- [s1] residual diff = move a1,a0 at entry + base reg of the three lw's (a1 vs a3); w-value homes (v1,a0,a1), loop, sb, idx chain, stores all match

- [s1] .greg proof: only 2 global pseudos (72=ptr, 73=i); 72's conflict set {v0,v1,a0}; w2 shares a1 with ptr by death/birth adjacency — any geometry ending ptr's life at w2's load leaves a1 shareable

- [s1] prior park b07abaa5 (2026-06-07): 27+ structural variants all >=5; its 'do-while not sanctioned' note is OBSOLETE after the 2026-07-06 owner ruling

- [s1] sibling func_8001CD68 (COMPLETED-C) gets a0->a3 naturally because real values (div magic consts, reload) occupy a0-a2 — confirms the required mechanism

- [s1] engine stripper counts a plain `register` (no asm) pointer alias as cheat-asm (369->370) — avoid that spelling in candidates
