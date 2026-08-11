# Evidence bank — func_80047A90

## Session 1 (2026-08-11, recon) — floor 20 → 14

### Function identity
- 84 target insns, verdict C (canonical gate). Two-phase table update in sound.c:
  - Loop 1 (17 iters): `D_800EF59C[i] = (Judge[D_800EF558[i] & 0xFFF] * 625) >> 10; D_800EF558[i] += 0x12;`
    (625 = 0x271; GCC synthesizes the multiply as the sll/addu/subu chain seen in target).
  - Loop 2 (8×17): row-smoothing over the 9×17 s32 table at D_800EF59C (row stride 0x44):
    `a0 = 2000 - (row[j] - prevrow[j]); v1 = (a0<0) ? (a0+15)>>4 : a0/10; row[j] += v1;`
    on row 8 also `g_snd_fade_curve[j] = v1;` (byte-offset addressing, lui/addu-at/sw %lo).
  - Tail: `*(s32* )D_800A3820++ = &D_800EF070;` (pointer-post-increment through the global).
- Inherited src body had 10 `register asm("$N")` pins (cheat-asm, 30 insns stripped by sandbox).
  REMOVED this session; floor was and is honest.

### Structural facts (measured)
1. **Clean pinless pointer-walk everywhere = 20.** Structure 84/84 exact; the 20 is purely a
   register permutation (4-cycle: counter a2→t0, judge-base a3→a2, pt1 t0→t1, a3off t1→a3).
2. **Loop 1 as real for-loop with array indexing = loop 1 matches EXACTLY** (incl. counter→t0
   when loop 2 was also for-form). Mechanism: loop.c LICM hoists Judge base into a late-created
   pseudo; strength reduction creates the a0/a1 walking-pointer givs; the biv (i) keeps the
   slti test. This is target's loop-1 shape byte-for-byte.
3. **Loop 2 must NOT be a loop.c loop.** Full nested-for probe = 41 (87 insns): LICM hoisted
   0x7D0, 0x66666667 magic, the constant 8, and kept sll-index addressing. Target keeps all
   constants INSIDE the inner loop ⇒ original loop 2 compiled without loop notes ⇒ goto-form
   source (source-level pointer walk, source-level `a3 += 4` byte offset, pointer exit compare
   `pa1 < pt3`).
4. **Init order `pt2; pt1; k=1;` = floor 14.** Alternatives measured: `k; pt2; pt1` = 21,
   `pt2; k; pt1` = 18, merged single counter (i both loops) = 17, merged + pt3-from-pa1 = 17.
   Mechanism: k init last shortens k's live range BUT the decisive effect is lengthening
   pt1/pt2 (init earlier ⇒ live 37/38) so their priority drops below a3off's and they fall
   through to t1/t2 (target). Side effect: k's priority rose above a3off ⇒ k stole a3.
5. **maspsx/scheduling all clean** — every probe with matching structure lands 84/84 insns;
   only registers differ. No nops, no reorderings. This is a pure global.c allocation-order
   problem now.

### lreg data for the 14-floor form (tmp/grind/func_80047A90/s1/sound.i.lreg)
Pseudo → (refs, live, disposition) [target reg]:
- 72 = i (loop1): 7 refs / 25 → **a2** [t0]   prio 2*7/25 = .56 (allocated first — the problem)
- 73 = k: 5 / 36 → **a3** [t0]                prio .278
- 74 = a3off: 4 / 29 → **t0** [a3]            prio .276
- 77 = pt2: 5 / 38 → t2 ✓                     prio .263
- 78 = pt1: 5 / 37 → t1 ✓                     prio .270
- 79 = pa1: 7 / 30 → a1 ✓                     prio .467
- 80 = pa2: 4 / 28 → a2 ✓ (shares with dead i) prio .286
- 81 = pt3: 2 / 27 → t3 ✓                     prio .074
- 75 = v1 temp → $v1 ✓, 76 = a0 temp (5/8) → $a0 ✓
- Judge base (LICM pseudo, ~3 refs/~25 live, prio ~.12) → a3 [a2]
Priority formula confirmed against source: floor_log2(refs)*refs/live_length (global.c
allocno_compare). NB floor_log2(4)=2? NO — floor_log2(4)=2, floor_log2(5..7)=2, floor_log2(8..15)=3.

### Remaining 14 diff (two independent 2-cycles)
- Loop 1: i↔judge-base. i (prio .56) allocated before judge (.12); target needs judge→a2
  first and i last→t0. i must sink below ~.12 OR i must conflict with a2/a3 holders so only
  t0 is free (see H-merged in hypotheses.md).
- Loop 2: k↔a3off. k (.278) marginally above a3off (.276); target needs a3off→a3 first,
  k last→t0 (k conflicts a0temp/pa1/pa2/a3off ⇒ t0 is first free once a3off holds a3).

### Target's second-loop init insn order (insns 30-33)
`k=1; lui pt2; addiu pt2; addiu pt1,pt2,0x44` — counter set FIRST in emitted order. Our 14-form
emits lui/addiu/addiu then k=1 (source order). The k-first SOURCE order scored 21 (wrong regs),
so the target's insn order likely comes from sched1 hoisting the k=1 into the lui shadow, or
from a source shape not yet found — do not chase insn order before the registers close; the
score counts these as the same masked slots (14 is all registers).

### Artifacts (tmp/grind/func_80047A90/s1/)
- diff.sh — rebuildable build-vs-target instruction diff (uses tmp/sandbox output)
- greg.sh — cc1 -da dump generator (writes sound.i, sound.i.greg, sound.i.lreg)
- sound.i.greg / sound.i.lreg — dumps for the CURRENT (14-floor) form
- build.txt / target.txt — last diff snapshot

### Notes
- sound.c has a pre-existing unrelated cc1 warning (func_800470B0: too few args to
  func_80052930) — harmless, predates this session.
- No sibling/duplicate leads checked this session (tmp/duplicates_leads.txt not consulted);
  low value now that the structure is fully explained.

- [s1] Honest floor: 20 at session start (pins were score-inert), 14 at session end; best form applied in src/sound.c and saved to memory/grind/func_80047A90/candidate.c

- [s1] 84/84 insns in every matching-structure probe; zero scheduling/nop diffs - this is now purely a global.c allocation-order problem

- [s1] lreg data for the 14-form banked in evidence.md: i 7refs/25live->a2 [wants t0], k 5/36->a3 [wants t0], a3off 4/29->t0 [wants a3], judge-base ~3/25->a3 [wants a2]; all other pseudos on target regs

- [s1] floor_log2 breakpoints matter: refs 4-7 -> log2=2, refs 8+ -> log2=3; k .278 vs a3off .276 is the marginal flip needed in loop 2

- [s1] Loop 2 emitted constants (0x7D0, 0x66666667, 8) prove no loop notes -> goto-form source; do-while would also get loop notes (KILLED family)

- [s1] Target insn order at loop-2 init is k-first (insn 30) but source k-first gives wrong registers (21) - insn order likely falls out of sched1 once registers close; do not chase it first
