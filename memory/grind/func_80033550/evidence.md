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

## s2 (2026-07-20, structural)

- **Floor unchanged: 4.** Pin-free candidate applied to src (HEAD's asm("a3")
  pin removed from the tree); 24 structural variants measured across 3 rounds
  (logs: tmp/grind/func_80033550/s2/logs/*.{json,diff,greg,lreg}).
- **THE FLIP IS POSSIBLE — v07 moved ptr a1→a2** (first movement ever):
  staging w2 through dead new_var (`new_var = arg0[2]; w2 = new_var;` after
  w1's load) makes .greg read `72 conflicts: 74 + hard {v0,a0,a1}` → first
  free = a2. Score 7: the staged global pseudo (74) takes v1 in global RA,
  and local-alloc then rotates w-homes to w0=a0, w1=a1, w2=v1 (target needs
  v1,a0,a1) = 3 extra diffs. Every staged placement tried: after-w1 7,
  mid 6, in-wrap 19, pre-idx 5 (kills the move entirely, like v09),
  stage-w1 6, double-stage-via-i 14 (i lands a1, ptr lands v1!).
- **No REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h** → global.c
  find_reg scans hard regs numerically: v0,v1,a0,a1,a2,a3. ptr reaches a3
  ONLY if a1 AND a2 both conflict. Confirmed empirically (v07 skipped
  exactly the conflict set).
- **Value census closes the geometry space:** the tail has exactly 5 values
  (i→v1, idx-chain→v0, w0→v1, w1→a0, w2→a1-after-ptr-death) + at-temps.
  The only possible a1/a2 occupants are the w's; any form where a w
  conflicts with ptr necessarily re-homes it off its target reg (measured
  min +2). Byte-free double-conflict therefore requires TWO zero-byte
  occupants: X in a1 conflicting {v0,v1,a0}+ptr, Y in a2 conflicting X+ptr
  — no C construct found this session produces one.
- **Copies cannot conjure conflicts byte-free** (measured): a second pointer
  handle either ties to incoming a0 (no-op deleted, v11/v19) or *becomes*
  the single a1 copy (v01-v04, v12); two overlapping un-tied handles emit a
  second move (v02: 13). found_idx=i at the found label ties to v1, no-op
  deleted, byte-identical (v22).
- **Inert probes (byte-identical to base):** per-load do-while wraps
  promoting w's to global pseudos (v21), constant-holder one=1 for the sb
  immediate (v23), decl order (v10), u32 idx/new_var (v15), i*3 (s1).
- **Wider-signature theory dead:** single caller (DispPracticeMenuTex_C,
  19BA4) sets ONLY a0 (from s4) in the jal delay slot; a1 holds a stale
  callee return. No evidence for extra live-in params (which are RA-inert
  anyway once flow deletes unused param copies).
- Artifacts: tmp/grind/func_80033550/s2/{sweep.sh,sweep2.sh,sweep3.sh,
  sweep4.sh,differ.py,splice.py,variants*/,logs/} (24 variant bodies,
  per-variant sandbox JSON + normalized diff, greg/lreg for rounds 2-3).

- [s2] No REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h -> find_reg scans numerically v0,v1,a0,a1,a2,a3; ptr reaches a3 only if a1 AND a2 both conflict (v07 empirically consistent)

- [s2] v07 .greg: '72 conflicts: 72 74 2 4 5 29', dispositions 72->a2, 74(staged new_var)->v1, w0(75)->a0, w1(76)->a1 — conflict injection works but rotates homes

- [s2] Tail value census: exactly 5 values (i, idx-chain, w0, w1, w2) + at-temps; the w's are the only possible a1/a2 occupants; every conflicting arrangement re-homes them off target (min +2 diffs measured)

- [s2] Copies cannot create conflicts byte-free: tied copies coalesce to no-op (deleted), un-tied copies emit real moves (v02=13); found_idx=i byte-identical

- [s2] Closing requirement now precise: TWO zero-byte occupants — X in a1 conflicting {v0,v1,a0}+ptr, Y in a2 conflicting X+ptr — no C construct found produces one

- [s2] Floor-4 candidate (pin-free, FAKE-annotated do-while wrap) applied in src/code6cac_b.c; final sandbox 4, cheat_asm_stripped 369 (file baseline)
