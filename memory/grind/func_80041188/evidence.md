# Evidence bank — func_80041188

- [s1 recon 2026-08-21] Chassis re-verified on current main: canonical verdict C
  (132 insns, distance 27); HEAD honest floor 27; candidate.c applied to
  src/text1a_pre.c (function name kept func_80041188, body otherwise verbatim)
  scores **5** — the wip-import floor is chassis-valid, no re-measurement of
  banked spelling conclusions needed. Residual fdiff re-run and byte-identical
  to the recorded 5-insn loop-2 cluster (artifact:
  tmp/grind/func_80041188/s1/fdiff_residual.txt). The candidate edits are IN
  PLACE in src/text1a_pre.c at session end.
- [s1 recon] Sibling scan: no hit in tmp/duplicates_leads.txt; the committed
  source's own annotation says "kengo:MED | my_hirahira/hirahira_w_ctrl | 132i
  | x2 size collision" — a Kengo name lead only, no transplantable body.
- [s1 recon] Untried sanctioned lever noticed for the open "+1 reference"
  problem: duplicated-statement-into-arms (the proven byte-free reg_n_refs
  lift for global-RA priority walls, per codegen-technique-index). Never
  attempted on this function per the ledger. See hypotheses.md H2.

- WIP rejected_form: {'form': 'split `out2` into per-loop locals (target demonstrably has two a4+0x20 registers)', 'score': 17, 'reason': 'each half scores nrefs 3 / livelen 42 -> pri 714, BELOW a3 (800), so it takes s8. At livelen 42 the reachable priorities are 714 (nrefs 3) and 1904 (nrefs 4); the window that would work is (1052, 1666) and nothing lands inside.'}

- WIP rejected_form: {'form': 'split-init accumulation on out2 (`out2 = (s32 *) pa4; out2 = ... + 0x20;`, and the +0x10/+0x10 staged spelling)', 'score': 17, 'reason': "copy-propagated away before flow analysis, so unlike stptr it adds NO references to the loop1 copy. It does take on loop2's copy (nrefs 5, pri 2040 -> s3), but loop1's stays at 714."}

- WIP rejected_form: {'form': 'def-position sweeps: all 6 orders of the three pre-loop initialisers x 3 positions of the loop2 init', 'score': 18, 'reason': "out2's live length only moves 88-91 (the window at nrefs 6 is [71,81]); stptr1's live length floors at 41. Exhausted - do not re-run."}

- WIP rejected_form: {'form': 'drop the a4 carrier entirely (use the parameter everywhere)', 'score': 21, 'reason': 'the parameter pseudo then has livelen 190 (pri 736) and sinks below a3 (808)'}

- WIP rejected_form: {'form': "parameter substitution at any a4 site other than loop2's func_800523E0", 'score': 42, 'reason': 'costs an instruction (133 or 134)'}

- WIP rejected_form: {'form': 'inline `a4 + 0x20` at the call sites (per-loop LICM hoist)', 'score': 25, 'reason': 'the hoisted pseudo scores nrefs 4 / livelen 20 -> pri 4000, far above the (1052,1666) window'}

- WIP rejected_form: {'form': 'u8 * retypings of the carrier / out2 / stptr; extra aliasing locals for out2 and a3', 'score': 17, 'reason': 'inert or worse - alias copies are propagated away before flow, so they never reach reg_n_refs'}

- WIP rejected_form: {'form': 'plain operand-order swaps without a cast (offset + a1 / offset + a2 / both)', 'score': 6, 'reason': 'inert; only the explicit (s32) cast moved the first add'}

- == imported from memory/wip notes.md ==
# hirahira_w_ctrl — WIP (opened 2026-08-04)

`src/text1a.c:753`. **16 regfix rules + 1 `tools/prologue_config.json` entry +
a register pin in source** (`register s32 *s7_a4 asm("s7") = a4;`) at HEAD.
No asmfix / frame_fix entries.

## Where it stands

| | score | frame | insns |
|---|---|---|---|
| committed HEAD | 27 | 72 | 132 |
| r1-r2 candidate | 23 | 72 | 132 |
| **`candidate.c` (r3-r16)** | **5** | 72 | 132 |
| target | 0 | 72 (`0x48`) | 132 |

Frame, instruction count, the prologue, the whole of loop 1, the schedule and
the epilogue all match target exactly. `candidate.c` is pure C — no register
pin, no `__asm__`, no volatile, no dead code.

**CORRECTION (operator, 2026-08-04 ~08:20) still stands: the pin retirement is
NOT a free commit.** It is inert under the honest sandbox but with the 16 regfix
rules ENABLED the emission shifts and the full build MISMATCHES (measured: SHA1
8e1160d1). The pin retires only as part of the COMPLETION, when all rules go.

## The residual: five instructions, one cluster

```
  72  TGT addiu s3,s7,32     OURS addiu s6,s7,32
  99  TGT move  a1,s3        OURS move  a1,s6
 110  TGT move  a0,s7        OURS move  a1,s6
 111  TGT move  a1,s3        OURS move  a2,s8
 112  TGT move  a2,s8        OURS lw    a0,88(sp)
```

Two causes: (a) target gives loop 2 its **own** `a4 + 0x20` register — `s3`,
shared with the by-then-dead loop1 `stptr` — where we reuse loop 1's `s6`;
(b) the parameter substitution that bought 17→11 costs a stack reload at
loop2's `func_800523E0` where target has `move a0,s7`.

## Method — ALLOCDBG priority arithmetic drove all of it

`pri = floor_log2(nrefs) * nrefs / livelen * 10000` (re-verified here). Tools:
`tmp/allocone.sh` (isolates the function into its own TU so there is no
block-alignment guesswork — allocno numbering, nrefs, livelen and pri are all
per-function, so an isolated compile gives the identical table), plus
`tmp/allocpick2.py` / `tmp/allocmatch.py` for the in-context log.

Allocno table of the 5-point form (all eight callee-saved registers now match
target):

| reg | role | nrefs | livelen | pri |
|---|---|---|---|---|
| s1 | a1 | 16 | 100 | 6400 |
| s2 | a2 | 16 | 100 | 6400 |
| s3 | stptr (loop1) | 7 | 41 | 3414 |
| s0 | stptr2 (loop2) | 6 | 47 | 2553 |
| s4 | i | 8 | 98 | 2448 |
| s5 | tbl | 4 | 48 | 1666 |
| s6 | out2 | 6 | 90 | 1333 |
| s7 | a4 carrier | 5 | 95 | 1052 |
| s8 | a3 | 4 | 100 | 800 |

## The six levers (27→5) — narrative in git history (r16 commit 44ab9f6e)
## Measured negatives — in git history (r16 commit); do not re-run the operand-order, split-init(+2), or alias(0) sweeps
## Resume here — the number to beat

Start from `candidate.c` (5). The task is a two-line spec:

> Give loop 2 its own `a4 + 0x20` allocno with **pri in (1052, 1666)** — at
> livelen 42 that needs nrefs 4, and nrefs 4 is only reachable together with a
> live range of 48–76 — **while restoring `move a0,s7`** at loop2's
> `func_800523E0` (i.e. finding a different way to hold the carrier's priority
> below `out2`'s 1333 than the parameter substitution).

Alternatively raise `tbl` from 1666 to ~2083 (nrefs 4 → 5) so that a split
`out2` at nrefs 4 / livelen 42 (1904) lands between `tbl` and `a4`. Both routes
need a **+1** reference lever; every one found so far moves +2 (split-init) or 0
(alias, copy-propagated). That is the open question.

The directed permuter is the untried rung — 132/132 at offset 0 with only five
instructions differing is a much better starting basin than saTan4FireDisp had.
Per [[grinder-permuter-orphan-stop-gate]] it must be supervised and harvested
inside the same session.

Instruments: tmp/hw*.py sweeps, tmp/hw_bank.py, tmp/sbs.sh, tmp/frame_probe.sh, tmp/allocdbg.sh, tmp/allocpick.py, tmp/perm_hw workspace.

## Permuter rung — CLOSED (62k iters, nothing beats candidate-5; details in git history)
## RA-solver session (2026-08-04 afternoon, tools/ra_solver)

Candidate-5 model validates 11/11. Solver findings:
- The 5-insn cluster = out2 (pseudo 86) CONFLICTS with stptr (87) in our
  spelling (one variable reused across both loops = one pseudo). Removing
  the conflict in the model gives out2->s3(19) = target.
- Real split spelling (out2b for loop2): out2b DOES land s3 (model 12/12
  confirmed) but re-shuffles {loop1-out, a4, saved} into a 3-cycle;
  sandbox 16 (worse than 5).
- Solver on the split model, spec {87:19, 86:22, 77:23, 75:30}: NO single
  atom; sufficient PAIRS include (86 refs+1 & 86 pref+22), (79 refs+1 &
  86 refs+1), (77 pref->23 & 86 live-8), etc.
- Tested spelling for 86 refs+1 (split-init the preamble def, sanctioned
  family): sandbox still 16 — the +1 did not materialize as modeled.
  NEXT: extract the split2 variant model (tmp/hw_split2.sh --keep, then
  extract) and compare 86 actual nrefs/pri vs predicted; then realize the
  next pair from the HIT list. Scripts: tmp/hw_solver.sh, tmp/hw_split*.sh.
- The candidate-5 form in candidate.c remains the best (apply as base).


- [s1] HEAD honest floor 27 (canonical verdict C, pure-C target, 132 insns); candidate floor 5 with edits in place in src/text1a_pre.c

- [s1] Residual byte-identical to the recorded cluster: loop2's a4+0x20 needs its own allocno in s3 (pri window (1052,1666) at livelen ~42 => nrefs 4) and move a0,s7 must return at loop2's func_800523E0 (currently lw a0,88(sp) from the lever-4 parameter substitution)

- [s1] Open problem unchanged: a +1 reg_n_refs lever — every spelling banked so far moves +2 (split-init) or 0 (alias, copy-propagated); RA-solver sufficient PAIRS banked in evidence.md

- [s1] Untried sanctioned family identified for the +1: duplicated-statement-into-arms (byte-free ref-lift for global-RA priority walls) — hypotheses.md H2, rule must be read end-to-end before use

- [s1] No sibling/duplicate shortcut exists (tmp/duplicates_leads.txt negative; Kengo lead is name-only)

## s2 (structural, 2026-08-21) — split-model RA vector space closed at depth<=3; three normalization walls proven with mechanisms

Chassis: candidate.c re-applied to src/text1a_pre.c, sandbox 5 reproduced
(132/132) at session start and re-verified at session end; edits in place.

### Real-chassis split-model allocno table (plain split out2b, sandbox 16)

pseudo -> role: 73=a1(16/100,s1) 74=a2(16/100,s2) 75=a3(4/100=800)
76=a4-param(4/194,memory) 77=pa4-carrier(5/95=1052) 78=i(8/98,s4)
79=tbl(4/48=1666,s5) 85=saved(2/48,memory) 86=loop1-out2(3/42=714)
87=out2b(3/48=625,s3 CORRECT already) 88=stptr(7/41,s3) 91=stptr2(6/47,s0).
Misallocation is EXACTLY a 3-cycle order problem: 77(1052)->s6, 75(800)->s7,
86(714)->s8; target wants 86->s6 > 77->s7 > 75->s8. 87 lands s3 in every
variant (conflict-free with 88/79/86) as long as 75/77/86 resolve.

### Full-goal solver result (artifact s2/inverse_fullgoal_depth3.txt)

inverse.py with the FULL 10-pseudo target disposition (earlier ledger pairs
came from an underspecified 4-pseudo goal — several old "sufficient pairs"
(e.g. 87refs+1&87pref22) actually scramble tbl/i/out2b and are VOID).
Depth<=3 sufficient sets, complete list:
  #1 (77 refs 5->4  & 86 live 42->34)
  #2 (79 refs 4->5  & 86 refs 3->4)
  #3 (79 live 48->40 & 86 refs 3->4)
  #4 (86 refs 3->4  & 86 live 42->50)
  #5 (77 pref+$s7   & 86 live 42->34)
  #6+ (86 refs+N & 86 pref+$s6) ladder — all need pref.
  #7 (75 pref+$fp & 77 pref+$s7)
Every set contains >=1 atom measured/proven unspellable below.

### Atom spellability verdicts (each with mechanism)

- **86 refs_up: STRUCTURALLY UNSPELLABLE.** Three independent walls, all
  dump-proven this session:
  (a) block-0 canon_reg rewrite — cse.c:826 make_regs_eqv: on a copy
      `new = old`, NEW becomes the qty's canonical (substitution-target)
      register ONLY if regno_last_uid[new] outlives regno_last_uid[old].
      out2 dies at loop1's end while pa4/76 lives into loop2, so 76 stays
      canonical and EVERY arithmetic use of 86 in block 0 is rewritten to
      76 (verified: 86's def reads 76 in .cse). This is also exactly why
      stptr's lever-5 split-init worked (+2: stptr outlives base) and why
      out2's split-init measured 0 in s0 — now explained, not just observed.
  (b) cse dest-swap normalization — for `A = expr; B = A` where A dies at
      the copy, cse1 swaps destinations to `B = expr; A = B` (verified
      .jump vs .cse, insns 164/167); the swapped copy is dead and
  (c) flow deletes dead stores UNCOUNTED. Both staging directions converge;
      rejected/split-staged-reinit-cse-destswap.c.
  The only 86-refs that count are its call-arg copies (plain hard-reg
  copies escape canon_reg); no third call exists; calls cannot be
  duplicated (family non-extension).
- **86 live_shrink -8: UNSPELLABLE by def position.** Moving the def to the
  last preamble slot moved livelen 42->41 only (-1, not -8) and cost bytes
  elsewhere (sandbox 18, lever-2/3 emission coupling). No later def
  position exists; last use is fixed.
- **86 live_extend +8: unspellable alone** (nothing after loop1 can read
  out2 without either the dest-swap wall or a materialized move).
- **79 refs+1 (tbl): SPELLABLE-BUT-BYTES-FAIL as arm-dup.** Back-edge dup of
  `offset = (*tbl)*6;` DID count (79: 4->5 = pri 2083 in .lreg) — the H2
  flow-counts-before-jump2 mechanism is real — but cross_jump did not
  re-merge (sched1 interleaves the fall-in copy with preamble insns; no
  identical tail), build 134, sandbox 30. rejected/split-armdup-tbl-backedge.c.
- **79 refs+2: spellable in principle** (`tbl = (s32*)((u8*)D_80094CFC - K);
  tbl = (s32*)((u8*)tbl + K);` — fold-back to the 2-insn symbol constant is
  COSTLIER than the 1-insn addiu, so cse's cost gate keeps it; combine
  re-merges) — but +2 puts tbl at 2500 > i(2448) and breaks s4/s5. Unused.
- **all pref_add atoms: unspellable** (s6/s7/fp are callee-saved; copy
  preferences only arise from hard-reg copies = call args/returns).
- **77 refs_down 5->4: unspellable** — the five 77-refs are def + 3 arg
  copies + out2b's def; every substitution at those sites was measured to
  cost an instruction (s0 ledger) or requires the memory-resident param 76
  (reload materializes an extra lw).

### Lever-4-reverted split world (loop2 523E0 takes pa4): floor 15

sandbox 15 (best split-family score; candidate-5 still 5 overall). 77 goes
to 7 refs (1473), 76 dies in the preamble (no allocation, no lw a0,88(sp)
residual class). Same 3-cycle; 86's window TIGHTENS to (1473,1666). The
canon_reg canonicality flips (77 outlives 76 -> 77 canonical) but the 86
walls are unchanged (86 still dies before 77). Conclusion: lever 4 is NOT
load-bearing in the split model (unlike candidate-5 where pa4 there scored
11); if the 3-cycle is ever resolved, prefer the reverted spelling — it
also fixes the move a0,s7 residual for free.

### Where this leaves the search

The split-model global-RA approach is CLOSED at depth<=3 over the honest
atom space. Remaining live directions, in order:
1. Masked deeper inverse: extend inverse.py to EXCLUDE the dead atoms
   ({86 refs_up, 86 live +-8, all pref_add, 77 refs_down}) and search
   depth 4-6 over the spellable space (79 atoms, 73/74/78/88/91 refs/live
   moves, conflict_add via overlap changes). If empty -> the split-RA
   axis is dead end-to-end and the function needs a different structural
   world (jump/sched-side attack on candidate-5's residual, or full m2c
   rederivation of the original variable structure).
2. Arm-dup byte-neutrality repair (fall-in copy contiguity) — only worth
   it if (1) surfaces a 79-using sufficient set.
3. If both die: this is a local-alloc/priority CLASS wall in the shape of
   sched-rank-class-tie-wall — but do NOT park; ladder has synthesis/
   forensics rungs left, and the s6-at-1333 candidate-5 world's residual
   has never been attacked from the jump2/sched side.

- [s2] cse.c:826 make_regs_eqv gate identified and dump-verified: a copy's dest becomes the qty-canonical (substitution-surviving) register only if regno_last_uid[dest] outlives regno_last_uid[src]; explains lever-5 stptr split-init (+2, stptr outlives base) AND why every out2 split-init measures 0 (out2 never outlives pa4) — the s0 'copy-prop pre-flow' anomaly is now a mechanism, not an observation

- [s2] cse1 dest-swap normalization proven (.jump vs .cse insns 164/167): 'A=expr; B=A' with A dying at the copy is rewritten to 'B=expr; A=B'; flow then deletes the dead copy UNCOUNTED - staging refs onto a dying pseudo is structurally impossible in GCC 2.7.2, both directions converge

- [s2] real-chassis split allocno table banked: 3-cycle is a pure allocation-order problem: 77(1052)->s6, 75(800)->s7, 86(714)->s8 vs target 86->s6>77->s7>75->s8; 87 (out2b) lands s3 correctly in every variant; full-goal depth<=3 sufficient sets are exactly 7 and each carries a dead atom

- [s2] s1's banked sufficient-pair list is VOID (goal underspecification: pairs like 87refs+1&87pref22 scramble tbl/i and pass the 4-pseudo goal while missing target)

- [s2] 86 live_shrink by def position measured: C statement position moved livelen only 42->41 (-1, not the -8 the insn count suggests) and costs bytes elsewhere (sandbox 18)

- [s2] lever-4-reverted split is the best split-family form (sandbox 15) and removes the stack-reload residual class for free; candidate-5 (floor 5) remains the overall best and is restored + re-verified in src/text1a_pre.c at session end

## s3 (structural, 2026-08-21) — FLOOR 5 -> 1; split-RA axis closed end-to-end; the winning structure found

Chassis: candidate.c (floor-5 form) applied at session start, sandbox 5
reproduced (132/132). Session ends with the NEW floor-1 form in place in
src/text1a_pre.c, sandbox 1 verified three times (after each probe restore).

### Frontier-1 KILLED: masked depth-4-6 inverse (artifact s3/masked_inverse_out.txt)

tmp/grind/func_80041188/s3/masked_inverse.py (a tmp-side masked copy of
tools/ra_solver/inverse.py's search; tools/ untouched) re-solved the FULL
10-pseudo goal on hw_split.model.json over the SPELLABLE atom inventory only
(excluded: all 86 atoms, 77 refs_down/live, 75 atoms, all pref atoms;
included: 79 refs+1/+2, 91 refs+2, 88 refs-2, live +-1/2/4/8 on
73/74/78/79/88/91, conflict_add on {75,77,86} x goal): depth 1-4 exhaustive
(137k combos) = ZERO hits. Analytic closure for ALL deeper depths: under the
mask pri(77)=1052 / pri(75)=800 / pri(86)=714 are constants, so the
allocation order 77->75->86 is fixed; the goal pins every higher-priority
pseudo to s0-s5, so s6 is FREE at 77's turn and find_reg's ascending
first-free scan hands it to 77 (a conflict atom cannot block a register
nobody holds; callee-saved copy-prefs are unreachable from C). 77->s7 is
therefore unreachable at ANY depth. **The split-RA axis is dead end-to-end**
— as frontier-1 anticipated.

### The breakthrough (frontier-2's m2c/structure route, resolved differently)

Candidate-5's greg (dumps/text1a_pre.greg) showed all callee-saved
dispositions ALREADY matched target; the residual was only that loop2's
pointer lived in out2's own reg (s6) instead of sharing dead stptr's s3.
One pseudo = one reg in GCC 2.7.2 => target needs a SECOND pseudo for
loop2's pointer that lands s3. The winning structure (floor 1):

    /* preamble2, statement order load-bearing: */
    stptr2 = saved + 0x750;
    stptr = (s32) out2;          /* REUSE loop1's dead stptr local */
    /* loop2 calls take (s32 *) stptr; func_800523E0 takes pa4 (lever-4
       REVERTED) */

Mechanisms, all dump-verified (dumps/text1a_pre.lreg regenerated on the
floor-1 form):
- stptr's merged pseudo (10 refs/88) keeps pri 3409 -> allocates 3rd,
  lands s3; the loop2 range simply shares the register the loop1 walker
  already owns = target's s3 sharing, with NO second-allocno ordering
  problem at all (the whole (1052,1666)-window framing of s0-s2 was an
  artifact of making the loop2 pointer a FRESH low-ref local).
- The `stptr = (s32) out2` read is out2's 4th flow-counted ref and extends
  its live range to 47 insns: out2 = 4 refs/47 = pri 1702.13 — EXACTLY
  tied with tbl (4/47); the tie breaks by allocno number (79 < 86), tbl
  takes s5, out2 takes s6 = target. Carrier (6 refs/95 = 1263) -> s7,
  a3 (4/99 = 808) -> fp. All callee-saved = target.
- Lever-4 reverted (loop2's 523E0 takes pa4): restores `move a0,s7`; the
  lw a0,88(sp) residual class is gone (consistent with s2's H3).
- Preamble2 is its own cse basic block (no label between the loop1
  conditional and loop2's label, but block0's EBB ends at the loop1
  label), so cse1 has NO qty knowledge of out2 there and the copy
  survives to flow uncounted-by-no-one: the ref counts.

### Measured kills this session (chassis = floor-1 form unless stated)

- Re-init from the carrier (`stptr = (s32)(((u8*)pa4)+0x20)`): sandbox 15.
  Emits target's exact addiu s3,s7,32 but loses out2's 4th ref -> out2
  3/42 = 714 sinks below carrier 1263 and a3 808 -> carrier steals s6,
  3-cycle returns. rejected/reuse-reinit-from-pa4.c.
- Statement order swapped (copy before stptr2's init): sandbox 10. out2's
  live length drops to 46 -> pri 1739 > 1702 -> out2 allocates BEFORE tbl
  and steals s5. The copy must be the LAST preamble2 statement.

### The single residual insn (artifact s3/residual_floor1.txt)

Slot 71/132: OURS `move s3,s6` vs TGT `addiu s3,s7,32` (the other listed
lines in the artifact are objdump reloc-name artifacts, not diffs). The
re-init spelling space is now characterized: out2-read forms emit the move
(and score 1); carrier-read forms emit the addiu (and score 15). To reach 0
the build needs BOTH the flow-counted out2 ref AND addiu-from-s7 bytes.
Analysis of the deletion windows: any out2-read that cancels/folds dies
PRE-flow (front-end fold within one expression; cse qty-fold + canon_reg
within the preamble2 block — canon_reg substitutes the older reg and folds
+out2-out2; a dead `d = out2` staging store is deleted by flow UNCOUNTED).
The only post-flow deleter that leaves no bytes is COMBINE (runs after
flow), and combine's syntactic cancellation needs an EVEN number of out2
reads (+2 refs -> 5/47 = 2127 -> out2 allocates 4th and steals s5, dead
unless the last read also moves INTO loop2: refs 5 needs live 59-79 for
the (1263,1702) window; a read inside loop2's body extends live to ~88 ->
909, too low). No spelling found this session that satisfies both; see
hypotheses s3 frontier for the two surviving directions.

- [s3] floor 1 (from 5): loop2 pointer reuses the dead loop1 stptr local, re-initialized by READING out2 (`stptr = (s32) out2;` as the last preamble2 statement), lever-4 reverted; 132/132; single residual insn: move s3,s6 vs addiu s3,s7,32 at slot 71
- [s3] split-RA axis (fresh out2b pseudo world) proven dead END-TO-END: masked inverse depth<=4 empty + analytic closure (77->s7 unreachable at any depth under the spellable mask)
- [s3] out2's floor-1 stats: 4 refs/47 insns = pri 1702.13, EXACT tie with tbl (4/47); tie breaks by allocno number (79<86) -> tbl s5, out2 s6. Knife-edge: live 46 (order swap) scores 10; refs 3 (pa4-read re-init) scores 15
- [s3] family status of the floor-1 construct: `stptr = (s32) out2` + loop2 uses is VARIABLE-REUSE (borrow of an EXISTING dead local, SOTN frozen-list "variable reuse for codegen control", defeat-licm-hoist-var-reuse; borrows gated by staged-value-reused-variable) — value is real and consumed by both loop2 calls, zero dead stores, live code. FAKE annotation + exhaustion ledger required at submission per the family-selection table; NOT yet annotated in candidate.c (floor is 1, not 0 — no submission this session)

- [s3] Floor 1 verified three times this session with edits in place in src/text1a_pre.c; candidate saved to memory/grind/func_80041188/candidate.c

- [s3] Split-RA axis dead end-to-end: masked inverse depth<=4 empty + analytic closure (artifact s3/masked_inverse_out.txt)

- [s3] out2 floor-1 stats: 4 refs/47 insns = pri 1702.13, exact tie with tbl; knife-edge measured: live 46 (statement-order swap) scores 10, refs 3 (pa4-read re-init, which emits target's exact addiu s3,s7,32) scores 15 (rejected/reuse-reinit-from-pa4.c)

- [s3] Residual deletion-window analysis: any out2-read cancellation dies pre-flow (front-end fold / cse qty-fold + canon_reg in the preamble2 block / flow uncounted dead-store deletion); only COMBINE deletes post-flow, and combine cancellation needs even out2-read parity (+2 refs -> 2127 steals s5 unless live 59-79, positions that don't exist)

- [s3] Floor-1 construct family: variable-reuse (borrow of EXISTING dead local, SOTN frozen list, defeat-licm-hoist-var-reuse; borrow gated by staged-value-reused-variable); value real and consumed, zero dead stores; FAKE annotation + exhaustion ledger required at submission, not yet annotated (no submission at floor 1)

## s4 (permuter, 2026-08-22) - FLOOR 1 -> 0; the function MATCHES honestly

Chassis at session start: s3's floor-1 form applied to src/text1a_pre.c
(HEAD carried the stale rule-era body with a `register s32 *s7_a4 asm("s7")`
pin; that pin is GONE from the candidate and from src as of this session).
sandbox --disable all reproduced 1, 132/132. Session ends with sandbox 0.

### Permuter infrastructure built this session (reusable)

A minimal-TU permuter workspace reproduces the honest sandbox distance exactly
- worth knowing for every future text1a_pre grind:
- base.c = `tmp/grind/func_80041188/s4/mini.c` preprocessed: just
  `#include "common.h"`, the five externs the function needs, and the function.
  Codegen is IDENTICAL to the full-TU build (validated: base.o vs target.o
  diff == the known single residual insn) because nothing in the rest of the
  TU feeds this function's codegen; maspsx's sdata decisions come from the
  repo-global `sdata_syms.txt`, not from TU contents.
- compile.sh replicates engine/pipeline.py exactly for a GP file:
  cc1 -O2 **-G8** ... -mel | prologue_fix | maspsx MASPSX_FLAGS_GP |
  `sed s/.align\t3/.align\t2/` (text1a_pre is in RODATA_ALIGN2_FILES) |
  multu_pad | regfix | regfix_stage2 | asmfix, with PROLOGUE_CONFIG /
  REGFIX_CONFIG / ASMFIX_CONFIG pointed at the SANDBOX'S FILTERED configs in
  `tmp/sandbox/func_80041188/cfg/` so the campaign optimises the HONEST
  distance, not the cheat-assisted one. Two gotchas cost a turn each:
  (a) maspsx strips the leading tab from directives, so a
  `/^\t\.ent\tfunc/` extraction awk silently matches nothing - use
  `/^[ \t]*\.ent[ \t]+func_80041188$/`; (b) extraction must start at
  `.ent`, not at the bare label, or `as` rejects `.frame` as outside `.ent`.
- Permuter's own weighted score is NOT the sandbox distance and ranks these
  chassis in the OPPOSITE order: floor-1 (sandbox 1, one wrong-opcode insn)
  scored 200, while the pa4-read chassis (sandbox 15, all register
  permutations) scored 88. That is why seeding the "worse" chassis was the
  productive move - register diffs are cheap in the permuter metric and it
  hill-climbs them, whereas the single opcode diff is a cliff.

### Campaign 1 - floor-1 chassis (tmp/grind/func_80041188/s4/perm), KILLED

22,296 iterations / 22.5 min, base 200, 2 finds (best 135), no novel find after
the first 3 minutes; harvested + stopped. The 135 find is INVALID (semantic
break: it hoists `stptr2 + 6` out of loop2, which advances 0x68/iteration -
banked as rejected/permuter-invariant-hoist-stptr2-semantic-break.c). Reading:
the floor-1 basin is exhausted for the permuter's transform set; its residual
really is the one-insn spelling-class question s3 characterised, and no
randomisation of that chassis reaches it.

### Campaign 2 - pa4-read chassis (tmp/grind/func_80041188/s4/perm2), WON

22,317 iterations / ~22 min, base 88, 2 finds (63 then 50). Both finds are
partial and BOTH are needed; each was hand-re-measured in the honest sandbox
rather than trusted:
- output-63-1 => `out2 = (s32 *) stptr;` immediately before loop2's
  `func_800523E0`, staging the out-pointer through the dead loop1 `out2`
  local. Sandbox 15 -> 9. This is the piece that DECOUPLES out2's flow ref
  count from the re-init spelling - the exact obstacle s3 characterised as
  "the ref and the bytes cannot come from the same honest statement".
- output-50-1 => `do { loop1: ...second func_8004A348... } while (0);`
  wrapping loop1's leading half with the label INSIDE the wrap. Sandbox
  9 -> 0 on the pa4-read chassis.

### The measured lever decomposition (all this session, honest sandbox)

    pa4-read re-init alone (s3's rejected form) ......... 15
    + out2 staging in loop2 ............................  9
    + do-while(0) wrap of loop1's leading half .........  0   <== MATCH
    out2-read re-init (s3 floor-1) + out2 staging ......  17  (rejected/)
    out2-read re-init (s3 floor-1) + the wrap ..........  1   (no change)
    chassis2 + wrap, WITHOUT the staging ...............  9

The last three rows are the load-bearing kills: the two levers are
CHASSIS-EXCLUSIVE. On the out2-read chassis out2 already owns its 4th ref, so
the staging line over-refs it and it steals tbl's s5 (17); the wrap alone
changes nothing there (1). Only on the pa4-read chassis - the one that emits
target's exact `addiu s3,s7,32` - does the pair pay, because the staging line
REPLACES the ref the pa4 re-init gave up and the wrap then re-weights the
remaining allocation order into target's callee-saved seating.

### Final form (memory/grind/func_80041188/candidate.c, in src/text1a_pre.c)

132/132 insns, frame 72 == target 0x48, sandbox --disable all == 0. Three
FAKE-annotated constructs, all inside sanctioned families and all vetted in
memory/grind/func_80041188/self_vet.md: the single-level do-while(0) wrap
(.claude/rules/do-while-zero-exception.md - owner ruling 2026-07-06 sanctions
it for ANY codegen effect incl. RA, single level needs no exhaustion gate),
the stptr reuse (s3's, staged-value-reused-variable), and the out2 staging
(staged-value-reused-variable, all six bounds stated in the vet). No register
pin, no inline asm, no volatile, no dead store, no unused local, no invented
variable, no regfix/asmfix dependency.

- [s4] FLOOR 1 -> 0. func_80041188 matches honestly with three FAKE-annotated sanctioned-family constructs; self_vet.md written; candidate.c updated; the rule-era `register ... asm("s7")` pin is gone from src
- [s4] The two closing levers are chassis-exclusive (out2 staging pays ONLY on the pa4-read re-init chassis: 15->9; on the s3 out2-read chassis it scores 17). The do-while(0) wrap is worth 9->0 on the pa4 chassis and 0 on the s3 chassis
- [s4] Minimal-TU permuter workspace for text1a_pre validated (identical codegen to the full TU) and pointed at the sandbox's FILTERED cheat configs, so campaign score tracks the HONEST distance; recipe + two extraction gotchas banked above
- [s4] Permuter weighted score inverts the sandbox ranking here (floor-1 chassis 200 vs pa4 chassis 88): seed the chassis whose residual is REGISTER-shaped, not the one with the lowest sandbox distance
- [s4] Campaign 1 (floor-1 chassis) KILLED: 22.3k iterations, zero valid finds; its only find was a semantics-breaking loop-invariant hoist (banked in rejected/)

## s5 (synthesis, 2026-08-22) — the governing arithmetic CORRECTED; the residual reduced to one see-saw statement

Chassis at session start: HEAD carries the rule-era body (16 regfix rules,
`register s32 *s7_a4 asm("s7")` pin) at honest 27. The s4 candidate.c (distance
0) was rejected at layer-1 review on 2026-08-22 23:24; its closing lever — a
`do { } while (0)` around loop1's LEADING HALF with the `loop1:` label inside
and the back-goto entering from outside — is now a BANNED construct for this
function. Session ends with the s3 floor-1 form (sandbox 1, 132/132) restored
and in place in src/text1a_pre.c, re-verified.

### THE CORRECTION (invalidates several s1–s4 arithmetic conclusions)

`pri = floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000`, and
**floor_log2(3) == 1**, not 2. Earlier sessions computed a 3-ref pseudo as
`2*3/L`. The real value is `1*3/L`. Consequence: a 3-reference `out2` scores
714.3 at live 42 — it can NEVER outrank the pa4 carrier (1473.7) at any
reachable live length (it would need live <= 20). Every "3-ref out2 needs live
<= 40" style reasoning in the s2/s3 notes is void. **out2 needs >= 4 refs,
full stop.**

The formula was re-validated against six measured chassis this session
(dispositions predicted correctly in all six).

### The complete allocation spec (measured, artifact s5/tbl_*.txt)

Target seating requires this priority ORDER:
`73 a1 (6464) , 74 a2 (6464) > 87 stptr (3409) > 90 stptr2 (2500) >
78 i (2474) > 79 tbl > 86 out2 > 77 pa4-carrier > 75 a3 (808)`
Every chassis measured this session gets a1/a2/stptr/stptr2/i/a3 right; the
ONLY defect anywhere is the relative order of tbl (79), out2 (86) and the
carrier (77). Required window: **pri(out2) strictly inside (pri(pa4), pri(tbl))**
— i.e. (1473.7, 1702.1) on the pa4-read chassis, (1263.2, 1702.1) on the
out2-read chassis. Reachable out2 (refs/live -> pri): 4/47 = 1702.1 (exact tie
with tbl, breaks our way on allocno number 79 < 86), 4/48–54 = 1666–1481,
5/59–67, 6/71–81.

### The see-saw: one statement carries both the ref and the bytes

The between-loops (MID) statement `stptr = <X> + 0x20` is a single
flow-ref token:
- `X = out2` -> out2 4 refs/47 = 1702.1, pa4 6 refs/95 = 1263.2. Seats are
  EXACTLY target; the emitted insn is `move s3,s6`. **sandbox 1** (= the s3
  floor, restored as candidate.c).
- `X = pa4` -> out2 3 refs/42 = 714.3, pa4 7 refs/95 = 1473.7. Emits target's
  exact `addiu s3,s7,32`; seats collapse (out2 falls below a3). **sandbox 15.**
There is no third spelling of that one statement. Closing the function
therefore requires a SECOND, byte-free, flow-counted out2 reference somewhere
in the MID basic block (block 2) — MID's six target insns are
`addiu s1,s1,108 / addiu s2,s2,108 / addiu s4,zero,18 / lw t0,0x18(sp) /
addiu s3,s7,32 / addiu s0,t0,1872` and none of them can take out2 as an input.

### Measured kills this session (each with mechanism)

- **Real-loop spellings (the honest source of the banned wrap's loop note).**
  loop1 as `do { ... } while (i < 0x12)`: 132 insns, **sandbox 28**. flow.c
  weights refs by `loop_depth`, so a loop note lifts EVERY pseudo referenced
  inside — and it lifts the carrier faster than out2 (pa4 7 -> 9 refs = 2872;
  out2 3 -> 7 refs but live drops to 41). loop2 as a real do-while: **135
  insns** (loop.c runs on a note-marked loop and emits 3 extra insns); both
  loops real: 28 / 135 insns. The wrap's effect is NOT reachable from any
  well-formed C loop here. rejected/real-loop-do-while-loop1.c.
- **tbl (79) refs+2 via the banked sym-K chain — REALISED, byte-neutral, and
  still dead.** `tbl = (s32*)((u8*)D_80094CFC - 0x10); tbl = (s32*)((u8*)tbl +
  0x10);` gives tbl 4 -> 6 refs at 132 insns (confirmed in .lreg). But 6 refs
  at tbl's live 47 = 2553.2 > i (2474.2), so tbl steals s4. The required window
  is (2381.0, 2474.2) = live 49–50 at 6 refs, and **tbl's live length is
  scheduler-pinned to 45–47 across all 24 combinations of preamble statement
  order x advance-statement position** swept this session. sandbox 12.
  rejected/tbl-symK-splitinit-refs6-steals-s4.c.
- **The (90 refs+2 & 78 refs+2) route that would re-open tbl's window
  (order 90 5000 > 87 3409 > 78 3092 > 79 2553 > 86 2381 > 77 1473): both
  lifts FOLD AWAY PRE-FLOW.** Split-init on stptr2 (`saved+0x700` then `+0x50`)
  and on i (`0x10` then `+2`) leave reg_n_refs *identical* (6 and 8) in .lreg.
  Mechanism: cse1's cost gate — the fold-back target is a ONE-insn addiu,
  cheaper than the chain, so cse folds and flow deletes the dead first store
  UNCOUNTED. The tbl chain survives ONLY because its fold-back is a TWO-insn
  lui/addiu symbol constant. **Split-init ref-lifts exist only on
  symbol-constant-based pseudos.** rejected/splitinit-i-stptr2-folds-preflow.c.
- **Staging-position sweep (6 positions).** The s4 staging store
  `out2 = (s32*)stptr;` is byte-free ONLY in the immediately-before-the-call
  position (out2 5 refs/42 = 2381, sandbox 9 = the s4 form minus the banned
  wrap). Every earlier position materialises the copy: preamble2 11/133,
  loop2-top 3/133, after func_80044DE4 3/133, mid-loop2 12/133,
  preamble2+both-uses 11/132, loop2-top+both-uses 3/133.
- **rejected/staging-position-sweep-133insn.c is the most informative negative
  in the ledger:** the loop2-top staging form has **EXACTLY target's register
  dispositions** (73>s1 74>s2 87>s3 90>s0 78>s4 79>s5 86>s6 77>s7 75>fp,
  out2 6 refs/81 = 1481.5, inside the window) and its ONLY defect is one extra
  `addu s6,s3,zero` at slot 73. sandbox 3.
- **Dropping the pa4 carrier entirely** (use the `a4` parameter at all six
  sites): 132 insns, sandbox 10, but the allocno world changes completely
  (out2 absorbs the merged 10-ref pseudo, the param scores 736.8 at live 190).
  Not closer. **Two carriers** (pa4 for loop1, a4 for loop2): 134 insns.
- Preamble statement-order sweep on the pa4-read chassis: out2 live moves only
  42 -> 41 (def last) or 43 (def first); pa4 live 94/95/96. None of these
  perturbations is large enough to matter now that floor_log2(3) = 1.

- [s5] Priority formula re-validated on six chassis; floor_log2(3)=1 correction
  invalidates the s2/s3 "3-ref out2" arithmetic. out2 needs >= 4 refs.
- [s5] The residual is ONE see-saw statement: MID's `stptr = <out2|pa4> + 0x20`
  supplies either out2's 4th ref (correct seats, `move` bytes, sandbox 1) or
  target's `addiu s3,s7,32` (correct bytes, collapsed seats, sandbox 15).
- [s5] Closing requires a SECOND byte-free flow-counted out2 read in MID
  (block 2); MID's six target insns cannot consume out2.
- [s5] Loop-note weighting (the banned wrap's mechanism) is NOT reachable from
  honest C loops: it lifts the carrier faster than out2, and loop2-as-real-loop
  costs 3 insns.
- [s5] Split-init ref-lifts work ONLY on symbol-constant-based pseudos (cse1
  cost gate); i and stptr2 cannot be lifted this way.
- [s5] tbl +2 refs is byte-neutral and reachable, but tbl's live length is
  scheduler-pinned to 45-47, and the window needs 49-50.

- [s4] CORRECTION: floor_log2(3) == 1, so a 3-ref out2 scores 714.3 at live 42, not the 1428 assumed in s2/s3. Every 'a 3-ref out2 just needs live <= 40' conclusion in the earlier ledger is void; out2 needs >= 4 refs unconditionally.

- [s4] The priority model was re-validated against six measured chassis this session and predicted the register dispositions correctly in all six (artifacts tmp/grind/func_80041188/s5/tbl_*.txt).

- [s4] Target seating requires the order 73/74 (6464) > 87 stptr (3409) > 90 stptr2 (2500) > 78 i (2474) > 79 tbl > 86 out2 > 77 pa4-carrier > 75 a3 (808). EVERY chassis measured this session already seats a1/a2/stptr/stptr2/i/a3 correctly; the sole defect anywhere is the relative order of tbl, out2 and the carrier.

- [s4] THE SEE-SAW: the between-loops (MID) statement stptr = <X> + 0x20 is one flow-ref token. X = out2 gives out2 4 refs/47 = 1702.1 and pa4 6 refs/95 = 1263.2 (seats EXACTLY target, bytes wrong: move s3,s6, sandbox 1). X = pa4 gives out2 3 refs/42 = 714.3 and pa4 7 refs/95 = 1473.7 (bytes exactly target's addiu s3,s7,32, seats collapse, sandbox 15). No third spelling of that statement exists.

- [s4] Closing the function therefore needs a SECOND byte-free flow-counted out2 read in MID (block 2). MID's six target insns are addiu s1,s1,108 / addiu s2,s2,108 / addiu s4,zero,18 / lw t0,0x18(sp) / addiu s3,s7,32 / addiu s0,t0,1872, and none of them can consume out2.

- [s4] out2's window is (1473.7, 1702.1) on the pa4-read chassis and (1263.2, 1702.1) on the out2-read chassis. Reachable landings: 4 refs/live 47 = 1702.1 (exact tie with tbl, breaks our way on allocno number 79 < 86), 4/48-54, 5/59-67, 6/71-81.

- [s4] Loop-note weighting (the banned wrap's mechanism) is NOT reachable from honest C loops: a real loop1 lifts the carrier (7->9 refs, 2872) faster than out2, and a real loop2 costs 3 instructions because loop.c runs on a note-marked loop.

- [s4] The banked tbl sym-K split-init ref-lift is REAL and byte-neutral (tbl 4->6 refs at 132 insns) - the first confirmed byte-neutral +2 reg_n_refs lever on this function - but tbl's live length is scheduler-pinned to 45-47 across all 24 preamble-order x advance-position combinations, and the needed window is live 49-50.

- [s4] Split-init ref-lifts work ONLY on symbol-constant-based pseudos: the same chain on stptr2 (register+immediate) and i (plain immediate) leaves reg_n_refs untouched because cse1's cost gate prefers the cheaper 1-insn fold-back and flow then deletes the dead store uncounted.

- [s4] The loop2-top staging form (rejected/staging-position-sweep-133insn.c) is the sharpest negative in the ledger: register dispositions EXACTLY target (out2 6 refs/81 = 1481.5, inside the window) with a single extra insn, addu s6,s3,zero at slot 73. sandbox 3 at 133 insns.

- [s4] The s4 distance-0 form minus the banned wrap scores 9 (banked as rejected/s4-form-minus-banned-wrap-floor9.c); the s3 floor-1 form is restored, re-verified at sandbox 1 / 132 insns, and is in place in src/text1a_pre.c and memory/grind/func_80041188/candidate.c.

## s6 (forensics, 2026-08-23) — the allocation model INSTRUMENTED end-to-end; the whole staged-copy ref-lift family closed with named passes

Chassis at session start: HEAD carries the rule-era body (16 rules) at honest 27;
candidate.c (the s3 floor-1 form) applied to src/text1a_pre.c re-measures
**sandbox --disable all == 1, 132/132 insns** — floor re-confirmed THIS session
and the candidate is in place in src at session end.

### The allocation model is no longer a model — it is instrumented

`tools/gcc-2.7.2/cc1` (instrumented) with `BB2_ALLOC_DEBUG=1` and
`BB2_FINDREG_DEBUG=<pseudo>`; driver `tmp/grind/func_80041188/s5/idump.sh`
(runs engine/buildconfig's exact cpp|cc1 with -da into a per-tag dir).
Measured on the pa4-read chassis (artifact s5/pa4/stderr.txt):

    ord pseudo hardreg refs live pri     role
    0   73     17 s1    16   99   6464   a1
    1   74     18 s2    16   99   6464   a2
    2   87     19 s3    10   88   3409   stptr (merged loop1 walker + loop2 ptr)
    3   90     16 s0     6   48   2500   stptr2
    4   78     20 s4     8   97   2474   i
    5   79     21 s5     4   47   1702   tbl
    6   77     22 s6     7   95   1473   pa4 carrier      <-- target wants s7
    7   75     23 s7     4   99    808   a3               <-- target wants fp
    8   86     30 fp     3   42    714   out2             <-- target wants s6
    9   85     -1  —      2   47    425   saved (SPILLED; target spills it too:
                                          `lw t0,0x18(sp)` + `addiu s0,t0,1872`)

`BB2_FINDREG_DEBUG` for pseudos 73 / 77 / 86 shows **`own_copy_prefs` empty,
`someone_prefers` empty for every one of them**: find_reg's preference-override
block (global.c ~line 1090) never fires in this function, and MIPS defines no
REG_ALLOC_ORDER, so the seat is simply *the lowest-numbered non-conflicting
callee-saved register at the moment the allocno is processed*. The only
non-obvious datum is that every allocno except 90 carries a hard-reg conflict
with reg 16 ($s0) — that is why a1 (first) takes s1, not s0.

**Consequence (the model is now closed):** the seats are a pure function of the
priority ORDER. Target's seats require the last three allocnos to be processed
out2 -> carrier -> a3, i.e. **pri(out2) must sit strictly inside
(carrier 1473.7, tbl 1702.1)** — exactly the s5 window, now first-principles
confirmed rather than inferred.

### The target's OWN register file proves the 4th out2 reference is post-flow

In asm/funcs/func_80041188.s the callee-saved seating is
s0 stptr2 / s1 a1 / s2 a2 / s3 stptr / s4 i / s5 tbl / s6 out2 / s7 carrier /
fp a3, and **s6 is referenced exactly three times in the final code** (def
`addiu s6,s7,0x20`, and the two loop1 `addu a1,s6,zero`); s6 does NOT appear in
loop2 at all. Three materialised refs at live ~42 is pri 714 — unreachable.
Therefore the original C carried at least one further out2 reference that was
deleted AFTER flow.c counted reg_n_refs (only combine.c can do that here), and
the entire remaining search is: *which byte-free, post-flow-deleted out2
reference shape lands (refs, live) in the window?*
Landing zones: 4 refs/live 48-54, 5/59-67, 6/71-81, 7/82-95.

### The staged-copy family is CLOSED — cse1 decides, and both branches lose

Measured this session, each with the pass named from the dumps:

1. **MID re-init chain** `out2 = pa4+0x20; stptr = (s32) out2;`
   (rejected/mid-chain-out2-reinit-cse1-destswap.c, artifacts s5/e1/*).
   .rtl has `86 = 77+32` then `87 = 86`; **.cse shows cse1 DEST-SWAPPING the
   pair into `87 = 77+32` then `86 = 87`** (cse.c cse_insn rewrites the
   producer's destination when its original dest dies in the copy); out2 is then
   dead, so flow.c's dead-store elimination removes the copy BEFORE reg_n_refs
   is taken. Result: byte-perfect MID (`addiu s3,s7,32`, 132 insns) but out2
   still 3 refs / 42 / 714 — **sandbox 15, allocation bit-identical to the plain
   pa4 chassis.** The lift is not merely byte-free, it is UNCOUNTED.

2. **Loop2-top stage with the use-swap** (the degree of freedom the s5 sweep
   never varied): copy at loop2 top, consumed by the *second* func_8004A348,
   with func_800523E0 keeping stptr
   (rejected/loop2top-stage-single-use-cse1-propagated.c, artifacts s5/va/*).
   Uses of reg 86 inside loop2: **.rtl 2 -> .cse 1 -> .flow 0** — cse1
   canonicalises the out2-spelled use back onto 87, the copy dies, flow deletes
   it uncounted. Allocation again bit-identical to the pa4 chassis.

3. Contrast with rejected/staging-position-sweep-133insn.c (the 6 refs/81 form
   with EXACTLY target's seats): there the C spells the LAST pointer use `out2`,
   and cse1 goes the other way — it pulls the EARLIER stptr use onto 86 as well.
   The .combine dump for that form shows `insn 171: (set (reg 86) (reg 87))`
   surviving with **two** consumers (insn 254 carries the LOG_LINK back to 171,
   insn 263 carries the REG_DEAD). combine's try_combine can only delete a
   producer whose destination dies at the single use it substitutes into, so the
   copy materialises: `addu s6,s3,zero`, 133 insns.

**The closure argument.** A staged copy `out2 = stptr` has exactly two fates,
both decided by cse1: if out2 is spelled at the LAST pointer use in the block,
cse1 pulls the earlier use onto out2 too => two consumers => combine cannot
delete it => +1 insn (case 3); otherwise cse1 propagates the use back to stptr
=> dead store => flow deletes it uncounted (cases 1-2). The only byte-free,
COUNTED position is the one the s5 sweep already found — copy immediately before
its single last use — whose def->use span is ~1 insn, pinning out2 at 5 refs /
live 42-44 = pri 2381 (sandbox 9). Reaching 5 refs/59-67 needs a ~20-insn
def->use span, and every such span necessarily steps over the second
func_8004A348 (the other pointer use), which flips cse1 into case 3.
**Frontier items #1 (a second byte-free out2 read in MID) and #2 (make the
loop2-top copy propagable) are therefore both dead.**

### Frontier #3 (the carrier-free V1 world) is dead too — it needs TWO inversions

ALLOCDBG on V1 (a4 used at all six sites; artifacts s5/v1/stderr.txt) gives
a3 4/99 = 808 -> s6, a4 7/190 = 736 -> s7, out2 3/42 = 714 -> fp. Target needs
out2 -> s6, a4 -> s7, a3 -> fp, i.e. order out2 > a4 > a3: out2 must pass both
AND a4 must additionally pass a3 (it is already 72 points below it). The
pa4-carrier chassis needs one inversion; V1 needs two.
Banked as rejected/v1-carrier-free-two-inversions.c.

- [s6] The seat rule is instrumented, not inferred: no copy preferences and no
  someone_prefers entries fire in this function, MIPS has no REG_ALLOC_ORDER, so
  seats = lowest free non-conflicting callee-saved reg in priority order.
- [s6] Target's own asm has only THREE materialised s6 references, so the 4th+
  out2 reference the priority window demands must have been deleted post-flow by
  combine in the original — the search is for a combine-deletable out2 mention.
- [s6] cse1's dest-swap (cse.c cse_insn) rewrites `A = expr; B = A` into
  `B = expr; A = B`, so a MID re-init chain on out2 is deleted by flow's
  dead-store elimination BEFORE reg_n_refs is counted (sandbox 15, allocation
  identical to the plain pa4 chassis).
- [s6] Staged-copy closure: cse1 either propagates the copy away (uncounted) or
  pulls a second consumer onto it (uncounted-> materialised +1 insn). The only
  byte-free counted position is span~1 (5 refs/42/2381, sandbox 9). No staged
  copy can reach 5 refs/59-67 or 6 refs/71-81 byte-free.
- [s6] V1 (carrier-free) requires two priority inversions vs one for the
  pa4-carrier chassis — strictly harder, axis closed.

- [s5] Floor re-verified THIS session: memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c scores sandbox --disable all == 1 at 132/132 insns, and is in place in src at session end.

- [s5] Instrumented ALLOCDBG for the pa4-read chassis (s5/pa4/stderr.txt): 73 a1 -> s1, 16 refs/99, 6464 | 74 a2 -> s2, 6464 | 87 stptr -> s3, 10/88, 3409 | 90 stptr2 -> s0, 6/48, 2500 | 78 i -> s4, 8/97, 2474 | 79 tbl -> s5, 4/47, 1702 | 77 carrier -> s6, 7/95, 1473 | 75 a3 -> s7, 4/99, 808 | 86 out2 -> fp, 3/42, 714 | 85 saved SPILLED, 2/47, 425 (target spills it too: lw t0,0x18(sp) + addiu s0,t0,1872).

- [s5] BB2_FINDREG_DEBUG (pseudos 73/77/86): own_copy_prefs empty, someone_prefers empty, pass 0 never succeeds; MIPS defines no REG_ALLOC_ORDER, so find_reg returns the lowest-numbered non-conflicting callee-saved register. Seats are a pure function of allocno_compare order.

- [s5] Every allocno except stptr2 (90) carries a hard-reg conflict with reg 16 ($s0) - which is why a1, allocated first, takes s1 rather than s0. Hard-reg conflicts are the one seat-level mechanism in this function that is not priority.

- [s5] Target's callee-saved map read off asm/funcs/func_80041188.s: s0 stptr2, s1 a1, s2 a2, s3 stptr, s4 i, s5 tbl, s6 out2, s7 carrier (a4, lw from 0x58(sp)), fp a3. s6 appears exactly 3 times and never in loop2; s7 appears 7 times.

- [s5] cse.c's cse_insn performs a DEST-SWAP: 'A = expr; B = A' becomes 'B = expr; A = B' (dumped on the MID re-init chain, s5/e1/text1a_pre.cse insns 167/170). flow.c then deletes the resulting dead copy before reg_n_refs is taken.

- [s5] cse1 canonicalises loop2's two pointer uses onto ONE register, and the direction depends on which use the C spells 'out2': last-use spelling -> both uses become reg 86 (the copy survives with two consumers and materialises as addu s6,s3,zero, 133 insns); earlier-use spelling -> the use reverts to reg 87 and the copy is deleted uncounted (s5/va: reg-86 uses inside loop2 go .rtl 2 -> .cse 1 -> .flow 0).

- [s5] combine.c try_combine deletes a copy's producer only when the producer's destination dies at the single use being substituted; the 133-insn form's insn 171 has a LOG_LINK consumer (insn 254) and a later REG_DEAD consumer (insn 263), so it can never be deleted.

- [s5] V1 (carrier-free) ALLOCDBG: a3 4/99 = 808 -> s6, a4 param 7/190 = 736 -> s7, out2 3/42 = 714 -> fp; the target order out2 > a4 > a3 needs two inversions versus one in the pa4-carrier chassis.

- [s5] Reusable tooling banked: tmp/grind/func_80041188/s5/idump.sh runs the instrumented cc1 (tools/gcc-2.7.2/cc1) with engine/buildconfig's exact flags plus -da into a per-tag directory, honouring BB2_ALLOC_DEBUG / BB2_FINDREG_DEBUG; tmp/grind/func_80041188/s5/swap.py swaps any candidate body into src/text1a_pre.c in one command.

## s7 (forensics, 2026-08-23; driver session 6, scratch tmp/grind/func_80041188/s6)

Chassis at session start: HEAD carries the rule-era body (16 rules) at honest 27;
candidate.c (the s3 floor-1 form) applied to src/text1a_pre.c re-measured
**sandbox --disable all == 1, 132/132** at session start and again at session end
(candidate.c is in place in src). Floor unchanged at 1.

### Frontier #1 RESOLVED (not a mystery, and not a lever): the cse1 sparing rule

The one existence proof of a counted-then-combine-deleted out2 copy
(rejected/s4-form-minus-banned-wrap-floor9.c) was dumped for the first time
(artifacts tmp/grind/func_80041188/s6/f9/*). Reg 86's history:

    .rtl     insn 256 `(set (reg 86) (reg 87))`   the staging copy
             insn 263 `(set (reg 5 a1) (reg 86))` its only consumer
    .cse     BOTH unchanged - cse1 does NOT propagate the copy away
    .flow    insn 263 carries LOG_LINK 256 + REG_DEAD 86; reg_n_refs = 5, live 42
    .combine BOTH GONE - try_combine substitutes 87 into 263 and deletes 256

**The sparing condition is the SAME cse.c make_regs_eqv canonicality rule the
ledger already had (s2), read in the other direction.** On a copy `new = old`,
NEW becomes the qty's canonical (substitution-target) register iff
`regno_last_uid[new] > regno_last_uid[old]`. In the floor-9 form the copy is
placed AFTER loop2's other pointer use (the second func_8004A348 takes stptr),
so 86's last textual reference (the func_800523E0 argument) is later than 87's,
86 is canonical, and cse1 has nothing to rewrite: the earlier 87 use precedes
the copy and never joins the qty. Hence the copy survives cse, flow COUNTS both
its def and its use, and combine deletes it byte-free.

That closes the family with one law rather than three observations:

    a staged copy of the loop2 pointer is COUNTED iff it is placed after every
    other use of that pointer in the block (86 canonical); it is COMBINE-
    DELETABLE iff exactly one use follows it. Both together pin the copy
    between the last-but-one and the last pointer use, i.e. a def->use span of
    ~1 insn => out2 5 refs / live 42 / pri 2381. The 5/59-67 and 6/71-81
    landings are unreachable, exactly as s6 concluded - the "unknown cse.c
    condition" it hypothesised does not exist.

### Frontier #2 KILLED: the hard-reg conflict set cannot be steered from C

WHY every allocno except stptr2 conflicts with hard reg 16 ($s0): local-alloc
assigns block-local quantities before global.c runs, and loop1's `offset`
temp (a block-1 local that is live across the first func_8004A348, so it needs a
callee-saved register) takes the LOWEST free one. tools/gcc-2.7.2/local-alloc.c
find_free_reg (~line 2249) scans `for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)`
with `#ifdef REG_ALLOC_ORDER` inactive on MIPS, so the scan is strictly
ascending; the only override is qty_phys_copy_sugg, which is populated ONLY from
hard-reg copies (call arguments/returns - all call-clobbered here).
**The target's own asm confirms the original has the same structure**:
asm/funcs/func_80041188.s uses $s0 in loop1 for the sll/addu offset chain
(0x800411F8-0x80041238) and only later as stptr2 (0x800412A8+).
Consequence: to place a conflict at hard reg 22 ($s6) - the frontier's
"seat fix with no ref lift" - the function would need SIX further
simultaneously-live, call-crossing, block-local quantities so the ascending scan
reaches 22, they would have to live in a region overlapping the pa4 carrier AND
a3 but NOT out2 (i.e. loop2 only), and the first of them would take reg 16 and
displace stptr2 from $s0. Byte-prohibitive and self-defeating: the conflict axis
is dead.

### THE FIND: a chassis with ALL-TARGET register seats (sandbox 4, 132 insns)

rejected/tbl-symK-ilate-alltarget-seats-emission-swapped.c =
  pa4-read MID re-init (emits target's `addiu s3,s7,32`)
+ the out2 staging store before loop2's func_800523E0 (the floor-9 lever)
+ the tbl sym-K split-init ref lift (tbl 4 -> 6 refs, byte-neutral)
+ **`i = 1` moved out of the declaration into the LAST preamble statement** (new
  this session; the s5 24-variant sweep never varied the i-def position).

    78 i      8 refs / live 96 = 2500.0 -> s4   (was 8/97 = 2474 -> s5)
    79 tbl    6 refs / live 48 = 2500.0 -> s5   (was 6/47 = 2553 -> s4)
    90 stptr2 6/48 = 2500 -> s0 | 87 stptr 10/88 = 3409 -> s3 |
    86 out2   5/42 = 2381 -> s6 | 77 pa4 7/95 = 1473 -> s7 |
    75 a3     4/99 = 808 -> fp | 73/74 -> s1/s2 | 85 saved spilled.

Every callee-saved disposition equals target for the first time in this ledger -
i and tbl tie at exactly 2500.0 and the tie breaks our way on allocno number
(78 < 79). Ten different (tbl-advance x i-position) spellings reach it; all ten
measure sandbox 4 with an identical residual.

**The residual is 4 insns of PREAMBLE EMISSION ORDER:**

    TGT  7 sw s4,48(sp) | 8 addiu s4,zero,1 | 9 sw s5,52(sp) | 10 lui s5 | 11 addiu s5
    OURS 7 sw s5,52(sp) | 8 lui s5 | 9 addiu s5 | 10 sw s4,48(sp) | 11 addiu s4,zero,1

i.e. target defines i BEFORE the tbl symbol; this form defines tbl first.

### The anti-correlation law (why that residual is structural; 15 variants)

- Emission order == C def order. The two preamble defs are independent, carry
  equal INSN_PRIORITY, and sched1's rank_for_schedule tie-break is LUID, so the
  pair defined first in the RTL is emitted first. Measured on five spellings
  that move tbl's symbol init out of the declaration into a statement placed
  after `i = 1;` (x1..x5): all five emit `li i` before `la tbl`.
- Live length is BINARY in that same order, not a distance. Whichever of i/tbl
  is defined first gets live+1 and the other live-1, no matter how many
  statements sit between them: i-first => i 97 / tbl 47 in ALL of a*i0 and
  x1-x5; tbl-first => 96 / 48 in all of a0i2..a3i5.
- Therefore i-first => tbl 2553 > i 2474 => tbl steals s4 (sandbox 12), and
  tbl-first => target seats but the emission pair is swapped (sandbox 4).
  **Seats and emission order are anti-correlated on this chassis.** Closing it
  needs i at >= 9 flow refs (pri 2783 > 2553, and <= 11 refs to stay below
  stptr's 3409) in the i-FIRST order, byte-free.

### Also killed this session

- **In-loop statement position cannot move a loop-carried pseudo's live length.**
  Sixteen positions of loop1's `tbl++` (t01..t16) give bit-identical allocno
  tables (tbl 6 refs / live 47 in every one). Mechanism: flow.c's
  reg_live_length counts every insn where the reg is live, and a pseudo live
  across the loop back edge is live over the WHOLE body regardless of where its
  uses sit. This retires the "advance position" axis for tbl and i once and for
  all - s5's "scheduler-pinned 45-47" is really "loop-carried, hence
  position-invariant".
- **Preamble uses of `i` cannot lift its ref count**: in the preamble i is the
  literal 1, so cse1 constant-propagates any use before flow counts it.
  Combined with the s5 law (split-init ref lifts exist only on symbol-constant
  pseudos, because cse1's cost gate prefers a 1-insn addiu fold-back), no
  byte-free +1 on i has been found yet.

- [s7] Frontier #1 resolved: the floor-9 staging copy survives cse1 because
  make_regs_eqv makes 86 canonical (its last textual use outlives 87's); flow
  counts it and combine deletes it. The rule "counted iff placed after every
  other pointer use in the block, combine-deletable iff exactly one use follows"
  pins the def->use span at ~1 insn and closes the staged-copy family for good.
- [s7] Frontier #2 killed: hard-reg conflicts come from local-alloc block-local
  quantities, and local-alloc.c find_free_reg scans hard regs strictly ascending
  (no REG_ALLOC_ORDER on MIPS; copy suggestions only from hard-reg copies), so a
  $s6 conflict would need six more overlapping call-crossing loop2 locals and the
  first of them would displace stptr2 from $s0. Target's own asm shows the same
  loop1 offset temp living in $s0.
- [s7] NEW BEST STRUCTURE (not a new floor): tbl sym-K lift + out2 staging +
  pa4-read + `i = 1` as the last preamble statement gives ALL-TARGET
  callee-saved seats at 132 insns, sandbox 4; the residual is a 4-insn preamble
  emission-order swap that is anti-correlated with the seats.
- [s7] Live length of a loop-carried pseudo is invariant to statement position
  inside the loop (16-variant proof); it moves by exactly +/-1 with the relative
  ORDER of the two preamble defs, and the emission order follows that same def
  order (sched1 LUID tie-break).
- [s7] Floor re-verified at session end: candidate.c re-applied to
  src/text1a_pre.c, sandbox --disable all == 1 at 132/132.

- [s6] Floor re-verified THIS session: memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c measures sandbox --disable all == 1 at 132/132 insns, at session start and again at session end; the candidate is in place in src.

- [s6] cse1's sparing rule for a staged pointer copy is make_regs_eqv canonicality: the copy is COUNTED by flow iff it is placed after every other use of that pointer in the block (so its destination's last textual reference outlives the source's), and it is COMBINE-DELETABLE iff exactly one use follows it. Together these pin the def->use span at ~1 insn (out2 5 refs / live 42 / pri 2381) - the 5/59-67 and 6/71-81 landings are unreachable byte-free.

- [s6] Dump evidence for the above (tmp/grind/func_80041188/s6/f9): .rtl and .cse both carry insn 256 `(set (reg 86) (reg 87))` and insn 263 `(set (reg 5 a1) (reg 86))` unchanged; .flow gives insn 263 a LOG_LINK to 256 plus REG_DEAD 86 and counts reg 86 at 5 refs / live 42; .combine has deleted both.

- [s6] The reg-16 ($s0) hard-reg conflict carried by every allocno except stptr2 is produced by local-alloc, not by anything spellable in C: loop1's `offset` temp is a block-local quantity live across the first func_8004A348 and local-alloc.c find_free_reg scans hard regs strictly ascending (no REG_ALLOC_ORDER on MIPS; qty_phys_copy_sugg only ever holds hard regs seen in copies, i.e. call-clobbered argument/return registers).

- [s6] asm/funcs/func_80041188.s independently confirms the original has the same local-alloc structure: $s0 holds loop1's sll/addu offset chain at 0x800411F8-0x80041238 and only becomes stptr2 at 0x800412A8.

- [s6] NEW BEST STRUCTURE (not a new floor): rejected/tbl-symK-ilate-alltarget-seats-emission-swapped.c = pa4-read MID re-init + out2 staging before loop2's func_800523E0 + the tbl sym-K split-init ref lift + `i = 1` moved into the preamble - all nine callee-saved dispositions equal target at 132 insns, sandbox 4.

- [s6] That chassis's whole residual is 4 preamble insns of emission ORDER: TGT `sw s4,48(sp) / addiu s4,zero,1 / sw s5,52(sp) / lui s5 / addiu s5` vs OURS `sw s5,52 / lui s5 / addiu s5 / sw s4,48 / addiu s4,zero,1`.

- [s6] The anti-correlation law: the preamble def written FIRST in the C is emitted first (equal INSN_PRIORITY, sched1 LUID tie-break) AND gets live+1 while the other gets live-1. i-first therefore gives i 2474 < tbl 2553 (tbl steals s4, sandbox 12) and tbl-first gives the target seats with the emission pair swapped (sandbox 4). 15 variants measured, no third behaviour.

- [s6] Closing that chassis needs `i` at >= 9 and <= 11 flow references in the i-FIRST order (9 refs = 2783 > tbl's 2553; 12 refs = 3711 would outrank stptr's 3409 and steal s3). Blockers already measured: preamble uses of i are constant-propagated by cse1 (i is the literal 1 there), split-init ref lifts work only on symbol-constant pseudos (s5), and dead stores are deleted uncounted by flow (s2).

- [s6] In-loop statement position is inert for reg_live_length of any loop-carried pseudo (16-variant proof on loop1's `tbl++`), which supersedes the s5 'scheduler-pinned' description with a mechanism.

## s7 (rederive, 2026-08-23) — the two-locals chassis: floor 1 reached with ORDINARY C

Modality: `rederive`. The mandate was to produce a structurally different C shape
rather than perturb the merged-stptr chassis every session s1–s6 assumed. That is
what happened, and it retired the FAKE.

### E-s7-1 — target holds `a4 + 0x20` in TWO different callee-saved registers
Read directly off `asm/funcs/func_80041188.s`: the loop1 output pointer is `$s6`
(`addiu $s6,$s7,0x20` at 0x800411E4, used at 0x8004125C and 0x80041270) and the
loop2 output pointer is `$s3` (`addiu $s3,$s7,0x20` at 0x800412A4, used at
0x80041310 and 0x80041340). Both hold `a4 + 0x20`; `$s7` is a4 throughout.
GCC 2.7.2 has no live-range splitting, so ONE C variable is ONE pseudo is ONE
hard register. Two registers therefore mean **two distinct source-level locals**.
Every ledger session s0–s6 assumed a single `out2` that was reused or re-initialised
before loop2, and spent itself on making that single pseudo do two jobs. That
assumption was wrong, and it is the reason the s4 form needed a do-while(0) wrap
and the s5/s6 form needed a `/* FAKE */` variable-reuse annotation.

### E-s7-2 — GCC does NOT hoist `a4 + 8` out of these goto-loops (LICM is not in play)
Probe: delete every out-pointer local and write `a4 + 8` inline at all four call
sites (`rejected/inline-a4plus8-no-licm-hoist.c`). Result: sandbox 63, 132 insns,
and the expression materialises INSIDE loop1 (`addiu $s1,$s8,32`) and INSIDE loop2
(`addiu $s0,$s8,32`) — no preheader, no preamble. So neither of target's two
`addiu ...,$s7,0x20` insns is loop-invariant-motion output; both are statements the
original programmer wrote. This kills, with a measurement, any hypothesis that
loop.c placed the between-loops pointer definition.

### E-s7-3 — the two-locals chassis is 1:1 with target and the residual is pure allocation
`rejected/two-locals-out3-from-a4-seat-permutation.c`: `out2 = a4 + 8` before loop1,
`out3 = a4 + 8` between the loops, separate `stptr` (loop1 walker) and `stptr2`
(loop2 walker), no reuse of any local for a second value, no wrap, no FAKE.
Result: **sandbox 15, 132/132 insns, and every single instruction lines up with
target 1:1 — same opcode, same offsets, same immediates, same order.** The entire
residual is a 3-cycle permutation of three callee-saved seats:
`out2 $fp/$s6`, `a4 $s6/$s7`, `a3 $s7/$fp` (ours/target). Nothing about the C shape
is wrong any more; only allocno priority is.

### E-s7-4 — the governing priority table, measured end-to-end from .lreg + .greg
`pri = floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000`, and the
`;; Register dispositions:` line of the .greg dump gives the seat directly (no
inference needed — use it, it is one grep). For the two-locals chassis the four
contested pseudos are tbl, out2, a4, a3 and target requires the strict order
`tbl > out2 > a4 > a3` so that ascending `find_free_reg` hands out `$s5, $s6, $s7, $fp`.

| pseudo | refs/live (out3-from-a4) | pri | seat | refs/live (out3-from-out2) | pri | seat |
|---|---|---|---|---|---|---|
| tbl  | 4/47 | 1702.1 | $s5 | 4/47 | 1702.1 | $s5 |
| out2 | 3/42 |  714.3 | $fp | **4/47** | **1702.1** | **$s6** |
| a4   | 7/95 | 1473.7 | $s6 | 6/95 | 1263.2 | $s7 |
| a3   | 4/99 |  808.1 | $s7 | 4/99 |  808.1 | $fp |
| out3 | 3/48 |  625.0 | $s3 | 3/47 |  638.3 | $s3 |

out2 and tbl end up in an **exact numerical tie at 1702.1**, broken by allocno
number (79 tbl < 86 out2), which is why tbl keeps `$s5` and out2 takes `$s6`.
This independently confirms the s5/s6 banked window `pri(out2) in (1473.7, 1702.1)`
— the answer sits on the closed upper endpoint of it.

### E-s7-5 — out2's 4th reference AND its live length both come from one statement
`out3 = out2;` placed as the LAST statement of the between-loops block gives out2
its 4th flow-counted reference and extends its live range through that block from
42 to 47. Position inside the block is load-bearing, because the live range grows
one insn at a time: sweep of the 5 possible slots for that statement (M0..M4) gave
scores 12, 12, 11, 10, **1**. Last slot only.

### E-s7-6 — arithmetic proof that out2 MUST have >= 4 references
a4 carries at least 6 materialised references (2 calls per loop, plus each of the
two out-pointer definitions) and a live length of ~95, so its pri floor is 1263.2.
a3 is a parameter with 4 references and live 99 = 808.1, and neither number can be
reduced (its 3 uses are 3 call arguments). out2 with 3 references and live <= 47
tops out at 638–714. No 3-reference spelling of out2 can outrank a4. A 4th
reference is therefore NECESSARY, not merely one option among several.

### E-s7-7 — where a 4th reference can and cannot live (cse1 ebb boundary)
Block 0 is a cse1 extended basic block in which `out2 = a4 + 0x20` is a known value,
so ANY out2-derived expression written in block 0 is folded back to a4 by cse1
before flow runs and never reaches `reg_n_refs` (this is why all four split-init
spellings — `out2 = a4; out2 += 8;`, `out2 = a4 + 4; out2 += 4;`, the same on out3,
and out2-from-`a4` instead of from `pa4` — scored an unchanged 15). loop1 begins a
NEW ebb (2 predecessors), so out2 is only a live-in value there and cse1 can fold
nothing: an out2-derived expression written in loop1 or later is real and
materialises an instruction (`rejected/out2-minus-8-loop1-ref-not-byte-free.c`,
`addiu $a1,$s5,-32` where target has `addu $a1,$s7,$zero`). The two behaviours are
complementary and leave no room for a "free" out2 expression anywhere.

### E-s7-8 — block-0 statement order is a dead lever
A 7-permutation sweep of the block-0 statement order (P0..P6, crossed with the
plain and `out2 - 8` spellings; `rejected/block0-statement-order-live-length-dead.c`)
produced exactly three score classes — 15, 10 and 18 — with byte-identical `.greg`
dispositions inside each class. Moving out2's definition to the very first statement
of the body raised its measured live length from 42 to only 43. Source position
inside block 0 is worth about one insn of liveness, not the ~8 a naive
source-order model predicts. Do not spend another session on declaration ordering.

### E-s7-9 — the `pa4 = a4` param-local alias is load-bearing
Dropping it and using `a4` directly everywhere, with the rest of candidate.c
unchanged, scores 13 instead of 1 (`rejected/no-pa4-param-alias.c`). It is ordinary
C — a pointer local initialised from a pointer parameter and read for its real
value at six sites — but a reviewer should be told up front that it is not
cosmetic.

### s7 net
Floor unchanged at 1, but the form that achieves it is now **completely ordinary C**:
no `/* FAKE */`, no variable reuse, no do-while(0) wrap, no register pin, no
volatile, no dead local. The single residual instruction is
`move $s3,$s6` where target has `addiu $s3,$s7,0x20` (slot 72), and the problem is
now stated exactly: find a byte-free 4th reference to out2 in the between-loops
block so that `out3` can be defined from a4 instead of from out2.

- [s7] Target holds a4+0x20 in TWO different callee-saved registers - $s6 across loop1 (defined `addiu $s6,$s7,0x20` at 0x800411E4) and $s3 across loop2 (defined `addiu $s3,$s7,0x20` at 0x800412A4). GCC 2.7.2 has no live-range splitting, so this proves the original source has two distinct out-pointer locals, not one reused variable. Every ledger session s0-s6 assumed one.

- [s7] GCC does not hoist `a4 + 8` out of these goto-loops at all: with the locals removed and the expression inline, it materialises inside both loop bodies (sandbox 63). Neither of target's `addiu ...,$s7,0x20` insns is LICM output; both are source-level statements.

- [s7] The two-locals chassis with `out3` defined from a4 is 1:1 with target across all 132 instructions - same opcodes, offsets, immediates and order - and its entire residual is a 3-cycle permutation of three callee-saved seats (ours/target: out2 $fp/$s6, a4 $s6/$s7, a3 $s7/$fp), sandbox 15.

- [s7] Measured priority table for candidate.c (.lreg refs/live -> pri -> .greg seat): tbl 4/47 = 1702.1 -> $s5; out2 4/47 = 1702.1 -> $s6; a4 6/95 = 1263.2 -> $s7; a3 4/99 = 808.1 -> $fp; out3 3/47 = 638.3 -> $s3; stptr 7/41 = 3414.6 -> $s3; stptr2 6/48 = 2553.2 -> $s0; i 8/97 = 2474.2 -> $s4; a1/a2 16/99 = 6464.6 -> $s1/$s2. tbl and out2 tie exactly at 1702.1 and the tie breaks on allocno number.

- [s7] The .greg dump's `;; Register dispositions:` line gives the pseudo-to-hard-register map directly (one grep, no inference) - use it instead of deducing seats from the disassembly.

- [s7] out2's 4th flow-counted reference AND its live-length extension from 42 to 47 both come from the single statement `out3 = out2;`, and only when it is the LAST statement of the between-loops block: the 5-slot position sweep scores 12, 12, 11, 10, 1.

- [s7] cse1 ebb boundary rule for this function: block 0 is an ebb where out2's value is known, so any out2-derived expression written there is folded back to a4 pre-flow and never counted; loop1 and everything after it are in a different ebb where out2 is only a live-in value, so any out2-derived expression there is real and costs an instruction. The two behaviours are complementary and leave no free out2 expression anywhere yet found.

- [s7] A 4th reference on out2 is provably necessary: a4 cannot drop below 6 materialised references at live ~95 (priority floor 1263.2) and a3 is fixed at 4 references / live 99 = 808.1, while out2 with 3 references and live <= 47 tops out at 714.3.

- [s7] Block-0 statement ordering is a dead lever: 7 permutations x 2 spellings collapse to three score classes with identical .greg dispositions, and moving out2's definition to the first statement of the body bought +1 insn of live length (42 -> 43).

- [s7] The `s32 *pa4 = a4;` param-local alias in candidate.c is load-bearing, not cosmetic: dropping it and using `a4` directly scores 13 instead of 1 with everything else identical.

- [s7] candidate.c is now ordinary C at floor 1 - the /* FAKE */ variable-reuse annotation that the s3-s6 chassis required has been retired, which also removes the layer-1 exposure that FAILed the s4 form.

## s8 evidence (rederive, 2026-08-23)

- **E-s8-0 (chassis).** `memory/grind/func_80041188/candidate.c` applied to
  `src/text1a_pre.c` re-measures at `sandbox func_80041188 --disable all` = 1,
  132 target insns / 132 build insns, at the start AND at the end of the session
  (src is left holding candidate.c). The s7 floor of 1 is real on today's
  chassis and no ledger number needed re-basing.

- **E-s8-1 (methodology, reusable).** A single-function extraction TU
  (`tmp/grind/func_80041188/s8/fd.c`: the five externs plus the function body,
  built by `tmp/grind/func_80041188/s8/mkfd.py`) reproduces the full-TU `.lreg`
  allocation table PSEUDO-FOR-PSEUDO - identical register numbers, refs and live
  lengths. That makes a variant cost one cc1 run (~1s via
  `tmp/grind/func_80041188/s8/one.sh`) instead of a sandbox build, and it is the
  reason s8 could sweep the priority arithmetic directly. `one.sh` also accepts a
  pseudo number and sets `BB2_FLOW_DEBUG`, but that hook fires on every
  life_analysis iteration, not just the final one, so its per-block counts are
  NOT comparable to the `.lreg` live length - do not use them as absolute values.

- **E-s8-2 (flow.c, dead sets are free of charge in BOTH directions).**
  `reg_n_refs[regno] += loop_depth` sits at `tools/gcc-2.7.2/flow.c:2081`, inside
  the branch taken only when the SET is needed; `reg_live_length[regno]++` is on
  the next lines with the comment that it exists to count the setting insn. A
  dead store is deleted by `propagate_block` and contributes NOTHING. Measured:
  `out3 = out2; out3 = pa4+0x20;` and `dead = out2; out3 = pa4+0x20;` both leave
  out2 at 3 refs / live 42, byte-identical to the plain form.

- **E-s8-3 (global.c, the exact tie rule).** `allocno_compare`
  (`tools/gcc-2.7.2/global.c:635-656`) computes
  `pri = (double)(floor_log2(n_refs) * n_refs) / live_length * 10000 * size`,
  TRUNCATES it to int, and on equality falls back to `*v1 - *v2` - the LOWER
  allocno number is ordered first. `allocno_size` is forced to 1 at
  `global.c:565` for every allocno here, so the size factor is inert. This is the
  formula the whole ledger has been using; s8 confirms it verbatim including the
  int truncation and the ascending-allocno tie-break.

- **E-s8-4 (tbl's live length is position-invariant).** Moving
  `tbl = D_80094CFC;` from the declaration to the LAST statement of block 0
  leaves the ENTIRE `.lreg` table bit-identical (tbl 4 refs / live 47) and the
  sandbox score at 1. Combined with s6's 16-position `tbl++` sweep (also
  bit-identical) and s6's finding that only the i/tbl relative DEF ORDER moves it
  by +/-1, tbl's reg_live_length is not reachable by statement position at all.

- **E-s8-5 (cse1 last-set propagation is what kills split-inits).** The same
  two-statement restore of out2 scores completely differently depending on
  whether one statement intervenes:
    adjacent  (`out2 = pa4+0x20; out3 = out2;`)        -> out2 3 refs / live 42
    separated (`out2 = pa4+0x20; stptr2 = ...; out3 = out2;`) -> out2 5 refs / 42
  Adjacent, cse1 substitutes the SET_SRC directly into the copy, the restore goes
  dead and flow deletes it uncounted. Separated, the cse table lookup returns the
  register, both insns reach flow and are counted, and combine merges them
  afterwards. This is the mechanism behind H-s7-3's "block-0 split-init is folded
  pre-flow" result and it says that result was about ADJACENCY, not about block 0.

- **E-s8-6 (combine-deleted references ARE counted - positive control).**
  `tmp = (s32 *)((u8 *)out2 + 0x10); out3 = (s32 *)((u8 *)tmp - 0x10);` in the
  between-loops block restores out2 to 4 refs / live 47 - the exact baseline
  table - and still emits 132 insns at sandbox 1, i.e. combine folded the pair
  back to a single insn. flow counts, combine deletes, the allocator sees the
  higher number. The premise of s7 frontier #1 is therefore correct.

- **E-s8-7 (but the between-loops block admits no byte-free out2 reference).**
  combine's candidate pairs come from LOG_LINKS, which `propagate_block` builds
  only within a basic block, and it deletes an insn by substituting its value
  into the surviving consumer. Target's between-loops block is exactly six insns
  (`asm/funcs/func_80041188.s:70-75`): `addiu $s1,$s1,0x6C`,
  `addiu $s2,$s2,0x6C`, `addiu $s4,$zero,0x12`, `lw $t0,0x18($sp)`,
  `addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`. None of the six contains $s6 or a
  value derived from out2, and the only one that could consume out2 is out3's
  definition - which then emits `move $s3,$s6`, the residual itself. s7 frontier
  #1 is closed by structure, not by search.

- **E-s8-8 (the I2 result - target's slot-72 form, produced for the first time).**
  With the separated restore, cc1's asm for the between-loops block contains
  `addu $19,$23,32` = `addiu $s3,$s7,0x20`, character-for-character target's
  slot 72. 132 insns, sandbox 9. The nine residual diffs are the $s5/$s6 swap
  between out2 and tbl (out2's block-0 definition emits `addu $21,$23,32`
  instead of `addu $22,$23,32`), because out2 now measures 5 refs / live 42 =
  2380.9 against tbl's 4/47 = 1702.1. This relocates the entire remaining problem
  from "find a byte-free out2 reference" (dead) to "raise pri(tbl) into
  [2380.9, 2474.2]".

- **E-s8-9 (real C loops remain dead in the two-locals chassis).** loop1 as
  `do { } while (i < 0x12)` keeps 132 insns and, with `out3 = out2`, produces the
  correct priority ORDER for the first time in the ledger - but scores 5, because
  loop.c now runs on the note-marked loop and rewrites the induction variables
  (loop1's walker becomes a NEW pseudo 125 at 9 refs / live 40, and five in-loop
  instructions change). With `out3 = pa4+0x20` it scores 13 and the order is
  wrong, since that spelling gives pa4 a 9th reference (2872) and no 5-reference
  out2 at live >= 41 can outrank it. s4's measurement that a note-marked loop2
  costs +3 insns still stands and was not re-run.

- [s8] Chassis re-verified twice this session: candidate.c in src/text1a_pre.c gives sandbox --disable all = 1 at 132/132 insns, at the start and again at the end (src is left holding candidate.c).

- [s8] A single-function extraction TU (tmp/grind/func_80041188/s8/fd.c, built by mkfd.py) reproduces the full-TU .lreg allocation table pseudo-for-pseudo - same register numbers, refs and live lengths - so a priority-table variant costs one cc1 run instead of a sandbox build. tmp/grind/func_80041188/s8/one.sh is the harness.

- [s8] global.c:635-656 allocno_compare computes pri = (double)(floor_log2(n_refs)*n_refs)/live_length*10000*size, truncates to int, and on equality orders the LOWER allocno first; allocno_size is forced to 1 for every allocno here at global.c:565, so the size factor is inert. The ledger's formula is confirmed verbatim.

- [s8] flow.c:2081 increments reg_n_refs only on the branch where a SET is needed, so a dead store is deleted by propagate_block without ever being counted - dead stores cannot buy references in this function.

- [s8] toplev.c pass order confirmed: flow_analysis (2983) -> combine_instructions (3004) -> sched1 (3033) -> regclass+local_alloc (3049) -> global_alloc (3077) -> sched2 -> cross-jumping jump_optimize (3142) -> dbr_schedule. Only combine sits between the reference count and the allocator, which is why combine is the only free-reference mechanism.

- [s8] A combine-deleted reference IS counted: `tmp = out2 + 0x10; out3 = tmp - 0x10;` restores out2 to the baseline 4 refs / live 47 at 132 insns / sandbox 1.

- [s8] But no byte-free out2 reference exists in the between-loops block: target emits exactly six insns there (asm/funcs/func_80041188.s:70-75) and none reads $s6 or an out2-derived value, while combine's LOG_LINKS are intra-block and it deletes an insn only by substituting the value into the survivor.

- [s8] cse1's last-set propagation, not block membership, is what folds a two-step definition: the SAME restore of out2 gives 3 refs when adjacent to its consumer and 5 refs when one statement intervenes.

- [s8] The I2 chassis (separated restore) measures 88 stptr 7/41=3414, 91 stptr2 6/48=2500, 78 i 8/97=2474, 86 out2 5/42=2380, 79 tbl 4/47=1702, 77 pa4 7/95=1473, 75 a3 4/99=808, 87 out3 3/47=638, 85 saved 2/47=425 - every seat is target's except tbl, which must land in [2380.9, 2474.2].

- [s8] Closed-form enumeration for tbl in that window: 4 refs needs live 32-33 (impossible, loop1's block is 41 insns), 5 refs needs live 41, 6 refs needs live 49 or 50, 7 refs needs live 57-58. The banked s4 sym-K chain already supplies 6 refs byte-free but pins live at 45-47 (2553-2666), above i.

- [s8] Because i is live across the whole function (97) and tbl across 47, adding N insns to block 0 after tbl's definition gives i = 24/(97+N) and tbl = 12/(47+N); tbl <= i requires N >= 3, and at N = 3 both are exactly 2400.0 with the tie broken by allocno 78 < 79 - i keeps $s4 and tbl takes $s5, in the i-FIRST preamble order that target emits (so it avoids the emission-order residual that rejected s6's all-target-seats form).

- [s8] Real C loops stay dead in the two-locals chassis: loop1-real scores 5 (loop.c rewrites the induction variables, new pseudo 125 at 9 refs / live 40) and loop1-real with out3 from pa4 scores 13.

- [s8] a4's two 0x20-byte objects are PsyQ MATRIX-sized and func_8004A348(SVECTOR-ish buf, m) / func_800523E0(m0, m1, ...) read as matrix construction - a `MATRIX *` typed spelling of pa4/out2/out3 has never been tried and is the most likely original form.

## s9 evidence (structural, 2026-08-23)

- **E-s9-0 (chassis).** `memory/grind/func_80041188/candidate.c` re-measures at
  `sandbox func_80041188 --disable all` = 1, 132 target / 132 build insns, both at
  the start and at the end of the session; src/text1a_pre.c is left holding it.
  The s8 harness (`tmp/grind/func_80041188/s8/mkfd.py` + `one.sh`) still reproduces
  the full-TU `.lreg` table pseudo-for-pseudo, and s9 added a mechanical seat
  checker on top of it (`tmp/grind/func_80041188/s9/run.sh`, which parses
  `;; Register dispositions:` out of `.greg` and prints `SEATS ALL-TARGET` or the
  list of wrong seats). candidate.c prints `SEATS ALL-TARGET`, confirming its
  single residual insn is a FORM difference, not an allocation difference.

- **E-s9-1 (the all-target-seats form on the two-locals chassis).** I2 (s8's
  separated between-loops restore, the only form that emits target's slot-72
  `addiu $s3,$s7,0x20`) + the s4/s5 tbl sym-K ref lift (tbl 4 -> 6 refs) + `i = 1`
  moved to the LAST statement of block 0 gives, for the first time on the s7
  two-locals chassis, EVERY callee-saved seat equal to target:
  stptr 7/41 = 3414 -> $s3, stptr2 6/48 = 2500 -> $s0, tbl 6/48 = 2500 -> $s5,
  i 8/96 = 2500 -> $s4 (tie with tbl broken by allocno 78 < 79), out2 5/42 = 2380
  -> $s6, pa4 7/95 = 1473 -> $s7, a3 4/99 = 808 -> $fp, out3 3/47 = 638 -> $s3,
  saved 2/47 = 425 spilled. sandbox --disable all = 4, 132/132 insns.
  (`rejected/m1-i2-symk-ilate-alltarget-seats-preamble-order4.c`.)

- **E-s9-2 (that form's whole residual is the preamble emission ORDER).**
  Disassembly of the sandbox object: ours emits
  `sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO / sw s4,48(sp) / li s4,1`;
  target emits `sw s4,48(sp) / li s4,1 / sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO`
  (`asm/funcs/func_80041188.s:9-13`). Everything else in 132 insns is identical.
  The register-save `sw`s track their register's first definition, so the whole
  4-slot residual is one binary fact: which of i's def and tbl's def is emitted
  first.

- **E-s9-3 (named mechanism for the s7 anti-correlation law).** Read out of the
  compiler, not inferred. `flow.c:1685` increments `reg_live_length` once per insn
  for every pseudo in the live set, walking the RTL insn order, and flow runs at
  `toplev.c:2983`, before sched1 at 3033. `sched.c rank_for_schedule` compares
  `INSN_PRIORITY` first, then a 3-way dependence class relative to
  `last_scheduled_insn`, and finally returns `INSN_LUID (y) - INSN_LUID (x)` -
  i.e. for two insns that tie on both, the ready list is ordered by descending
  LUID, and because GCC 2.7.2 schedules each block BACKWARD the insn popped first
  is placed LAST. The `.sched` dump for this function shows both defs at
  `priority = 1, ref_count = 0` (i's def is a `li`, tbl's a symbol load; neither
  has an in-block consumer), so the LUID fallback decides, and emission order ==
  RTL order == the order flow used to compute live lengths. Live length and
  emission order are therefore the SAME variable for this pair; s7's measured
  "whichever is defined first gets live+1 and the other live-1" law now has its
  mechanism.

- **E-s9-4 (new lever: `i = 0x12`'s position inside the between-loops block).**
  i's live range has a HOLE between loop1's exit test and its re-definition, and
  moving `i = 0x12;` later inside the 6-statement between-loops block grows the
  hole and shortens `reg_live_length[i]`: 3rd statement (the committed position)
  = 97 (2474), 4th = 96 (2500), 5th = 95 (2526), 6th/last = 95 (2526).
  95 is the floor. This lever was not in the ledger before s9.
  (`rejected/i12-last-in-between-block-ilive95-floor.c`.)

- **E-s9-5 (the hole cannot be grown from its left edge).** Insns added to the
  between-loops block BEFORE `i = 0x12` are live-NEUTRAL for i: a separated
  self-update of stptr2 (`stptr2 = saved; out3 = out2; stptr2 += 0x750;`) is
  demonstrably a real added insn - stptr2 goes 6 -> 8 refs and a1/a2's live length
  goes 99 -> 100 - yet i stays at 8/95 and tbl at 6/47. flow.c:1685 only counts
  insns in which the pseudo is LIVE, and i is dead there.
  (`rejected/between-block-hole-insns-are-live-neutral-for-i.c`.)

- **E-s9-6 (tbl's reg_live_length has a hard ceiling of 47).** Nine distinct
  block-0 mutations measured this session - the sym-K advance in five different
  slots, the stptr split-init in three placements, the saved split-init, the
  base two-step, a fresh-intermediate copy, and three placements of `i = 1`
  within block 0 - all leave tbl at 45 or 47 at refs 6, never 48. 47 = loop1's
  41 insns plus a fixed 6-insn block-0 tail. Combined with s8's tbl-assignment
  sweep and s6's 16-position `tbl++` sweep, the ONLY mutation in the entire
  ledger that has ever produced tbl live 48 is moving `i = 1` past tbl's
  definition. (`rejected/tbl-live-ceiling-47-advance-position-sweep.c`,
  `rejected/i-def-position-inside-block0-is-inert.c`.)

- **E-s9-7 (closed form: the i-FIRST order is arithmetically shut for a 6-ref
  tbl).** In the i-first definition order - the one that yields target's preamble
  emission - tbl is pinned at 6 refs / live 47 = 2553 and i's live floor is 95 =
  2526. tbl outranks i by 27 points and steals $s4. Adding insns inside tbl's live
  range raises i's live by the same amount (both are live there), so a single
  SURVIVING insn placed after tbl's definition is exactly what is needed and
  sufficient: it makes tbl 6/48 = 2500 and i 8/96 = 2500, an exact tie that
  breaks our way on allocno 78 < 79.

- **E-s9-8 (the one surviving-insn spelling found, and why it fails).** `saved =
  base; out2 = ...; saved += 0x94;` - a separated SELF-update of the `saved`
  pseudo - is the only block-0 mutation that both survives cse1 into flow and
  raises tbl to live 48 while keeping the i-FIRST order. It produces the winning
  arithmetic outright (tbl 6/48 = 2500, i 8/96 = 2500, tie to allocno 78 < 79,
  stptr still top at 3414, out2 down at 2272). It fails because it lifts `saved`
  from 2 refs / pri 425 to 5 refs / pri 2083 - a TENTH claimant on nine
  callee-saved seats - so global_alloc seats `saved` and SPILLS a3, where target
  seats a3 in $fp and spills `saved` (`sw $t0,0x18($sp)` / `lw $t0,0x18($sp)`).
  Corollary that constrains every future probe: the donor of the surviving insn
  must be a pseudo that ALREADY holds a callee-saved seat in target.
  (`rejected/saved-selfupdate-split-raises-tbl-to-48-but-spills-a3.c`.)

- **E-s9-9 (cse1 survival rule, refined and now three-way).** A two-step
  definition reaches flow (and is counted) only when the second step is a
  SELF-update of the SAME pseudo with a register-valued first step and at least
  one intervening statement. The two other shapes both die before flow:
  a fresh-intermediate copy (`b2 = base; ...; saved = b2 + 0x94;`) is
  copy-propagated and deleted, leaving the .lreg table bit-identical
  (`rejected/fresh-intermediate-copy-is-propagated-away-not-counted.c`); and a
  CONSTANT two-step (`i = 0x10; ...; i += 2;`, `stptr2 = saved + 0x740; ...;
  stptr2 += 0x10;`) is constant-folded, also bit-identical
  (`rejected/i-constant-splitinit-folded-no-9th-ref.c`). This supersedes the
  looser s8 statement that "separation" alone is what makes a split survive.

- **E-s9-10 (i cannot get a ninth reference from a constant).** A ninth
  flow-counted reference would put i at 27/95 = 2842, comfortably above tbl's
  2553 ceiling, and would close the i-FIRST order outright without touching any
  other pseudo. The constant split-init route is dead (E-s9-9); what is still
  untried is a register-valued self-update of i.

- [s9] The s9 seat checker (tmp/grind/func_80041188/s9/run.sh) prints SEATS ALL-TARGET or the exact wrong seats straight from .greg; candidate.c is already ALL-TARGET, so its floor-1 residual is a form difference, not an allocation one.
- [s9] I2 + tbl sym-K + `i = 1` last in block 0 is the first ALL-TARGET-SEATS form on the two-locals chassis: sandbox 4, 132/132, residual is only the 5-insn preamble emission order (ours puts tbl's lui/addiu pair and its sw before i's li and its sw; target the reverse).
- [s9] Mechanism for s7's anti-correlation law, read out of the compiler: flow.c:1685 counts live length over RTL order (flow at toplev.c:2983, before sched1 at 3033) and sched.c rank_for_schedule falls back to INSN_LUID(y)-INSN_LUID(x) for the two priority-1/ref_count-0 leaf defs, with backward block scheduling. Emission order and live length are the same variable for this pair.
- [s9] NEW LEVER: moving `i = 0x12;` later inside the between-loops block grows the dead hole in i's live range - position 3rd/4th/5th/6th gives i live 97/96/95/95. 95 is the floor.
- [s9] Insns added to the between-loops block before `i = 0x12` are live-neutral for i (they sit in the hole where i is dead), even when they demonstrably survive to flow.
- [s9] tbl's reg_live_length has a hard ceiling of 47 at 6 refs: nine distinct block-0 mutations this session all give 45 or 47, never 48. Only moving `i = 1` past tbl's definition has ever produced 48.
- [s9] The i-first (target emission) order needs exactly ONE surviving block-0 insn after tbl's definition: that makes tbl 6/48 = 2500 and i 8/96 = 2500, an exact tie won by allocno 78 < 79.
- [s9] The only such surviving spelling found (a separated self-update of `saved`) produces exactly that arithmetic but lifts `saved` from pri 425 to 2083, making it a tenth claimant on nine callee-saved seats: global_alloc seats saved and spills a3. Target spills saved, so the donor must be a pseudo that already holds a seat.
- [s9] cse1 survival rule refined to three cases: SELF-update of the same pseudo with a register-valued first step and an intervening statement SURVIVES; a fresh-intermediate copy is copy-propagated away; a constant two-step is constant-folded. Only the first is counted by flow.

- [s9] Chassis re-verified twice: candidate.c in src/text1a_pre.c gives sandbox --disable all = 1 at 132/132 insns at the start and again at the end of the session; src is left holding candidate.c.

- [s9] New tooling: tmp/grind/func_80041188/s9/run.sh extends the s8 extraction-TU harness with a mechanical seat checker that reads ';; Register dispositions:' out of .greg and prints SEATS ALL-TARGET or the exact wrong seats, in the same ~1s run that prints the allocno priority table.

- [s9] candidate.c already holds ALL-TARGET callee-saved seats, so its single residual insn (move $s3,$s6 vs addiu $s3,$s7,0x20) is a FORM difference, not an allocation difference - a fact no prior session had established.

- [s9] I2 (the s8 separated between-loops restore) + the s4/s5 tbl sym-K ref lift + `i = 1` moved to the last statement of block 0 is the first ALL-TARGET-SEATS form on the s7 two-locals chassis: stptr 7/41=3414 -> $s3, stptr2 6/48=2500 -> $s0, tbl 6/48=2500 -> $s5, i 8/96=2500 -> $s4 (tie won by allocno 78 < 79), out2 5/42=2380 -> $s6, pa4 7/95=1473 -> $s7, a3 4/99=808 -> $fp, out3 3/47=638 -> $s3, saved 2/47=425 spilled. sandbox 4, 132/132.

- [s9] That form's entire residual is the preamble emission order: ours emits sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO / sw s4,48(sp) / li s4,1, target emits sw s4,48(sp) / li s4,1 / sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO (asm/funcs/func_80041188.s:9-13). The register-save sw insns track their register's first definition, so the whole 4-slot residual is one binary fact.

- [s9] Mechanism for s7's anti-correlation law, read out of the compiler rather than inferred: flow.c:1685 increments reg_live_length once per insn for every live pseudo walking RTL order, flow runs at toplev.c:2983 before sched1 at 3033, and sched.c rank_for_schedule falls back to INSN_LUID(y)-INSN_LUID(x) for the two priority-1 / ref_count-0 leaf defs. With backward block scheduling this makes emission order == RTL order == the order flow measured. Live length and emission order are the same variable for this pair.

- [s9] NEW LEVER: moving `i = 0x12;` later inside the between-loops block grows the dead hole in i's live range - 3rd/4th/5th/6th statement gives i live 97/96/95/95, i.e. pri 2474/2500/2526/2526. 95 is the hard floor.

- [s9] Insns added to the between-loops block BEFORE `i = 0x12` are live-neutral for i even when they demonstrably survive to flow (stptr2 self-update: stptr2 6 -> 8 refs, a1/a2 live 99 -> 100, i unchanged at 8/95), because they sit where i is already dead.

- [s9] tbl's reg_live_length has a hard ceiling of 47 at 6 refs: nine distinct block-0 mutations this session (sym-K advance in five slots, stptr split in three placements, saved split, base two-step, fresh-intermediate copy, three `i = 1` placements) all give 45 or 47, never 48. 47 = loop1's 41 insns + a fixed 6-insn block-0 tail.

- [s9] The i-FIRST (target emission) order needs exactly ONE surviving block-0 insn placed after tbl's definition: that makes tbl 6/48 = 2500 and i 8/96 = 2500, an exact tie won by allocno 78 < 79, with out2 down at 2272 and stptr still top at 3414.

- [s9] The only surviving-insn spelling found (a separated self-update of `saved`: `saved = base; out2 = ...; saved += 0x94;`) produces exactly that arithmetic but lifts saved from pri 425 to 2083, making it a tenth claimant on nine callee-saved seats; global_alloc seats saved and spills a3. Target spills saved (sw $t0,0x18($sp) / lw $t0,0x18($sp)), so the donor of the surviving insn must be a pseudo that ALREADY holds a callee-saved seat.

- [s9] cse1 survival rule refined to three cases, superseding the looser s8 statement that separation alone suffices: a SELF-update of the same pseudo with a register-valued first step and an intervening statement SURVIVES into flow and is counted; a fresh-intermediate copy is copy-propagated away; a constant two-step is constant-folded. Only the first is counted at flow.c:2081.

## s10 evidence (structural, 2026-08-23)

- **E-s10-0 (chassis).** `memory/grind/func_80041188/candidate.c` re-measures at
  `sandbox func_80041188 --disable all` = 1, 132 target / 132 build insns, at the start
  and again at the end of the session; src/text1a_pre.c is left holding it. The s8/s9
  harness (`tmp/grind/func_80041188/s8/mkfd.py` + `one.sh` + the s9 seat checker) is
  unchanged; s10 added `tmp/grind/func_80041188/s10/batch.sh`, which runs the priority
  table + seat verdict for a whole directory of variants in one WSL call (~1 s each),
  and `tmp/grind/func_80041188/s10/gen.py`, which enumerates the between-loops block.

- **E-s10-1 (112-variant grid over the between-loops block).** Generated every
  interleaving of `i = 0x12` (I) and the s8 separated out2 restore (O) with the three
  fixed statements `a1 += 0x6C` (A), `a2 += 0x6C` (B), `stptr2 = saved + 0x750` (P),
  crossed with sym-K on/off (K1/K0) and `i = 1` first/last in block 0 (L0/L1);
  `out3 = out2;` always last. 112 variants, all measured. Exactly two families reach
  SEATS ALL-TARGET: **K0_L0 without O** (that is candidate.c and its three
  between-block permutations - floor 1, residual `move $s3,$s6`), and **K1_L1 with or
  without O** (the s9 M1 family - i defined last, so the preamble emits tbl before i,
  sandbox 4). **No K1_L0 (i-first) variant with O reaches all-target seats**: all 24
  of them differ from target in exactly one way, i and tbl swapping $s4/$s5.
  (`tmp/grind/func_80041188/s10/g/`.)

- **E-s10-2 (closed form: why i-first + O is shut for an 8-ref `i`).** In the i-FIRST
  order every insn of block 0 that lies after tbl's definition is also inside i's live
  range, and i additionally covers its own definition, the between-block tail and the
  whole of loop2. Measured across the entire grid the difference is a constant:
  `reg_live_length(i) = reg_live_length(tbl) + 49`, in every one of the 24 K1_L0
  variants (tbl 47 / i 96 with `i = 0x12` last; 97, 98, 99 as `i = 0x12` moves earlier).
  With tbl at 6 refs (floor_log2(6)*6 = 12) and i at 8 (floor_log2(8)*8 = 24) the
  priority comparison is `24/(L+49) > 12/L`, i.e. **tbl's live length must exceed 49**.
  With tbl at 5 refs (10/47 = 2127) or 4 refs (8/47 = 1702) tbl instead falls BELOW
  out2, which in the O-chassis is pinned at 5 refs / live 42-44 = 2272-2380 and cannot
  be lowered (4 refs needs live > 58, and out2 dies at loop1's exit). So on the
  O-chassis tbl must have exactly 6 refs, and then i needs tbl live >= 50.

- **E-s10-3 (tbl's live ceiling is 48, not 50 - the escape from E-s10-2 is shut).**
  The single surviving block-0 insn of E-s9-8 lifts tbl 47 -> 48 and no further:
  a two-step separated self-update of `saved` gives tbl 6/48, and a THREE-step one
  (`saved = base; out2 = ...; saved += 0x50; saved += 0x44;`) also gives tbl 6/48 while
  taking `saved` to 7 refs / 2916 - so the extra steps demonstrably survive cse1 and
  still do not lengthen tbl. `stptr`'s three-step and four-step splits raise stptr to
  9 and 10 refs with tbl fixed at 47 in both placements. Combined with s9's nine
  block-0 mutations, sixteen distinct mutations now bound tbl at <= 48.
  **Therefore the i-first + O chassis is arithmetically CLOSED for an 8-reference `i`.**
  (`rejected/saved-three-step-does-not-lift-tbl-live-past-48.c`,
  `rejected/stptr-three-step-refs-rise-tbl-live-inert.c`.)

- **E-s10-4 (the 9th-reference axis PAYS, and the payoff is exactly one insn).**
  K1_L0_ABOPI (sym-K tbl, `i = 1` first, the s8 separated out2 restore, `i = 0x12` last)
  plus ONE extra flow-counted reference to `i` - spelled `stptr2 = saved + 0x750 +
  (i - 0x12);` placed BEFORE the `i = 0x12;` reset - gives i 9 refs / live 99 = 2727
  against tbl 6/47 = 2553 and prints **SEATS ALL-TARGET**. This is the first form in
  the whole ledger in which the i-FIRST definition order (which produces target's
  preamble emission order, E-s9-2/E-s9-3) and the O-chassis (which produces target's
  slot-72 `addiu $s3,$s7,0x20`, E-s8) co-exist with every callee-saved seat equal to
  target. `sandbox --disable all` = **3**, build 133 insns vs target 132: the entire
  residual is the single extra insn that computes the `(i - 0x12)` zero.
  A BYTE-FREE 9th flow-counted reference to `i` puts this form at distance 0.
  The measured spelling is not shippable (semantic no-op arithmetic: T1, T2 and T3 of
  the cheat checklist all fail), and it is banked as proof that the axis pays, not as
  a candidate. (`rejected/ninth-i-ref-alltarget-seats-ifirst-ochassis-score3.c`.)

- **E-s10-5 (where a 9th reference to `i` may and may not be placed).**
  `i = 0x12; stptr2 = saved + (i * 0x68);` is arithmetically exact (18 * 0x68 == 0x750)
  and register-valued, but cse1 already knows `i == 0x12` there, folds the multiply to
  the constant, and the .lreg table comes back bit-identical to the unmodified form.
  The reference must therefore sit BEFORE the `i = 0x12;` reset, reading loop1's exit
  value, which cse1 does not know is 0x12.
  (`rejected/i-times-0x68-after-reset-is-cse-folded.c`.)

- **E-s10-6 (the honest reference vector, read off target).** target's own insns fix
  `i` at 8 references (`li 1`, `addiu +1`, `slti`, `li 0x12`, `addiu +1`, `slti`),
  `tbl` at 4 (`lui/addiu` pair = one def, `lw 0($s5)`, `addiu $s5,$s5,4`), `out2` at 3
  and `out3` at 3. Both the sym-K tbl lift (4 -> 6) and any 9th reference to `i` are
  therefore INVENTIONS relative to the original source, and so is the `stptr = base;
  stptr += 0xFC;` split (target has a single `addiu $s3,$v0,0xFC`). That is a
  constraint on the endgame, not on the search: some pseudo in our chassis still
  differs from the original's, and the 9th-ref result of E-s10-4 says the difference is
  worth exactly one flow-counted reference on `i`.

- [s10] candidate.c re-measures at sandbox --disable all = 1, 132 target / 132 build insns, at the start and again at the end of s10; src/text1a_pre.c is left holding it.

- [s10] 112-variant grid over the between-loops block x sym-K x `i = 1` position: exactly two families reach SEATS ALL-TARGET - K0_L0 without the separated restore (candidate.c and its three permutations, floor 1, residual `move $s3,$s6`) and K1_L1 with or without it (the s9 M1 family, i defined last so the preamble emits tbl before i, sandbox 4). All 24 i-first variants carrying the separated restore differ from target in exactly one way: i and tbl swap $s4/$s5.

- [s10] reg_live_length(i) = reg_live_length(tbl) + 49 in every i-first variant measured, which turns the i-vs-tbl contest into `tbl live >= 50` for a 6-ref tbl against an 8-ref i.

- [s10] tbl's reg_live_length ceiling is 48, not 47: the two-step separated self-update of `saved` reaches 48, and a THREE-step one also reaches 48 while taking saved to 7 refs - so surviving insns past the first buy nothing. Sixteen distinct block-0 mutations across s9 and s10 now bound it.

- [s10] stptr is not a usable donor for the E-s9-7 lever despite already holding a callee-saved seat: its three- and four-step splits raise stptr to 9 and 10 refs (so they do survive cse1 into flow) with tbl fixed at live 47 in both placements.

- [s10] A ninth flow-counted reference to `i` gives SEATS ALL-TARGET on the i-first + separated-restore chassis at sandbox 3 / 133 insns - one insn more than target, and that insn is the reference itself. A byte-free ninth reference puts the form at distance 0.

- [s10] cse1 folds a ninth-reference spelling whose operand value it already knows (`i * 0x68` after `i = 0x12`), so the reference must be placed before the reset and read loop1's exit value.

- [s10] The honest reference vector read straight off target's insns is i 8, tbl 4, out2 3, out3 3, with a single `addiu $s3,$v0,0xFC` for stptr - so the sym-K tbl lift, the stptr split and any ninth i-reference are all inventions relative to the original source, which bounds what an acceptable endgame form may look like.

## s11 evidence (escalation / disposition, 2026-08-23)

- **E-s11-0 (chassis, driver-measured).** `src/text1a_pre.c` at HEAD still carries the
  RULE-ERA body (a `register s32 *s7_a4 asm("s7")` pin, single `out2` re-initialised
  before loop2); `sandbox func_80041188 --disable all` reports `rules_dropped: 16,
  cheat_asm_stripped: 2`. Applying `memory/grind/func_80041188/candidate.c` re-measures
  at **score 1, 132 target / 132 build insns**, and the s10 seat checker
  (`tmp/grind/func_80041188/s10/batch.sh`, run under WSL) prints **SEATS ALL-TARGET**
  with the exact s7 priority table (stptr 7/41=3414, stptr2 6/48=2500, i 8/97=2474,
  out2 4/47=1702, tbl 4/47=1702, pa4 6/95=1263, a3 4/99=808, out3 3/47=638). src is
  left holding candidate.c. NOTE for future sessions: `batch.sh` must be run through
  WSL (`bash tools/wsl.sh '... bash tmp/.../batch.sh ...'`); run from Git Bash on the
  Windows host it silently reports a STALE `fd.greg` seat verdict.

- **E-s11-1 (out2's live-length ceiling on the target-residual chassis is 43 - the
  4-reference priority window is EMPTY).** The only chassis that emits target's slot-72
  `addiu $s3,$s7,0x20` is `out3 = (s32 *)((u8 *)pa4 + 0x20);`, which kills out2 at
  loop1's exit. Moving out2's definition to the FIRST statement of block 0 maximises its
  live range: measured `r86 = 3 refs / live 43` (score 18, 132 insns,
  `rejected/out2-live-ceiling-43-out3-from-pa4-window-empty.c`). On that chassis pa4
  gains out3's reference (`r77 = 7/96 = 1458`) and tbl stays `4/47 = 1702`, so target's
  seating (tbl > out2 > pa4 > a3 -> $s5,$s6,$s7,$fp) needs out2's priority strictly
  inside **(1458, 1702)**, i.e. `floor_log2(4)*4/L*10000` with **L in (47.0, 54.9)**.
  With L capped at 43 the reachable values are 3 refs = 697 (below pa4 -> the measured
  score-18 seat permutation) and 4 refs = 1860 (above tbl -> the s10 i/tbl swap). **The
  window is arithmetically empty.** This is the block-0 counterpart of E-s8's
  between-block kill and closes the last remaining side of the two-locals chassis.

- **E-s11-2 (the F1 combine-foldable chain-extender CAN buy out2's 4th flow-counted
  reference, and it is dead in both directions anyway).** F1
  (`.claude/rules/dead-store-fake-exception.md:32`, owner ruling 2026-07-01) is the last
  sanctioned byte-free `reg_n_refs` family that had never been spent on this function.
  Probe: route the LIVE block-0 computation `saved = base + 0x94` through out2 -
  `saved = (s32)out2 + (base + 0x94 - 0x20 - (s32)pa4);` (algebraically exact, since
  out2 == pa4 + 0x20). Measured: out2 does reach **4 refs / live 46 = 1739**, so the
  counting premise holds inside block 0 as well. But (i) the detour must mention **pa4**
  to cancel - out2 is the only pa4-derived value in scope - which takes pa4 to
  **8 refs / 99 = 2424**, i.e. ABOVE out2, permuting exactly the seats it was meant to
  fix; and (ii) it **MATERIALIZES**: `sandbox` = 27 at **134 build insns vs target 132**.
  F1's own prerequisite ("verify the fold actually emits zero bytes ... a chain that
  MATERIALIZES is a real code change, not this lever") therefore fails on its face.
  Adding the s4/s5 sym-K tbl lift on top (tbl 6/49 = 2448) does not rescue it: pa4 at
  2424 still outranks out2 at 1739.
  (`rejected/block0-chain-extender-lifts-pa4-and-materializes-134.c`,
  `rejected/symk-plus-block0-detour-pa4-still-outranks-out2.c`.)
  Generalisation, and the reason no other spelling escapes: any block-0 expression that
  references out2 once and still computes its own real value must subtract a pa4-derived
  term, so it ALWAYS hands pa4 the reference it hands out2, and pa4 (6 refs before,
  7 with out3-from-pa4) crosses out2 at every count. The only pa4-free compensation is
  `out2 - out2`, the forbidden cancellation-pair / opaque-arithmetic spelling E-s8
  already named.

- **E-s11-3 (candidate.c's `stptr = base; stptr += 0xFC;` split IS itself a load-bearing,
  un-annotated F1 chain-extender - a latent layer-1 exposure).** target emits a single
  `addiu $s3,$v0,0xFC` (asm/funcs/func_80041188.s:26), and our split is byte-neutral
  (132 insns either way). Un-splitting it to `stptr = base + 0xFC;` drops stptr from
  **7 refs / 41 = 3414 to 5 / 41 = 2439**, below stptr2's 2500, which permutes
  stptr/i/out3 and takes the floor from **1 to 15**
  (`rejected/stptr-unsplit-floor15-proves-split-is-loadbearing-chain-extender.c`).
  So the split's ONLY surviving effect is the extra `reg_n_refs` count - the exact
  definition of the F1 family. candidate.c's header claim of "ORDINARY C ... NO /* FAKE */
  construct" is therefore inaccurate: any future candidate built on this chassis owes
  either a `/* FAKE: ... */` F1 annotation on that split or a replacement for it, and a
  layer-1 reviewer would otherwise FAIL it as an un-annotated chain-extender.

- [s11] Chassis re-measured this session: HEAD's committed body is the rule-era form (a register-asm pin on $s7; sandbox reports rules_dropped: 16, cheat_asm_stripped: 2). Applying memory/grind/func_80041188/candidate.c gives sandbox --disable all = 1 at 132 target / 132 build insns with SEATS ALL-TARGET (stptr 7/41=3414, stptr2 6/48=2500, i 8/97=2474, out2 4/47=1702, tbl 4/47=1702, pa4 6/95=1263, a3 4/99=808, out3 3/47=638). src/text1a_pre.c is left holding candidate.c.

- [s11] Harness gotcha for future sessions: tmp/grind/func_80041188/s10/batch.sh MUST be run through WSL (bash tools/wsl.sh with the batch.sh invocation inside). Run from Git Bash on the Windows host, cc1 cannot be spawned and the script silently reports a STALE fd.greg seat verdict - it printed a false 'i=21 tbl=20' for the known-all-target candidate before the mistake was caught.

- [s11] On the out3-from-pa4 chassis (the only one that emits target's slot-72 addiu $s3,$s7,0x20), out2's reg_live_length has a hard ceiling of 43 - measured with its definition made the first statement of block 0 - so the 4-reference priority window (live 47..55) that target's seating requires is arithmetically EMPTY. This is the block-0 mirror of s8's between-block kill and closes the two-locals chassis on both sides.

- [s11] A block-0 F1 chain-extender does buy out2 a 4th flow-counted reference (4 refs / live 46 = 1739), confirming the counting premise inside block 0, but every spelling must reference pa4 to cancel, which lifts pa4 to 8 refs / 2424 above out2 - and the detour materialises two insns (134 vs 132), independently failing F1's zero-bytes prerequisite. The only pa4-free compensation is the forbidden out2 - out2 cancellation pair.

- [s11] candidate.c's `stptr = base; stptr += 0xFC;` is itself a load-bearing, un-annotated F1 combine-foldable chain-extender: byte-neutral (132 insns either way), worth 14 points of floor (un-splitting gives 15), and target emits the single addiu. Any future candidate on this chassis owes a FAKE annotation on it or a replacement.

- [s11] GATE (a) canonical-asm FAILS: tools/scan_hand_coded.py --single func_80041188 = tier LOW, score 0/8, 'no strong hand-coded indicators' (S1 0 multu pairs, S2 none, S3 132 insns/11 spills/15 regs, S4 burst 3, S5 no siblings, S6 none, S7 all saved, S8 none); canonical verdict is C.

- [s11] GATE (b) SOTN precedent FAILS: dead stores are measured inert here (flow.c:2081 counts a SET only on the branch where it is needed); duplicated-statement-into-arms has no arms in this function and its Non-extension list excludes the CALL statements that are out2's only references; the do-while(0) wrap is Judge-BANNED for this function (layer-1 FAIL 2026-08-22 23:24) and separately measured to trigger loop.c induction rewriting; F1 is measured materializing. No PSX entry in docs/reference/sotn-construct-index.md exhibits a cancelling-term reference of this shape.

- [s11] Disposition filed this session as a standing-ruling entry in docs/grind/decisions.md (2026-08-23, func_80041188, REFUSED / OWNER-ACCEPTED INCOMPLETE) with both gates' evidence and the 11-session / 5-modality / 42-rejected-form exhaustion record.

## s12 (escalation, 2026-08-24)

Chassis re-measured at session start with `memory/grind/func_80041188/candidate.c`
applied to `src/text1a_pre.c`: **score 1, 132 target / 132 build insns**,
`rules_dropped: 16, cheat_asm_stripped: 2`. The ledger floor is confirmed against
the current chassis; nothing about the toolchain or the surrounding TU has moved.

**E-s12-1 — the `MATRIX *` chassis is the single-local chassis, not a new one.**
`MATRIX` is `{ s16 m[3][3]; u16 pad; s32 t[3]; }` (`include/gte.h:29`) = exactly
0x20 bytes, so `MATRIX *m = (MATRIX *)a4;` with `&m[0]` / `&m[1]` is a semantically
faithful spelling of the two adjacent matrices a4 points at. It is NOT a different
RTL address form: GCC's C front end folds the array index to a constant byte
offset, so `&m[1]` and `((u8 *)pa4) + 0x20` produce the identical
`(plus (reg) (const_int 32))`. Consequence, measured: cse1 collapses all four
occurrences to one value and the .lreg table drops from 11 pseudos to 8 — `out2`
and `out3` cease to exist as separate allocnos and `pa4` rises to 8 refs / live 94
(from 6 / 95) because `&m[0]` is now inline at four call sites. Build is
**125 insns against target's 132 (sandbox 43)** — the seven missing insns are the
copies/definitions the two-locals chassis needs. This re-derives s7's finding from
a different direction: target holds `a4 + 0x20` in TWO callee-saved registers
(`$s6` across loop1, `$s3` across loop2) and GCC 2.7.2 has no live-range splitting,
so the source MUST have two distinct locals. A typedef cannot change that.
Artifact: `tmp/grind/func_80041188/s12/A_matrix.c`,
banked `rejected/matrix-single-local-collapses-out2-out3-125insn.c`.

**E-s12-2 — an out2 defined inside loop1 gets 4 refs / live 21 / priority 3809,
and the in-loop definition position is inert.** Four spellings (`B0_loop1top`,
`B1_beforecall1`, `B2_aftercall1`, `B3_midlate`) on the `out3 = pa4 + 0x20`
chassis produced BYTE-IDENTICAL .lreg priority tables and .greg
`Register dispositions` lines:

    r74=16/99:6464  r73=16/99:6464  r86(out2)=4/21:3809  r88(stptr)=7/42:3333
    r91(stptr2)=6/48:2500  r78(i)=8/97:2474  r79(tbl)=4/47:1702
    r77(pa4)=6/95:1263  r75(a3)=4/99:808  r87(out3)=3/47:638  r85=2/47:425

cse1 normalises the in-loop definition to one place regardless of where the
statement is written, so position is a dead lever inside the loop exactly as s7
showed it is dead inside block 0. `B0_loop1top` scores **sandbox 23 at 132 insns**
and its seats are wholesale permuted (out2 → `$s3`, tbl → `$s6`, stptr → `$s4`).
Artifacts: `tmp/grind/func_80041188/s12/B{0,1,2,3}*.c`, banked
`rejected/out2-def-inside-loop1-live21-pri3809-overshoots-window.c`.

**E-s12-3 — THE COMPLETE out2 WINDOW ENUMERATION (closes s11's proof on the
second side).** target's seating requires `tbl > out2 > pa4 > a3`, i.e. out2's
global.c allocno priority strictly inside **(1263, 1702)** — pa4 is pinned at
6 refs / live 95 = 1263 and a3 at 4 / 99 = 808 by the call signatures (they are
call arguments in both loops; neither reference count nor live length is
source-controllable), and tbl at 4 / 47 = 1702 with its live length bounded at 48
by sixteen distinct block-0 mutations (s10). With
`pri = floor_log2(n) * n / live * 10000`, the band admits exactly these
(n, live) pairs:

| n (refs) | live length that lands in (1263, 1702) |
|---|---|
| 3 | 17.6 .. 23.7 |
| 4 | 47.0 .. 54.9 |
| 5 | 58.7 .. 79.2 |
| 6 | 70.5 .. 95.0 |

and out2's ACHIEVABLE (n, live) pairs are now measured exhaustively, because
out2's definition can sit in exactly two places and its last use in exactly two:

- **definition in block 0, no reference after loop1** (the chassis that emits
  target's `addiu $s3,$s7,0x20`): live is pinned to **42 or 43** — 42 normally,
  43 with the definition made the very first statement of the body (s7's 7-way
  order sweep, s11's ceiling measurement). n = 3 → 714 / 697 (BELOW pa4 and even
  below a3: the whole {out2, pa4, a3} triple permutes, sandbox 15). n = 4 (s7's
  `out2 - 8` in-loop reference) → **1904** (ABOVE tbl: out2 steals `$s5`,
  sandbox 10). The band's n = 4 requirement of live ≥ 47 is unreachable because
  nothing after loop1 references out2 on this chassis.
- **definition inside loop1** (this session): live **21**, n **4** → **3809**,
  more than double the top of the band (sandbox 23). Position-inert.
- **definition in block 0 WITH a reference in the between-loops block**: live
  **47**, n **4** → **1702.1**, inside the band and tied with tbl, broken the
  right way by allocno number (79 < 86). This is `candidate.c` — ALL-TARGET seats,
  132/132 insns, **and the between-loops reference is precisely the insn that
  materialises as `move $s3, $s6` instead of target's `addiu $s3, $s7, 0x20`.**
- **n = 5 or n = 6** would need live 59..95, i.e. out2 live through loop2 — which
  is the single-local chassis (E-s12-1, s7): one pseudo cannot hold `$s6` across
  loop1 and `$s3` across loop2, and the build loses seven insns.

**Conclusion.** target's out2 priority is reachable ONLY with a between-loops
reference to out2, and s8 proved no such reference can be byte-free: combine's
LOG_LINKS are intra-block, and the six insns target emits in the between-loops
block (`addiu $s1,+0x6C`, `addiu $s2,+0x6C`, `addiu $s4,0x12`, `lw $t0,0x18($sp)`,
`addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`) consume neither `$s6` nor any
out2-derived value, so the only possible consumer of an out2 reference there is
out3's own definition — which is the residual insn itself. Flow-level dead stores
are separately measured uncounted (s8, `flow.c:2081`;
`rejected/dead-store-out2-uncounted-by-flow.c`). The residual is a fixed point of
the priority arithmetic, not an unfound spelling.

- [s12] Chassis re-measured at session start with memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2, ALL-TARGET callee-saved seats. The ledger floor is confirmed against the current chassis.

- [s12] Gate (a) re-run this session: python3 tools/scan_hand_coded.py --single func_80041188 = tier LOW, score 0/8, 'no strong hand-coded indicators' (S1 0 multu pairs, S2 no empty-body branches, S3 132 insns / 11 spills / 15 distinct regs, S4 max load burst 3, S5 no high-similarity siblings, S6 no BIOS jumptable, S7 all callee-saves have $sp saves, S8 no redundant mask-before-shift). The canonical gate independently routes the function C. Gate (a) FAILS.

- [s12] Gate (b) FAILS: the closing construct would be a byte-free extra reg_n_refs reference on out2 in the between-loops block; docs/reference/sotn-construct-index.md carries no PSX entry exhibiting one, and every family that could host one is measured dead for this shape (dead stores uncounted by flow.c:2081; duplicated-statement-into-arms has no arms since the body is two goto loops; the do{}while(0) weighting is Judge-BANNED for this function per the layer-1 FAIL of 2026-08-22 23:24; the F1 chain-extender materialises 134 insns and inverts pa4 above out2).

- [s12] MATRIX is {s16 m[3][3]; u16 pad; s32 t[3];} = exactly 0x20 bytes (include/gte.h:29), so &m[0]/&m[1] is a semantically faithful spelling of the two adjacent matrices -- but it is codegen-identical to the pointer arithmetic, because the front end folds the index to a constant. Measured: 125 build insns vs target 132, sandbox 43, 8 pseudos instead of 11.

- [s12] In-loop definition position for out2 is inert: four distinct source positions inside loop1 produce byte-identical .lreg priority tables and .greg register dispositions (r86=4/21:3809, r88=7/42:3333, r91=6/48:2500, r78=8/97:2474, r79=4/47:1702, r77=6/95:1263, r75=4/99:808, r87=3/47:638).

- [s12] Options the owner does not need to weigh, because they are already measured dead: extending the frozen SOTN family list does NOT close this function (no byte-free reference exists in the between-loops block under ANY family); INCLUDE_ASM conversion is dead (2026-08-24 deferral probe: sha1=3e9fe25b... != oracle, ROLLED BACK -- this is why the function is byte-coupling-deferred); and candidate.c cannot ship as COMPLETED-C at floor 1, and separately carries a load-bearing un-annotated F1 chain-extender (stptr = base; stptr += 0xFC -- un-splitting it takes the floor 1 -> 15 at an unchanged 132 insns).

- [s12] src/text1a_pre.c was restored to its committed rule-era body at the end of the session; the working tree is clean apart from the ledger, the two banked rejected forms, and the decisions.md packet.

## s13 (permuter, 2026-08-24) — FLOOR 1 held; the banned do-while(0) wrap proven to be the permuter's UNIQUE route to zero

Chassis re-measured at session start and again at session end with
`memory/grind/func_80041188/candidate.c` applied to `src/text1a_pre.c`:
`sandbox func_80041188 --disable all` = **score 1, 132 target / 132 build insns**
(`rules_dropped: 16, cheat_asm_stripped: 2`). HEAD's committed body is still the
stale rule-era one carrying `register s32 *s7_a4 asm("s7")`; the candidate was
applied on top of it for every measurement in this session and restored at the end.

### Infrastructure (reusable, fixes an s4 defect)

The s4 minimal-TU permuter recipe (`tmp/grind/func_80041188/s4/mkws.sh`) is still
valid, but its extraction awk is BROKEN as committed: `/^\t\.ent\tfunc_80041188$/`
matches nothing because maspsx strips the leading tab from directives (the gotcha
s4's own evidence text records but its script does not implement), so `as` fails
with `.frame outside of .ent`. The working pattern is
`/^[ \t]*\.ent[ \t]+func_80041188$/` … `/^[ \t]*\.end[ \t]+func_80041188$/`.
Fixed copies live in `tmp/grind/func_80041188/s13/perm_*/compile.sh`.

NEW this session: the permuter's transform set can be constrained per-campaign
through `settings.toml`:

    [weight_overrides]
    perm_ins_block = 0

`perm_ins_block` (tools/decomp-permuter/src/randomizer.py:1092, default weight 10
in `default_weights.toml:19`) is the randomizer that wraps a statement run in
`do { ... } while (0);` — i.e. it is the SOLE source of the construct this
function's Judge has BANNED. Zeroing its weight turns "is the banned wrap
necessary?" into a directly measurable question, and that is what s13 measured.

### Campaign 1 — floor-9 chassis, FULL transform set (`s13/perm_f9`)

Base = `rejected/s4-form-minus-banned-wrap-floor9.c` (the s4 distance-0 form with
the banned wrap removed): honest sandbox 9, 132/132, permuter base score 63.
Its objdump residual is a PURE `$s5`/`$s6` swap between `tbl` and `out2` plus the
three prologue `sw`/`lui`/`addiu` slots that follow the seats — the build already
emits target's `addiu $sN,$s7,0x20` at slot 24.

**2,588 iterations / 125 s → `output-0-1`, a distance-ZERO find.** Its only delta
from the base is the BANNED construct, re-derived verbatim: a
`do { loop1: … func_8004A348(buf, out2); } while (0);` wrapping loop1's leading
half with the `loop1:` label INSIDE the wrap and the back edge entering it from
outside. This is the same construct s4 found (its `output-50-1`) and the same one
the 2026-08-22 layer-1 cheat-reviewer FAILed. Banked as
`tmp/grind/func_80041188/s13/f9_zero_banned_wrap.c` — NOT re-proposed.

### Campaign 2 — same chassis, `perm_ins_block = 0` (`s13/perm_f9b`)

**22,418 iterations / 1,121 s → ZERO finds.** Not a single candidate scored below
the base 63 — no partial improvement, no register-shaped near-miss, nothing. The
contrast with campaign 1 is the measurement: with block insertion available the
permuter closes this chassis in 2.6k iterations; with it removed, 22.4k iterations
(8.7x) produce no improvement at all. Within the permuter's transform set the
`do { } while (0)` wrap is not *a* route from the floor-9 chassis to zero, it is
the ONLY route.

### Campaign 3 — loop1-as-a-real-do-while chassis (`s13/perm_lr`), `perm_ins_block = 0`

Base = `rejected/loop1-real-do-while-newchassis-score5.c` (honest sandbox 5 — the
lowest-distance non-candidate chassis in the bank, and the only one whose allocno
priority ORDER is already target's). Permuter base score **420**, i.e. the metric
ranks it far WORSE than the sandbox does, because its residual is loop.c's
induction-variable rewrite (`addiu $s3,$v0,308` vs target `252`; `move $a3,$s3` /
`sh $t0,-50($s3)` vs target `addiu $a3,$s3,56` / `sh $v0,6($s3)`) and reordering
diffs cost 60 each in the permuter's weighting. **4,737 iterations / 6 finds,
best 355** — the campaign never got within 300 points of the base's own sandbox
distance. This re-confirms s4's rule in the opposite direction: the permuter
hill-climbs REGISTER-shaped residuals and is blind to strength-reduction-shaped
ones, so a low honest distance is NOT a reason to seed a chassis.

### Campaign 4 — two-locals / `out3 = pa4 + 0x20` chassis (`s13/perm_2l`), `perm_ins_block = 0`

Base = `rejected/two-locals-out3-from-a4-seat-permutation.c` (honest sandbox 15;
permuter base 88 — identical to s4's winning chassis2 base, and a pure
{out2, pa4, a3} seat permutation, exactly the residual shape the permuter is good
at). This chassis had never been permuted: it is the TWO-LOCALS analogue of the
single-local chassis s4 won on, and it is one of the two chassis that emit
target's `addiu $s3,$s7,0x20` natively.

**21,853 iterations / 1,100 s → exactly ONE find, score 63**, produced in the
first 29 seconds and never improved on afterwards. The find is
`out2 = (s32 *) (((u8 *) pa4) + 0x20);` re-executed as a statement in the
between-loops block, immediately followed by `out3 = out2;` — i.e. a SAME-VALUE
re-store of `out2` (dead-store family, `.claude/rules/dead-store-fake-exception.md`,
FAKE-requiring) whose only effect is to hand `out2` the between-loops reference the
priority arithmetic needs. Hand-re-measured in the honest sandbox rather than
trusted: **score 9, 132/132 — byte-identical in distance to the floor-9 chassis of
campaigns 1-2.** So the two-locals chassis does not open a new basin at all; its
single reachable improvement funnels into the SAME floor-9 basin that campaign 2
then proved has no non-`ins_block` exit. Banked as
`rejected/twolocals-permuter-samevalue-restore-funnels-to-floor9-basin.c`.

### What s13 establishes

- [s13] The banned `do { } while (0)` wrap is the permuter's UNIQUE route from the floor-9 chassis to distance 0: full transform set closes it in 2,588 iterations; with `perm_ins_block` weighted to 0, 22,418 iterations produce no improvement whatsoever over the base score
- [s13] `perm_ins_block = 0` under `[weight_overrides]` in a permuter `settings.toml` is a working, reusable way to forbid a Judge-banned construct AT THE SEARCH LEVEL instead of filtering it out of the finds afterwards — it converts "is this construct necessary?" into a measurement
- [s13] The two-locals `out3 = pa4 + 0x20` chassis (sandbox 15) is NOT an independent basin: its only permuter-reachable improvement is a same-value re-store of `out2` that lands at sandbox 9, byte-for-byte the floor-9 chassis, which campaign 2 then exhausted
- [s13] The loop1-real chassis (honest sandbox 5) is permuter-hostile (base score 420, best find 355 over 4.7k iterations): its residual is loop.c strength reduction, which the permuter's weighting buries under reordering penalties. Honest distance does NOT predict permuter tractability — residual SHAPE does (s4's rule, re-confirmed)
- [s13] The s4 workspace recipe's extraction awk is broken as committed (maspsx strips the leading tab from `.ent`); use `/^[ \t]*\.ent[ \t]+func_80041188$/`. Working copies: `tmp/grind/func_80041188/s13/perm_*/compile.sh`
- [s13] Three of the four campaigns ran with `perm_ins_block = 0`, so every find banked this session is wrap-free by construction; the only wrap this session produced is campaign 1's, kept solely as the proof that it is the unique route

- [s13] Chassis re-measured with memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c at session start AND again at session end: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. HEAD's committed body is still the stale rule-era one carrying register s32 *s7_a4 asm("s7"); the candidate was applied on top of it for every measurement this session and restored at the end.

- [s13] The banned do-while(0) wrap is the permuter's UNIQUE route from the floor-9 chassis to distance 0: the stock transform set closes it in 2,588 iterations / 125 s, while with perm_ins_block weighted to 0, 22,418 iterations / 1,121 s produce no candidate better than the base score 63 at all.

- [s13] '[weight_overrides] perm_ins_block = 0' in a permuter settings.toml is a working, reusable way to forbid a Judge-banned construct AT THE SEARCH LEVEL instead of filtering it out of the finds afterwards -- it converts 'is this construct necessary?' into a direct measurement. perm_ins_block is tools/decomp-permuter/src/randomizer.py:1092, default weight 10 at default_weights.toml:19, and is the only randomizer that emits do{...}while(0).

- [s13] The two-locals out3 = pa4 + 0x20 chassis (honest sandbox 15, permuter base 88) is NOT an independent basin: its single permuter-reachable improvement over 21,853 iterations is a same-value re-store of out2 that measures honest sandbox 9, byte-for-byte the floor-9 chassis that campaign 2 then exhausted.

- [s13] The loop1-real chassis is permuter-hostile despite its honest sandbox 5: permuter base 420, best find 355 over 4,737 iterations. Its residual is loop.c strength reduction, which the permuter's 60-per-reordering penalty buries.

- [s13] Three of the four campaigns ran with perm_ins_block = 0, so every find banked this session is wrap-free by construction; the one wrap produced (campaign 1) is retained only as the proof that it is the unique route, and is explicitly NOT re-proposed.

- [s13] TOOLING FIX (reusable): the s4 workspace recipe's extraction awk (/^\t\.ent\tfunc_80041188$/) matches nothing, because maspsx strips the leading tab from directives, so as fails with '.frame outside of .ent'. The working pattern is /^[ \t]*\.ent[ \t]+func_80041188$/ ... /^[ \t]*\.end[ \t]+func_80041188$/. Fixed copies live in tmp/grind/func_80041188/s13/perm_*/compile.sh.

- [s13] No campaign outlived the session: all four were harvested with --stop (perm_f9 self-stopped on zero), 18 worker processes were killed in total, and pgrep shows no permuter process remaining.

## s14 (synthesis, 2026-08-24) — the residual decomposes into TWO independent priority requirements, and one of them is now solvable in ordinary C

Chassis re-measured at the start of the session: `memory/grind/func_80041188/candidate.c`
spliced into `src/text1a_pre.c` scores `sandbox func_80041188 --disable all` = **1**,
132 target / 132 build insns, `rules_dropped: 16, cheat_asm_stripped: 2`. Every number
below is from that chassis measured this session through
`tmp/grind/func_80041188/s10/batch.sh`, which must be run through WSL —
`bash tools/wsl.sh 'bash tmp/grind/func_80041188/s10/batch.sh <files>'`. The Windows-side
Bash tool cannot spawn the cc1 harness; it returns a bare `FileNotFoundError` that
`batch.sh` swallows into a plausible-looking but FAKE `|SEATS ...` line. That cost this
session one wasted probe and will cost the next session one too unless it uses `wsl.sh`.

### E-s14-1 — the empirical seat rule, stated and validated on two chassis

Across every measured form the six contested allocnos `{stptr, i, tbl, out2, pa4, a3}`
take the seats `$s3, $s4, $s5, $s6, $s7, $fp` **in DESCENDING allocno-priority order**
(`pri = floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000`), with exact ties
broken by ascending allocno number. Validated in both directions this session:

* `candidate.c` — stptr 3414, i 2474, tbl 1702, out2 1702, pa4 1263, a3 808 →
  `$s3,$s4,$s5,$s6,$s7,$fp` = **ALL-TARGET**.
* `V1_honest_both` — i 2474, stptr 2439, tbl 1702, pa4 1473, a3 808, out2 714 →
  `$s3,$s4,$s5,$s6,$s7,$fp` = i, stptr, tbl, pa4, a3, out2, which is EXACTLY the measured
  `.greg` disposition (`i=19 out2=30 pa4=22 a3=23 stptr=20 out3=20`).

Target's seats (`asm/funcs/func_80041188.s`: `$s3`=stptr then out3, `$s4`=i, `$s5`=tbl,
`$s6`=out2, `$s7`=pa4, `$fp`=a3) therefore require the priority order
**stptr > i > tbl > out2 > pa4 > a3**, i.e. TWO independent requirements:

* **(A) `stptr > i`.**
* **(B) `tbl > out2 > pa4 > a3`** — out2 strictly inside the band bounded by pa4 below
  and tbl above.

Sessions s7..s13 folded these together and attacked only (B). They are separable, and (A)
has an ordinary-C solution (E-s14-3).

### E-s14-2 — target's OWN honest reference vector is exactly reproducible, and it does NOT seat like target

Reference counts read directly off `asm/funcs/func_80041188.s`: stptr 5, out2 3, out3 3,
pa4 7, tbl 4, i 8, stptr2 6, a3 4, a1/a2 16. `tmp/grind/func_80041188/s14/V1_honest_both.c`
(candidate.c with `stptr = base + 0xFC;` un-split AND `out3 = (s32 *)((u8 *)pa4 + 0x20);`)
measures **exactly that vector**: `r88=5/41:2439 r86=3/42:714 r87=3/47:638 r77=7/95:1473
r79=4/47:1702 r78=8/97:2474 r91=6/48:2500 r75=4/99:808`. Banked as
`rejected/honest-vector-exact-target-refs-seats-permute.c`.

It fails BOTH requirements: i (2474) > stptr (2439), and out2 (714) sits below pa4 (1473)
AND below a3 (808). **So the honest reference vector alone cannot produce target's register
assignment.** This is the sharpest statement of the residual the ledger has carried: it is
not "out2 needs a 4th reference", it is "target's source contains at least one construct
that flow counts and that never reaches the emitted bytes", and that construct has to move
(A) and/or (B).

Corrected band for (B) on the honest chassis: with `out3 = pa4 + 0x20`, pa4 carries out3's
reference and measures **7 refs / 95 = 1473** — NOT the 6 refs / 1263 that s12's E-s12-3
enumeration used, which is the `out3 = out2` chassis's value. The required band is
therefore **(1473, 1702)**, which at 3 references means live length 17.6..20.4 and at 4
references means live length 47.0..54.3. s11's ceiling (block-0 definition, live <= 43) and
s12's floor (in-loop definition, 4 refs / live 21 = 3809) still exclude every achievable
pair, so E-s12-3's conclusion survives the correction unchanged — but the numbers it was
argued from were wrong for the only chassis that emits target's slot-71 insn, and are
replaced here.

### E-s14-3 — (A) is satisfiable in ORDINARY C: `i` live length 99 retires the FAKE chain-extender

s11 proved `candidate.c`'s `stptr = base; stptr += 0xFC;` is an un-annotated F1
combine-foldable chain-extender whose ONLY effect is lifting stptr from 5 refs to 7 refs
(2439 -> 3414), and that un-splitting it drops the floor 1 -> 15. This session shows WHY it
is load-bearing and what can replace it: the split exists solely to satisfy (A).

With stptr un-split (5 refs / 41 = 2439) the ONLY defect versus target is (A) —
`V2_unsplit_o2` measures `i=19 stptr=20 out3=20` and every other seat correct. Moving
`i = 0x12;` to be the **first** statement of the between-loops block closes the dead gap
between loop1's last read of `i` and its redefinition, raising `reg_live_length(i)` from 97
to 99 and dropping its priority 2474 -> **2424, below stptr's 2439**:

| form | `i = 0x12;` position in the between-block | i refs/live/pri | seats |
|---|---|---|---|
| `X0_ipos` | 1st | 8 / 99 / **2424** | **ALL-TARGET** |
| `X1_ipos` | 2nd | 8 / 98 / 2448 | i=$s3, stptr=$s4 |
| `X2_ipos` | 3rd (= candidate.c) | 8 / 97 / 2474 | i=$s3, stptr=$s4 |
| `X3_ipos` | 4th | 8 / 96 / 2500 | i=$s3, stptr=$s4 |

`X0_ipos` therefore reaches **ALL-TARGET callee-saved seats with NO FAKE construct anywhere
in the body** — no split chain-extender, no variable reuse, no dead store, no do-while(0).
Its `sandbox --disable all` score is **3 at 132/132 insns**, and a normalized objdump
comparison (`tmp/grind/func_80041188/s14/cmp.py`) shows the residual is exactly three slots:
`li $s4,18` emitted at slot 67 instead of slot 69 (the two `addiu $s1/$s2,,0x6C` insns shift
down one each), plus the long-known slot-71 `move $s3,$s6` vs `addiu $s3,$s7,0x20`. Banked
as `memory/grind/func_80041188/alt_fakefree_floor3_s14.c`.

The +2 is a sched1 emission-order cost and it is structurally coupled to the win: flow
computes `reg_live_length` on PRE-sched RTL (source order) while sched1's
`rank_for_schedule` falls back to `INSN_LUID` (also source order) for the three
dependency-free between-block insns, so the same statement order that buys i live 99 also
forces `i = 0x12` to be emitted first. Three independent attempts to decouple them failed:

* `Y1_ifirst` (declare `s32 i = 1;` first) — i stays 8 / 97. The parameter copies emitted by
  `expand_function_start` always precede it, so i's live range cannot be extended at the
  front; only the allocno NUMBERING changes (i becomes r77, pa4 becomes r78).
* `Y2_ilate_init` (split `s32 i;` declaration from an `i = 1;` statement) — i 8 / **96**,
  which is worse.
* `Y3_stptr_first` (define stptr before saved/out2, satisfying (A) from the other side by
  shortening stptr's live range to 40) — stptr goes to 5 / **44** = 2272, worse. stptr's
  live range is pinned to loop1's insn count and cannot shrink.

### E-s14-4 — with (A) satisfied honestly, the honest chassis reduces to a pure 3-cycle on {out2, pa4, a3}

`Z0_honest_ifirst` = the honest vector (un-split stptr, `out3 = pa4 + 0x20`) PLUS the
`i = 0x12;`-first fix: `r88=5/41:2439 r78=8/99:2424 r79=4/47:1702 r77=7/95:1473 r75=4/99:808
r86=3/42:714`, seats `out2=30 pa4=22 a3=23` — stptr, i, tbl, stptr2, a1 and a2 all land on
target and the ONLY defect left is the {out2, pa4, a3} 3-cycle, i.e. requirement (B) alone.
`sandbox --disable all` = **17** at 132/132 insns. Banked as
`rejected/honest-vector-ifirst-out2-714-3cycle.c`. This is the cleanest statement of what
remains: **one number — out2's allocno priority — must be lifted from 714 into (1473, 1702)
without adding an emitted byte.**

### E-s14-5 — cse1 substitutes `out2` for `pa4 + 0x20` whenever their EBBs coincide (mechanism read out of the RTL)

Probing why an in-loop1 definition of out2 measures 4 references rather than the 3 the
source shape suggests (`W1_o2_loop1top` and `W2_o2_beforecall2` are byte-identical in
`.lreg`, re-confirming s12's position-inertness on the un-split chassis as well), the
`.flow` dump names the mechanism directly. In `tmp/grind/func_80041188/s8/fd.flow` the
source's `out3 = (s32 *)((u8 *)pa4 + 0x20);` has been rewritten by cse1 into
`(insn 164 (set (reg/v:SI 87) (reg/v:SI 86)))` — a plain `out3 = out2` copy. With out2's
definition inside loop1, the between-loops block lies in the SAME cse extended basic block
as that definition, so cse1 knows `out2 == pa4 + 0x20` and substitutes the cheaper register.
The 4th reference IS that substituted copy — and it also means the in-loop-def chassis can
NEVER emit target's `addiu $s3,$s7,0x20`, no matter how out3 is spelled in the source.

The contrapositive is the useful half, and it is now proven rather than assumed:
**target's slot-71 `addiu $s3,$s7,0x20` requires out2's definition to sit in a DIFFERENT
extended basic block from the between-loops block** — i.e. in block 0 — because only there
does cse1 leave the `pa4 + 0x20` expression alone. Block 0 is also exactly where s11
measured out2's live length capped at 43. The two constraints together are what make (B)
hard, and they are now attributable to a named pass (cse.c EBB scoping) rather than to an
empirical coincidence.

### s14 artifacts
`tmp/grind/func_80041188/s14/` — `V0_candidate.c`, `V1_honest_both.c`, `V2_unsplit_o2.c`,
`V3_split_o3.c`, `W1_o2_loop1top.c`, `W2_o2_beforecall2.c`, `W3_o2_endblock0.c`,
`X0_ipos.c` .. `X3_ipos.c`, `Y1_ifirst.c`, `Y2_ilate_init.c`, `Y3_stptr_first.c`,
`Z0_honest_ifirst.c`, `splice.py` (candidate-into-src splicer), `cmp.py` (normalized
objdump-vs-splat slot comparator), `text1a_pre.orig.c`.

- [s14] Chassis re-measured this session: memory/grind/func_80041188/candidate.c spliced into src/text1a_pre.c scores `sandbox func_80041188 --disable all` = 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. Unchanged from the ledger floor.

- [s14] SEAT RULE (E-s14-1, validated in both directions this session): the six contested allocnos {stptr, i, tbl, out2, pa4, a3} take $s3,$s4,$s5,$s6,$s7,$fp in DESCENDING allocno-priority order, exact ties broken by ascending allocno number. candidate.c (3414/2474/1702/1702/1263/808) -> ALL-TARGET; V1_honest_both (i 2474, stptr 2439, tbl 1702, pa4 1473, a3 808, out2 714) -> the measured .greg disposition i=$s3, stptr=$s4, tbl=$s5, pa4=$s6, a3=$s7, out2=$fp, exactly as the rule predicts.

- [s14] Target's seats therefore require the strict priority order stptr > i > tbl > out2 > pa4 > a3, i.e. TWO independent requirements: (A) stptr > i, and (B) tbl > out2 > pa4 > a3. Sessions s7..s13 attacked only (B).

- [s14] Target's honest reference vector, read directly off asm/funcs/func_80041188.s (stptr 5, out2 3, out3 3, pa4 7, tbl 4, i 8, stptr2 6, a3 4, a1/a2 16), is reproduced EXACTLY by V1_honest_both: r88=5/41:2439 r86=3/42:714 r87=3/47:638 r77=7/95:1473 r79=4/47:1702 r78=8/97:2474 r91=6/48:2500 r75=4/99:808 — and it fails BOTH requirements. Target's own reference counts cannot produce target's register assignment, so the original source contains at least one construct that flow counts and that never reaches the emitted bytes.

- [s14] CORRECTION to s12's E-s12-3 arithmetic: on the `out3 = pa4 + 0x20` chassis (the only one that emits target's slot-71 insn) pa4 carries out3's reference and measures 7 refs / 95 = 1473, not the 6 refs / 1263 the enumeration used. The band for (B) is (1473, 1702): 3 refs needs live 17.6..20.4, 4 refs needs live 47.0..54.3. E-s12-3's CONCLUSION survives — s11's ceiling (block-0 def, live <= 43) and s12's floor (in-loop def, 4 refs / 21 = 3809) still exclude every achievable pair — but the numbers it was argued from are replaced.

- [s14] (A) HAS AN ORDINARY-C SOLUTION. `i = 0x12;` as the FIRST statement of the between-loops block closes i's dead gap and raises reg_live_length(i) 97 -> 99, dropping i's priority 2474 -> 2424, below an UN-SPLIT stptr's 5 refs / 41 = 2439. Sweep: position 1 -> live 99 / 2424 / ALL-TARGET SEATS; position 2 -> 98 / 2448 / fail; position 3 (= candidate.c) -> 97 / 2474 / fail; position 4 -> 96 / 2500 / fail.

- [s14] memory/grind/func_80041188/alt_fakefree_floor3_s14.c is the resulting form: ALL-TARGET callee-saved seats, sandbox --disable all = 3 at 132/132 insns, and ZERO FAKE constructs — no split chain-extender, no variable reuse, no dead store, no do-while(0) wrap. It is the first such form in fourteen sessions. Its residual is `li $s4,18` at slot 67 instead of slot 69 (2 insns) plus the slot-71 `move $s3,$s6` vs `addiu $s3,$s7,0x20` (1 insn).

- [s14] The +2 is a sched1 INSN_LUID emission-order cost structurally coupled to the win: flow reads reg_live_length off PRE-sched RTL (source order) while sched1's rank_for_schedule falls back to INSN_LUID (also source order) for the three dependency-free between-block insns. Three decoupling attempts failed: Y1_ifirst (i declared first) leaves i at 8/97 because expand_function_start's parameter copies always precede any user statement; Y2_ilate_init gives 8/96 (worse); Y3_stptr_first gives stptr 5/44 = 2272 (worse — stptr's live range is pinned to loop1's insn count and only lengthens when its definition moves earlier).

- [s14] Z0_honest_ifirst (un-split stptr + out3 = pa4 + 0x20 + `i = 0x12;` first) reduces the honest chassis to requirement (B) ALONE: stptr, i, tbl, stptr2, a1, a2 all seat on target and the only defect is the {out2, pa4, a3} 3-cycle. sandbox 17 at 132/132. The whole remaining problem is one scalar: out2 from 714 into (1473, 1702) with no emitted byte.

- [s14] MECHANISM NAMED (E-s14-5, read out of tmp/grind/func_80041188/s8/fd.flow, not guessed): with out2's definition inside loop1, cse1 rewrites the source's `out3 = (s32 *)((u8 *)pa4 + 0x20);` into `(insn 164 (set (reg/v:SI 87) (reg/v:SI 86)))` — a plain `out3 = out2` copy — because the between-loops block is then in the SAME cse extended basic block as out2's definition. This explains the unattributed 4th reference s12 measured (4 refs / live 21) AND proves the contrapositive: target's slot-71 `addiu $s3,$s7,0x20` REQUIRES out2's definition in block 0, which is exactly where s11 capped out2's live length at 43.

- [s14] HARNESS WARNING for the next session: tmp/grind/func_80041188/s10/batch.sh must be run through WSL (`bash tools/wsl.sh 'bash tmp/grind/func_80041188/s10/batch.sh <files>'`). Run from the Windows-side Bash tool it cannot spawn cc1, raises a bare FileNotFoundError that the script swallows, and emits a plausible-looking but FABRICATED `|SEATS ...` line with no priority table in front of it. That cost this session one wasted probe.

## s15 (solver, 2026-08-24) — the solver suite runs on this function for the first time

Chassis re-measured at the start of s15: `alt_fakefree_floor3_s14.c` applied to
`src/text1a_pre.c` → `sandbox func_80041188 --disable all` = **score 3, 132 target /
132 build insns** (rules_dropped 16, cheat_asm_stripped 2), matching the s14 record.
`candidate.c` was NOT re-measured this session (s14 measured it at 1); every number
below comes from a form measured this session.

- **[E-s15-0] INFRASTRUCTURE: the solver modality was structurally blocked on this TU
  and is now unblocked.** `tools/gcc-2.7.2/cc1` (the instrumented compiler both solvers
  extract from) **segfaults on `func_80040CB8`** in `src/text1a_pre.c` — exit 139, output
  truncated after `func_80040B44` — so the stock `extract.py` never reaches
  `func_80041188` and emits no model for it (`parity=False`, 22 funcs, none of them
  ours). `tools/gcc-2.7.2/build/cc1` (the build compiler) compiles the TU fine, which is
  why every earlier session's `-da` dumps worked and only the solvers were blocked.
  **Workaround (reusable for any text1a_pre function):**
  `tmp/grind/func_80041188/s15/reduce_i.py` rewrites the preprocessed TU keeping every
  declaration and stubbing every OTHER function body to `{}`. Measured consequences:
  instrumented cc1 == build/cc1 on the reduced TU (**parity=True**), and the reduced
  TU's `func_80041188` asm is **byte-identical to the full-TU build except CODE_LABEL
  NUMBERS** (`.L166`→`.L15`), i.e. `label_num` only. Shims:
  `s15/extract_red.py` (sched_solver), `s15/ra_extract_red.py` (ra_solver),
  `s15/remodel.sh` (one call: cpp → reduce → RA model → honest/target asm).

- **[E-s15-1] The +2 emission-order gap between `alt_fakefree_floor3_s14.c` (floor 3)
  and `candidate.c` (floor 1) has EXACTLY ONE scheduler vector, and s15 spelled it.**
  `tools/sched_solver` on the alt chassis, pass 1, target pinned to a `tgt.head.s` built
  from HEAD source (regfix indexes against HEAD): blocks 0, 1 and 3 are `GOAL == OURS
  (identity)`; **block 2 (5 insns) differs in 3 slots** — ours `li $20,0x12 / addu
  $17,108 / addu $18,108`, target `addu $17,108 / addu $18,108 / li $20,0x12`. Searching
  30 single atoms plus all pairs (`--atoms luid,luid_move --depth 2`) returns
  **1 vector**: `luid_move 152 -> immediately before 161` = move the `i = 0x12;`
  statement to sit immediately before `stptr2 = saved + 0x750;`. Spelled and measured:
  the emission order becomes target's and the score goes **3 → 15**, because that source
  position is also what sets `reg_live_length(i) = 97` instead of 99. So s14's
  requirement (A) and the block-2 emission order are not two problems but ONE: both are
  functions of the same statement position, and no scheduler-side spelling separates
  them. (Form saved as `memory/grind/func_80041188/alt_P1_honest_ipos3_s15.c` — "P1".)

- **[E-s15-2] Typed triage of the two chassis (`inverse_compose.py classify`, target
  pinned to `tgt.head.s`).** On P1 / alt (`out3 = out2`) the first divergence is
  **PRE-RA**: registers-blanked multisets differ, ours `addu $#,$#,$#` vs target
  `addu $#,$#,32` — that is the slot-71 residual, and it means RA/sched searches on this
  chassis are fiction. On `Z0_honest_ifirst` (`out3 = (s32 *)((u8 *)pa4 + 0x20)`) the
  multiset MATCHES and the verdict is **RA**: a pure 3-cycle, ours `$s6→$s7`, `$fp→$s6`,
  `$s7→$fp` (13 substituted operands over 12 insns). This confirms mechanically what the
  ledger asserted structurally: requirement (B) is a PRE-RA insn-shape question on the
  `out3 = out2` chassis and an RA-seat question on the `out3 = pa4+0x20` chassis.

- **[E-s15-3] The Z0 (pa4-derived out3) chassis: the minimal RA solution is 2 atoms, and
  the four minimal vectors SUPERSEDE s11/s12's "the out2 priority window is empty".**
  Z0's extracted allocno table: 73/74 a1,a2 16/99 = 6464 · 91 stptr2 6/48 = 2500 ·
  88 stptr 5/41 = 2439 · 78 i 8/99 = 2424 · 79 tbl 4/47 = 1702 · 77 pa4 7/95 = 1473 ·
  75 a3 4/99 = 808 · **86 out2 3/42 = 714** · 87 out3 3/47 = 638 · 85 saved 2/47 = 425
  (no hard reg — target spills it to `0x18($sp)` too). `inverse.py global` with the FULL
  target disposition as the goal (`{73:17,74:18,88:19,91:16,78:20,79:21,86:22,77:23,
  75:30,87:19}`), at depth 2 and depth 3, reports **minimal solution size 2, four
  vectors**:
  (#1) `tbl refs 4→5` + `out2 refs 3→4`; (#2) `tbl live 47→39` + `out2 refs 3→4`;
  (#3) `out2 refs 3→4` + `out2 live 42→50`; (#4) `pa4 refs 7→4` + `out2 live 42→34`.
  s11/s12 concluded the residual was enumeration-complete because out2's priority had to
  land inside (1473, 1702) and its live-length ceiling on this chassis is 43. That is only
  true **with tbl held at 1702**: at 4 refs / live 42 out2 is 1904, which overshoots tbl,
  and vectors #1/#2 fix that by lifting tbl (5 refs → 2127, or live 39 → 2051) instead of
  by shrinking out2. The window is not empty; it was measured with one variable pinned.
  Vector #4 needs no 4th out2 reference at all. All four remain unspelled.

- **[E-s15-4] THE LOAD-BEARING RESULT — P1 is ONE atom from an all-target, FAKE-free
  chassis.** P1's own extracted model: 91 stptr2 6/48 = 2500 · **78 i 8/97 = 2474** ·
  **88 stptr 5/41 = 2439** · 79 tbl 4/47 = 1702 · 86 out2 4/47 = 1702 · 77 pa4 6/95 =
  1263 · 75 a3 4/99 = 808 · 87 out3 3/47 = 638. The ONLY defect is `i` outranking `stptr`
  by 35 priority units, which shifts `stptr`, `i` and `out3` one seat each (measured
  sandbox 15). Forward-replaying global.c on that model over every ±1..4 live-length and
  ±1..2 reference perturbation of all ten contested pseudos
  (`tmp/grind/func_80041188/s15/probe_ra.py`) gives **ALL-TARGET seats under exactly three
  single-atom families, all on the same two pseudos**:
  `stptr` live 41→40 (also 39/38/37, pri 2439→2500); `stptr` refs 5→6 (also 7, pri
  2439→2926); `i` live 97→99 (pri 2474→2424 — the s14 alt route, and the one that costs
  the +2 emission order per E-s15-1).
  Nothing else reaches it: `i` refs 8→7 drops i to 1443, below tbl, and permutes
  {i, tbl, out2}; every perturbation of tbl, out2, pa4, a3, stptr2 misses. **So the whole
  honest problem is now one unit of `stptr` live range or one honest `stptr` reference** —
  exactly what candidate.c buys with the F1 chain-extender `stptr = base; stptr += 0xFC;`
  (5 refs → 7), which s11 proved is an un-annotated FAKE and therefore not committable.
  Note the arithmetic: with 5 refs the priority is `100000/live`, so live 41 gives 2439
  and live 40 gives 2500 — there is no value strictly between 2474 and 2500 at 5 refs, and
  2500 exactly ties `stptr2`; the tie is decided in `stptr`'s favour (lower allocno number
  is ordered first) and the replay confirms ALL-TARGET, so live 40 is a genuine solution,
  not a knife-edge failure.

- **[E-s15-5] KILLED: the two-counter chassis (separate loop-2 counter `j`).** Never tried
  in 14 sessions and a natural reading of target (which re-initialises `$s4` with
  `addiu $s4,$zero,0x12` between the loops). Spelled on P1 and measured: **sandbox 19**,
  132/132. It DOES solve requirement (A) outright — i 4/48 = 1666 and j 4/49 = 1632 both
  fall below `stptr` 2439 — but it costs more than it buys: both counters now sit BELOW
  tbl (1702) and out2 (1702), so four seats permute. Forward replay over all single-pseudo
  perturbations of the two-counter model: **zero** reach ALL-TARGET (probe_ra2.py); over
  all {i, j} pairs (refs +1/+2, live −2/−3/−4): **zero** (probe_ra3.py). The residual is
  always the same `j`/`out3` inversion (`j` takes $s3, `out3` takes $s4) because `j` and
  `out3` conflict across loop 2 and the higher-priority one takes the lower-numbered
  register, while target wants the LOWER-priority `out3` in $s3. The minimum that works is
  a **3-atom lift** — `i` refs 4→5 AND `j` refs 4→5 AND `out3` refs 3→5 (measured
  ALL-TARGET in replay; `out3` refs 6+ then steals `stptr2`'s $s0). Three simultaneous
  byte-free reference lifts is strictly worse than P1's one, so the one-counter chassis is
  the correct base. Form banked at `rejected/two-counters-needs-3atom-lift-i-j-out3.c`.

- [s15] INFRASTRUCTURE: tools/gcc-2.7.2/cc1 (the instrumented compiler both solvers extract from) SEGFAULTS on func_80040CB8 in src/text1a_pre.c (exit 139, asm truncated after func_80040B44), so the stock extract.py never reaches func_80041188 - the solver modality was structurally unavailable on this TU for 14 sessions. tools/gcc-2.7.2/build/cc1 compiles the TU fine, which is why -da dumps always worked.

- [s15] The workaround is reusable for any text1a_pre function: tmp/grind/func_80041188/s15/reduce_i.py keeps every declaration and stubs every other function body; on the reduced TU instrumented cc1 == build/cc1 (parity=True) and func_80041188's asm is byte-identical to the full-TU build except CODE_LABEL numbers (.L166 -> .L15). Shims: s15/extract_red.py (sched), s15/ra_extract_red.py (RA), s15/remodel.sh (one call).

- [s15] Chassis re-measured at session start: alt_fakefree_floor3_s14.c applied to src = sandbox score 3, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. candidate.c was not re-measured (s14: 1); the reported floor 1 is the ledger's.

- [s15] sched_solver on the alt chassis: blocks 0, 1, 3 are GOAL == OURS (identity); block 2 (5 insns) differs in 3 slots (ours li $20,0x12 / addu $17,108 / addu $18,108; target addu $17,108 / addu $18,108 / li $20,0x12). Exactly one vector in 30 single atoms + all pairs reaches the goal.

- [s15] Spelling that vector (P1 = alt with `i = 0x12;` moved to immediately before `stptr2 = ...`) measures sandbox 15 / 132 of 132: correct emission order, broken requirement (A).

- [s15] inverse_compose.py classify types the two chassis: out3 = out2 is PRE-RA (multiset differs, ours addu $#,$#,$# vs target addu $#,$#,32 - the slot-71 residual), out3 = pa4+0x20 is RA (a pure 3-cycle $s6->$s7, $fp->$s6, $s7->$fp over 13 operands / 12 insns).

- [s15] Z0 allocno table (extracted): stptr2 6/48=2500, stptr 5/41=2439, i 8/99=2424, tbl 4/47=1702, pa4 7/95=1473, a3 4/99=808, out2 3/42=714, out3 3/47=638, saved 2/47=425 (spilled, as target spills it to 0x18($sp)).

- [s15] P1 allocno table (extracted): stptr2 6/48=2500, i 8/97=2474, stptr 5/41=2439, tbl 4/47=1702, out2 4/47=1702, pa4 6/95=1263, a3 4/99=808, out3 3/47=638. The single defect is i outranking stptr by 35 units.

- [s15] At 5 references the priority is 100000/live, so stptr live 41 = 2439 and live 40 = 2500: no value lies strictly between i's 2474 and stptr2's 2500, and the 2500 tie is decided in stptr's favour (lower allocno number ordered first) - the forward replay confirms live 40 gives ALL-TARGET, so it is a genuine solution, not a knife-edge.

- [s15] Two-counter chassis measured at sandbox 19 / 132 of 132 and killed by replay: zero single-pseudo and zero {i,j}-pair perturbations reach all-target; the minimum is a 3-atom lift (i +1 ref, j +1 ref, out3 +2 refs), and out3 at 6+ refs then steals stptr2's $s0.

- [s15] src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched.

## s16 (forensics, 2026-08-24) — the live-length side of the residual is CLOSED, and target's own source is proven to carry a combine-deleted `stptr` reference

Chassis this session: `memory/grind/func_80041188/alt_P1_honest_ipos3_s15.c` ("P1")
spliced into `src/text1a_pre.c`. Dumps captured with `pwsh tools/grinder/dump.ps1
func_80041188` (build/cc1, `-da`) into `tmp/grind/func_80041188/dumps/`; per-pseudo
priority tables and per-block live SEGMENTS captured with the INSTRUMENTED cc1
(`tools/gcc-2.7.2/cc1`) on the s15 reduced TU via
`tmp/grind/func_80041188/s16/model.sh` and `.../sll.sh`. `src/text1a_pre.c` was
restored to HEAD at the end of the session; no build-pipeline file was touched.

### E-s16-1 — WHO OWNS `reg_live_length` AT ALLOCATION TIME: sched1, not flow

Read out of the compiler source, not inferred. `flow.c` computes `reg_n_refs`
(`flow.c:2081`, `+= loop_depth`) and a first `reg_live_length` (`flow.c:1685`,
`flow.c:2087`). `combine.c:55-56` states in its own header comment that
"reg_live_length is not updated" and "reg_n_refs is not adjusted". But
`sched.c:5074-5106` OVERWRITES `reg_live_length[regno]` with
`sched_reg_live_length[regno]`, which sched1 accumulates per basic block in
`finish_sometimes_live` (`sched.c:3165`) over the SCHEDULED insn list.

Consequence (this is the mechanism that makes the whole F1 family work here, and it
was previously stated in the ledger only as an empirical observation):
**a reference that `combine` deletes is counted in `reg_n_refs` and costs NOTHING in
`reg_live_length`**, because refs are frozen pre-combine and live lengths are
recomputed post-combine. `combine` is the only pass between the two that deletes
insns (toplev pass order banked in s8), and `combine` only combines insns linked by
LOG_LINKS, which `flow.c` builds WITHIN a basic block — so any such construct must
be spelled entirely inside one basic block.

### E-s16-2 — MEASURED per-block live SEGMENTS (BB2_SLL_DEBUG), and the proof that `stptr` live 40 is unreachable

`BB2_SLL_DEBUG=<pseudo>` on the instrumented cc1 prints one `SLLDBG reg=R seg=N
total=T` line per basic-block segment. Measured on P1:

| pseudo | segments (block0, loop1, between, loop2) | total |
|---|---|---|
| 73 (a1) | 8, 40, 5, 46 | 99 |
| 78 (i) | (8), 40, (3), 46 | 97 |
| 79 (tbl) | (7), 40 | 47 |
| 86 (out2) | (2), 40, 5 | 47 |
| 77 (pa4) | (4), 40, 5, 46 | 95 |
| **88 (stptr)** | **(1), 40** | **41** |

The scheduled block sizes are therefore block0 = 9, block1 (loop1) = 40, block2
(between-loops) = 5, block3 (loop2) = 46 insns at sched1 time. `stptr`'s block-0
segment is **1**: its defining insn is ALREADY the last insn of the scheduled block 0
(confirmed in the RTL: `(insn 43 ... (set (reg/v:SI 88) (plus (reg/v:SI 80)
(const_int 252))))` immediately precedes `(code_label 44 "loop1")` in
`tmp/grind/func_80041188/dumps/text1a_pre.flow`), and it cannot be pushed later
because the next insn is the loop label. Its block-1 segment is the FULL 40 insns
because `stptr` is loop-carried (live across the back edge), so no scheduling or
statement order can shorten it.

**KILLED: `stptr` live 41 -> 40, one of the three single-atom families E-s15-4
enumerated.** Reaching live 40 requires loop1 to contain one fewer insn at sched1
time, which is a byte change. (The only theoretical loophole is converting one
sched1-era loop1 insn into an insn that `reload` re-materialises — loop1 emits 43 asm
insns from 40 sched1 insns — but no source lever for that is known and it is not a
C-level construct.) The P1 frontier therefore reduces from three atoms to one:
`stptr` refs 5 -> 6 (or -> 7).

### E-s16-3 — TARGET'S OWN SOURCE PROVABLY CONTAINS A COMBINE-DELETED `stptr` REFERENCE

Target's block 0 (`asm/funcs/func_80041188.s:2-28`) is insn-for-insn identical to
ours in ORDER and in insn kind — `addiu $sp,-0x48 / sw $s1 / addu $s1,$a1 / sw $s2 /
addu $s2,$a2 / sw $fp / addu $fp,$a3 / sw $s4 / addiu $s4,$zero,1 / sw $s5 / lui+addiu
$s5,D_80094CFC / sll $a0,2 / sw $ra / sw $s7 / sw $s6 / sw $s3 / sw $s0 / lui+addu+lw
$v0,D_800A9A10 / lw $s7,0x58($sp) / addiu $t0,$v0,0x94 / addiu $s6,$s7,0x20 /
**addiu $s3,$v0,0xFC** / sw $t0,0x18($sp)` — and P1's emitted block 0 matches it
position for position (only the register numbers differ). Target's block 2 emits
`addu $s1,108 / addu $s2,108 / addiu $s4,$zero,0x12 / lw / addiu $s3,$s7,0x20 /
addiu $s0,$t0,1872`, i.e. `i`'s redefinition is the 3rd scheduled insn of block 2 —
which is exactly P1's emission order (that is what s15's sched_solver vector bought).

Because live lengths are a function of the SCHEDULED block layout (E-s16-1/E-s16-2)
and that layout is identical, target's own `reg_live_length` values are ours:
`stptr` = 1 + 40 = **41**, `i` = 8 + 40 + 3 + 46 = **97**. Target's `i` reference
vector read off its bytes is 8 (`addiu $s4,1`; `addu $s4,$s4,1`; `slt ...,18`;
`addiu $s4,0x12`; `addu $s4,$s4,1`; `slt ...,20` — set+use insns counted twice), so
target's `pri(i) = floor_log2(8)*8/97*10000 = 2474`. Target seats `stptr` in `$s3`
ABOVE `i` in `$s4`, which by the s14 seat rule requires `pri(stptr) > 2474`. At live
41 that is impossible with 5 references (2439); it needs **>= 6** (6 refs -> 2926,
7 refs -> 3414).

Target's bytes contain exactly FIVE insns referencing `$s3` in loop1 (`addiu
$s3,$v0,0xFC`; `addiu $a3,$s3,0x38`; `sh $v0,0x6($s3)`; `addiu $s3,$s3,0x68` = set+use).
**Therefore the original source referenced `stptr` at least once more than its bytes
show, and that reference was deleted between `flow` and `global_alloc` — i.e. by
`combine`.** This is the first ledger result that derives the existence of the
construct from TARGET rather than from our search: `candidate.c`'s F1 chain-extender
`stptr = base; stptr += 0xFC;` is a reconstruction of a construct the original
demonstrably had, not an artefact of our chassis.

### E-s16-4 — four new atom spellings measured; all four fold BEFORE `flow` and lift nothing

All measured as single-atom deltas on P1 through `s16/model.sh` (instrumented cc1
`ALLOCDBG` table = `nrefs`/`livelen`/`pri` per pseudo, the exact inputs
`allocno_compare` uses):

| spelling | intent | measured | verdict |
|---|---|---|---|
| `tb1 = tbl + 1; offset = tb1[-1] * 6;` | tbl refs 4 -> 5 (use-only fold-back) | tbl stays 4 refs; one extra insn survives (every live length +1) | KILLED |
| `o2b = (s32*)((u8*)out2 + 4); func_8004A348(buf, (s32*)((u8*)o2b - 4));` | out2 refs +1 (use-only fold-back) | out2 refs unchanged | KILLED |
| `out2 = pa4; out2 = (s32*)((u8*)out2 + 0x20);` | out2 refs +2 (copy-then-modify — the EXACT shape that works on stptr) | out2 stays 4 refs / 47, table otherwise identical to P1 | KILLED |
| `if (i - 0x12 < 0)` in loop1 | i refs 8 -> 9 (compare split, hoping `combine`'s `simplify_comparison` absorbs it) | i stays 8 refs | KILLED |

The third row is the important one: **the F1 chain-extender is NOT shape-portable.**
`stptr = base; stptr += 0xFC;` survives cse and lifts `stptr` 5 -> 7 refs, while the
character-for-character identical shape on `out2` (`out2 = pa4; out2 += 0x20;`) is
folded away before `flow` and lifts nothing. Survival is a per-pseudo property of
cse's cost gate / qty canonicalisation, not of the syntax. Any future ref-lift
proposal on this function must be MEASURED, never predicted from a working sibling.

Banked: `rejected/useonly-foldback-plus1-tbl-and-out2-both-cse-folded-preflow.c`,
`rejected/out2-copy-then-modify-plus2-folds-preflow-unlike-stptr.c`,
`rejected/i-compare-split-minus-form-folds-preflow-refs-inert.c`,
`rejected/tbl-symk-on-P1-2553-outranks-i-2474-steals-s3.c`.

### E-s16-5 — P1's table re-read: requirement (B) is ALREADY satisfied on the `out3 = out2` chassis by an allocno-number TIE

The instrumented `ALLOCDBG` table for P1 (same numbers as s15, re-measured):
`73/74 a1,a2 16/99 = 6464 · 91 stptr2 6/48 = 2500 · 78 i 8/97 = 2474 ·
88 stptr 5/41 = 2439 · 79 tbl 4/47 = 1702 · 86 out2 4/47 = 1702 ·
77 pa4 6/95 = 1263 · 75 a3 4/99 = 808 · 87 out3 3/47 = 638 · 85 saved 2/47 spilled`.
`tbl` and `out2` are EXACTLY TIED at 1702 and the tie is broken by ascending allocno
number (79 before 86, `global.c:635-656`), which happens to be target's order
(`$s5` then `$s6`). So on this chassis requirement (B) holds for free and the sole
defect is (A). The `out2` 4th reference here is the `out3 = out2;` copy — which is
also precisely the statement that costs the slot-71 `addiu $s3,$s7,0x20`. That is the
floor-1 lock restated in one line: **the only construct that gives `out2` its 4th
reference on this chassis is the one construct that makes slot 71 wrong.**

### E-s16-6 — the B2 re-measurement on P1 (tbl sym-K chain), for the record

`tbl = (s32*)((u8*)D_80094CFC - 0x10);` + `tbl = (s32*)((u8*)tbl + 0x10);` still
survives cse on the P1 chassis and gives `tbl` 6 refs / 47 = **2553**, which now
outranks `i` (2474) and takes hard reg 19 (`$s3`), pushing `stptr` to 21. So the
sym-K lift remains realisable and remains wrong on this chassis for the same reason
s8 recorded: its only landing zone is the window (2381, 2474), i.e. 6 refs at live
49-50, and tbl's live length is 47 = 7 (block 0) + 40 (loop1), with the block-0 part
bounded above by the parameter copies that `expand_function_start` always emits
first (s14 Y1).

### s16 artifacts
`tmp/grind/func_80041188/dumps/` (P1 `-da` set: .flow .lreg .greg .sched .combine .cse2 …),
`tmp/grind/func_80041188/s16/` — `sll.sh` (BB2_SLL_DEBUG per-block segment harness),
`model.sh` (one-call instrumented ALLOCDBG priority table from `src/`), `sweep.sh`
(splice+model over a list of variants), `splice.py`, `A_refs.c`, `B1_out2copy.c`,
`B2_tblsymk.c`, `B3_icmp.c`, `text1a_pre.HEAD.c`, `cc1.err`, `red.i`, `red.s`.

- [s16] MECHANISM FROM SOURCE: reg_n_refs is frozen at flow (flow.c:2081) and never recomputed (combine.c:55-56 says so in its own header comment), while reg_live_length is OVERWRITTEN by sched1 (sched.c:5106) from its own per-block accounting (sched.c:3165) over the SCHEDULED insn list. A combine-deleted reference therefore costs zero live length — this is why the F1 chain-extender works — and it must be spelled inside ONE basic block, because combine only follows LOG_LINKS, which flow builds within a block.

- [s16] MEASURED per-block live segments on P1 via BB2_SLL_DEBUG: scheduled block sizes are block0 = 9, loop1 = 40, between = 5, loop2 = 46. stptr (88) = 1 + 40 = 41, i (78) = 8 + 40 + 3 + 46 = 97, tbl (79) = 7 + 40 = 47, out2 (86) = 2 + 40 + 5 = 47, pa4 (77) = 4 + 40 + 5 + 46 = 95.

- [s16] KILLED (one of E-s15-4's three single atoms): stptr live 41 -> 40 is structurally unreachable. stptr's defining insn is already the LAST insn of scheduled block 0 (RTL insn 43 immediately precedes code_label 44), so its block-0 segment is the minimum 1, and it is loop-carried so its block-1 segment is the whole 40-insn loop. Live 40 would require loop1 to lose an insn at sched1 time = a byte change. The P1 frontier is now exactly one atom: stptr refs 5 -> 6 (or 7).

- [s16] PROVEN FROM TARGET, not from our search: target's block-0 emission (asm/funcs/func_80041188.s:2-28) and block-2 emission are insn-for-insn identical in order to P1's, so target's own reg_live_length values are ours (stptr 41, i 97) and its i reference vector is 8 -> pri(i) = 2474. Target seats stptr above i, which at live 41 needs >= 6 references, but target's bytes contain only FIVE stptr-referencing insns. The original source therefore contained at least one stptr reference that combine deleted. candidate.c's F1 chain-extender reconstructs a construct the original demonstrably had.

- [s16] KILLED (four new atom spellings, all fold before flow so no reference is counted): use-only fold-back +1 on tbl (`tb1 = tbl + 1; offset = tb1[-1] * 6;`); use-only fold-back +1 on out2 (`o2b = out2 + 4; call(o2b - 4)`); copy-then-modify +2 on out2 (`out2 = pa4; out2 += 0x20;`); compare-split +1 on i (`if (i - 0x12 < 0)`).

- [s16] THE F1 CHAIN-EXTENDER IS NOT SHAPE-PORTABLE. `stptr = base; stptr += 0xFC;` survives cse and lifts stptr 5 -> 7 refs; the character-for-character identical shape on out2 (`out2 = pa4; out2 += 0x20;`) is folded away pre-flow and lifts nothing. Survival is a per-pseudo property of cse's cost gate, not of the syntax — every ref-lift proposal must be measured, never predicted from a working sibling.

- [s16] P1's re-measured table shows tbl (79) and out2 (86) EXACTLY TIED at 1702, broken by ascending allocno number into target's own order, so requirement (B) is already satisfied on the `out3 = out2` chassis and (A) is the sole defect. But out2's 4th reference there IS the `out3 = out2;` copy — the same statement that makes slot 71 emit `move $s3,$s6` instead of target's `addiu $s3,$s7,0x20`. The floor-1 lock in one line: the only construct giving out2 a 4th reference on this chassis is the one construct that makes slot 71 wrong.

- [s16] Chassis measured THIS session: memory/grind/func_80041188/candidate.c spliced into src/text1a_pre.c gives `sandbox func_80041188 --disable all` = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. src/text1a_pre.c was restored to HEAD afterwards; no build-pipeline file was touched.

- [s16] MECHANISM FROM COMPILER SOURCE: reg_n_refs is counted at flow (flow.c:2081, += loop_depth) and never recomputed -- combine.c:55-56 states 'reg_live_length is not updated' and 'reg_n_refs is not adjusted' in its own header comment -- while reg_live_length IS overwritten by sched1 (sched.c:5106) from its per-block accumulation in finish_sometimes_live (sched.c:3165) over the SCHEDULED insn list. A combine-deleted reference is therefore counted and costs zero live length. This is the mechanism the F1 family runs on, previously banked only as an empirical observation.

- [s16] combine only follows LOG_LINKS, which flow builds WITHIN a basic block, so any combine-deleted reference construct must be spelled entirely inside one basic block -- a cross-block staging pair cannot work.

- [s16] MEASURED per-block live segments on P1 with BB2_SLL_DEBUG (instrumented cc1, s15 reduced TU): scheduled block sizes are block0 = 9, loop1 = 40, between-loops = 5, loop2 = 46 insns. stptr (88) = 1 + 40 = 41; i (78) = 8 + 40 + 3 + 46 = 97; tbl (79) = 7 + 40 = 47; out2 (86) = 2 + 40 + 5 = 47; pa4 (77) = 4 + 40 + 5 + 46 = 95; a1 (73) = 8 + 40 + 5 + 46 = 99.

- [s16] stptr's defining insn is ALREADY the last insn of scheduled block 0 (RTL insn 43 immediately precedes code_label 44 'loop1' in the .flow dump), so its block-0 segment is at its floor of 1, and it is loop-carried so its loop1 segment is the full 40. stptr live 40 is structurally unreachable without deleting an insn from loop1 -- E-s15-4's three single atoms reduce to one (refs 5 -> 6 or 7).

- [s16] Target's block 0 (asm/funcs/func_80041188.s:2-28) is insn-for-insn identical in ORDER to P1's emitted block 0, differing only in register numbers, and target's block 2 emits i's redefinition 3rd exactly as P1 does. Target's own reg_live_length values are therefore ours: stptr 41, i 97, pri(i) = 2474.

- [s16] Target's loop1 contains exactly FIVE $s3-referencing insns, which at live 41 gives pri = 2439 < 2474 -- yet target seats stptr in $s3 above i in $s4. The original source therefore carried at least one stptr reference that combine deleted. This is the ledger's first target-derived (rather than search-derived) proof that the F1 chain-extender in candidate.c reconstructs a construct the original had.

- [s16] Four new atom spellings measured on P1 and KILLED, all folding before flow so no reference is counted: use-only fold-back +1 on tbl (`tb1 = tbl + 1; offset = tb1[-1] * 6;`); use-only fold-back +1 on out2 (`o2b = out2 + 4; call(o2b - 4)`); copy-then-modify +2 on out2 (`out2 = pa4; out2 += 0x20;`); compare-split +1 on i (`if (i - 0x12 < 0)`).

- [s16] THE F1 CHAIN-EXTENDER IS NOT SHAPE-PORTABLE: `stptr = base; stptr += 0xFC;` survives cse and lifts stptr 5 -> 7 refs, while the character-for-character identical shape on out2 folds away pre-flow and lifts nothing. Every future ref-lift proposal must be measured, never predicted from a working sibling.

- [s16] P1's re-measured ALLOCDBG table: 73/74 a1,a2 16/99 = 6464; 91 stptr2 6/48 = 2500; 78 i 8/97 = 2474; 88 stptr 5/41 = 2439; 79 tbl 4/47 = 1702; 86 out2 4/47 = 1702; 77 pa4 6/95 = 1263; 75 a3 4/99 = 808; 87 out3 3/47 = 638; 85 saved 2/47 spilled. tbl and out2 are EXACTLY TIED at 1702 and the tie breaks by ascending allocno number (global.c:635-656) into target's own order, so requirement (B) already holds on the out3 = out2 chassis and (A) is the sole defect -- but out2's 4th reference there IS the `out3 = out2;` copy, the same statement that makes slot 71 emit `move $s3,$s6` instead of target's `addiu $s3,$s7,0x20`.

- [s16] The tbl sym-K chain re-measured on P1: tbl 6 refs / 47 = 2553, which now outranks i (2474) and takes hard reg 19 ($s3), pushing stptr to 21. Its only landing zone remains the window (2381, 2474) = 6 refs at live 49-50, and tbl's live 47 = 7 (block 0) + 40 (loop1) with the block-0 part bounded above by the parameter copies expand_function_start always emits first (s14 Y1).

- [s16] OWNER DIRECTIVE EXECUTED (partially): candidate.c's stptr chain-extender now carries a complete /* FAKE: ... */ annotation with all three required parts -- what, a named GCC-pass mechanism (flow.c:2081 / combine.c:55-56 / sched.c:5106 / global.c:635-656), and a lever-exhaustion pointer into hypotheses.md s7..s16 -- and the s7 header sentence that wrongly claimed 'NO FAKE construct' is corrected in place. The change is comment-only: candidate.c re-measures at sandbox 1 with it applied.

## s17 (forensics, 2026-08-24) — the cse decision that governs F1 chain-extender survival is NAMED, and it is a two-term arithmetic test

Chassis re-measured at the START of this session with `memory/grind/func_80041188/candidate.c`
spliced into `src/text1a_pre.c`: `sandbox func_80041188 --disable all` = **score 1**, 132 target /
132 build insns, `rules_dropped: 16`, `cheat_asm_stripped: 2`. `src/text1a_pre.c` was restored to
HEAD afterwards; no build-pipeline file was touched.

### E-s17-1 — THE LAW: a chain-extender survives cse1 iff the DEFINED reg outlives the SOURCE reg

s16 recorded, as a bare empirical fact, that `stptr = base; stptr += 0xFC;` survives cse and lifts
`reg_n_refs` 5 -> 7 while the character-for-character identical `out2 = pa4; out2 += 0x20;` folds
before `flow` and lifts nothing, and concluded "the F1 chain-extender is NOT shape-portable ...
survival is a per-pseudo property of cse's cost gate". That conclusion is **wrong in its
attribution and wrong in its consequence**: there is no cost gate involved, the deciding code is
cse.c's quantity canonicalisation, and the property is not "per-pseudo" — it is a two-term test on
live ranges that can be computed from the source before compiling anything.

Read out of the RTL, side by side (both from `-da` dumps of the reduced TU, this session):

| variant | pre-cse (`.rtl`) | post-cse1 (`.cse`) |
|---|---|---|
| `candidate.c` (stptr) | `43: (set (reg 88) (reg 80))` ; `46: (set (reg 88) (plus (reg 88) 252))` | **UNCHANGED** — both insns survive |
| `s16/B1_out2copy.c` (out2) | `40: (set (reg 86) (reg 77))` ; `43: (set (reg 86) (plus (reg 86) 32))` | `43` rewritten to `(plus (reg 77) 32)`; the copy is now dead and `flow` deletes it |

The pass and the decision:

- `cse.c:2569` (`canon_reg`) replaces every pseudo occurrence with
  `qty_first_reg[reg_qty[REGNO(x)]]` — the quantity's canonical register. That is the rewrite seen
  in the B1 row.
- `qty_first_reg` is chosen by `make_regs_eqv` (`cse.c:826-881`). When insn `X = Y` makes X
  equivalent to Y, X displaces Y as canonical **iff**

      (uid_cuid[regno_last_uid[X]] > cse_basic_block_end
       || uid_cuid[regno_first_uid[X]] < cse_basic_block_start)
      && uid_cuid[regno_last_uid[X]] > uid_cuid[regno_last_uid[Y]]

  (`cse.c:844-857` — note the first clause is an **OR**, and the second conjunct is the
  load-bearing one here). If X becomes canonical, `canon_reg(X)` returns X, the `X += K` insn keeps
  referring to X, the copy stays live, and `flow` counts **+2** references for X. If Y stays
  canonical, `canon_reg` substitutes Y, the copy dies, and the lift is zero.

**Stated as a usable law:** `X = Y; X += K;` lifts `reg_n_refs(X)` by 2 **iff X's last use is after
Y's last use** (and X's live range leaves the current cse extended basic block, which every
loop-carried local in this function does). Nothing about the identity of X, its type, the constant,
or the "cost" of the expression enters into it.

### E-s17-2 — the law validated in BOTH directions, on two different pseudos

Four measured points, each predicted before the run (ALLOCDBG `nrefs` from the instrumented cc1 via
`tmp/grind/func_80041188/s17/dump17.sh`):

| variant | X | Y | last(X) vs last(Y) | predicted | measured `nrefs(X)` |
|---|---|---|---|---|---|
| `candidate.c` | stptr | base (dies at the copy) | later | survive (+2) | **7** (unsplit baseline 5) |
| `s16/B1_out2copy.c` | out2 | pa4 (lives through loop2) | earlier | fold (+0) | **4** (baseline 4) |
| `s17/V1_out2_from_a4_chain.c` | out2 | the raw `a4` param pseudo, with `pa4 = a4` moved after the chain so `a4` dies in block 0 | later | survive (+2) | **6** |
| `s17/V3_base_outlives_stptr.c` | stptr | base, with `saved = base + 0x94;` moved into the between-loops block so base now dies after loop1 | earlier | fold (+0) | **5** |

V3 is the decisive one: it changes *nothing* about the chain-extender itself — only where `base`'s
last use sits — and `stptr` drops 7 refs / pri 3414 to 5 refs / pri 2439, exactly the two references
the F1 construct exists to buy. The construct's survival is a property of the SURROUNDING code's
live ranges, not of the construct.

Banked: `rejected/base-used-after-loop1-folds-stptr-chain-refs-7to5.c`,
`rejected/out2-chain-from-a4-survives-cse-but-materializes-pa4-move.c`.

### E-s17-3 — the law CLOSES the out2 reference-lift axis by derivation, not by enumeration

Requirement (B) needs a 4th `out2` reference on the `out3 = pa4 + 0x20` chassis. By E-s17-1 a
chain-extender on `out2` can only lift if its source pseudo dies before `out2` does. `out2`'s value
is `a4 + 0x20`, so the only possible sources are `pa4` (lives through loop2 — folds, measured in B1)
or a second, short-lived copy of the raw `a4` param (lifts, measured in V1). V1 measures `out2` at
6 refs / live 51 **and 143 emitted insns instead of 142**: the extra insn is `move $23,$2` in block
0, because once `a4`'s pseudo has a second use the `pa4 = a4` alias no longer coalesces into the
stack load. Target's block 0 emits `lw $s7,0x58($sp)` followed directly by `addiu $s6,$s7,0x20`
(`asm/funcs/func_80041188.s:2-28`) — in target, `a4` and `pa4` ARE the same register. So the only
spelling that can lift `out2` is the one spelling target's bytes forbid. **There is no byte-free F1
lift for `out2` on this function**, and that is now derived from cse.c plus target's own emission
rather than sampled one spelling at a time.

### E-s17-4 — frontier-3 KILLED: reload creates exactly ONE insn in the loop1 region, and it is not in loop1

s16's frontier item 3 asked whether a source form could push a fourth loop1 insn across the
sched1 -> reload boundary (which would drop `stptr`'s live length to 40 for free), on the premise
that "loop1 emits 43 asm insns from 40 sched1-era RTL insns". Measured directly on `candidate.c`
this session by counting RTL insns between `code_label loop1` and `code_label loop2` in the `-da`
dumps:

    .combine 45      .lreg (post-sched1) 45      .greg (post-reload) 46      emitted asm 46

The single insn reload adds is uid **300**, `(set (reg t0) (mem (plus (reg sp) 24)))` — the reload
of the SPILLED pseudo 85 (`saved`, ALLOCDBG `hardreg=-1`), inserted immediately before
`stptr2 = saved + 0x750`, i.e. in the **between-loops block, not in loop1**. loop1 proper gains ZERO
insns at reload: its emitted count equals its post-sched1 count. The premise of frontier 3 was a
mismatch between two different counters (BB2_SLL_DEBUG's per-block accounting vs the RTL insn
count), not a real gap. Together with E-s16-2 (stptr's def is already the last insn of scheduled
block 0, and the range is loop-carried over all of loop1) this **closes the live-length side of
requirement (A) completely**: `stptr` live 41 is invariant under every source form that keeps
loop1's bytes.

### s17 artifacts

`tmp/grind/func_80041188/s17/` — `dump17.sh` (per-variant `-da` + ALLOCDBG harness), `xsec.py`
(per-function section extractor for `-da` dumps), `V1_out2_from_a4_chain.c`,
`V3_base_outlives_stptr.c`, and the full dump sets `CAND/`, `B1/`, `V1/`, `V3/`
(`red.i.rtl .cse .loop .cse2 .flow .combine .lreg .greg .sched`, `red.s`, `cc1.err`).

- [s17] Chassis re-measured: candidate.c spliced into src/text1a_pre.c gives sandbox --disable all = score 1, 132/132 insns, rules_dropped 16, cheat_asm_stripped 2. src restored to HEAD; no pipeline file touched.
- [s17] THE LAW (mechanism from compiler source, validated in both directions): a chain-extender `X = Y; X += K;` lifts reg_n_refs(X) by 2 iff cse1's make_regs_eqv (cse.c:844-857) makes X the quantity's canonical register, which requires uid_cuid[last_uid[X]] > uid_cuid[last_uid[Y]] (plus X leaving the current cse extended basic block). Otherwise canon_reg (cse.c:2569) substitutes Y and flow deletes the dead copy. NOT a cost gate, NOT per-pseudo — a two-term live-range test computable from the source.
- [s17] CORRECTS s16's E-s16-4 conclusion ("the F1 chain-extender is not shape-portable; survival is a per-pseudo property of cse's cost gate"). It IS portable; s16 happened to sample two pseudos whose source-operand live ranges differ.
- [s17] MEASURED (rule validation, decisive): moving `saved = base + 0x94;` out of block 0 into the between-loops block — leaving the chain-extender itself untouched — makes `base` outlive `stptr` and drops stptr from 7 refs / 3414 to 5 refs / 2439, exactly the lift the F1 construct exists to buy. rejected/base-used-after-loop1-folds-stptr-chain-refs-7to5.c.
- [s17] MEASURED (rule validation, other direction): chaining out2 off the raw `a4` param pseudo with `pa4 = a4` moved after it makes out2 outlive its source, the copy survives cse, and out2 goes 4 refs / live 47 to 6 refs / live 51 — the first out2 reference-lift ever measured on this function. But it emits 143 insns instead of 142: `pa4 = a4` stops coalescing into `lw $s7,0x58($sp)` and materialises as `move $23,$2`. rejected/out2-chain-from-a4-survives-cse-but-materializes-pa4-move.c.
- [s17] DERIVED KILL of the out2 reference-lift axis: out2's value is a4+0x20, so its only possible chain sources are pa4 (outlives out2 -> folds) or a second short-lived copy of a4 (lifts, but forces the pa4 alias to materialise a move, which target's block 0 forbids since target emits `lw $s7,0x58($sp)` / `addiu $s6,$s7,0x20` with a4 and pa4 the same register). No byte-free F1 lift for out2 exists on this function.
- [s17] KILLED s16 frontier item 3: loop1's RTL insn count is 45 at .combine, 45 at .lreg (post-sched1), 46 at .greg (post-reload), and 46 emitted. The one reload-created insn is uid 300, `lw $t0,24($sp)`, the reload of the SPILLED pseudo 85 (`saved`), and it lands in the between-loops block, not in loop1. loop1 proper gains zero insns at reload, so there is no sched1/reload-boundary route to stptr live 40. With E-s16-2 the live-length side of requirement (A) is now completely closed.

- [s17] Chassis re-measured THIS session: memory/grind/func_80041188/candidate.c spliced into src/text1a_pre.c gives `sandbox func_80041188 --disable all` = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. src/text1a_pre.c restored to HEAD afterwards; no build-pipeline file touched.

- [s17] THE LAW (from compiler source, validated in both directions): `X = Y; X += K;` lifts reg_n_refs(X) by 2 iff cse1's make_regs_eqv (cse.c:844-857) makes X the quantity's canonical register, which requires uid_cuid[last_uid[X]] > uid_cuid[last_uid[Y]] (plus X's range leaving the current cse extended basic block). Otherwise canon_reg (cse.c:2569) substitutes Y and flow deletes the dead copy uncounted. No cost gate is involved and the property is not per-pseudo — it is a two-term live-range test computable from the source before compiling.

- [s17] This SUPERSEDES s16's E-s16-4 conclusion ('the F1 chain-extender is NOT shape-portable; survival is a per-pseudo property of cse's cost gate'). It is portable; s16 happened to sample two pseudos whose source-operand live ranges differ.

- [s17] RTL evidence for the law: candidate.c pre-cse `43: (set (reg 88) (reg 80))` / `46: (set (reg 88) (plus (reg 88) 252))` is UNCHANGED post-cse1, while s16/B1_out2copy.c's `43: (set (reg 86) (plus (reg 86) 32))` is rewritten to `(plus (reg 77) 32)` and the copy dies.

- [s17] Rule validation, decisive direction: moving `saved = base + 0x94;` out of block 0 into the between-loops block — leaving the chain-extender itself character-for-character untouched — makes `base` outlive `stptr` and drops stptr from 7 refs / 3414 to 5 refs / 2439. Banked as rejected/base-used-after-loop1-folds-stptr-chain-refs-7to5.c.

- [s17] Rule validation, other direction: chaining out2 off the raw `a4` param pseudo with `pa4 = a4` moved after it lifts out2 from 4 refs / live 47 to 6 refs / live 51 — but emits 143 insns instead of 142 (`move $23,$2`, the pa4 alias losing its coalesce into `lw $s7,0x58($sp)`). Banked as rejected/out2-chain-from-a4-survives-cse-but-materializes-pa4-move.c.

- [s17] DERIVED KILL of the out2 reference-lift axis: out2's value is a4+0x20, so its complete chain-source set is {pa4 (outlives out2 -> folds), a second short-lived copy of a4 (lifts, but materialises the pa4 move that target's block 0 forbids)}. No byte-free F1 lift for out2 exists on this function — derived over the whole source set rather than sampled.

- [s17] loop1 RTL insn counts on candidate.c: .combine 45, .lreg (post-sched1) 45, .greg (post-reload) 46, emitted asm 46. The single reload-created insn is uid 300, `lw $t0,24($sp)`, the reload of spilled pseudo 85 (`saved`), and it sits in the between-loops block, not loop1. With E-s16-2 this closes the live-length side of requirement (A) completely: stptr live 41 is invariant under every source form that preserves loop1's bytes.

- [s17] candidate.c updated (comment-only): an s17 addendum records the law, and the FAKE annotation's lever-exhaustion pointer now reads s7..s17. The code is byte-for-byte the same body that measured 1 this session.

## s18 (rederive, 2026-08-24) — the memory layout is rederived, the +1 reference wall is DERIVED and then BROKEN by loop-note weighting

Chassis re-measured at the START of s18 with `candidate.c` applied to
`src/text1a_pre.c`: `sandbox func_80041188 --disable all` = **score 1, 132 target /
132 build insns**, rules_dropped 16, cheat_asm_stripped 2. `src/text1a_pre.c` was
restored to `candidate.c` at the end of the session. The floor did NOT move this
session; what moved is the frontier.

### E-s18-1 — the function's memory layout, rederived from the constants

`0x94 + 0x68 == 0xFC` and `0x94 + 0x750 == 0x7E4`, and `0x750 == 18 * 0x68`. So the
three "unrelated" block-0/block-2 constants are ONE array: **entries of stride 0x68
based at `base + 0x94`**, `saved` is `&entries[0]`, loop1's `stptr` starts at
`&entries[1]`, loop2's `stptr2` starts at `&entries[18]`, and `i` (1..0x11 then
0x12..0x13) IS the entry index. Loop1 runs 17 iterations, so `stptr`'s FINAL value is
`0xFC + 17*0x68 = 0x7E4` — **exactly `stptr2`'s initial value**: the two loops walk one
continuous array, and the original recomputes the pointer rather than carrying it
(target's `lw $t0,0x18($sp)` + `addiu $s0,$t0,0x750`). This is the first semantic
reading of the layout in 18 sessions; it also explains the `saved` spill (a value
computed in block 0 whose only use is in block 2, and which combine cannot fold into
`base + 0x7E4` because the two insns sit in different basic blocks).

### E-s18-2 — cse1 REASSOCIATES any plus-derivation inside the definition's EBB, even when the derived value is real

The natural spellings that the E-s18-1 layout suggests, measured on the candidate
chassis with the instrumented cc1 (`s17/dump17.sh`, ALLOCDBG = the exact
`allocno_compare` inputs):

| spelling | intent | measured | verdict |
|---|---|---|---|
| `stptr = saved + 0x68;` (V1a) | derive loop1's pointer from the array base | table identical to the un-split baseline: `stptr` 5 refs / 41 = 2439, `saved` 2 refs | inert |
| `stptr = saved; stptr += 0x68;` (V2a) | s17-law chain sourced from `saved` | `stptr` 5 / 41 = 2439 — folded, no lift (donor `saved` outlives `stptr`, exactly as the s17 law predicts) | KILLED |
| `stptr = base + 0xFC; saved = stptr - 0x68;` (V3a) | give `stptr` a 6th reference through a REAL, later-consumed value | `stptr` 5 refs / **42** (live +1 from the block-0 order change), `saved` 2 refs — cse1 reassociated `(plus (plus base 252) -104)` into `base + 148` and the reference vanished before flow | KILLED |

V3a is the load-bearing row: it extends s16's E-s16-4 (which only sampled artificial
fold-backs) to a *semantically real* derivation. **Any reassociable arithmetic
derivation from a pseudo whose definition is in the SAME cse extended basic block is
folded before `flow` counts it.** Only a REG-COPY chain survives (s17's make_regs_eqv
law), and only when the donor dies first. Enumerating block-0 pseudos for such a donor
(the carried frontier item 1): `base` (works — the current F1 construct), `saved`
(KILLED above), the `D_800A9A10 + a0*4` address regs and `a0` (their values are a LOAD
away from `base`, so no constant `K` exists). **Frontier item 1 is closed: `base` is the
only donor.** Banked: `rejected/saved-from-stptr-cse1-reassociates-no-stptr-lift.c`,
`rejected/stptr-chain-from-saved-folds-donor-outlives.c`.

### E-s18-3 — WHY reference deltas were quantised to +2 (derivation, not sampling)

`flow` counts a pseudo's references before `combine` runs, so a byte-free lift needs an
insn that `flow` sees and `combine` deletes. `combine` deletes an insn only by merging
it into a consumer **in the same basic block**. Merging insn A (`q = R + k`) into insn B
(`... = q ...`) yields ONE insn mentioning `R` once — A+B mentioned `R` once too, so a
use-only extra reference can never survive as a lift. The only shape that gains is an
insn that both SETS and USES the pseudo itself (`R = D; R += k;`), which flow counts
twice. Hence **+2 or nothing** — with the consequence that s15's four minimal Z0
inverse-solver vectors were all unreachable in principle (#1 `tbl 4->5` + `out2 3->4`,
#2 `tbl live 47->39` + `out2 3->4`, #3 `out2 3->4` + `out2 live 42->50` all demand an
ODD delta; #4 `pa4 refs 7->4` demands deleting three semantically necessary uses of the
matrix pointer). That closed the Z0 chassis — until E-s18-4. NOTE the scope: this
derivation assumes the no-loop-note regime, i.e. it is a statement about
combine-deletable insns, not about `flow`'s counting rule.

### E-s18-4 — THE LEVER: `do { } while (0);` is a per-reference +1 dial (flow.c:2081)

`flow.c:2081` is `reg_n_refs[regno] += loop_depth;`, and `loop_depth` is the **NOTE
nesting depth**: `find_basic_blocks` sets `basic_block_loop_depth` from
NOTE_INSN_LOOP_BEG/END (flow.c:440-471), `propagate_block` re-derives it while scanning
backwards (flow.c:1385-1449), and flow.c:1453 `abort()`s if it ever reaches 0 — i.e. the
no-note baseline is 1 and each enclosing loop note adds 1. A `do { ... } while (0);`
wrap emits LOOP_BEG/END notes with **no back edge**, so every reference inside it is
counted twice by flow while `loop.c` leaves the region alone. **This is the only
construct in this compiler that produces an odd reference delta**, and it makes s15's
Z0 vector #1 spellable. (Family: `.claude/rules/do-while-zero-exception.md` — sanctioned
for ANY codegen effect incl. register allocation, owner ruling 2026-07-06, with a
mandatory inline FAKE annotation; the rule's own example annotation is literally
"loop-note ref weighting seats tbl in s5".)

MEASURED (form `memory/grind/func_80041188/alt_V7_wraps_alltarget_s18.c`, three wraps —
`offset = (*tbl) * 6;` / `func_8004A348(buf, out2);` / `*((s16 *)(stptr + 6)) = 2;` —
honest un-split `stptr = base + 0xFC;`, Z0 block 2 `out3 = (s32 *)((u8 *)pa4 + 0x20)`):
the ALLOCDBG table came out EXACTLY as predicted and **every callee-saved seat is
target's** — `stptr` 6/41 = 2926 -> $s3, `stptr2` 6/48 = 2500 -> $s0, `i` 8/97 = 2474 ->
$s4, `tbl` 5/47 = 2127 -> $s5, `out2` 4/42 = 1904 -> $s6, `pa4` 7/95 = 1473 -> $s7,
`a3` 4/99 = 808 -> $fp, `out3` 3/47 -> $s3, `saved` spilled. This is **the first form in
18 sessions that has target's seats AND target's block-2 insn shape
(`addiu $s3,$s7,0x20`) simultaneously** — the "floor-1 lock" of E-s16-5 (the only
construct that gives out2 a 4th reference is the one that makes slot 71 wrong) is
BROKEN. Sandbox: **8, 132 target / 133 build insns.** The whole residual is ONE insn.

### E-s18-5 — the residual is a scheduling side effect of WHERE the wrap sits

Per-placement sweep (`s18/sweep.ps1`, each measured on the same chassis):

| form | wraps | sandbox | build insns |
|---|---|---|---|
| T1 | tbl load only | 33 | **132** |
| T3 | `*(s16*)(stptr+6) = 2;` only | 15 | **132** |
| T2 | `func_8004A348(buf, out2);` only | 30 | 134 |
| V7 | all three | 8 | 133 |
| U4 | tbl + {`i++` + a2-half stores + out2 call}, `stptr` via the F1 chain | **6** | 133 |
| U1 / U5 | tbl + stptr + {a2-half stores (+`p=`/`i++`) + out2 call} | 7 | 133 |
| U6 | tbl + {`offset += a2` .. out2 call}, `stptr` via F1 chain | 24 | **132** |
| U8 | U6 + a symmetric a1-half wrap | 11 | 135 |

Reading: the tbl and stptr wraps are insn-free; the out2 wrap is not. Objdump of U4
names the mechanism — with the wrap in place the scheduler hoists `offset = offset +
(s32) a2;` ABOVE the first `jal` (target keeps it after) and `i++` then lands before
`lhu $v0,0($s0)` instead of filling its load-delay slot, so a `nop` appears. Pulling
`offset += a2` INSIDE the wrap fixes the insn count (U6 = 132) but adds a reference to
`a2`, which breaks the `a1`/`a2` 16/16 tie (a2 17 -> 6868 > a1 6464) and permutes their
seats (sandbox 24); restoring the tie with a symmetric a1-half wrap costs three insns
(U8 = 135). **The s19 problem is exactly: give `out2` its +1 from a wrap whose boundary
neither frees `offset += a2` to hoist nor adds a reference to `a1`/`a2`.**

### E-s18-6 — a REAL `do { } while (i < 0x12)` loop1 also reaches all-target seats, and is killed by loop.c instead

Same weighting mechanism from a genuine loop note (`rejected/loop1-real-dowhile-honest-
alltarget-seats-loopc-giv-score5.c`; honest un-split `stptr`, `out3 = out2`): all seats
target's (`stptr` 9/40 = 6750 -> $s3, `i` 11/97 -> $s4, `tbl` 7/47 -> $s5, `out2` 6/46 ->
$s6, `pa4` 8/94 -> $s7, `stptr2` 6/48 -> $s0, `a3` 5/99 -> $fp), 132 build insns,
**sandbox 5** — the best score ever recorded on a form carrying no FAKE construct at
all. Its residual is `loop.c`'s strength reduction, which the do-while(0) wrap does NOT
trigger: with a real back edge loop.c biases the biv to `base + 0x134` and emits
`move $7,$19` + `sh $8,-50($19)` where target has `addiu $a3,$s3,0x38` +
`sh $v0,0x6($s3)`. Also note `stptr`'s live length is 40 there (not 41) — the other atom
s15 asked for. So the note-weighting regime is byte-usable ONLY through wraps.
The Z0 variant of the same real-loop form (V6) is NOT all-target: the loop weighting
lifts `pa4` to 9 refs / 94 = 2872 above `out2` 5/41 = 2439 and the two swap seats.

### s18 artifacts

`tmp/grind/func_80041188/s18/` — `run.sh`, `sweep.ps1`, `apply.py`, `xdiff.py`,
`ledger.py`, and the variant bodies (`V1a/V2a/V3a/V5/V6/V7/T1..T4/U1/U4/U5/U6/U7/U8.c`);
ALLOCDBG dumps and cc1 asm under `tmp/grind/func_80041188/s17/<TAG>/` (the reused s17
dump script writes there).

- [s18] Chassis re-measured at session start AND end with candidate.c applied: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. src/text1a_pre.c was restored to HEAD at the end; no build-pipeline file was touched.

- [s18] Memory-layout rederivation: 0x94 + 0x68 = 0xFC, 0x750 = 18*0x68, 0x94 + 0x750 = 0x7E4 = 0xFC + 17*0x68. One 0x68-stride array based at base+0x94; loop1 walks entries 1..17, loop2 entries 18..19, and loop1's exit pointer equals loop2's entry pointer. `saved` is the array base; `i` is the entry index.

- [s18] V3a (`stptr = base + 0xFC; saved = stptr - 0x68;`) measured stptr 5 refs / live 42: cse1 reassociates a plus-derivation inside the definition's EBB even when the derived value is real and consumed in a later block. This extends E-s16-4 from artificial fold-backs to real values.

- [s18] V1a/V2a measured stptr 5 refs / 41 = 2439 (identical to the un-split baseline): a copy-chain sourced from `saved` folds, because `saved` outlives `stptr` — s17's make_regs_eqv law validated for a third time, and the block-0 donor enumeration is now complete with `base` as the only donor.

- [s18] flow.c:2081 is `reg_n_refs[regno] += loop_depth;` and loop_depth is the loop-NOTE nesting depth (flow.c:440-471, 1385-1449, abort at 0 => baseline 1). Therefore any do-while(0) wrap is a +1 reference dial over exactly the references it encloses, and it is the only construct in this compiler that produces an ODD reference delta.

- [s18] V7 (3 wraps, honest un-split stptr, Z0 block 2) measured ALL-TARGET callee-saved seats — stptr 6/41=2926 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 5/47=2127 $s5, out2 4/42=1904 $s6, pa4 7/95=1473 $s7, a3 4/99=808 $fp, out3 3/47 $s3, saved spilled — while emitting target's block-2 addiu $s3,$s7,0x20. Sandbox 8 at 133 build insns.

- [s18] Wrap placement is also a scheduling constraint: tbl-load wrap alone = 132 insns (sandbox 33), stptr-store wrap alone = 132 (15), out2-call wrap alone = 134 (30), V7 = 133 (8), U4 = 133 (6), U6 (out2 wrap widened over `offset += a2`) = 132 (24, a2 17 refs breaks the a1/a2 tie), U8 (symmetric a1 wrap) = 135 (11).

- [s18] Objdump of U4 names the extra insn: with the out2 wrap present the scheduler hoists `offset = offset + (s32) a2;` above the first jal (target keeps it after), so `i++` no longer fills the `lhu $v0,0($s0)` load-delay slot and a nop appears.

- [s18] V5 (real do-while loop1, no FAKE construct anywhere) = ALL-TARGET seats, 132 insns, sandbox 5; its residual is loop.c's giv rewrite (biv biased to base+0x134, `sh $8,-50($19)`), which the note-only do-while(0) wrap does not trigger. stptr's live length there is 40, the other atom s15's replay asked for.

## s19 (rederive, 2026-08-25) — the loop is REAL: loop.c's giv replaces the FAKE chain-extender, and the Z0 residual shrinks to ONE reference

Chassis re-measured at the START of s19 with `candidate.c` applied to `src/text1a_pre.c`:
`sandbox func_80041188 --disable all` = **score 1, 132 target / 132 build insns**,
rules_dropped 16, cheat_asm_stripped 2. `src/text1a_pre.c` was restored to HEAD at the
end of the session; no build-pipeline file was touched. The floor did not move; a new
chassis and a much sharper frontier did.

### E-s19-1 — the rederivation: write s18's array layout literally and loop.c produces target's pointer insns for free

s18 (E-s18-1) proved the constants describe ONE 0x68-stride array based at `base + 0x94`
with `i` as the entry index. s19 wrote that reading as C — loop1 as a real
`do { ... } while (i < 0x12);` with the entry address written INLINE at its two use
sites (`ents + i * 0x68 + 0x38`, `*(s16 *)(ents + i * 0x68 + 6)`) and no pointer local
at all. Measured (`memory/grind/func_80041188/alt_W4_realloop_giv_honest_s19.c`, tag W4):
**sandbox 3, 132 target / 132 build insns, ALL-TARGET callee-saved seats, ZERO FAKE
constructs.** loop.c strength-reduces the multiply into a giv whose initial value folds
to target's `addiu $s3,$v0,0xFC`, whose increment is target's `addiu $s3,$s3,0x68` in
the branch delay slot, and whose allocno is **9 refs / live 40 = 6750** — obtained for
free, where candidate.c has to buy `stptr` 7 refs with the FAKE-annotated F1
chain-extender. Loop2's initial value comes out as `ents + 0x750`, i.e. target's own
`lw $t0,0x18($sp)` + `addiu $s0,$t0,0x750`, so the block-0 spill of `ents` is now
EXPLAINED by the layout rather than hand-placed. This is the first FAKE-free form whose
walking-pointer priority is honest.

### E-s19-2 — the induction address must NOT be a named local (loop.c copies into user variables)

The same body with the address in a local (`ent = ents + i * 0x68;`) scores **80**. Cause,
read out of `tmp/grind/func_80041188/s19/W1/red.i.combine` insn 339: because the giv's
destination is a user variable (REG_USERVAR_P), loop.c does not rename it to the giv —
it emits `ent = <giv>` at the top of every iteration. That copy is a second quantity live
across the calls, local_alloc seats it in $s1, the nine callee-saved seats are exhausted
before `a3` is reached and **a3 spills**. Banked:
`rejected/realloop-giv-uservar-copy-steals-s1-a3-spills.c`. Corollary for any future
strength-reduction attempt in this project: never name an induction address.

### E-s19-3 — the real-loop regime carries a FIXED +2 tax: loop.c hoists the `li 2`, global.c spills it, reload picks $t0

W4's three residual insns are `addiu $t0,zero,2` / `sh $t0,6($s3)` (target: $v0) plus the
standing lock. Mechanism, read from the dumps and confirmed in loop.c source: a loop with
a single-basic-block body always satisfies `scan_loop`'s movable test — the `li 2`'s
destination is a compiler pseudo, so test (2) at loop.c:686-700 (`! REG_USERVAR_P &&
! REG_LOOP_TEST_P`) passes, and test (3) `reg_in_basic_block_p` passes too — and
`move_movables`' profitability test `threshold * savings * m->lifetime >= insn_count`
(loop.c:1631) passes for ANY lifetime, because `threshold = (loop_has_call ? 1 : 2) *
(1 + n_non_fixed_regs)` (loop.c:532) is ~31 against an insn_count of ~40. The constant is
therefore hoisted into the preheader, becomes a global allocno (3 refs / live 92),
global.c spills it, and reload rematerialises it into $t0 — where target's in-place `li`
is a local quantity that local_alloc gives $v0. **All three escape routes are closed by
the source of the test itself** (making the value a user variable does not help: tests
(1) and (3) still pass), so every real-loop form of loop1 starts 2 insns behind. This
also decomposes s18's V5 score of 5 exactly: 2 (giv rewrite) + 2 (this tax) + 1 (lock),
and W4 removes the first term.

### E-s19-4 — on the real-loop chassis with target's block-2 spelling, the ENTIRE residual is ONE reference on out2

W6 = W4 with target's own `out3 = (s32 *)((u8 *)pa4 + 0x20)` (so block 2 emits
`addiu $s3,$s7,0x20`). Measured table (`s19/W6/cc1.err`): giv 9/40=6750 $s3, i 11/97=3402
$s4, tbl 7/47=2978 $s5, **pa4 9/94=2872 $s6**, stptr2 6/48=2500 $s0, **out2 5 refs /
live 41 = 2439 $s7**, a3 5/99=1010 $fp, out3 3/47 $s3. Only out2 and pa4 are swapped;
every other seat is target's. The arithmetic then states the requirement exactly: out2
must land in (2872, 2978), and at floor_log2 quantisation the ONLY solution is **6 refs
at live 41** = 2926 — and out2's live length on this chassis is ALREADY 41. So the whole
endgame of this chassis is one extra flow-counted, byte-free reference to out2. (Contrast
the old goto-loop regime, where E-s11-1 needed out2 live 47..55 at 4 refs and the window
was empty.) Banked: `rejected/realloop-Z0-out2-5refs-live41-one-reference-short.c`.

### E-s19-5 — the +1 model is CONFIRMED by construction, and the in-loop wrap costs three nops

W8 = W6 + `do { func_8004A348(buf, out2); } while (0);` inside loop1. The predicted table
appears exactly: **out2 6 refs / live 41 = 2926, seated $s6, pa4 $s7, tbl $s5, i $s4, giv
$s3, stptr2 $s0, a3 $fp, out3 $s3** — all-target seats WHILE block 2 emits target's
`addiu $s3,$s7,0x20`, the combination that E-s16-5 called impossible on the goto chassis.
Cost: **135 build insns** (cc1 emits the same 142 raw insns; the three extra are
load-delay nops maspsx must insert because the extra LOOP_BEG/END notes stop the
scheduler filling those slots). Banked:
`rejected/realloop-Z0-inloop-wrap-alltarget-seats-costs-3-nops.c`.

### E-s19-6 — a do-while(0) wrap in BLOCK 0 is byte-neutral, but it can only weight a reference that survives cse1

W10 = W6 + `o2t = (u8 *)pa4 + 0x20; do { out2 = (s32 *)o2t; } while (0);`. Two independent
facts: (1) the block-0 wrap costs **zero insns** (132 build insns) — unlike the in-loop
wrap of W8 — so wrap placement OUTSIDE the loop is scheduling-free on this function;
(2) it lifts nothing (out2 stays 5 refs). The wrapped insn is a register copy whose
destination outlives its source, so by the s17 make_regs_eqv law (cse.c:844-857) `out2`
is the canonical register, cse1 rewrites `o2t`'s definition to set out2 directly, and the
copy is gone before flow counts it. W11 (the same split without the wrap) measures
identical to W6, confirming the fold. **Rule for the next session: a block-0 wrap is
free, but the reference it weights must survive cse1 — which by the s17 law means the
donor must outlive the recipient, and the only block-0 donor for out2 is `pa4`, whose own
reference the same wrap would also weight (pa4 -> 10 refs / 3191, back above out2).**

### s19 artifacts

`tmp/grind/func_80041188/s19/` — `apply.py`, `run.sh`, `dump19.sh`, `sweep.ps1`,
`odiff.py` (normalising objdump-vs-target insn differ), variant bodies
`BASE.c`/`W1.c`..`W11.c`, and per-tag cc1 dump directories (`W1/`, `W2/`, `W4/`, `W6/`,
`W8/`, `W10/`, `BASE/`) holding `red.i.*` pass dumps + `cc1.err` with the ALLOCDBG
tables.

- [s19] Chassis re-measured at session start with candidate.c applied: sandbox func_80041188 --disable all = score 1, 132/132. src/text1a_pre.c restored to HEAD at the end.
- [s19] Writing s18's array layout literally (real do-while loop1 + inline `ents + i * 0x68` addressing) gives sandbox 3 at 132/132 with ALL-TARGET seats and ZERO FAKE constructs — the giv's 9 refs / live 40 replace the FAKE F1 chain-extender's whole purpose. Form: alt_W4_realloop_giv_honest_s19.c.
- [s19] Naming the induction address (`ent = ents + i * 0x68;`) makes loop.c emit `ent = <giv>` per iteration (REG_USERVAR_P destinations are not renamed), which costs a callee-saved seat and spills a3 (sandbox 80).
- [s19] Every real-loop form pays a fixed +2: loop.c hoists the invariant `li 2` (scan_loop tests at loop.c:686-700 and the threshold test at loop.c:1631 both pass unconditionally here), global.c spills the hoisted pseudo (3 refs / live 92) and reload rematerialises it into $t0 where target has $v0. Decomposes s18's V5 = 2 giv + 2 tax + 1 lock.
- [s19] On the real-loop chassis with target's block-2 spelling (W6), out2 is 5 refs / live 41 = 2439 and pa4 9/94 = 2872; every other seat is target's. The needed window is (2872, 2978), whose unique quantised solution is out2 6 refs at live 41 = 2926 — one reference, live length already correct.
- [s19] W8 confirms the model by construction: one in-loop do-while(0) wrap on the out2 call gives out2 6/41 = 2926, ALL-TARGET seats AND target's `addiu $s3,$s7,0x20` simultaneously — at a cost of three maspsx load-delay nops (135 insns).
- [s19] A do-while(0) wrap in BLOCK 0 costs zero insns (W10 = 132), but weights nothing unless the enclosed reference survives cse1; a copy whose destination outlives its source is folded by make_regs_eqv (cse.c:844-857) before flow counts it (W10 lifts out2 by 0; W11, the unwrapped control, is byte-identical to W6).

- [s19] Chassis re-measured at session start with candidate.c applied: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.

- [s19] W4 (loop1 a real do-while + inline `ents + i * 0x68` addressing, no pointer local) = sandbox 3 at 132/132 with ALL-TARGET callee-saved seats and ZERO FAKE constructs: a1 17/99 $s1, a2 17/99 $s2, giv 9/40=6750 $s3, i 11/97=3402 $s4, tbl 7/47=2978 $s5, out2 6/46=2608 $s6, pa4 8/94=2553 $s7, stptr2 6/48=2500 $s0, a3 5/99=1010 $fp, out3 3/47 $s3.

- [s19] W4's residual is exactly 3 insns and both causes are named: 2 = addiu $t0,zero,2 / sh $t0,6($s3) (target $v0; loop.c constant hoist + spill + reload rematerialisation) and 1 = move $s3,$s6 vs addiu $s3,$s7,0x20 (the standing out3 lock).

- [s19] loop.c's strength reduction supplies the walking pointer 9 references / live length 40 for free, which is the honest structural replacement for candidate.c's FAKE-annotated F1 chain-extender (`stptr = base; stptr += 0xFC;`), whose sole purpose was buying stptr enough references to outrank i.

- [s19] Naming the induction address makes loop.c emit a per-iteration copy into the user variable (red.i.combine insn 339), which costs a callee-saved seat and spills a3 (sandbox 80).

- [s19] Both loops converted to real loops (W2/W7) inflates every reference (a1/a2 to 29 refs, pa4 to 10) and also spills a3: sandbox 78 / 20. The note-weighting regime is usable on one loop at a time.

- [s19] W6 (real-loop chassis + target's block-2 spelling) = sandbox 11 at 132/132 with out2 5 refs / live 41 = 2439 and pa4 9/94 = 2872 the only pair out of target order; the required window (2872, 2978) has the unique quantised solution out2 6 refs at live 41 = 2926, and the live length is already correct.

- [s19] W8 proves that solution: one in-loop do-while(0) wrap on the out2 call produces out2 6/41 = 2926 with ALL-TARGET seats AND target's addiu $s3,$s7,0x20 at once (the combination E-s16-5 called impossible on the goto chassis), at a cost of three maspsx load-delay nops.

- [s19] A do-while(0) wrap in BLOCK 0 costs zero insns (W10 = 132 build insns) whereas the same wrap inside loop1 costs three; but a block-0 wrap weights nothing unless the enclosed reference survives cse1 (W10 lifts out2 by 0; W11, the unwrapped control, is byte-identical to W6).

- [s19] s18's V5 score of 5 now decomposes exactly: 2 (giv/biv rewrite, removed by W4's indexed form) + 2 (loop.c constant hoist tax, killed by derivation) + 1 (the out3 lock).

## s20 (structural, 2026-08-25) — the Z0 route on the GOTO chassis is closed by enumeration: out2's 4th reference has exactly three possible sites and all three are dead

Chassis re-measured at the START of s20 with `candidate.c` applied to
`src/text1a_pre.c`: `sandbox func_80041188 --disable all` = **score 1, 132 target /
132 build insns**. `src/text1a_pre.c` was restored to HEAD at the end of the session;
no build-pipeline file was touched. The floor did not move. s20 worked the GOTO
chassis (candidate.c's chassis), complementing s19, which worked the real-loop one.

All s20 forms are variants of s18's V7 (goto loop1, tbl-load wrap + stptr-store wrap,
Z0 block-2 spelling `out3 = (s32 *)((u8 *)pa4 + 0x20)`) with V7's costly out2-CALL wrap
removed. Tags/bodies + ALLOCDBG dumps: `tmp/grind/func_80041188/s20/`.

### E-s20-1 — a block-0 do-while(0) wrap DOES lift out2, correcting the scope of s19's E-s19-6

s19 concluded from W10 that a block-0 wrap "weights nothing unless the enclosed
reference survives cse1", having wrapped a redundant COPY (`out2 = o2t`) that cse1
deleted. s20 wrapped out2's own DEFINITION instead —
`do { out2 = (s32 *)(((u8 *)pa4) + 0x20); } while (0);` (form Y1) — and the lift is
real: **out2 3 -> 4 refs, live 42, priority 714 -> 1904**, at 132 build insns. The
distinction is that a definition-by-plus is not deletable by cse1, so flow counts it
at loop_depth 2. E-s19-6's rule should be read as being about *redundant copies*, not
about block-0 wraps in general.

1904 is exactly where the seat order wants out2 on this chassis (the required window
is (pa4 1473, tbl 2127) with the tbl wrap in place). **The wrap nevertheless fails,
for a different reason:** flow.c:2081 weights EVERY reference in the enclosed insn, and
out2's definition necessarily references pa4, so pa4 goes 7 -> 8 refs (1473 -> 2526) and
jumps above out2, tbl AND i in one step. Y1 measures **sandbox 27** with
stptr 6/41=2926 $s3, pa4 8/95=2526 $s4, stptr2 6/48=2500 $s0, i 8/97=2474 $s5,
tbl 5/47=2127 $s6, out2 4/42=1904 $s7, a3 808 $fp.
Banked: `rejected/block0-wrap-out2-def-lifts-pa4-to-8refs-2526.c`.

### E-s20-2 — writing the wrapped definition from the PARAMETER is inert; SPLITTING the parameter copy works but materialises one insn

Y2a (Y1 with the wrapped definition written `(u8 *)a4 + 0x20` while the declaration
`s32 *pa4 = a4;` stays the first statement) measures an ALLOCDBG table **byte-identical**
to Y1 (pa4 8/95 = 2526, sandbox 27): cse1 canonicalises a4 and pa4 to one quantity inside
block 0's EBB — pa4 outlives a4, so pa4 is the canonical register (s17 make_regs_eqv law,
cse.c:844-857) — and the reference lands on pa4 anyway.

Y2b demotes the declaration to `s32 *pa4;` and writes `pa4 = a4;` as a statement AFTER
the a4-sourced wrapped definition, so the equivalence class is only formed after out2's
definition has already referenced a4. The two pseudos stay distinct and the table
becomes **ALL-TARGET seats WITH target's block-2 `addiu $s3,$s7,0x20`** — the combination
E-s16-5 declared impossible on the goto chassis: stptr 6/41=2926 $s3, stptr2 6/48=2500
$s0, i 8/98=2448 $s4, tbl 5/48=2083 $s5, out2 4/43=1860 $s6, pa4 6/93=1290 $s7,
a3 4/100=800 $fp, out3 3/47 $s3. **Sandbox 12 at 133 build insns.** Y4 (the same split
with the stptr wrap dropped and stptr bought with candidate.c's F1 chain-extender
instead — one loop1 wrap in total) measures the same 12/133 with stptr 7/41=3414;
dropping the tbl wrap as well (Y9) costs the tbl/out2 order and scores 15.

The single defect is named by objdump: target loads the parameter straight into its
callee-saved seat (`lw $s7,0x58($sp)`), while the split makes a4 a **block-0-local
quantity**, so local_alloc seats it in a call-clobbered register ($v0) before
global_alloc ever sees pa4, and the copy `addu $s7,$v0,$zero` is emitted (133 vs 132).
GCC 2.7 has no coalescing that merges a local quantity into a global allocno, so the
copy is structural, not schedulable away. Banked:
`rejected/a4-sourced-wrapped-out2-def-cse1-canonicalises-to-pa4-inert.c`,
`rejected/split-pa4-copy-alltarget-seats-plus-Z0-costs-one-param-copy-insn.c`,
`rejected/split-pa4-copy-f1-stptr-alltarget-seats-still-one-copy-insn.c`.

### E-s20-3 — no block-0 arithmetic derivation of one matrix pointer from the other survives cse1, in EITHER direction

Z1 back-derives the first pointer from the second — `out2 = (s32 *)((u8 *)a4 + 0x20);
pa4 = (s32 *)((u8 *)out2 - 0x20);` — to buy out2 a fourth reference from a
semantically real statement with no wrap at all. Measured **sandbox 23 at 133 insns
with out2 at 3 refs / live 43 = 697**, i.e. the intended reference does not exist; the
ALLOCDBG table and score are IDENTICAL to Z2 (`pa4 = a4;` written plainly), proving cse1
reassociated `(plus (plus a4 32) -32)` to `a4` before flow counted anything. This
extends E-s18-2 from the ADD direction to the SUBTRACT direction and to a derived value
consumed throughout the function. Banked:
`rejected/pa4-backderived-from-out2-cse1-reassociates-to-a4.c`.

### E-s20-4 — loop1 wraps are insn-count-neutral but NOT emission-order-neutral on the goto chassis

Z3 (Y1 with the ledger's `out3 = out2;` spelling instead of Z0) reaches **ALL-TARGET
seats at 132 build insns** — stptr 6/41=2926 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4,
tbl 5/47=2127 $s5, out2 5/47=2127 $s6 (exact tie with tbl, broken the right way by
allocno number 79 < 86), pa4 7/95=1473 $s7, a3 808 $fp — yet scores **8**, where
candidate.c scores 1 with the SAME seats. objdump attributes every extra diff to the
LOOP_BEG/END notes' effect on sched1's emission order, not to allocation: `tbl++`
(`addiu $s5,$s5,0x4`) is displaced past the `addiu $a0,$sp,0x10` / `addu $a1,$s7,$zero`
argument setup, `addu $s0,$s0,$s2` moves with it, and `sw $t0,0x18($sp)` is displaced in
block 0. **A loop1 do-while(0) wrap is therefore strictly worse than candidate.c's F1
chain-extender on this chassis even when it produces identical seats** — which also
means the wrap family is not a cheaper substitute for the F1 annotation here. Banked:
`rejected/wraps-alltarget-seats-notes-cost-six-sched-order-diffs.c`.

### E-s20-5 — the enumeration: the Z0 route on the goto chassis is CLOSED

On the goto chassis with target's block-2 spelling, the seat order needs out2's priority
inside (pa4, tbl); with pa4 at its natural 7 refs / 95 = 1473 the only quantised
solutions are 4 refs at live 42 (1904, tbl must be wrapped to 2127) or 4 refs at live 47
(1702, tie with an unwrapped tbl broken the right way). Either way out2 needs a FOURTH
flow-counted reference, and there are exactly three sites for it:
  1. **block 0** — out2's only block-0 insn is its definition from pa4. Weighting it
     co-weights pa4 (E-s20-1); sourcing it from the parameter is inert (E-s20-2);
     splitting the parameter copy to dodge the co-weighting materialises one insn
     (E-s20-2); deriving either pointer from the other is reassociated away (E-s20-3).
  2. **loop1** — closed by E-s18-3's +2-or-nothing derivation plus E-s16-4's measured
     fold-backs: a use-only extra reference cannot survive combine's merge.
  3. **block 2** — closed by E-s17-3/E-s14-5: a reference there survives cse1 (different
     EBB) but combine can only delete it by merging it into a consumer in the same basic
     block, and target's block 2 contains no insn that could absorb an out2 reference
     (`addiu $s1/$s2,0x6C`, `addiu $s4,zero,0x12`, `lw $t0,0x18($sp)`,
     `addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`). The un-absorbed copy IS the residual
     `move $s3,$s6`.

**Consequence:** target's block-2 `addiu $s3,$s7,0x20` and target's seats cannot be held
simultaneously and byte-free on the goto chassis. The remaining live routes are (a) the
real-loop chassis, where the +1 comes from the loop-note dial and the residual is s19's
loop.c constant-hoist tax, and (b) making the Y2b/Y4 parameter copy free.

### s20 artifacts

`tmp/grind/func_80041188/s20/` — `apply.py`, `run.sh`, `dump20.sh`, `sweep.ps1`,
`odiff.py`, the variant bodies `CAND.c`/`Y0.c`/`Y1.c`/`Y2a.c`/`Y2b.c`/`Y3.c`/`Y4.c`/
`Y9.c`/`Z1.c`/`Z2.c`/`Z3.c`, and per-tag cc1 dump directories (`Y0/`, `Y1/`, `Y2a/`,
`Y2b/`, `Y3/`, `Y4/`, `Z1/`, `Z3/`) holding `red.i.*` pass dumps + `cc1.err` with the
ALLOCDBG tables.

- [s20] Chassis re-measured at session start with candidate.c applied: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.
- [s20] A block-0 do-while(0) wrap around out2's DEFINITION does lift out2 (3 -> 4 refs, 714 -> 1904) at zero insn cost — correcting the scope of s19's E-s19-6, which had only wrapped a cse1-deletable copy. It fails because the same wrap co-weights pa4 (7 -> 8 refs, 1473 -> 2526) and pa4 jumps above out2, tbl and i (Y1, sandbox 27).
- [s20] Writing the wrapped definition from the parameter `a4` while `s32 *pa4 = a4;` remains the first statement is inert (Y2a's ALLOCDBG table is byte-identical to Y1's): cse1 canonicalises a4 to pa4 inside block 0's EBB.
- [s20] Splitting the parameter copy (`s32 *pa4;` + `pa4 = a4;` AFTER the a4-sourced wrapped definition) keeps the pseudos distinct and produces ALL-TARGET seats together with target's block-2 addiu $s3,$s7,0x20 on the GOTO chassis for the first time (out2 4/43=1860 $s6, pa4 6/93=1290 $s7) — sandbox 12 at 133 insns, the single extra insn being `addu $s7,$v0,$zero`, because a4 becomes a block-0-local quantity that local_alloc seats in a call-clobbered register before global_alloc sees pa4.
- [s20] Y4 (the same split with only the tbl wrap left, stptr bought with the F1 chain-extender) also scores 12/133 with stptr 7/41=3414; dropping the tbl wrap too (Y9) scores 15. The stptr wrap is emission-neutral on this chassis; the tbl wrap is load-bearing for the tbl/out2 order.
- [s20] Back-deriving pa4 from out2 (`pa4 = (s32 *)((u8 *)out2 - 0x20);`) is reassociated to `pa4 = a4` by cse1 before flow counts anything — Z1 and Z2 measure identically (sandbox 23, out2 3 refs). E-s18-2 now covers the subtract direction and real, function-wide-consumed derivations.
- [s20] Z3 (wraps + `out3 = out2`) holds ALL-TARGET seats at 132 insns yet scores 8 vs candidate.c's 1 with the same seats: the LOOP notes displace `tbl++` past the argument setup and `sw $t0,0x18($sp)` in block 0. loop1 wraps are insn-count-neutral but not emission-order-neutral, so the do-while(0) family is not a cheaper substitute for the F1 chain-extender here.
- [s20] ENUMERATION: on the goto chassis with target's block-2 spelling, out2's required 4th flow-counted reference has exactly three possible sites (block 0 / loop1 / block 2) and all three are now measured or derived dead — so target's addiu $s3,$s7,0x20 and target's seats cannot be held simultaneously and byte-free on this chassis.

- [s20] Chassis re-measured at session start with candidate.c applied to src/text1a_pre.c: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns, rules_dropped 16, cheat_asm_stripped 2. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.

- [s20] s19's E-s19-6 rule ('a block-0 wrap weights nothing unless the enclosed reference survives cse1') is now correctly scoped: it is a statement about redundant COPIES. A block-0 wrap around out2's DEFINITION lifts out2 3 -> 4 refs at zero insn cost (Y1, 132 build insns).

- [s20] flow.c:2081's weighting is per-INSN, not per-symbol: wrapping out2's definition also lifts pa4 (7 -> 8 refs, 1473 -> 2526), which is why Y1 scores 27 despite out2 landing at exactly the required 1904.

- [s20] cse1 canonicalises the parameter a4 to the local copy pa4 inside block 0's EBB whenever `s32 *pa4 = a4;` precedes the use, so writing a wrapped definition from a4 is a measured no-op (Y2a's ALLOCDBG is byte-identical to Y1's).

- [s20] Writing `pa4 = a4;` as a statement AFTER an a4-sourced out2 definition keeps the two pseudos distinct and yields ALL-TARGET callee-saved seats together with target's block-2 addiu $s3,$s7,0x20 on the goto chassis (Y2b: out2 4/43=1860 $s6, pa4 6/93=1290 $s7, stptr 6/41=2926 $s3, i 8/98=2448 $s4, tbl 5/48=2083 $s5, stptr2 6/48=2500 $s0, a3 4/100=800 $fp) -- sandbox 12 at 133 insns.

- [s20] Y2b's single extra insn is `addu $s7,$v0,$zero`: target loads the parameter straight into its callee-saved seat (lw $s7,0x58($sp)), while the split makes a4 a block-0-local quantity that local_alloc seats in a call-clobbered register before global_alloc ever sees pa4; GCC 2.7 has no coalescing that merges a local quantity into a global allocno.

- [s20] Y4 (the same split with only the tbl wrap left and stptr bought with candidate.c's F1 chain-extender) also measures 12 at 133 insns with stptr 7/41=3414 -- the stptr wrap is emission-neutral on this chassis; Y9 (tbl wrap also dropped) costs the tbl/out2 order at 15.

- [s20] Back-deriving one matrix pointer from the other (`pa4 = (s32 *)((u8 *)out2 - 0x20);`) is reassociated to `pa4 = a4` by cse1 before flow counts anything: Z1 and Z2 measure identically (sandbox 23, out2 3 refs / live 43). E-s18-2 therefore holds in the subtract direction too.

- [s20] Z3 (wraps + `out3 = out2;`) holds ALL-TARGET seats at 132 insns yet scores 8 where candidate.c scores 1 with the same seats: loop1 do-while(0) wraps are insn-count-neutral but NOT sched1-emission-order-neutral (tbl++ and sw $t0,0x18($sp) displaced), so the wrap family is not a cheaper substitute for the F1 chain-extender on this chassis.

- [s20] ENUMERATION (E-s20-5): on the goto chassis with target's block-2 spelling, out2's required 4th flow-counted reference has exactly three possible sites -- block 0, loop1, block 2 -- and all three are now measured or derived dead, closing the chassis for that spelling.

### E-s21-1 - deleting the `pa4` local REOPENS E-s20-5 site 1 (block 0)

s20 closed the block-0 site because the do-while(0) wrap around out2's definition
necessarily co-weights the matrix pointer, and on the pa4 chassis that pushed pa4
7 -> 8 refs / live 95 = 1473 -> 2526, above out2, tbl and i (Y1, sandbox 27).
s21 deleted the `pa4` local entirely and wrote the parameter `a4` at all six use
sites. The co-weighting still happens, but a4's live length is 190 (it spans both
loops, where pa4's was 95), so 8 refs give 1263 -- across the same floor_log2(8)=3
boundary, yet landing between out2 (1904) and a3 (808).

MEASURED (form N1 = s20's Y1 minus the pa4 local; `tmp/grind/func_80041188/s21/N1.c`):
stptr 6/41 = 2926 -> $s3, stptr2 6/48 = 2500 -> $s0, i 8/97 = 2474 -> $s4,
tbl 5/47 = 2127 -> $s5, out2 4/42 = 1904 -> $s6, a4 8/190 = 1263 -> $s7,
a3 4/99 = 808 -> $fp, out3 3/47 -> $s3, saved spilled. **ALL-TARGET callee-saved
seats WITH target's block-2 `addiu $s3,$s7,0x20`, at 132 build insns of 132,
sandbox 7.** The same construct on the pa4 chassis scored 27. Every one of the
seven remaining diffs is sched1 EMISSION ORDER; there is no allocation defect and
no insn-count defect left on this chassis.

### E-s21-2 - the pa4-free chassis is NOT a superset: candidate.c dies without pa4

Control CANDF (candidate.c verbatim, `s32 *pa4 = a4;` deleted, pa4 -> a4): a4 falls
to 6 refs / live 190 = 631, BELOW a3's 4/99 = 808, and the two swap $s7/$fp;
sandbox 1 -> 13 at 132 insns. The `pa4 = a4` copy insn is the 7th reference that
keeps the pointer above a3 on candidate.c's chassis. The pa4-free chassis only
works where `out3 = (s32 *)((u8 *)a4 + 0x20);` supplies an 8th reference. The two
chassis are disjoint, and "delete the redundant local" is a chassis switch, not a
cleanup.

### E-s21-3 - on the pa4-free chassis tbl needs no reference lift, only a later definition

tbl's live length is the distance from its defining insn to `tbl++` in loop1.
Demoting `s32 *tbl = D_80094CFC;` to `s32 *tbl;` plus a plain `tbl = D_80094CFC;`
as the LAST statement of block 0 shortens it 47 -> 41, lifting tbl 1702 -> 1951 at
its natural 4 references -- clearing out2's 1860 with no wrap on tbl at all.
Form Q1 (`memory/grind/func_80041188/alt_Q1_pa4free_alltarget_s21.c`) measures
sandbox 7 / 132 insns / ALL-TARGET seats with exactly two constructs: the block-0
out2 wrap and candidate.c's F1 stptr chain-extender. Position ladder (all 132
insns, seats correct until noted): last statement 7, after `stptr = base;` 8,
after the out2 wrap 8, after `saved` 11 (tbl live 45, priority under out2),
declaration initialiser 11. With the tbl assignment itself wrapped the same ladder
reads 13 / 8 / 8 / 8 / 7 (N4 / N5 / P3 / P4 / P5) -- i.e. the wrap on tbl buys
nothing anywhere the plain assignment already works.

### E-s21-4 - the whole residual is one positional dilemma on tbl's definition

target's first four block-0 insns are `addiu $s4,zero,1`, `sw $s5,0x34($sp)`,
`lui $s5,%hi(D_80094CFC)`, `addiu $s5,$s5,%lo(D_80094CFC)` -- so in the original
source tbl's definition is early (a declaration initialiser, right after `i = 1`).
sched1 cannot lift a late-LUID block-0 insn with no in-block successors to the top,
so a late tbl definition emits its lui/addiu (and the dependent `sw $s5` prologue
save that must precede the clobber) ~7 slots after target's position: that trio
plus `sw $t0,0x18($sp)` two slots early is Q1's entire 7-diff residual.
But an EARLY tbl definition means live 47, where 4 references give 1702, under
out2's 1860/1904 -- so tbl then needs a FIFTH reference, and per E-s18-3 the
loop-note dial is the only construct in GCC 2.7.2 that yields an odd reference
delta, so a second do-while(0) wrap is required, and its notes cost emission order
elsewhere. Measured horns: early tbl + loop1 wrap on `offset = (*tbl) * 6;`
(N1 / N3) = 7, of which 1 diff is block 0 (`sw $t0,0x18($sp)`) and ~6 are loop1
(tbl++ hoisted past the `addiu $a0,$sp,0x10` / `addu $a1,$s7,$zero` argument setup,
`addu $s0,$s0,$s2` dragged with it); the wrap moved onto `tbl++` materialises an
insn (N9, 133, sandbox 21); one wrap enclosing BOTH definitions T3 = 8, T1 = 12,
T4 = 13; and eight block-0 statement permutations of Q1 (R1..R8) score 8..14, none
better than Q1's order.

### E-s21-5 - deleted-by-flow is uncounted; only deleted-by-combine is counted

The obvious byte-free escape for E-s20-5's site 3 is a dead store: write
`out3 = out2;` immediately before `out3 = (s32 *)((u8 *)a4 + 0x20);`. Measured
(form S1): the build stays at 132 insns -- the store really is byte-free -- but
out2 stays at **3 refs / live 42 = 714** and the seats collapse (a3 -> $s6,
a4 -> $s7, out2 -> $fp), sandbox 10. flow.c's dead-code elimination runs INSIDE
life analysis (propagate_block deletes the insn during the same backward scan that
increments reg_n_refs), so a flow-deleted insn's registers are never counted, where
a combine-deleted insn's registers already have been. This is the operative law for
every remaining byte-free-reference hunt on this function: the reference must
survive into combine, not merely disappear.

### s21 artifacts

`tmp/grind/func_80041188/s21/` -- `apply.py`, `dump20.sh`, `sweep.ps1`, `odiff.py`,
`text1a_pre.c.orig`, the variant bodies `CAND.c` / `CANDF.c` / `N1..N9.c` /
`P3..P5.c` / `Q1..Q4.c` / `R1..R8.c` / `S1.c` / `T1,T3,T4.c`, and per-tag cc1 dump
directories (`N1/`, `N5/`, `P5/`, `S1/`, `CANDF/`) holding `red.i.*` pass dumps +
`cc1.err` with the ALLOCDBG tables.

- [s21] Chassis re-measured at session start with candidate.c applied to src/text1a_pre.c: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.
- [s21] Deleting the `pa4` local (writing the parameter a4 at all use sites) REOPENS E-s20-5's block-0 site: the do-while(0) wrap around out2's definition still co-weights the matrix pointer, but a4's live length is 190 rather than pa4's 95, so 8 refs = 1263 lands between out2 (1904) and a3 (808) instead of above everything. Form N1 = ALL-TARGET seats WITH target's block-2 addiu $s3,$s7,0x20 at 132 insns, sandbox 7 (the same construct scored 27 on the pa4 chassis).
- [s21] The two chassis are disjoint: candidate.c with pa4 deleted (CANDF) drops a4 to 6/190 = 631, below a3's 808, swapping $s7/$fp -- sandbox 1 -> 13. The pa4-free chassis works only where out3 is defined from a4, that definition being a4's 8th reference.
- [s21] On the pa4-free chassis tbl needs no reference lift: demoting its declaration initialiser to a plain assignment as the LAST statement of block 0 shortens live 47 -> 41 and lifts it 1702 -> 1951, clearing out2's 1860. Form Q1 = ALL-TARGET seats, 132 insns, sandbox 7, with only two constructs (block-0 out2 wrap + the F1 stptr chain-extender).
- [s21] The residual is now a single positional dilemma: target's tbl definition is EARLY (its lui/addiu are block 0's insns 3-4), but an early definition means live 47, where 4 refs = 1702 sits under out2, so tbl would need a fifth (odd -> loop-note-only) reference whose notes then cost emission order. Early-tbl forms pay ~6 loop1 order diffs, late-tbl forms pay 4 block-0 positions; both horns and eight block-0 permutations measure 7..14, none below 7.
- [s21] A dead store `out3 = out2;` before the out3 definition is byte-free (132 insns) but flow-INVISIBLE: out2 stays at 3 refs. flow.c deletes dead insns inside life analysis, so their registers are never counted -- unlike combine deletions, which happen after counting. Every byte-free-reference spelling on this function must survive into combine.

- [s21] Chassis re-measured at session start with candidate.c applied to src/text1a_pre.c: sandbox func_80041188 --disable all = score 1, 132 target / 132 build insns. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.

- [s21] Form N1 (pa4 local deleted, block-0 out2 wrap, tbl loop1 wrap, F1 stptr): ALL-TARGET callee-saved seats WITH target's block-2 addiu $s3,$s7,0x20, 132 of 132 insns, sandbox 7 -- the same construct scored 27 on the pa4 chassis (s20 Y1).

- [s21] a4's live length is 190 on the pa4-free chassis versus pa4's 95, which is why the block-0 wrap's unavoidable second +1 (8 refs, floor_log2 = 3) yields 1263 and lands harmlessly between out2 (1904) and a3 (808) instead of above everything at 2526.

- [s21] Form Q1 (memory/grind/func_80041188/alt_Q1_pa4free_alltarget_s21.c): same seats and same 132 insns at sandbox 7 with ONE fewer construct -- tbl's definition demoted to the last statement of block 0 shortens its live range 47 -> 41 and lifts it 1702 -> 1951, so no tbl wrap is needed.

- [s21] tbl definition-position ladder on the pa4-free chassis (all 132 insns): last statement 7, after `stptr = base;` 8, after the out2 wrap 8, after `saved` 11, declaration initialiser 11 -- the last two because tbl's live length reaches 45..47 and its priority falls under out2's.

- [s21] Target's block 0 opens addiu $s4,zero,1 / sw $s5,0x34($sp) / lui $s5,%hi(D_80094CFC) / addiu $s5,$s5,%lo(D_80094CFC), so the original source defines tbl early -- which on this chassis costs tbl the priority it needs, giving the dilemma that is now the entire residual.

- [s21] A dead store `out3 = out2;` is byte-free (132 insns) yet leaves out2 at 3 refs: flow.c deletes dead insns inside life analysis, so their registers are never counted, unlike combine deletions which happen after counting. Every remaining byte-free-reference spelling must survive into combine.

- [s21] candidate.c does NOT survive deletion of its pa4 local (a4 6/190 = 631 falls below a3's 808; sandbox 1 -> 13), so the pa4-free chassis is a switch, not a cleanup, and the two chassis are disjoint.

- [s21] Eight block-0 statement permutations of Q1 (R1..R8) score 8..14 -- the order saved / out2 / stptr / tbl is already optimal on that chassis.

- [s21] A wrap around `tbl++` instead of the tbl read materialises an insn (133 build insns, sandbox 21); a single wrap enclosing both the tbl and out2 definitions scores 8 (T3) / 12 (T1) / 13 (T4).

## s22 (synthesis, 2026-08-25) — the real-loop chassis is FORECLOSED by exact loop.c arithmetic, and the F1 chain-extender is closed for out2 by a derived law

Chassis re-measured at the START of s22: `memory/grind/func_80041188/candidate.c`
remains the floor at **sandbox 1**; this session worked the two ALTERNATE chassis
(s19's real loop, s21's pa4-free goto) and re-measured both controls — `W4` = **3**
at 132/132 and `Q1`/A0 = **7** at 132/132, both reproducing their banked tables
insn-for-insn and allocno-for-allocno. `src/text1a_pre.c` was restored to HEAD at the
end of the session; no build-pipeline file was touched. The floor did not move. What
moved is the size of the live search space: one whole chassis and one whole construct
family are now closed with named mechanisms and exact numbers.

### E-s22-1 — the real-loop chassis (s19 W4/W6/W8, s21 frontier item 3) is FORECLOSED: loop.c's hoist of the invariant `li 2` is unconditional here, by arithmetic

s19's E-s19-3 asserted that `move_movables`' profitability test "passes for ANY
lifetime once the loop has calls". That is imprecise, and the precise form is what
closes the chassis. The loop dump prints the decision verbatim
(`tmp/grind/func_80041188/s22/W4/red.i.loop:109-111`):

```
Loop from 42 to 172: 48 real insns.
Insn 155: regno 122 (life 1), move-insn savings 1  moved to 315
```

so `insn_count = 48`, `m->lifetime = 1`, `savings = 1`. The test at loop.c:1631 is
`threshold * savings * m->lifetime >= insn_count`, with
`threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532).
`n_non_fixed_regs` is counted in regclass.c:380-387 over `FIXED_REGISTERS`, which for
this target (mips.h:1188) holds **8** ones out of `FIRST_PSEUDO_REGISTER = 68`
(mips.h:1181) — so `n_non_fixed_regs = 60` and **threshold = 61**. 61 * 1 * 1 = 61 >=
48. The margin is 13 and `insn_count` (the loop's real-insn count) is the ONLY term a
C spelling can move: blocking the hoist would require thirteen more real instructions
inside loop1, which is by definition not byte-free.

The three eligibility tests at loop.c:686-700 are an OR, and each passes
INDEPENDENTLY here, so no single spelling change disqualifies the movable:
  (1) `! maybe_never && ! loop_reg_used_before_p` — loop1's only jump is its trailing
      conditional, so `maybe_never` is 0 at the `li`;
  (2) `! REG_USERVAR_P && ! REG_LOOP_TEST_P` — true for a compiler pseudo;
  (3) `reg_in_basic_block_p` — loop1's body is a single basic block (calls do not end
      a basic block in GCC 2.7.2), so the `li`/`sh` pair lives entirely inside it.
**Measured control W4b** (`{ s16 two = 2; *((s16 *)(ents + i*0x68 + 6)) = two; }`,
which falsifies test (2) and nothing else) = **sandbox 3 at 132/132, byte-identical to
W4**. Banked: `rejected/realloop-const-user-variable-still-hoisted-threshold-61.c`.

The hoisted pseudo (122) is then live across every call in the loop, so it cannot take
a call-clobbered hard register and taking a callee-saved one would cost a save/restore
pair; global.c spills it (3 refs / live 92, hardreg -1) and reload rematerialises the
constant into `$t0` where target's in-place, local-alloc-owned `li` gets `$v0`
(`s22/W4/red.s:258-259` — `li $8,2` / `sh $8,6($19)`). That is W4's +2, in full.

**Consequence.** Any form of this function in which loop1 is a real C loop starts two
diffs behind, permanently. s19's W4 = 3 decomposes as 2 (this tax) + 1 (the out3 lock),
so even a perfect solution of the out3 lock leaves the real-loop chassis at 2, against
candidate.c's 1. s21's frontier item 3 ("rebuild W4 with the pa4 local deleted") cannot
reach 0 and is withdrawn. The loop-note reference dial (E-s18-3) is therefore only ever
available at a cost of 2 on loop1, or as an isolated `do { } while (0)` wrap.

### E-s22-2 — the F1 split-init chain-extender is closed for `out2` on EVERY chassis, by a derived iff-law

Form A1 = s21's Q1 (pa4-free goto chassis) with the block-0 `do { out2 = ...; } while (0)`
FAKE wrap replaced by an honest-looking split-init chain sourced from the parameter:
`out2 = (s32 *) a4; out2 = (s32 *)(((u8 *) out2) + 0x20);`. **Measured: sandbox 10 at
132 of 132 build insns** — the chain is genuinely byte-free on this chassis (unlike the
pa4-chassis attempt banked as `rejected/out2-chain-from-a4-survives-cse-but-materializes-pa4-move.c`,
which materialised a copy) — **but out2 stays at 3 refs / live 42 = 714** and is seated
`$fp`, with a3 4/99 = 808 in `$s6` and a4 7/190 = 736 in `$s7`.

The dumps name the mechanism exactly. In `s22/A1/red.i.cse` insns 34 and 37 are still
present but insn 37's source has been rewritten from `(plus (reg 85) 32)` to
`(plus (reg 76) 32)`: reg 76 is the parameter a4, which OUTLIVES out2, so by the s17
make_regs_eqv law (cse.c:844-857) a4 — not out2 — is the canonical register of the
equivalence class. That leaves insn 34 (`out2 = a4`) dead, and it is **absent from
`s22/A1/red.i.flow`**: per E-s21-5, flow.c deletes dead insns inside life analysis, so
its registers are never counted.

Stating it as a law, which is what makes this a closure rather than one more measured
case: **an F1 split-init chain-extender delivers its +1 flow-counted reference if and
only if the RECIPIENT outlives the DONOR.** `stptr = base; stptr += 0xFC;` works
because `base` dies at that point, so stptr is canonical, the chain survives cse1, and
combine (which runs after flow's counting) merges it. Nothing sourced from the matrix
pointer can ever work for out2, on any chassis, because the matrix pointer outlives
out2 by construction — and it is the only block-0 quantity out2 can honestly be derived
from (E-s20-3 already closed the reverse direction). No further spelling of this idea
should be measured. Banked:
`rejected/out2-chain-from-param-cse-canonicalises-flow-deletes.c`.

### E-s22-3 — s21's frontier item 2 (give tbl's definition an in-block consumer) is closed by enumeration of target's own block 0

s21 proposed rescuing the late-tbl horn by giving tbl's `lui`/`addiu` pair an in-block
successor, so sched.c's `rank_for_schedule` would lift it on INSN_PRIORITY instead of
sinking it on INSN_LUID. That requires a block-0 insn target ALREADY emits to be
legitimately sourced from tbl. Target's block 0 (asm/funcs/func_80041188.s:2-28) is
exhaustively: the frame `addiu $sp`; nine callee-saved `sw`s plus `sw $ra`; the
parameter homing copies `addu $s1,$a1` / `addu $s2,$a2` / `addu $fp,$a3`;
`addiu $s4,zero,1` (i); the tbl `lui`/`addiu` pair itself; `sll $a0,$a0,2` plus
`lui $at` / `addu $at,$a0` / `lw $v0,%lo(D_800A9A10)($at)` (base);
`lw $s7,0x58($sp)` (a4); `addiu $t0,$v0,0x94` (ents); `addiu $s6,$s7,0x20` (out2);
`addiu $s3,$v0,0xFC` (stptr); `sw $t0,0x18($sp)` (the ents spill). **Not one of these
values is a function of `D_80094CFC`** — every one derives from a parameter, from the
`D_800A9A10[a0]` load, or from a literal. There is therefore no zero-insn in-block
consumer for tbl, and the late-tbl horn's 4-position emission-order cost is structural.
Closed by derivation; do not spend a measurement on it.

### E-s22-4 — the A1 table is the first measurement of the "everything natural" allocno vector, and it localises the whole problem to two adjacent priorities

A1 (no wrap, no tbl trick, out2 at its natural 3 references) measures
stptr 7/41 = 3414 `$s3`, stptr2 6/48 = 2500 `$s0`, i 8/97 = 2474 `$s4`,
tbl 4/47 = 1702 `$s5`, a3 4/99 = 808 `$s6`, a4 7/190 = 736 `$s7`,
out2 3/42 = 714 `$fp`, out3 3/47 `$s3`. **Four of the seven contested callee-saved
seats are already target's with zero constructs**, tbl sits at its EARLY-definition
priority (1702, the value s21's dilemma wants), and the only defect is that out2 (714)
sits below a3 (808) instead of above it. Every quantised repair of that one gap needs
out2 at 4 references — 4 refs at live 42..47 gives 1904..1702, which then also demands
that tbl be lifted above it (the s21 dilemma) — or a3 pushed below 714, which needs a3's
live length above 113 against a measured 99. The function's entire remaining residual,
on every chassis that is still alive, is **one flow-counted, byte-free reference on
out2**, and s22 has now removed the last untried honest delivery mechanism for it.

### E-s22-5 — the merged map of the whole function after 22 sessions

Three chassis have been carried in parallel; s22 reduces them to one.
  * **goto chassis with `out3 = out2` (candidate.c)** — sandbox **1**. Its 4th out2
    reference IS an emitted `move $s3,$s6`, and that emitted move IS the single residual
    diff against target's `addiu $s3,$s7,0x20`. Needs the `stptr = base; stptr += 0xFC;`
    F1 chain-extender (which E-s22-2 now explains as legitimate-by-mechanism: `base`
    dies, so the chain survives cse1 and is merged by combine).
  * **goto chassis with target's block-2 spelling** — needs out2's 4th reference from
    somewhere else. Enumerated dead on the pa4 chassis (E-s20-5); reachable on the
    pa4-free chassis only via the block-0 `do { } while (0)` wrap (s21 N1/Q1, sandbox 7),
    which is a FAKE construct and drags the tbl positional dilemma (E-s21-4) behind it;
    and the honest chain-extender substitute is now closed (E-s22-2).
  * **real-loop chassis** — FORECLOSED (E-s22-1).
So the endgame question is singular and unchanged in shape but much sharper in
statement: **is there a C construct that puts a fourth flow-counted reference on out2,
which (a) survives cse1 — i.e. is not a copy whose destination dies before its source —
and (b) is removed by COMBINE rather than by flow.c or by nothing at all?** Every
family tried to date fails exactly one of those two clauses: dead stores and copies fail
(a)/(b) by flow deletion (E-s21-5, E-s22-2); `out3 = out2` satisfies both clauses'
letter but is never deleted, so it costs the residual insn; the do-while(0) wrap
sidesteps the clauses entirely by re-weighting rather than re-counting, at the price of
being a FAKE construct plus emission-order damage.

### s22 artifacts

`tmp/grind/func_80041188/s22/` — `apply.py`, `dump.sh`, `sweep.ps1`, `odiff.py`,
`text1a_pre.c.orig`, the variant bodies `W4.c` / `W4b.c` / `Q1.c` / `A0.c` / `A1.c` /
`A2.c`, and the per-tag cc1 dump directories `W4/` and `A1/` holding the full `red.i.*`
pass dumps (including `red.i.loop` with the movable decision and `red.i.cse` /
`red.i.flow` with the canonicalisation) plus `cc1.err` with the ALLOCDBG tables.

- [s22] Chassis re-measured at session start: candidate.c is still the floor at sandbox 1; the two alternate chassis reproduce exactly (W4 = 3 at 132/132, Q1/A0 = 7 at 132/132). src/text1a_pre.c restored to HEAD; no build-pipeline file touched.
- [s22] The real-loop chassis is FORECLOSED. loop.c's hoist of loop1's invariant `li 2` is decided by threshold * savings * lifetime >= insn_count = 61 * 1 * 1 >= 48; threshold = 1 * (1 + n_non_fixed_regs) with n_non_fixed_regs = 60 (mips.h FIXED_REGISTERS has 8 ones, FIRST_PSEUDO_REGISTER = 68). insn_count is the only C-side term and the margin is 13 real insns, so no byte-free spelling blocks the hoist.
- [s22] The three scan_loop eligibility tests (loop.c:686-700) are an OR and each passes independently here; W4b (the stored value made a user variable inside the loop) measures 3 at 132/132, byte-identical to W4, confirming test (2) is not load-bearing.
- [s22] Because the hoisted pseudo is live across every call in loop1 it cannot take a call-clobbered hard reg and a callee-saved one would cost a save/restore pair, so global.c spills it and reload rematerialises `li 2` into $t0 where target's local-alloc-owned in-place `li` gets $v0. The real-loop +2 is structural; s19's W4 = 2 (tax) + 1 (out3 lock); s21's frontier item 3 cannot reach 0 and is withdrawn.
- [s22] Form A1 (pa4-free chassis, block-0 wrap replaced by `out2 = (s32 *) a4; out2 = (s32 *)((u8 *) out2 + 0x20);`) is byte-free at 132/132 but leaves out2 at 3 refs / live 42 = 714 -- sandbox 10. A2 (same plus tbl restored to a declaration initialiser) also 10.
- [s22] LAW: an F1 split-init chain-extender delivers its +1 flow-counted reference IFF the recipient outlives the donor. cse1 canonicalises the class to the longer-lived register (make_regs_eqv, cse.c:844-857), which for out2 is always the matrix pointer, so the copy dies and flow.c deletes it inside life analysis before counting (E-s21-5). Read directly out of s22/A1/red.i.cse (insn 37 rewritten to `(plus (reg 76) 32)`) and red.i.flow (insn 34 absent). The chain-extender family is closed for out2 on EVERY chassis, by derivation.
- [s22] s21 frontier item 2 is closed by enumeration: target's block 0 contains no value that is a function of D_80094CFC, so tbl's lui/addiu pair can have no zero-insn in-block consumer and cannot be lifted by sched.c's INSN_PRIORITY ranking; the late-tbl horn's emission-order cost is structural.
- [s22] A1 is the first "all natural" allocno vector: stptr 7/41=3414 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 4/47=1702 $s5, a3 4/99=808 $s6, a4 7/190=736 $s7, out2 3/42=714 $fp. Four contested callee-saved seats are already target's with zero constructs, tbl already sits at its early-definition priority, and the single defect is out2 (714) below a3 (808).
- [s22] E-s22-2 also supplies the mechanistic defence of candidate.c's `stptr = base; stptr += 0xFC;`: it is not an arbitrary reference pump but the only spelling whose donor (`base`) dies at the split point, which is precisely why it survives cse1 and is merged by combine. That is a testable structural property, not a coincidence of search.

- [s22] Chassis re-measured this session: candidate.c = sandbox 1 (floor unchanged); the two alternate chassis reproduce exactly — W4 (real loop) = 3 at 132/132, Q1/A0 (pa4-free goto) = 7 at 132/132. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.

- [s22] loop.c's hoist of loop1's invariant `li 2` is unconditional here by arithmetic: insn_count = 48, m->lifetime = 1, savings = 1 (printed verbatim in s22/W4/red.i.loop:109-111), against threshold = 1 * (1 + 60) = 61 derived from mips.h:1188 FIXED_REGISTERS (8 ones) and mips.h:1181 FIRST_PSEUDO_REGISTER = 68. insn_count is the only C-side dial and the margin is 13 real insns.

- [s22] The three scan_loop movable-eligibility tests (loop.c:686-700) are an OR and each passes independently on this loop; W4b (the stored constant made a user variable, falsifying test (2) only) measures 3 at 132/132, byte-identical to W4.

- [s22] The hoisted constant pseudo is live across every call in loop1, so it must spill; reload rematerialises `li 2` into $t0 where target's in-place local quantity gets $v0. The real-loop +2 is therefore structural, s19's W4 = 2 (tax) + 1 (out3 lock), and the WHOLE real-loop chassis is foreclosed for a byte-exact match — it cannot beat candidate.c's 1, let alone reach 0.

- [s22] LAW (s22 E-s22-2): an F1 split-init chain-extender delivers its +1 flow-counted reference IF AND ONLY IF the recipient outlives the donor. cse1 canonicalises an equivalence class to the longer-lived register (make_regs_eqv, cse.c:844-857); if the donor wins, the copy goes dead and flow.c deletes it inside life analysis, before counting. Read directly out of s22/A1/red.i.cse (insn 37 rewritten to (plus (reg 76) 32)) and red.i.flow (insn 34 absent).

- [s22] Corollary: nothing sourced from the matrix pointer can ever pay out2 a reference on ANY chassis, because the matrix pointer outlives out2 by construction — and E-s20-3 already closed the reverse derivation direction. The chain-extender family is closed for out2 by derivation, not by cases.

- [s22] Corollary 2 (the defence of candidate.c's construct): `stptr = base; stptr += 0xFC;` pays its reference precisely BECAUSE `base` dies at the split point, which makes stptr the canonical register, the chain survive cse1, and combine merge the pair back into target's single `addiu $s3,$v0,0xFC`. It is a structural property with a named pass at each end, not a coincidence of search.

- [s22] s21 frontier item 2 closed by enumeration: no insn in target's block 0 is a function of D_80094CFC, so tbl's lui/addiu pair can have no zero-insn in-block consumer and sched.c will always sink a late-LUID tbl definition. The late-tbl horn's 4-position emission-order cost is structural.

- [s22] A1's 'everything natural' table (first ever measured): stptr 7/41=3414 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 4/47=1702 $s5, a3 4/99=808 $s6, a4 7/190=736 $s7, out2 3/42=714 $fp. Four contested seats already target's, zero constructs, and one adjacent-priority defect.

- [s22] MERGED MAP after 22 sessions — three chassis reduce to one live chassis and one live atom. out2 needs a fourth flow-counted reference that is byte-free; flow.c deletes dead insns inside life analysis, so 'byte-free by being dead' is self-defeating; combine runs after flow's counting, so the ONLY viable shape is an insn that is live and useful through cse1 and flow and that COMBINE then merges into a consumer. `out3 = out2` is live but never merged (it IS the residual insn); dead stores and copy chains are flow-deleted; the do-while(0) wrap does not add a reference at all, it re-weights an existing one, and is FAKE plus emission-order-costly.

## s23 (synthesis, 2026-08-25) — the cse1 EBB law: out2's fourth reference may not live in block 2 OR in loop1, and a3 is not a dial

Chassis re-measured at the START of s23: HEAD (`INCLUDE_ASM`-era committed body) scores
**27**; `memory/grind/func_80041188/candidate.c` applied to `src/text1a_pre.c` re-measures
**sandbox 1 at 132 target / 132 build insns** — the floor is unchanged and chassis-valid.
`src/text1a_pre.c` was restored to HEAD at the end of the session; no build-pipeline file
was touched. The floor did not move. What moved is that the s22 frontier's item 2 is dead,
item 1 has been redirected to a different set of insns, and a new pass-level law now
partitions the whole remaining search space.

### E-s23-1 — THE CSE-EBB LAW: which block a value is computed in decides whether cse1 can fold it into block 2

`cse.c`'s `cse_end_of_basic_block` builds an extended basic block by following
single-predecessor successors. In this function the CFG is: block 0 -> loop1's body
(TWO predecessors: block 0's fall-through and the `goto loop1` back edge) -> block 2 (ONE
predecessor) -> loop2's body (two predecessors). Therefore:
  * the EBB that starts at **block 0 TERMINATES at loop1's head**, so nothing computed in
    block 0 is in cse1's table when block 2 is processed;
  * the EBB that starts at **loop1's body EXTENDS THROUGH block 2**, so everything
    computed inside loop1 IS in cse1's table when block 2 is processed.

Two measured controls, both on the `pa4` chassis with target's block-2 spelling
`out3 = (s32 *)(((u8 *) pa4) + 0x20);`:
  * **D5/D6** — out2's definition moved INSIDE loop1 (D5 just before its first use, D6 at
    the top of the body). `s23/D5/red.i.flow` insn 167 is `(set (reg/v:SI 87) (reg/v:SI 86))`:
    cse1 rewrote out3's definition to `out3 = out2`. Sandbox **23** at 132/132; out2
    measures **4 refs / live 21 = 3809** (the 4th reference IS the fold), which then
    outranks tbl.
  * **G1** — out2 defined normally in block 0 AND recomputed identically at the end of
    loop1's body. `s23/G1/red.i.cse` insn 170 is again `(set (reg/v:SI 87) (reg/v:SI 86))`.
    Sandbox **12 at 133 of 132 insns** (the recompute materialises), out2 5 refs / 42 = 2380.
  * The negative control is the whole measured B0/C0/C3/C4 family: with out2 defined ONLY in
    block 0, out3's `(plus (reg a4) 32)` is NOT folded (out2 stays at 3 references) and
    target's `addiu $s3,$s7,0x20` is emitted.

**The law, and it is a DEFINITION law, not a reference law:** an in-loop1 *definition* of
out2's value enables cse1's fold in block 2; an in-loop1 *use* of out2 does not (a use puts
no expression in cse's table).

**Consequence — the site partition for out2's missing 4th flow-counted reference is now
three-way, and two thirds of it is dead:**
  1. **In block 2** — E-s20-5 already enumerated this dead on the pa4 chassis; the cse law
     now explains WHY, generally: any block-2 recomputation of `a4 + 0x20` is folded to
     `out2`, which is precisely the residual `move $s3,$s6`.
  2. **Inside loop1, as a DEFINITION** — dead by this law: it forces the same fold and so
     destroys target's block-2 `addiu` even when it is byte-free (D5/D6).
  3. **Inside loop1 as a USE, or anywhere in block 0** — the only two sites compatible with
     target's block-2 spelling. Block 0's byte-free deliveries are already enumerated: the
     `do { } while (0)` loop-note wrap (FAKE; s21 Q1 = 7, with the tbl positional dilemma)
     and nothing else, since E-s22-2's iff-law closed the chain-extender and E-s21-5 closed
     everything flow deletes. **The in-loop1 USE is the one site no session has probed.**

### E-s23-2 — a3's allocno is RIGID: 4 references / live length 99 in ten measured forms

s22's frontier item 2 ("push a3 down instead of lifting out2") is closed. a3 is measured at
exactly **4 refs / live 99 / pri 808** in every one of B0, C0, C3, C4, D1, D2, D5, D6, F2
and F1 — including:
  * **D1** — loop2's `*((s16 *)(stptr2 + 6)) = 1;` moved BEFORE the `func_800523E0` that is
    a3's last use, and **D2** — `i++` moved before it. Both were built specifically to
    lengthen a3's live range past its last consumer. Both leave a3 at 99 exactly
    (D1 sandbox 15, D2 sandbox 13).
  * **F1/F2** — an explicit `s32 pa3 = a3;` homing-copy local used at all three call sites,
    i.e. the exact analogue of the `pa4` local that halves a4's live length (190 -> 95) and
    lifts it 736 -> 1263. a3 is unmoved at 4/99 = 808 on both chassis; the copy's only
    visible effect is a1/a2 live 99 -> 100 and the schedule shift that follows (F1 = 5 on
    the candidate chassis, F2 = 15 on the pa4-free chassis).
The arithmetic the frontier item rested on needed a3's live length at >= 109 (to fall below
a4's 736) or >= 114 (below out2's 714) — a +10 to +15 move, against a dial that does not
move by 1. Banked: `rejected/a3-homing-copy-local-livelen-99-rigid.c`.

### E-s23-3 — THE RATIO LAW closes the block-0 position sweep on the pa4-free chassis

On the pa4-free goto chassis out2 has 3 references and a4 has 7, so `floor_log2` gives out2
`30000/L(out2)` and a4 `140000/L(a4)`, and **out2 outranks a4 iff L(a4) > 4.667 x L(out2)**.
The full block-0 position sweep of out2's definition was measured (ALLOCDBG in
`s23/{C0,B0,C3,C4}/cc1.err`):

| form | out2 def position | out2 | a4 | ratio | sandbox |
|---|---|---|---|---|---|
| C0 | first statement of block 0 | 3/43 = 697 | 7/192 = 729 | 4.465 | 13 |
| B0 | natural (after `saved`) | 3/42 = 714 | 7/190 = 736 | 4.524 | 10 |
| C3 | last statement of block 0 | 3/41 = 731 | 7/188 = 744 | 4.585 | 13 |
| C4 | after `tbl = D_80094CFC;` | 3/41 = 731 | 7/188 = 744 | 4.585 | 13 |

Moving the definition later shortens out2's live range AND a4's, in the same direction, so
the ratio creeps toward 4.667 but the sweep is monotone and exhausted at 4.585. No block-0
position of out2's definition seats it above a4 on the pa4-free chassis. Banked:
`rejected/out2-block0-position-sweep-ratio-below-4.667.c`.

### E-s23-4 — the short-live-length window is real, reachable, and structurally unusable

The priority arithmetic admits a SECOND solution that no session had noticed: on the `pa4`
chassis (where a4 = 6/95 = 1263 and tbl = 4/47 = 1702) out2 seats correctly at **3
references and live length 18..23** (30000/L inside (1263,1702)) — with no fourth reference
at all. That window is physically reachable: D5/D6 measure out2 at live **21**, at zero insn
cost (132/132). It is nevertheless unusable, for two independent reasons, both proven this
session: (i) live 21 requires a definition INSIDE loop1, which triggers E-s23-1's fold and
hands out2 a 4th reference it does not want (3809, above tbl); and (ii) an in-loop1
definition emits out2's `addiu` inside loop1, while target emits `addiu $s6,$s7,0x20` in
block 0. Banked: `rejected/out2-inloop-def-cse-ebb-folds-out3-to-move.c`.

### E-s23-5 — the merged attack after 23 sessions

Everything collapses into one sentence. **out2 needs a fourth flow-counted reference; that
reference must be (a) an insn that survives cse1 and is present when flow.c counts, (b)
absorbed by combine rather than deleted by flow (E-s21-5), and (c) sited either in BLOCK 0
or as a USE inside loop1 — because a definition in loop1, or any recomputation in block 2,
is folded by cse1 into the residual `move` (E-s23-1).** Block 0's deliveries are enumerated
and only the FAKE `do { } while (0)` wrap survives there. The untouched quadrant is
therefore **an in-loop1 USE of out2 that combine absorbs**. s22's frontier item 1 pointed
the combine-merge enumeration at target's BLOCK-2 insns; E-s23-1 proves that is the wrong
list. The right list is target's own loop1 insns: the `addiu $a0,$sp,0x10` buf-address
setup, the `addu $a1,...` argument moves, the `addiu $a2,$s3,0x38` / `addiu $a3,...`
argument computations for `func_800523E0`, the `sh` into `6($s3)`, and the
`addiu $s3,$s3,0x68` walk.

### s23 artifacts

`tmp/grind/func_80041188/s23/` — `apply.py`, `dump.sh`, `sweep.ps1`, `odiff.py`,
`text1a_pre.c.orig`, the variant bodies `CAND.c` / `A1.c` / `B0.c` / `C0.c` / `C3.c` /
`C4.c` / `D1.c` / `D2.c` / `D5.c` / `D6.c` / `F1.c` / `F2.c` / `G1.c`, and the per-tag cc1
dump directories `B0/` `C0/` `C3/` `C4/` `D1/` `D2/` `D5/` `D6/` `F1/` `F2/` `G1/` holding
the full `red.i.*` pass dumps (`red.i.cse` and `red.i.flow` carry the fold evidence) plus
`cc1.err` with the ALLOCDBG tables.

- [s23] Chassis re-measured at session start: HEAD = 27; candidate.c applied = sandbox 1 at 132/132. Floor unchanged. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched.
- [s23] CSE-EBB LAW: block 0's cse1 extended basic block terminates at loop1's head (two predecessors), while loop1's EBB extends through block 2 (one predecessor). So a value computed in block 0 is invisible to cse1 in block 2, and a value computed inside loop1 is visible there and gets folded. Measured in both directions: D5/D6 (out2 defined in loop1) rewrite block 2's `out3 = (u8*)pa4+0x20` to `out3 = out2` (s23/D5/red.i.flow insn 167); G1 (out2 defined in block 0 AND recomputed in loop1) does the same (s23/G1/red.i.cse insn 170); B0/C0/C3/C4 (block-0 definition only) do not fold and keep target's addiu.
- [s23] It is a DEFINITION law, not a reference law: an in-loop1 definition of out2's value enables the fold; an in-loop1 USE of out2 does not, because a use records no expression in cse's table. That is what leaves the in-loop1-use quadrant alive.
- [s23] Consequence: out2's missing 4th flow-counted reference cannot be sited in block 2 (folded to the residual move — this generalises E-s20-5 from enumeration to mechanism) and cannot be an in-loop1 definition (same fold; D5/D6 sandbox 23 at 132/132). Only block 0 (enumerated — FAKE do-while(0) wrap only) and an in-loop1 USE remain. s22's frontier-1 combine-merge enumeration was aimed at target's block-2 insns and must be re-aimed at loop1's insns.
- [s23] a3 is NOT a dial. Measured 4 refs / live 99 / pri 808 in ten forms (B0, C0, C3, C4, D1, D2, D5, D6, F1, F2), including D1/D2 which move statements past a3's last use and F1/F2 which add `s32 pa3 = a3;` at all three call sites (the exact analogue of the pa4 local that halves a4's live length). The frontier needed +10 to +15 on a dial that does not move by 1. s22 frontier item 2 is KILLED.
- [s23] RATIO LAW on the pa4-free chassis: out2 (3 refs) outranks a4 (7 refs) iff L(a4) > 4.667*L(out2). Full block-0 position sweep of out2's definition: C0 4.465 (sandbox 13), B0 4.524 (10), C3/C4 4.585 (13). The sweep moves both live lengths the same way, is monotone, and is exhausted below 4.667.
- [s23] NEW SOLUTION WINDOW, found and closed in the same session: on the pa4 chassis out2 also seats correctly at 3 references with live length 18..23 (no fourth reference at all), and live 21 IS reachable at 132/132 via an in-loop1 definition. Unusable for two independent reasons — the definition triggers the cse fold (out2 becomes 4/21 = 3809, above tbl), and it emits out2's addiu inside loop1 where target emits it in block 0.
- [s23] A same-value recompute of out2 at the end of loop1 (the dead-store family, in a position no session had tried) is byte-COSTLY here: out2's block-0 value is not in cse1's table for loop1's EBB, so the insn is not recognised as redundant and survives to the output (G1 = sandbox 12 at 133 of 132 insns).

- [s23] Chassis re-measured at session start: HEAD (INCLUDE_ASM-era committed body) = sandbox 27; memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c = sandbox 1 at 132 target / 132 build insns. Floor unchanged. src/text1a_pre.c restored to HEAD at the end of the session; no build-pipeline file touched.

- [s23] CSE-EBB LAW (E-s23-1): cse.c's cse_end_of_basic_block extends an extended basic block through single-predecessor successors. loop1's head has two predecessors (block 0 fall-through + the `goto loop1` back edge), so the EBB starting at block 0 terminates there and nothing computed in block 0 is in cse1's table when block 2 is processed. Block 2 has one predecessor, so loop1's EBB extends through it and everything computed inside loop1 IS in the table there.

- [s23] The law is measured in both directions: D5/D6 (out2's definition moved inside loop1) rewrite block 2's `out3 = (u8*)pa4 + 0x20` into `out3 = out2` - s23/D5/red.i.flow insn 167 is `(set (reg/v:SI 87) (reg/v:SI 86))`; G1 (out2 defined in block 0 AND recomputed in loop1) does the same at s23/G1/red.i.cse insn 170; B0/C0/C3/C4 (block-0 definition only) do NOT fold and keep target's `addiu $s3,$s7,0x20`.

- [s23] It is a DEFINITION law, not a reference law: an in-loop1 definition of out2's value enables the fold, an in-loop1 USE does not, because a use records no expression in cse's hash table. That is what leaves the in-loop1-use quadrant alive.

- [s23] Consequence for the atom: out2's missing 4th flow-counted reference cannot be sited in block 2 (this generalises E-s20-5 from a per-chassis enumeration to a mechanism) and cannot be an in-loop1 DEFINITION (D5/D6, sandbox 23 at 132/132). Only block 0 - already enumerated down to the FAKE do-while(0) wrap - and an in-loop1 USE remain. s22's frontier item 1 aimed the combine-merge enumeration at target's BLOCK-2 insns; that is now proven to be the wrong list, and the right list is target's own loop1 insns.

- [s23] a3 is NOT a dial (E-s23-2): 4 refs / live 99 / pri 808 in ten measured forms (B0, C0, C3, C4, D1, D2, D5, D6, F1, F2), including D1/D2 which move statements past a3's last use inside loop2 and F1/F2 which add `s32 pa3 = a3;` at all three call sites - the exact analogue of the pa4 local that halves a4's live length 190 -> 95. The frontier needed +10 to +15 live length on a dial that does not move by 1.

- [s23] RATIO LAW (E-s23-3) on the pa4-free chassis: out2 (3 refs) outranks a4 (7 refs) iff L(a4) > 4.667*L(out2). The complete block-0 position sweep of out2's definition gives C0 4.465 (out2 697 / a4 729, sandbox 13), B0 4.524 (714 / 736, sandbox 10), C3 and C4 4.585 (731 / 744, sandbox 13). The sweep moves both live lengths the same way, is monotone, and is exhausted below 4.667.

- [s23] NEW SOLUTION WINDOW found and closed in one session (E-s23-4): on the pa4 chassis out2 also seats correctly at 3 references with live length 18..23 (no fourth reference at all), and live 21 IS reachable at 132/132 via an in-loop1 definition - but that definition triggers the cse fold (out2 becomes 4/21 = 3809, above tbl) and emits out2's addiu inside loop1 where target emits it in block 0.

- [s23] A same-value recompute of out2 at the end of loop1 is byte-COSTLY here (G1 = sandbox 12 at 133 of 132 insns): out2's block-0 value is not in cse1's table for loop1's EBB, so the insn is not recognised as redundant and survives to the output.

- [s23] Full ALLOCDBG tables banked for B0/C0/C3/C4/D1/D2/D5/D6/F1/F2/G1 in tmp/grind/func_80041188/s23/*/cc1.err, each with the matching red.i.* pass dumps.

- [s23] The standing owner directive's outstanding item is unchanged and unanswered: candidate.c's `stptr = base; stptr += 0xFC;` still needs either the sanctioned-split-init ruling, the /* FAKE: F1 */ annotation, or a replacement before this body could land.

- [s24 synthesis] Chassis re-measured at session start: candidate.c applied to
  src/text1a_pre.c = sandbox 1 at 132 target / 132 build insns, rules_dropped 16,
  cheat_asm_stripped 2. Floor unchanged at 1. src/text1a_pre.c restored to HEAD at the
  end; no build-pipeline file touched.
- [s24] E-s24-1 (THE SYNTHESIS RESULT — target's own allocno vector is FORCED, and it is a
  TWO-reference problem, not a one-reference problem). Target's `$s6` (out2) has exactly
  three EMITTED references (asm/funcs/func_80041188.s:25 def, :56, :61) and its last one is
  inside loop1, so target's out2 live length is 42 — the same 42 our pa4-chassis forms
  measure — because reg_live_length is recomputed post-combine (sched.c:5106) while
  reg_n_refs is not (flow.c:2081, fixed pre-combine). E-s23-5 independently proves out2
  must carry at least FOUR flow-counted references. Four references at live 42 is
  pri = 2*4/42*10000 = 1904. tbl at its natural, target-like EARLY definition measures
  4 refs / live 47 = 1702. 1904 > 1702, so out2 outranks tbl and takes tbl's seat. Therefore
  the original source CANNOT have had tbl at four references: it must have carried a FIFTH
  flow-counted tbl reference (5/47 = 2127 > 1904), or tbl at four references with live <= 41
  (1951), which is s21's Q1 late-definition horn and is proven emission-costly (E-s22-4).
- [s24] E-s24-2 (measured, the kill that forces E-s24-1). Form P1 = candidate.c chassis with
  target's block-2 spelling restored (`out3 = (s32 *)(((u8 *) pa4) + 0x20);`) plus ONE
  zero-insn +1 reference on out2 in loop1 (a `do { func_8004A348(buf, out2); } while (0);`
  loop-note weight used purely as an INSTRUMENT to buy the reference — not a candidate).
  ALLOCDBG: out2 4 refs / live 42 = 1904 -> hardreg 21 ($s5); tbl 4 / 47 = 1702 -> hardreg 22
  ($s6). The two seats are SWAPPED with respect to target. This kills the shape s20-s23 were
  all searching for — "one byte-free +1 reference on out2, everything else natural" — for any
  delivery mechanism whatsoever, because the arithmetic depends only on the reference count
  and the live length, not on how the reference is spelled. Artifact:
  tmp/grind/func_80041188/s24/P1/, rejected/out2-inloop1-plus1-alone-swaps-s5-s6.c.
- [s24] E-s24-3 (the required vector, reproduced and re-measured on the current chassis).
  s18's V7 already spells the vector E-s24-1 derives, and it reproduces exactly:
  stptr 6/41 = 2926 -> $s3, stptr2 6/48 = 2500 -> $s0, i 8/97 = 2474 -> $s4,
  tbl 5/47 = 2127 -> $s5, out2 4/42 = 1904 -> $s6, a4 7/95 = 1473 -> $s7,
  a3 4/99 = 808 -> $fp, out3 3/47 = 638 -> $s3. ALL-TARGET callee-saved seats WITH target's
  block-2 `addiu $s3,$s7,0x20`. Form V8 (= V7 with the stptr loop-note wrap replaced by
  candidate.c's F1 `stptr = base; stptr += 0xFC;` chain-extender, stptr 7/41 = 3414) measures
  the identical seat vector: sandbox 8 at 133 of 132 insns. Saved as
  memory/grind/func_80041188/alt_V8_alltarget_133_s24.c.
- [s24] E-s24-4 (attribution of V8's 133rd insn — it is ONE load-delay nop, and the whole
  residual is sched1 order). fdiff of V8 against target: indices 0..39 identical; at index 44
  ours emits `lhu $v0,0($s0)` followed by a `nop` where target fills that load-delay slot
  with `addiu $s4,$s4,1` (`i++`), and every later insn is shifted by one. Target also emits
  `addiu $s5,$s5,4` (`tbl++`) at index 40 where ours sinks it to 48. So the all-target-seat
  form's entire cost is sched1's ordering inside loop1, displaced by the wraps' loop notes —
  not allocation, not an extra computation. Artifact: tmp/grind/func_80041188/s24/od.txt +
  s24/fdiff.py.
- [s24] E-s24-5 (WHICH construct costs the nop — the tbl +1 is byte-free, the out2 +1 is not).
  V11 = V8 with the out2 loop-note wrap removed and the tbl loop-note wrap kept measures
  132 of 132 build insns (sandbox 19, seats wrong because out2 falls back to 3 refs). So a
  loop-note +1 on tbl is BYTE-FREE on this chassis, while the loop-note +1 on out2 is what
  materialises the 133rd insn. Combined with E-s24-1 this reduces the entire function to a
  single open question: a byte-free fourth flow-counted reference on out2 sited in loop1.
- [s24] E-s24-6 (the address-constant split-init family is DEAD on tbl, in both directions).
  T1 = V8 with the tbl loop-note wrap replaced by an honest-looking address-constant chain
  extender (`tbl = D_80094CFC - 1; tbl += 1;`, which is value-identical and needs no new
  symbol). It is NOT folded away before flow — tbl measures 6 refs / live 47 = 2553 — so the
  mechanism does deliver references. It fails twice: (i) it delivers +2, not +1, and 2553
  outranks `i` (8/97 = 2474), so tbl takes $s4 and i takes $s5 (measured hardregs 20 / 21);
  (ii) combine does NOT fold `la BASE` + `addiu off` back into a single address constant, so
  the form costs a real instruction — 134 of 132 build insns, sandbox 19. Both failures are
  independent of which base symbol is chosen, so the "tbl is really a member of a larger
  table" reading buys nothing at the byte level either. Artifact:
  rejected/tbl-address-constant-split-plus2-refs-costs-insn.c.
- [s24] E-s24-7 (source statement order is not a lever on the delay slot). V14 = V8 with
  `i++` moved to sit immediately after `buf[0] = p[0];` — exactly where target's schedule
  puts it — measures byte-identically to V8 (sandbox 8, 133 insns). sched1 owns the slot;
  the C-level position of the increment inside the block does not move it. Artifact:
  rejected/v8-iplusplus-reorder-inert-sched1-owns-delay-slot.c.
- [s24] E-s24-8 (directive execution — frontier item 3 is ANSWERED by the standing owner
  directive, not open). The queue item's owner directive for this function states verbatim
  that "candidate's stptr chain-extender still needs FAKE annotation or replacement on land",
  i.e. the owner has already classified `stptr = base; stptr += 0xFC;` as the F1 family
  requiring the /* FAKE */ annotation, NOT as sanctioned split-init accumulation. candidate.c
  already carries that annotation with what + mechanism + lever-exhaustion at its site
  (candidate.c:289). No ruling-request is needed and none should be filed again; the s22/s23
  frontier item asking the question is retired.

- [s24] Chassis re-measured this session: candidate.c applied to src/text1a_pre.c = sandbox 1 at 132 target / 132 build insns (rules_dropped 16, cheat_asm_stripped 2). Floor unchanged at 1. src/text1a_pre.c restored to HEAD at session end; no build-pipeline file touched.

- [s24] E-s24-1: target's $s6 (out2) has exactly three emitted references (asm/funcs/func_80041188.s:25 def, :56, :61), all inside block 0 / loop1, so the original's out2 live length is 42 post-combine; with the >= 4 references E-s23-5 proves necessary its priority is 1904, ABOVE a naturally-defined tbl at 4/47 = 1702. The original source must therefore have carried a fifth flow-counted tbl reference (2127) - a requirement no session had derived.

- [s24] E-s24-2: form P1 measures the kill directly - out2 4/42 = 1904 seats at hardreg 21 ($s5) and tbl 4/47 = 1702 at hardreg 22 ($s6), the two target seats exchanged. Because the priority formula reads only (refs, live length), this closes the entire 'one byte-free +1 on out2' search direction that s20-s23 pursued, for every possible spelling.

- [s24] E-s24-3: the required vector is reproducible on the current chassis - V7 (three loop-note wraps) and V8 (V7 with the stptr wrap replaced by candidate.c's F1 chain-extender) both give ALL-TARGET callee-saved seats together with target's block-2 addiu $s3,$s7,0x20, at sandbox 8 / 133 insns. V8 saved as memory/grind/func_80041188/alt_V8_alltarget_133_s24.c.

- [s24] E-s24-4: fdiff attributes V8's single extra instruction precisely - at index 44 ours emits `lhu $v0,0($s0)` then a load-delay `nop`, where target fills that slot with `addiu $s4,$s4,1`; target also emits `addiu $s5,$s5,4` at index 40 where ours sinks it to 48. Indices 0..39 are identical. The all-target-seat form's whole cost is sched1 ordering inside loop1, not allocation and not an extra computation.

- [s24] E-s24-5: V11 (tbl loop-note wrap kept, out2 wrap removed) measures 132 of 132 build insns, so a +1 on tbl is BYTE-FREE on this chassis while the out2 +1 delivery is what materialises the nop. Half of the two-reference requirement is therefore already solved byte-free; the open half is out2's.

- [s24] E-s24-6: the address-constant split-init family is dead on tbl - it delivers +2 references (tbl 6/47 = 2553, overshooting i at 2474 and stealing $s4) and combine does not fold `la BASE` + `addiu off`, so it costs an instruction (134 insns, sandbox 19). Independent of which base symbol is chosen, so the 'tbl is really a member of a larger table' reading buys nothing at the byte level.

- [s24] E-s24-7: C-level statement order is not a lever on sched1's delay-slot choice (V14 byte-identical to V8).

- [s24] E-s24-8 (owner directive executed): the queue item's standing directive states that candidate.c's stptr chain-extender 'still needs FAKE annotation or replacement on land', which classifies `stptr = base; stptr += 0xFC;` as the F1 family requiring the /* FAKE */ annotation rather than as sanctioned split-init accumulation. candidate.c already carries that annotation with what + mechanism + lever-exhaustion (candidate.c:289). The s22/s23 frontier item asking this question is retired - no ruling-request should be filed for it again.

## s25 (escalation, 2026-08-25) — a NEW byte-free reference-lift class is found (split at an increment site), and the residual is re-stated on a chassis that is emission-order identical to target

Chassis re-measured at the start of s25 by applying `alt_V8_alltarget_133_s24.c` to
`src/text1a_pre.c`: `sandbox func_80041188 --disable all` = **score 8, 132 target / 133 build
insns**, `rules_dropped: 16, cheat_asm_stripped: 2` — reproducing s24's record exactly. The
ledger floor (candidate.c, 1) was NOT re-measured this session; every number below comes from a
form measured this session. `src/text1a_pre.c` was restored to HEAD at the end of the session and
no build-pipeline file was touched.

- **[E-s25-1] The owner directive of 2026-08-24 is now executed and its premise is corrected.**
  The directive says "solver modality has never run on it"; s15 (2026-08-24) had in fact already
  run both solvers. What had never run — frontier item 2 as carried since s24 — was
  `tools/sched_solver` on an ALL-TARGET-SEAT form. s25 ran it. The auto goal derivation
  (`goalmap.py --goal-from-target`) **cannot** be used on this function's loop1: it reports
  `GOAL INVALID: 4 dependence violation(s)` because the aligner mis-pairs the two identical
  `sh $2,20($sp)` texts in the two calls. The goal has to be hand-built from
  `asm/funcs/func_80041188.s`, undoing reorg.c's four delay-slot fills (target insns 43, 56, 61,
  66 are the fills for the jals at 42, 55, 60 and the branch at 65). Target's block-1 pick order
  in our UIDs is
  `170,169,166,163,161,158,156,154,152,150,136,126,124,134,123,120,118,132,117,114,109,112,103,97,90,88,100,87,84,82,95,81,78,93,76,73,59,58,56,53`
  and it is dependence-valid (perturb accepts it and reports `baseline exact`). Recipe banked at
  `tmp/grind/func_80041188/s25/` (`mk.sh` = pin target from HEAD + extract the reduced-TU model;
  `tlist.py` = indexed target listing; `probe.sh` = apply + ALLOCDBG dump).

- **[E-s25-2] KILLED at depth 1: no single spellable scheduler atom reaches target's loop1
  emission order on the V8 (wrap-bearing) chassis.** `perturb.py --pass 1 --block 1
  --atoms luid,luid_move --depth 1` searched **2340 single atoms**: none reaches the goal. The
  depth-2 search was launched and had not converged after ~40 minutes; it was stopped before the
  turn ended (no orphan). This confirms E-s24-7 mechanically and from the scheduler's own model:
  the V8 chassis's loop1 order damage is NOT repairable by C statement order. The damage is caused
  by the do-while(0) wraps' loop notes, and the correct response is to remove the wraps, not to
  move statements — which is what E-s25-3 does.

- **[E-s25-3] THE LOAD-BEARING RESULT — a byte-free +2 reference lift exists at the INCREMENT
  site of a loop-carried variable, and it replaces the do-while(0) loop-note wrap.** Writing
  `i++;` as `i += 2; i -= 1;` inside loop1 lifts `i` from 8 refs / live 97 = 2474 to
  **10 refs / live 97 = 3092** with no change in emitted instructions; the same rewrite of
  `tbl++;` lifts `tbl` from 4 / 47 = 1702 to **6 / 47 = 2553**, also byte-free. Combine folds the
  pair back into the single `addiu` target emits; flow.c:2081 has already counted both references
  by then. This is the F1 chain-extender law (E-s22-2 / E-s17-1) applied at a REDEFINITION of a
  loop-carried pseudo rather than in straight-line block-0 code, and it is the first reference-lift
  delivery for `tbl` that costs nothing — s24's T1 address-constant split delivered the same +2 but
  materialised an instruction (134 vs 132). NOTE for the next session: this construct is an
  unannotated FAKE at best and has no cited SOTN precedent; it is used here as a MEASUREMENT
  INSTRUMENT and as proof that the reference is deliverable byte-free, not as a shippable form.

- **[E-s25-4] Form V15a: 132 of 132 insns, TARGET EMISSION ORDER IN ALL FOUR BLOCKS, no
  do-while(0) wrap, and a residual that is a pure register 3-cycle.** V15a = V8 with both wraps
  deleted and both increments split (`tbl += 2; tbl -= 1;` and `i += 2; i -= 1;`).
  `sandbox --disable all` = **score 15 at 132 build / 132 target insns**, and
  `goalmap.py --model` reports **`block 0/1/2/3: GOAL == OURS (identity)`** with `hon=131 tgt=131`.
  Allocno table: stptr 7/41 = 3414 → $s3 · i 10/97 = 3092 → $s4 · tbl 6/47 = 2553 → $s5 ·
  stptr2 6/48 = 2500 → $s0 · pa4 7/95 = 1473 → **$s6** · a3 4/99 = 808 → **$s7** ·
  out2 3/42 = 714 → **$fp** · out3 3/47 = 638 → $s3. Six of the nine seats are target's; the
  entire residual is the {out2, pa4, a3} 3-cycle caused by out2 sitting at three references.
  Banked as `memory/grind/func_80041188/alt_V15a_targetorder_purera_s25.c`.

- **[E-s25-5] The vector is CONFIRMED end to end: V15a + one more out2 reference = target's
  COMPLETE disposition.** V23 = V15a with the loop1 `func_8004A348(buf, out2)` call wrapped in
  `do { } while (0);` purely as a reference-count instrument. ALLOCDBG: stptr $s3 · i $s4 ·
  tbl $s5 · stptr2 $s0 · **out2 4/42 = 1904 → $s6** · **pa4 1473 → $s7** · **a3 808 → $fp** ·
  out3 → $s3 — i.e. **ALL-TARGET seats together with target's block-2 `addiu $s3,$s7,0x20`**
  (V15a derives out3 from pa4, not from out2, so this body does not contain candidate.c's
  residual `move $s3,$s6` at all). Its only cost is the wrap itself: sandbox 9 at **134** insns,
  because on this chassis the loop note costs two instructions rather than V8's one. **So the
  entire function now reduces to a single, fully-specified question: a byte-free fourth
  flow-counted reference on `out2`.** Everything else — all six other contested seats, target's
  block-2 spelling, and target's emission order in every block — is delivered by arithmetic on
  this chassis. Banked as
  `rejected/v15a-plus-out2-wrap-alltarget-seats-but-wrap-costs-two-insns.c`.

- **[E-s25-6] KILLED: three of `i`'s four split sites are inert; only the increment site is a
  dial.** s24's frontier item 3 asked whether `i` could be lifted 8 → 9 references at any of four
  sites. Measured on the V8 chassis with ALLOCDBG: split initialiser (`s32 i = 0;` + `i++;` in
  block 0) leaves `i` at **8 refs** (live 97 → 96, pri 2500) — the constant pair folds before
  flow; split reset (`i = 0x11; i++;`) leaves it at **8 refs** and is otherwise bit-identical to
  V8; duplicated compare (`if (i < 0x12) { if (i < 0x12) goto loop1; }`) leaves it at **8 refs**
  — jump.c folds the second test away. Only the in-loop increment moves it, and it moves it by
  **+2, never +1** (E-s25-3). Banked at `rejected/i-split-initialiser-refs-rigid-at-8.c`,
  `rejected/i-split-reset-refs-rigid-at-8.c`,
  `rejected/i-duplicated-compare-folded-refs-rigid-at-8.c`.

- **[E-s25-7] KILLED by direct measurement, on two chassis and in six spellings: NO block-0
  chain-extender lifts `out2`.** Every one of these leaves out2 at exactly **3 refs / live 42 =
  714**: `out2 = pa4; out2 = out2 + 0x20;` (V17, on the V8 chassis; V16, on the V15a chassis);
  `out2 = pa4 + 0x21; out2 = out2 - 1;` (V15b); a fresh donor local
  `tmp = pa4; out2 = tmp; out2 = out2 + 0x20;` (V19); a fresh donor carrying half the offset
  `tmp = pa4 + 0x10; out2 = tmp; out2 = out2 + 0x10;` (V21); and a three-step
  `out2 = pa4 + 0x20; out2 += 0x20; out2 -= 0x20;` (V22, which only shortens live 42 → 41).
  This confirms E-s17-3 by measurement rather than by derivation and extends it: the E-s17 law's
  "donor's last use precedes the recipient's" condition is NECESSARY BUT NOT SUFFICIENT — V19 and
  V21 satisfy it with donors that die in block 0 and still fold, because cse1 folds a
  constant-offset chain rooted in a pseudo already in its table at that point. The surviving lifts
  in this function (stptr's `base` chain; tbl's and i's increment splits) all have a donor that is
  either a memory-load result used again later (`base`) or the loop-carried variable itself.
  Banked at `rejected/out2-chain-extender-fresh-donor-cse-folds-refs-rigid-3.c` and
  `rejected/out2-chain-extender-split-constant-donor-cse-folds-refs-rigid-3.c`.

- **[E-s25-8] Gate (a) re-measured this session, unchanged.** `python3 tools/scan_hand_coded.py
  --single func_80041188` = **tier LOW, score 0/8** ("no strong hand-coded indicators"; S1 0 multu
  pairs, S2 no empty-body branches, S3 132 insns / 11 spills / 15 distinct regs, S4 max load burst
  3, S5 no high-similarity siblings, S6 no BIOS jumptable, S7 all callee-saves have `$sp` saves,
  S8 no redundant mask-before-shift). The canonical-asm grant path stays closed, and the owner
  DECLINED the LOW-tier override on 2026-08-24 as auto-reject class.

- [s25] DISPOSITION NOTE. The driver dispatched this session in `escalation` modality, but both
  dispositions that modality can file are foreclosed for this function by the owner's own
  2026-08-24 rulings: the canonical-asm LOW-tier override was explicitly DECLINED as auto-reject
  class, and the 2026-08-23 "REFUSED / OWNER-ACCEPTED INCOMPLETE" terminal entry was explicitly
  SUPERSEDED by the rules-to-zero campaign with the instruction "keep grinding". Under the
  auto-reject clause (a packet whose YES would lower a standard or accept the debt must NOT be
  filed; the residual stays ACTIVE), no packet was filed and no decisions.md entry was added.
  The honest outcome is `progress` with the s25 kills and the new chassis banked — and, unusually
  for a disposition session, the search space GREW: a reference-lift class no session had, and a
  chassis on which six of seven contested seats plus target's emission order come free.

- [s25] Chassis re-measured this session: alt_V8_alltarget_133_s24.c applied to src/text1a_pre.c gives sandbox func_80041188 --disable all = score 8, 132 target / 133 build insns, rules_dropped 16, cheat_asm_stripped 2 - reproducing the s24 record exactly.

- [s25] V15a (V8 minus both do-while(0) wraps, plus 'tbl += 2; tbl -= 1;' and 'i += 2; i -= 1;') = sandbox 15 at 132 build / 132 target insns, and goalmap.py --model reports 'block 0/1/2/3: GOAL == OURS (identity)' - target's emission order in every block. Banked as memory/grind/func_80041188/alt_V15a_targetorder_purera_s25.c.

- [s25] The increment split is byte-free and counted: i 8 refs/97 = 2474 -> 10/97 = 3092 and tbl 4/47 = 1702 -> 6/47 = 2553, with no change in emitted instructions. This is a reference-lift delivery no previous session had; s24's T1 address-constant split delivered the same +2 on tbl but materialised an instruction (134 vs 132).

- [s25] V23 (V15a + a do-while(0) wrap on the loop1 out2 call, used purely as an instrument) has ALL-TARGET callee-saved seats - stptr $s3, i $s4, tbl $s5, stptr2 $s0, out2 4/42 = 1904 -> $s6, pa4 $s7, a3 $fp, out3 $s3 - together with target's block-2 addiu $s3,$s7,0x20. Its only cost is the instrument (sandbox 9 at 134 insns).

- [s25] tbl's lift to 2553 widens out2's admissible priority band from (1473, 1702) to (1473, 2553), which at three references corresponds to a live-length window of 12..20 that no earlier session's arithmetic contained.

- [s25] sched_solver's automatic goal derivation is unusable on this function's loop1: goalmap reports 'GOAL INVALID: 4 dependence violation(s)' because the target aligner mis-pairs the two identical 'sh $2,20($sp)' texts. The hand-built goal (target's four reorg.c delay-slot fills undone) is dependence-valid and perturb accepts it with 'baseline exact'.

- [s25] perturb.py --pass 1 --block 1 --atoms luid,luid_move --depth 1 on the V8 chassis: 2340 single atoms, none reaches target's loop1 order. Depth 2 did not converge in ~40 minutes and was stopped before the turn ended.

- [s25] i's reference count is rigid at 8 across the split initialiser, the split reset and a duplicated compare; only the in-loop increment moves it, and only by +2.

- [s25] out2's reference count is rigid at 3 across six distinct block-0 chain-extender spellings on two chassis, confirming E-s17-3 by measurement and extending it (donor death point is not the governing condition; cse1 foldability is).

- [s25] Gate (a) re-measured: tools/scan_hand_coded.py --single func_80041188 = tier LOW, score 0/8, no strong hand-coded indicators. Gate (b) unchanged: no in-hand SOTN-master precedent for a byte-free reg_n_refs construct on out2.

- [s25] No decision packet was filed. Both dispositions available to the escalation modality are foreclosed for this function by the owner's own 2026-08-24 rulings: the canonical-asm LOW-tier override was DECLINED as auto-reject class, and the 2026-08-23 REFUSED / OWNER-ACCEPTED INCOMPLETE entry was SUPERSEDED by the rules-to-zero campaign with the instruction 'keep grinding'. Per the auto-reject clause, a packet whose YES would lower a standard or accept the debt must NOT be filed and the residual stays ACTIVE.

- [s25] src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched and no commit was made.

## Session 26 (2026-08-27) — modality dispatched `escalation`, worked as grind per the owner's 2026-08-27 ruling

- **[s26-DISPOSITION] No decision packet filed, and this is the owner's own instruction, not a
  dodge.** The driver dispatched s26 in `escalation` modality, but the queue item carries
  OWNER RULING 2026-08-27 (`docs/grind/decisions.md:14565`): "the 2026-08-25 auto-filed
  exhaustion-backstop escalation is SPENT … returns to ACTIVE … the owner directs continued honest
  grinding to COMPLETED-C, with the explicit goal of ZERO regfix + ZERO asmfix rules remaining.
  The canonical-asm LOW-tier override remains DECLINED (unchanged from the 2026-08-24 ruling)."
  Both dispositions the escalation modality can file are therefore pre-decided NO: gate (a) is
  LOW (re-measured s25, E-s25-8, and the owner declined the override twice), and a
  "REFUSED / OWNER-ACCEPTED INCOMPLETE" entry is exactly the debt-accepting shape the 2026-08-24
  auto-reject clause forbids. Per the role prompt's own words ("a packet whose YES would LOWER a
  standard … is PRE-DECIDED NO and must NOT be filed; that residual stays ACTIVE"), the honest
  outcome is `progress`. This is the second consecutive session to reach that conclusion (s25);
  the driver's exhaustion counter is the thing that is stale, not the function.

- **[E-s26-1] THE LOAD-BEARING RESULT — the reference-count dial the banned do-while(0) wrap was
  faking has an HONEST source: a real loop CONSTRUCT.** GCC 2.7.2's flow.c increments
  `REG_N_REFS` by `loop_depth`, and `loop_depth` is driven by NOTE_INSN_LOOP_BEG/END notes, which
  `expand_start_loop` emits **only for real loop constructs** (`while` / `for` / `do`). A
  `goto`-based loop emits none, so every in-loop reference counts exactly once — which is why
  every allocno in this function has been ref-starved on every goto chassis, and why the
  do-while(0) wrap moved the needle at all. Rewriting loop1 as a plain
  `do { … } while (i < 0x12);` doubles the count of every reference inside it. Measured on the
  V15a chassis (form Y2 = V15a with a real do-while loop1 and the two FAKE increment splits
  REMOVED): out2 goes **3 refs / live 42 = 714 → 5 refs / live 41 = 2439**, i 10/97 → 11/97,
  tbl 6/47 → 7/47, pa4 7/95 → 9/94 = 2872, a3 4/99 → 5/99 = 1010, at **132 build insns** and
  `sandbox --disable all` = **13** (vs V15a's 15 WITH its two FAKE splits). This is the first
  reference lift in this function's whole history that is an ordinary C loop.

- **[E-s26-2] The F1 stptr chain extender is UNNECESSARY on the real-loop chassis — form Z1 is
  FAKE-free.** `stptr = base; stptr += 0xFC;` was the one construct in the V15a/candidate lineage
  that needed a `/* FAKE */` annotation. Replacing it with the plain `stptr = base + 0xFC;`
  (form Z1) is **bit-identical to Y2** — same allocno table, same 132 insns, same
  `sandbox --disable all` = **13** — because loop.c strength-reduces the stptr biv away before
  its ref count matters. Z1 therefore contains **no FAKE construct of any kind**: a real
  do-while loop1, a goto loop2, the pa4 param alias, the out2/out3 duplicated definition, and
  ordinary arithmetic. Banked as `memory/grind/func_80041188/alt_Z1_realloop_honest_s26.c`.
  (Control E-s26-6 shows the chain extender genuinely did work on the goto chassis, so this is a
  chassis property, not a re-measurement error.)

- **[E-s26-3] The allocno priority formula is now exact and can be used arithmetically:**
  `pri = floor_log2(nrefs) * nrefs * 10000 / live_length`. Verified against all eleven allocnos
  of V15a to the unit (16/99→6464, 10/97→3092, 7/41→3414, 6/47→2553, 4/99→808, 3/42→714).
  Consequence for the seat problem: out2's admissible band is `(pa4_pri, stptr2_pri)`, and at
  **live 42 both 4 refs (1904) and 5 refs (2380) land inside the goto-chassis band (1473, 2500)**
  — i.e. the +2 lifts that the increment split delivers are as usable as a +1, which no earlier
  session's arithmetic showed.

- **[E-s26-4] The Y2/Z1 residual is exactly TWO named things — enumerated by diffing our honest
  stream against the pinned target stream.** (a) **the out2/pa4 seat swap**: ours gives pa4 $s6
  and out2 $s7, target the reverse, because out2 2439 < pa4 2872; it accounts for six of the
  diff hunks. (b) **loop.c's strength reduction of the stptr biv**: loop.c eliminates the biv in
  favour of the `stptr + 0x38` giv, emitting `addu $19,$2,308` / `addu $7,$19,$zero` /
  `sh $8,-50($19)` where target has `addu $19,$2,252` / `addu $7,$19,56` / `sh $2,6($19)`
  (252+56 = 308, 6-56 = -50). Target's loop1 is therefore provably NOT a loop.c-processed loop,
  which is the exact tension this chassis creates: the loop notes that supply the references also
  supply loop.c.

- **[E-s26-5] KILLED — no no-op statement lifts a reference. Four spellings, all inert.**
  On the V15a chassis, `out2 = out2;` before the loop1 use (W1), `out2 += 0;` (W2),
  `out2 = out2;` in block 0 (W3) and at the top of loop1 (W4) ALL leave out2 at exactly
  **3 refs / live 42 / pri 714**, 142 insns, and an allocno table identical to V15a's. The law
  this establishes: a reference-lift statement must produce a genuinely NEW value at RTL
  expansion (as `i += 2; i -= 1;` and `stptr = base; stptr += 0xFC;` do). A statement whose RHS is
  the LHS's current value never becomes an insn, so flow.c has nothing to count. Banked at
  `rejected/out2-self-assign-*.c` and `rejected/out2-plus-zero-in-loop1-inert-refs-rigid-3.c`.
  This also disposes of the dead-store family as a delivery vehicle for THIS residual.

- **[E-s26-6] Control measurement: the base-rooted chain extender really does survive cse1 on the
  goto chassis.** Form X3 = V15a with `stptr = base + 0xFC;` in place of the split: stptr falls
  from **7 refs / 41 = 3414 to 5 refs / 41 = 2439** and loses its $s3 seat. So the s25 asymmetry
  (base-rooted chains survive, pa4-rooted chains fold) is real and reproducible, and E-s25-7's
  refinement stands: cse1 foldability, not donor death point, is the governing condition.

- **[E-s26-7] KILLED — moving out2's definition into loop1 is dead on BOTH chassis, for two
  different reasons.** On the goto chassis (X1/X2, def placed either immediately before out2's
  first use or at the top of loop1 — identical results) out2 does reach **4 refs**, but its live
  length collapses to **21**, giving pri **3809**, far above the (1473, 2500) band, and it seizes
  $s3; worse, the build drops to **141 insns** because target's block-0 `addiu $s6,$s7,0x20`
  ceases to exist in block 0. On the real-loop chassis (Z3) LICM hoists the definition straight
  back to the preheader and the result is bit-identical to Y2 — out2's definition POSITION is not
  a dial there at all. Banked at `rejected/out2-def-into-loop1-pri-3809-and-block0-addiu-lost.c`
  and `rejected/out2-def-in-realloop-licm-hoists-back-inert.c`.

- **[E-s26-8] KILLED — the block-0 chain extender on out2 is chassis-independent.** Re-measured on
  the real-loop chassis (Z2): `out2 = pa4; out2 = (s32 *)((u8 *)out2 + 0x20);` leaves out2 at
  **5 refs / live 41**, bit-identical to Z1/Y2. cse1 folds it exactly as it did on the goto
  chassis (E-s25-7). Banked at `rejected/out2-chain-extender-folds-on-realloop-chassis.c`.

- **[E-s26-9] KILLED — splitting pa4's loop1 uses into a second local reorders the seats correctly
  but spills a3.** Form Z4 (`pm = pa4;` in block 0, loop1's two pa4 uses rewritten to pm) drops
  pa4 to 6 refs / 95 = 1263 and lets out2 (5/41 = 2439) take **$s6** with pm at $s7 — the seat
  arithmetic works — but a3 loses its hard register (`hardreg=-1`) and the build grows to
  **143 insns**. Banked at `rejected/pm-split-drops-pa4-but-spills-a3-143-insns.c`.

- **[E-s26-10] The seat swap on the real-loop chassis is an arithmetic dead end in every direction
  measured.** To take $s6, out2 needs pri > pa4's 2872. Raising out2 to 6 refs would give 2926
  (admissible), but out2 is rigid at 5 across every spelling tried (E-s26-5/7/8). Shortening
  out2's live from 41 to <= 34 is impossible: on this chassis LICM pins the definition to the
  preheader and the last use is the loop's final call, so out2's live range IS the loop body.
  Lowering pa4 by ref count needs 9 -> 7 (8 refs still gives 2553 > 2439), i.e. deleting BOTH the
  out2 and out3 definitions' references to pa4, and out2 == out3 == pa4 + 0x20 makes that
  semantically impossible. Lowering pa4 by live length needs live >= 111 against a function that
  is only ~99 insns long. The remaining untried direction is the OTHER residual: kill loop.c's
  biv elimination (E-s26-4b) so that stptr stays an allocno, which re-lays the whole priority
  landscape.

- **[E-s26-11] Freshly noticed structural fact, unexploited: `stptr` at loop1 exit and `stptr2` at
  loop2 entry are THE SAME VALUE.** stptr walks base+0xFC in 17 steps of 0x68 to base+0x7E4;
  stptr2 = saved + 0x750 = (base+0x94) + 0x750 = base+0x7E4. Target nevertheless spills
  `base+0x94` to 0x18($sp) in block 0 and recomputes (`lw $t0,0x18($sp)` / `addu $s0,$t0,1872`),
  so the original source did keep them as separate variables — but the coincidence is strong
  evidence about the original's data layout (a 0x68-stride record array with a second field at
  +0x94) and it is the one free way to give `stptr` a use that survives loop1, which is exactly
  what loop.c's biv elimination needs to be blocked. Not measured this session.

- [s26] src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was
  touched, no commit was made, and no permuter campaign was launched.

- [s26] The allocno priority formula is now exact and usable as arithmetic: pri = floor_log2(nrefs) * nrefs * 10000 / live_length. Verified to the unit against all eleven V15a allocnos (16/99->6464, 10/97->3092, 7/41->3414, 6/47->2553, 4/99->808, 3/42->714). Consequence: at live 42 BOTH 4 refs (1904) and 5 refs (2380) sit inside the goto-chassis admissible band (1473, 2500), so a +2 reference lift is as usable as a +1 — arithmetic no earlier session had.

- [s26] Form Z1 (real do-while loop1, no increment splits, no stptr chain extender, no wrap) measures sandbox --disable all = 13 at 132 build / 132 target insns, rules_dropped 16. It is the first chassis in this grind's history that obtains loop-depth reference weighting with no coercion construct at all.

- [s26] The Z1 residual, enumerated by diffing our honest stream against the pinned target stream, is exactly TWO things: (a) the out2/pa4 seat swap (ours pa4=$s6 / out2=$s7, target the reverse) because out2 2439 < pa4 2872 — six diff hunks; (b) loop.c eliminating the stptr biv in favour of the stptr+0x38 giv, emitting `addu $19,$2,308` / `addu $7,$19,$zero` / `sh $8,-50($19)` where target has `addu $19,$2,252` / `addu $7,$19,56` / `sh $2,6($19)` (252+56 = 308, 6-56 = -50).

- [s26] Target's loop1 is provably NOT a loop.c-processed loop (it carries no strength-reduced giv), which is the exact tension this chassis creates: the loop notes that supply the references also supply loop.c. Whether the two can be separated is the whole remaining question on this chassis.

- [s26] The out2/pa4 seat swap is an arithmetic dead end in every direction measured on the real-loop chassis: out2 is rigid at 5 refs across self-assign, +=0, definition relocation and chain extension; out2's live length IS the loop body (LICM pins the def to the preheader, the last use is the loop's final call) so it cannot be shortened below 34; pa4 needs 9->7 refs (8 still gives 2553 > 2439), which means deleting BOTH the out2 and out3 definitions' references to pa4, and out2 == out3 == pa4 + 0x20 makes that semantically impossible; and lowering pa4 by live length needs live >= 111 against a ~99-insn function.

- [s26] Newly noticed, unexploited structural fact: stptr at loop1 exit and stptr2 at loop2 entry are THE SAME VALUE (base+0xFC + 17*0x68 = base+0x7E4 = (base+0x94)+0x750). Target nevertheless spills base+0x94 to 0x18($sp) and recomputes, so the original kept them as separate variables — but this is the one free way to give stptr a use that outlives loop1, which is precisely what loop.c's biv elimination needs to be blocked.

- [s26] DISPOSITION: no decision packet was filed, on the owner's own instruction. The queue item carries OWNER RULING 2026-08-27 (docs/grind/decisions.md:14565): the 2026-08-25 exhaustion-backstop escalation is SPENT, func_80041188 returns to ACTIVE, and the owner directs continued honest grinding to COMPLETED-C with the explicit goal of zero regfix + zero asmfix carriers; the canonical-asm LOW-tier override remains DECLINED. Gate (a) is LOW (re-measured s25, E-s25-8) and gate (b) has no in-hand SOTN precedent, so the only packet available would be the debt-accepting 'REFUSED / OWNER-ACCEPTED INCOMPLETE' shape that the 2026-08-24 auto-reject clause forbids filing. Per that clause the residual stays ACTIVE and the honest outcome is progress with the kills banked. This session did not merely decline to dispose: it GREW the search space with a construct class (honest loop notes) that no prior session had.

- [s26] src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched, no commit was made, and no permuter campaign was launched (nothing to orphan).

## s27 (forensics, 2026-08-27) — the out2 reference lift is SOLVED as a construct: a same-value re-store of `out2` in loop1 delivers 5 refs and the COMPLETE target callee-saved disposition; the whole residual collapses to one surplus instruction

Chassis re-measured at session start by applying `alt_V15a_targetorder_purera_s25.c`
to `src/text1a_pre.c`: `sandbox func_80041188 --disable all` = **score 15, 132 target /
132 build insns**, `rules_dropped: 16` — reproducing s25's E-s25-4 exactly. Every number
below was measured this session. `src/text1a_pre.c` was restored to HEAD at the end and
no build-pipeline file was touched.

### E-s27-1 — THE LOAD-BEARING RESULT: a same-value re-store of `out2` inside loop1 lifts it 3 -> 5 references and buys ALL SEVEN target callee-saved seats

Form **D1** = V15a + `out2 = (s32 *)(((u8 *) pa4) + 0x20);` appended at the bottom of
loop1 (after `stptr += 0x68;`). The statement is semantically a no-op — out2 already
holds that value — but it is a REDEFINITION, so cse1 cannot fold it (loop1 is its own
extended basic block, E-s23-1) and it makes out2 loop-carried. ALLOCDBG:

| allocno | V15a | D1 |
|---|---|---|
| stptr | 7/41 = 3414 -> $s3 | 7/42 = 3333 -> $s3 |
| i | 10/97 = 3092 -> $s4 | 10/98 = 3061 -> $s4 |
| tbl | 6/47 = 2553 -> $s5 | 6/48 = 2500 -> $s5 |
| stptr2 | 6/48 = 2500 -> $s0 | 6/48 = 2500 -> $s0 |
| **out2** | 3/42 = 714 -> **$fp** | **5/42 = 2380 -> $s6** |
| **pa4** | 7/95 = 1473 -> **$s6** | 7/96 = 1458 -> **$s7** |
| **a3** | 4/99 = 808 -> **$s7** | 4/100 = 800 -> **$fp** |
| out3 | 3/47 = 638 -> $s3 | 3/47 = 638 -> $s3 |

`sandbox --disable all` = **score 3 at 133 build / 132 target insns**. The object diff
against the pinned target (tmp/grind/func_80041188/s23/odiff.py) is **three positions and
nothing else**: our loop-back branch delay slot holds the re-store's `addiu $s6,$s7,0x20`
and `addiu $s3,$s3,0x68` has been pushed up before the `slti`. Every other instruction,
including every register name in every block, is target's. This is the first form in the
grind's history in which the entire residual is ONE surplus instruction.

### E-s27-2 — position sweep: the surplus instruction is ABSORBABLE, and at one position the build returns to 132 insns

The re-store was swept over four in-loop1 positions (it may not precede out2's first use,
or the block-0 definition becomes dead and is deleted — that is E-s26-7's X1 at 141 insns):

| form | position | build insns | sandbox |
|---|---|---|---|
| D6 | after `func_8004A348(buf, out2);` (first use) | **132** | 8 |
| D5 | after `func_800523E0(pa4, out2, a3, stptr + 0x38);` | 133 | **2** |
| D7 | after `*((s16 *) (stptr + 6)) = 2;` | 133 | **2** |
| D1 | after `stptr += 0x68;` (loop bottom) | 133 | 3 |

**D6 is the structural result**: at that position sched1 hoists `addiu $a0,$sp,0x10` into
loop1's load-delay slot, so the `nop` target carries at `asm/funcs/func_80041188.s:30`
disappears and the extra addiu is paid for out of the nop — 132 build insns, seats still
target's. The price is that the whole loop1 body is re-ordered (score 8). So the residual
on the D chassis is no longer "one instruction too many"; it is a **sched1 emission-order
problem at 132/132 with target's complete register disposition** — precisely the shape
`tools/sched_solver` + s25's hand-built loop1 goal (E-s25-1) exist to attack.
Banked: `alt_D5_alltargetseats_score2_s27.c`, `alt_D6_132insns_schedresidual_s27.c`.

### E-s27-3 — PASS ATTRIBUTION (forensics): why every block-0 chain extender on out2 folds, named to the insn

Instrumented-cc1 `-da` dumps of form B1 (`out2 = pa4; out2 = (s32 *)((u8 *)out2 + 0x20);`),
in `tmp/grind/func_80041188/s27/B1/`:

- `red.i.rtl` insn 40 `(set (reg/v:SI 86) (reg/v:SI 77))`, insn 43
  `(set (reg/v:SI 86) (plus (reg/v:SI 86) (const_int 32)))`.
- `red.i.cse` insn 43 has become `(set (reg 86) (plus (reg/v:SI 77) (const_int 32)))` —
  **cse1's canon_reg replaced reg 86 with its quantity's first register, 77 (pa4)**.
  Insn 40 is thereby dead.
- `red.i.flow` no longer contains insn 40: **flow.c deleted it**, and per E-s21-5 an insn
  deleted by flow is never counted in REG_N_REFS. out2 stays at 3 refs.

The control case is in the SAME dump: insn 46 `(set (reg 88) (reg/v:SI 80))` / insn 49
`(set (reg 88) (plus (reg 88) (const_int 252)))` — the `stptr = base; stptr += 0xFC;`
chain. cse1 did **not** substitute reg 80 there; insn 46 is still present in `red.i.flow`
(so both references are counted) and is gone from `red.i.combine` — **combine merged it**,
which is why that chain is byte-free AND counted. So the s25/s26 empirical asymmetry
("base-rooted chains survive, pa4-rooted chains fold") is now attributed to a specific
pass and a specific decision: cse1 canon_reg substitution vs no substitution, with flow
deletion (uncounted) vs combine deletion (counted) as the consequence. The remaining
unknown is the cse.c predicate that distinguishes donor 80 from donor 77; the only
difference visible in the dumps is that 77's definition is a MEM with a REG_EQUAL note
(the incoming 5th argument, `(mem (plus ($0) 16))`) while 80's is a MEM with a register
address and no note.

### E-s27-4 — KILLED: the owner-sanctioned cancel-pair (`out2 += 8; out2 -= 8;`) cannot be placed, by arithmetic

The split/redundant-arithmetic class the owner allowed on 2026-08-27 moves a reference
count in quanta of **+4** when applied to a variable that has no existing increment
(2 insns x (1 set + 1 use)), not the +2 it delivers at an existing `i++` site. Measured:
form A1 (pair before the out2 call) and A2 (pair at loop1 top) both give out2
**7 refs / live 43 = 3255**; A3 (byte-cast spelling, +/-0x20) is identical. 3255 is not
placeable: to rank 5th, out2 must sit below stptr2 and above pa4, so tbl and stptr2 would
have to land between 3255 and stptr's 3333, and tbl's own reference quantum (6 -> 8 refs at
live 47/48 = 2553 -> 5106) contains no value in that interval. Lowering 7 refs into the
band instead needs live length >= 57 (pri = 2*7*10000/live < 2500), and E-s27-5 shows
out2's live length cannot be pushed past 43. Banked
`rejected/out2-cancel-pair-gives-plus4-refs-7-unplaceable.c`. Form A4 (the pair in
block 0) is worse still: cse1 folds it exactly as it folds B1 and out2 stays at
**3 refs / 42 / 714** — `rejected/out2-cancel-pair-block0-cse-folds-refs-rigid-3.c`.

### E-s27-5 — KILLED: out2's live length is not a dial (block-0 position is worth at most +1)

Form D3 hoists out2's definition to the FIRST statement of the function (before `saved`,
`stptr`, `base`) and adds the cancel pair: out2's live length moves 42 -> **43**, not the
~55 the insn distance suggests. Block-0 position is therefore worth at most one unit of
live length, which closes both the "3 refs / live 12..20" window from below and the
"7 refs / live >= 57" window from above. Banked
`rejected/out2-def-hoisted-to-block0-top-livelen-only-43.c`.

### E-s27-6 — KILLED: the same-value re-store OVERSHOOTS on the real-loop (Z1) chassis

Form E1 = `alt_Z1_realloop_honest_s26.c` + the loop-bottom re-store. flow.c's loop_depth
weighting counts every in-loop reference twice, so out2 goes 5/41 = 2439 ->
**8 refs / 42 = 5714**, outranking everything except the two parameter allocnos, and the
build grows to 143 model insns. The re-store construct and the real-loop chassis are
mutually exclusive: the construct is a goto-chassis instrument. Banked
`rejected/out2-redef-on-realloop-chassis-8refs-5714-overshoot.c`.

### E-s27-7 — KILLED: rooting out2's split in the PARAMETER is the same fold

`out2 = a4; out2 = (s32 *)((u8 *)out2 + 0x20);` (C1) and `out2 = (s32 *)((u8 *)a4 + 0x20);`
(C2) both leave out2 at **3 refs / 42 / 714**. The dumps explain why: cse1/copy propagation
has already merged the local alias `pa4` and the incoming parameter `a4` into ONE pseudo
(reg 77 — nrefs 7, live 95), so "root it in the parameter instead of the alias" is not a
distinct experiment at all. Banked
`rejected/out2-def-split-rooted-in-param-a4-same-pseudo-folds.c`.

### E-s27-8 — TOOL FINDING: the reduced-TU model is faithful here, but the FULL-TU dump path is broken, and `text1a_pre` is a `-G8` file

`src/text1a_pre.c` is in the Makefile's `GP_FILES` (Makefile:118), so the real build
compiles it with **-G8**, while `tmp/grind/func_80041188/s24/dump.sh` (used by s24-s26 and
this session) uses **-G0**. Re-running the reduced TU under -G8 for V15a and D1 gives
allocno tables and instruction counts **identical to -G0**, so the model's verdicts are
not affected — but the discrepancy should be fixed in the next session's dump script
rather than re-derived. Attempting a full-TU ALLOCDBG dump (the honest model) fails on
both cc1 binaries: `tools/gcc-2.7.2/cc1` (instrumented) segfaults and
`tools/gcc-2.7.2/build/cc1` exits 33 on `conflicting types for D_80094C68`
(include/code6cac.h:92 vs src/text1a_pre.c:310) when invoked from a preprocessed FILE
rather than the Makefile's stdin pipe. Artifacts: `tmp/grind/func_80041188/s27/full_*/`.

### s27 artifacts

`tmp/grind/func_80041188/s27/` — `probe.sh`, `dump.sh`, `g8run.sh`, `fulldump.sh`,
`fd2.sh`, `text1a_pre.HEAD.c`, the variant bodies `V15a.c` `A1.c` `A2.c` `A3.c` `A4.c`
`B1.c` `C1.c` `C2.c` `D1.c` `D3.c` `D5.c` `D6.c` `D7.c` `Z1.c` `E1.c`, and the per-tag cc1
dump directories `V15a/ A1/ A2/ A3/ A4/ B1/ C1/ C2/ D1/ D3/ Z1/ E1/` holding the full
`red.i.*` pass dumps (`red.i.rtl`, `red.i.cse`, `red.i.flow`, `red.i.combine` carry the
E-s27-3 attribution) plus `cc1.err` with the ALLOCDBG tables.

- [s27] Chassis re-measured: V15a applied to src/text1a_pre.c = sandbox 15 at 132/132, rules_dropped 16. HEAD restored at end of session; no build-pipeline file touched, no commit, no permuter campaign launched.
- [s27] The owner's 2026-08-27 split-increment ruling was executed: V15a (which carries both sanctioned split increments) was used as the chassis for every probe, and the honest out2 lift the ruling asked for was FOUND — but it is a different construct (a same-value re-store of a local), not a split increment.
- [s27] D1/D5/D7 (same-value re-store of out2 inside loop1) = sandbox 2-3 at 133 build / 132 target insns with ALL SEVEN callee-saved seats target's AND target's block-2 `addiu $s3,$s7,0x20`. Residual = one surplus instruction.
- [s27] D6 (re-store immediately after out2's first use) = 132 build / 132 target insns, sandbox 8: sched1 pays for the extra addiu out of loop1's load-delay nop. The residual there is emission order, not instruction count.
- [s27] Pass attribution for the block-0 fold, from the instrumented-cc1 dumps: cse1's canon_reg rewrites `out2 = out2 + 32` to `out2 = pa4 + 32` (red.i.cse insn 43), which kills the copy insn 40, which flow.c then deletes (absent from red.i.flow) — and flow-deleted insns are never counted. The `stptr = base; stptr += 0xFC` control in the same dump is NOT substituted by cse1, survives into red.i.flow (counted) and is deleted by combine (byte-free).
- [s27] The sanctioned cancel-pair moves a reference count by +4, not +2, when the variable has no existing increment site; out2 at 7 refs / 43 = 3255 is arithmetically unplaceable and out2's live length is rigid at 42-43.
- [s27] src/text1a_pre.c is a -G8 (GP_FILES) file; the s24 dump script uses -G0. Verified this session that -G8 and -G0 give identical allocno tables and insn counts for V15a and D1, so no prior conclusion is invalidated.

- [s27] Chassis re-measured this session: alt_V15a_targetorder_purera_s25.c applied to src/text1a_pre.c gives sandbox func_80041188 --disable all = score 15 at 132 build / 132 target insns, rules_dropped 16 — reproducing s25's E-s25-4 exactly. src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched, no commit was made, and no permuter campaign was launched.

- [s27] D1/D5/D7 (a same-value re-store of the local out2 inside loop1, three different positions) measure sandbox 3/2/2 at 133 build / 132 target insns with ALL SEVEN callee-saved seats target's (stptr $s3, i $s4, tbl $s5, stptr2 $s0, out2 $s6, pa4 $s7, a3 $fp) AND target's block-2 addiu $s3,$s7,0x20 present. The object diff for D1 is three positions and nothing else.

- [s27] D6 (the same re-store placed immediately after out2's first use) measures 132 build / 132 target insns at sandbox 8: sched1 fills loop1's load-delay slot (target's nop at asm/funcs/func_80041188.s:30) with addiu $a0,$sp,0x10 and the extra addiu costs nothing, at the price of re-ordering loop1.

- [s27] The allocno arithmetic that makes it work: out2 5 refs / live 42 = 2380 sits inside the (pa4 1458, stptr2 2500) band; the priority formula pri = floor_log2(nrefs) * nrefs * 10000 / live_length (E-s26-3) predicted it exactly.

- [s27] Pass attribution for the block-0 fold, read from instrumented-cc1 -da dumps rather than inferred: cse1's canon_reg rewrites out2 = out2 + 32 into out2 = pa4 + 32 (red.i.cse insn 43), the copy insn 40 becomes dead, and flow.c deletes it (absent from red.i.flow) — flow-deleted insns are never counted in REG_N_REFS. The stptr = base; stptr += 0xFC control chain in the same dump is NOT substituted by cse1, survives into red.i.flow and is deleted by combine, which is why it is byte-free AND counted.

- [s27] The owner-sanctioned split/redundant-arithmetic cancel pair moves a reference count by +4, not +2, on a variable with no existing increment site; out2 at 7 refs / live 43 = 3255 is arithmetically unplaceable and out2's live length is rigid at 42-43 regardless of where in block 0 its definition sits.

- [s27] cse1 has already merged the local alias pa4 and the incoming parameter a4 into one pseudo (reg 77, 7 refs / live 95), so 'root the chain in the parameter instead of the alias' is not a distinct experiment.

- [s27] TOOL FINDING: src/text1a_pre.c is a -G8 file (Makefile:118, GP_FILES) while the s24 dump script used by s24-s27 compiles the reduced TU with -G0. Re-measured both flags for V15a and D1: allocno tables and insn counts are identical, so no prior conclusion is invalidated, but the script should be pinned to -G8. A full-TU ALLOCDBG dump is currently impossible: tools/gcc-2.7.2/cc1 (instrumented) segfaults on the whole file and tools/gcc-2.7.2/build/cc1 exits 33 on 'conflicting types for D_80094C68' when fed a preprocessed file instead of the Makefile's stdin pipe.

## s28 (rederive, 2026-08-27) — the byte-free 4th reference to `out2` is CLOSED by a two-sided law: cse1's fold_rtx reassociation kills every same-EBB chain BEFORE flow counts, and combine — the only pass that deletes insns AFTER flow counts — has no deletable shape that target's bytes can host for `out2`

Chassis re-measured at session start (all three numbers measured this session,
`sandbox func_80041188 --disable all`): **HEAD = score 27**,
`alt_V15a_targetorder_purera_s25.c` = **score 15**,
`memory/grind/func_80041188/candidate.c` = **score 1** — all at 132 build / 132 target
insns, `rules_dropped: 16`. `src/text1a_pre.c` was restored to HEAD at the end of the
session; no build-pipeline file was touched, nothing was committed, and no permuter
campaign was launched.

### E-s28-1 — KILLED: the EXPRESSION-ROOTED block-0 split of `out2` (the last unspent horn of the F1 chain-extender class)

s22's E-s22-2 stated the F1 iff-law — "a split-init chain-extender delivers its +1
flow-counted reference iff the RECIPIENT outlives the DONOR" — and derived it from
cse.c's `make_regs_eqv` (cse.c:826-882): the copy `out2 = pa4` merges the two pseudos
into one quantity, `qty_first_reg` becomes whichever register lives longer and beyond the
current extended basic block, and `canon_reg` then rewrites the second insn's `out2` into
that canonical register. That law is reached ONLY when the chain's first insn has a bare
REG source, because `insert_regs` (cse.c:1024-1034) calls `make_regs_eqv` only when it
finds a REG element in the source's equivalence class.

So this session probed the horn the law does not cover: a chain whose first insn is an
EXPRESSION, which gives out2 its own fresh quantity (`make_new_qty`) and therefore no
canonical donor register to be rewritten into.

| form | block-0 spelling of out2's definition | out2 allocno | model insns | sandbox |
|---|---|---|---|---|
| V15a | `out2 = (u8*)pa4 + 0x20` | 3 refs / 42 / **714** -> `$fp` | 142 | 15 |
| F1 | `out2 = (u8*)pa4 + 0x28; out2 = (u8*)out2 - 8;` | 3 refs / 42 / **714** -> `$fp` | 142 | 15 |
| F2 | `out2 = (u8*)pa4 + 0x30; out2 = (u8*)out2 - 0x10;` | identical to F1 | 142 | 15 |
| F3 | `out2 = (u8*)pa4 - 0x10; out2 = (u8*)out2 + 0x30;` | identical to F1 | 142 | 15 |

Every allocno in the full ALLOCDBG table is bit-identical to V15a's in all three forms.

**PASS ATTRIBUTION, read from the dumps (`tmp/grind/func_80041188/s28/F1/`), not inferred.**
`red.i.rtl` insn 40 = `(set (reg/v:SI 86) (plus (reg/v:SI 77) (const_int 40)))`, insn 43 =
`(set (reg/v:SI 86) (plus (reg/v:SI 86) (const_int -8)))`. In `red.i.cse` insn 43 has
become `(set (reg/v:SI 86) (plus (reg/v:SI 77) (const_int 32)))` — cse.c's `fold_rtx`
substituted reg 86's KNOWN VALUE `(plus 77 40)` into the second insn and simplified the
two constants. This is **REASSOCIATION, a mechanism distinct from the canon_reg
substitution E-s27-3 attributed to the copy-rooted chain**, and it does not consult
lifetimes at all. Insn 40 is thereby dead and is absent from `red.i.flow` (flow.c deletes
it; per E-s21-5 a flow-deleted insn is never counted in `reg_n_refs`). out2 stays at 3
references.

**The block-0 lift class is now closed on both horns**: copy-rooted chains by the
make_regs_eqv/canon_reg law (E-s22-2, E-s27-3), expression-rooted chains by this
reassociation law. Because cse1 is an EBB pass, and block 2 sits inside loop1's EBB while
block 0 is its own EBB (E-s23-1), the same reassociation also closes a cancel/split chain
sited in block 2: within a single EBB cse1 always knows the intermediate's value. Banked:
`rejected/out2-block0-expr-rooted-split-cse1-reassociates-refs-rigid-3-F1.c` (plus the
`-F2` and `-F3` spellings).

### E-s28-2 — THE COMBINE-DELETION ENUMERATION: what a byte-free flow-counted reference can even look like here, and why `out2` cannot have one

`reg_n_refs` is fixed by flow.c's life analysis and nothing recomputes it (standing ledger
fact, E-s11-3). GCC 2.7.2's `toplev.c` runs `jump`, `cse1`, `loop` and `cse2` BEFORE flow
and `combine` immediately after it, so **combine is the only pass that can delete an insn
whose references flow has already counted.** A byte-free flow-counted reference is
therefore, by definition, a reference living in an insn that combine deletes.

Measured on the V15a chassis by diffing the insn ids present in `red.i.flow` (102 insns)
against `red.i.combine` (93 insns): combine deletes exactly nine insns, in exactly two
shapes.

- **Shape A — a reg-reg copy whose destination has a single use**: insn 4
  `(set 72 (reg a0))`, the parameter homing copy; and insn 43 `(set 88 (reg 80))`, the
  `stptr = base; stptr += 0xFC;` F1 chain-extender that candidate.c ships annotated.
- **Shape B — `(set p (plus R c))` merged into its single use**: insn 130
  `(set 107 (plus 88 56))` = `stptr + 0x38` folded into `func_800523E0`'s 4th argument;
  insns 177 `(set 110 (plus 91 76))` and 262 `(set 121 (plus 91 56))`, the same thing for
  `stptr2` in loop2; insns 88 and 100, V15a's two owner-allowed split increments
  (`tbl += 2; tbl -= 1;` and `i += 2; i -= 1;`); and insns 27/32, the `D_800A9A10`
  symbol/address pair.

This is why `stptr` legitimately carries 7 flow references while target emits only 5
`$s3` body insns, and why `stptr2` carries 6: both variables are used as `X + constant`
address expressions that TARGET ITSELF EMITS (`addiu $a3,$s3,0x38`, `addiu $a3,$s0,0x4C`,
`addiu $a3,$s0,0x38`). Their surplus references are not a trick; they are what the
original source's own call arguments compile to.

**`out2` has no such site.** `$s6` appears in exactly five lines of
`asm/funcs/func_80041188.s`: the prologue `sw` (asm:17), the epilogue `lw` (asm:126), the
definition `addiu $s6,$s7,0x20` (asm:25), and two identical argument moves
`addu $a1,$s6,$zero` (asm:56, asm:61). Target's 132 bytes contain **no `addiu $aN,$s6,c`**
— no shape-B host — and **no second `$s6`-rooted copy** — no shape-A host, since a
shape-A copy `X = out2` needs a single-use destination, and out2's only single-use
consumers are those two argument moves, into which combine merges the copy back to
exactly one out2 reference (net zero, as the s21-era in-loop1 detour forms measured).

**Statement of the closure.** On the two-locals goto chassis a byte-free fourth
flow-counted reference to `out2` does not exist: every pre-flow siting is folded by cse1
(canon_reg for copies, fold_rtx reassociation for expressions — E-s28-1), and every
post-flow siting needs a combine-deletable shape that target's own bytes cannot host. The
reference must therefore COST an instruction — which is exactly what s27's D family
measured (D1/D5/D7 = 133 insns at sandbox 2-3 with all seven target seats; D6 = 132 insns
at sandbox 8 because sched1 pays for the extra addiu out of loop1's load-delay nop).
s27's frontier item 2 ("a byte-free version of the reference lift exists — read cse.c and
name the predicate") is ANSWERED AND CLOSED: the predicate exists and is now named, but
the class it selects is empty against target's instruction stream.

### E-s28-3 — corollary: target's own emitted code has `out2` at three references, so the original's extra reference was combine-deleted — and both combine-deletable shapes are now enumerated and excluded

Counting target's bytes directly (previous paragraph), `$s6` appears in three body insns.
Under the s26/s27 priority formula `pri = floor_log2(nrefs) * nrefs * 10000 / live_length`
that is 3 refs / live 42 = 714, which seats out2 in `$fp`, not `$s6`. So the ORIGINAL
source cannot have compiled to exactly the reference count we can count in target's bytes:
it had at least a fourth reference that a post-flow pass removed. E-s28-2 enumerates the
only two shapes the only such pass removes, and neither can be hosted by an `$s6` value in
target's instruction stream.

The remaining ways out of that contradiction, in the order a future session should test
them:

(a) **Our model of some OTHER allocno is wrong**, so out2 at 714 is admissible after all
and a higher allocno is mis-counted. The least independently verified entries are the
`a1`/`a2` pair at 16 refs / live 99 and `i` at 10 refs / live 97-98. The test is cheap:
count `$s1`/`$s2`/`$s4` occurrences in target's bytes the way this session counted `$s6`,
and check them against the shape-A/shape-B hosting rule — every surplus reference a
variable carries must be visible in target as either an `X + constant` argument/address
expression or a single-use copy.

(b) **out2's live length below the 3-reference ceiling.** At 3 references out2 outranks
pa4 (1473) iff `30000 / live > 1473`, i.e. live <= 20; E-s23-4 called that window "real,
reachable and structurally unusable" and E-s27-5/E-s12-3 bounded block-0 definitions at
41-43 and in-loop1 definitions at 21 (with the cse-EBB fold destroying block 2). Now that
the reference axis is PROVABLY closed, the live-length axis is the only survivor on this
chassis and deserves one more direct assault — specifically an in-loop1 definition sited
so that block 2's `out3` is still computed from `pa4` (the fold that killed it in s23 is a
consequence of the definition being visible in loop1's EBB, so the question is whether any
in-loop1 definition can be placed AFTER out2's last use in loop1, where the EBB no longer
reaches block 2's `out3` computation).

(c) **The original is not the two-locals goto chassis at all.** E-s28-2's hosting rule is
the sharpest structural test the grind has produced: EVERY local's flow reference count
must be explainable from target's bytes as (emitted `$reg` insns) + (shape-A copies) +
(shape-B `X + constant` expressions). Applying that test to a candidate chassis before
measuring it would foreclose whole families in one read.

### s28 artifacts

`tmp/grind/func_80041188/s28/` — `probe.sh`, `dump.sh`, the variant bodies `V15a.c`
`F1.c` `F2.c` `F3.c` `CAND.c`, `text1a_pre.HEAD.c`, and the per-tag cc1 dump directories
`V15a/ F1/ F2/ F3/` holding the full `red.i.*` pass dumps plus `cc1.err` with the ALLOCDBG
tables. The E-s28-1 attribution is in `F1/red.i.rtl` (insns 40/43), `F1/red.i.cse` (insn
43 rewritten, insn 40 dead) and `F1/red.i.flow` (insn 40 absent); the E-s28-2 enumeration
is the insn-id set difference between `V15a/red.i.flow` and `V15a/red.i.combine`.

- [s28] Chassis re-measured this session: HEAD = sandbox 27, alt_V15a_targetorder_purera_s25.c = 15, candidate.c = 1 — all at 132 build / 132 target insns, rules_dropped 16. src/text1a_pre.c restored to HEAD at end of session; no build-pipeline file touched, nothing committed, no permuter campaign launched.
- [s28] KILLED the expression-rooted block-0 split of out2 in three constant spellings (+0x28/-8, +0x30/-0x10, -0x10/+0x30), all bit-identical to V15a at out2 = 3 refs / live 42 / pri 714: cse.c's fold_rtx REASSOCIATES `(plus 86 -8)` into `(plus 77 32)` using reg 86's known value, the first insn dies, and flow deletes it (uncounted). This closes the second and last horn of the F1 chain-extender class for out2; the copy-rooted horn was closed by the make_regs_eqv law in E-s22-2.
- [s28] Enumerated from the flow->combine insn-id difference on V15a the ONLY two shapes combine deletes in this function: a reg-reg copy whose destination has a single use, and `(set p (plus R c))` merged into its single use. Since flow fixes reg_n_refs and combine is the only later pass that deletes insns, a byte-free flow-counted reference MUST live in one of those two shapes.
- [s28] target's bytes host neither shape for out2: `$s6` occurs in exactly three body insns (`addiu $s6,$s7,0x20` at asm:25 and `addu $a1,$s6,$zero` at asm:56 and asm:61), with no `addiu $aN,$s6,c` and no second $s6 copy. A byte-free fourth flow-counted reference to out2 therefore does not exist on the two-locals goto chassis, and s27's frontier item 2 is CLOSED rather than open.
- [s28] New structural test for any future chassis (the hosting rule): every local's flow reference count must be explainable from target's own bytes as emitted-register insns plus shape-A single-use copies plus shape-B `X + constant` argument/address expressions. stptr's 7 and stptr2's 6 pass that test honestly (`addiu $a3,$s3,0x38`, `addiu $a3,$s0,0x4C`, `addiu $a3,$s0,0x38`); out2's required 4-or-5 fails it, which is evidence AGAINST the two-locals goto chassis being the original shape.

- [s28] Chassis re-measured this session with `sandbox func_80041188 --disable all`: HEAD = score 27, alt_V15a_targetorder_purera_s25.c = score 15, memory/grind/func_80041188/candidate.c = score 1 — all at 132 build / 132 target insns, rules_dropped 16. src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched, nothing was committed, and no permuter campaign was launched.

- [s28] The expression-rooted block-0 split of out2 is KILLED in three constant spellings (+0x28/-8, +0x30/-0x10, -0x10/+0x30), all bit-identical to V15a at out2 = 3 refs / live 42 / pri 714 and 142 model insns.

- [s28] PASS ATTRIBUTION from the instrumented-cc1 dumps (tmp/grind/func_80041188/s28/F1/): cse.c's fold_rtx rewrote `(set 86 (plus 86 -8))` into `(set 86 (plus 77 32))` by substituting reg 86's known value `(plus 77 40)` — reassociation, a mechanism distinct from the canon_reg substitution E-s27-3 attributed to the copy-rooted chain, and one that ignores lifetimes entirely. The dead first insn is absent from red.i.flow, i.e. flow.c deleted it and (per E-s21-5) never counted it.

- [s28] Both horns of the F1 chain-extender class are now closed for out2: copy-rooted chains by the make_regs_eqv/canon_reg law (E-s22-2), expression-rooted chains by this reassociation law. Because block 2 lies inside loop1's extended basic block while block 0 is its own (E-s23-1), the same reassociation also closes a cancel/split chain sited in block 2.

- [s28] COMBINE-DELETION ENUMERATION (new): on the V15a chassis combine deletes exactly nine insns in exactly two shapes — (A) a reg-reg copy whose destination has a single use, (B) `(set p (plus R c))` merged into its single use. Since flow fixes reg_n_refs and combine is the only later pass that deletes insns, a byte-free flow-counted reference must live in one of those two shapes.

- [s28] target's bytes host neither shape for out2: `$s6` appears in exactly three body insns (`addiu $s6,$s7,0x20` at asm:25, `addu $a1,$s6,$zero` at asm:56 and asm:61). A byte-free fourth flow-counted reference to out2 therefore does not exist on the two-locals goto chassis, and s27's frontier item 2 ('a byte-free version of the reference lift exists — read cse.c and name the predicate') is ANSWERED AND CLOSED: the predicate is named, and the class it selects is empty against target's instruction stream.

- [s28] THE HOSTING RULE (the reusable result): every flow-counted reference a local carries must be visible in target's own bytes as an emitted insn with that register, a shape-A single-use copy, or a shape-B `X + constant` argument/address expression. stptr's 7 references and stptr2's 6 decompose that way honestly — `addiu $a3,$s3,0x38`, `addiu $a3,$s0,0x4C`, `addiu $a3,$s0,0x38` are target's own call arguments, so those surplus references are what the ORIGINAL source compiled to, not a trick. out2's required 4-or-5 references have no such decomposition, which is the strongest evidence the grind has produced that the two-locals goto chassis is not the original's shape.

- [s28] Corollary for the next session: because target's emitted code has out2 at 3 references / live 42 = pri 714 (which seats it in $fp, not $s6), the original necessarily carried a reference a post-flow pass removed. With both post-flow shapes now enumerated and excluded, either another allocno in our model is mis-counted (a1/a2 at 16 refs, i at 10, tbl at 6 are the least independently verified), or out2's live length must drop below 20 (the 3-reference window), or the chassis is wrong.

- [s28] candidate.c is unchanged as the floor (sandbox 1, 132/132, all-target callee-saved seats) and now carries an s28 addendum recording the two closures and the hosting rule.

## s29 (rederive, 2026-08-27)

Chassis re-measured at the START of s29: `memory/grind/func_80041188/candidate.c` applied to
`src/text1a_pre.c` gives `sandbox func_80041188 --disable all` = **score 1 at 132 build / 132
target insns**, `rules_dropped: 16`, `cheat_asm_stripped: 0`. HEAD itself measures 27. So the
ledger floor is intact and candidate.c is still the shipping chassis. `src/text1a_pre.c` was
restored to HEAD at the end of the session; no build-pipeline file was touched, no commit was
made, no permuter campaign was launched.

### E-s29-1 — FRONTIER ITEM 1 EXECUTED AND KILLED: no allocno is mis-counted. The target-hosted reference census is complete and the model is exact.

s28's frontier item 1 asked whether one of the allocnos this grind never independently checked
against target's bytes (a1, a2, i, tbl, stptr, stptr2) is mis-counted, which would re-lay the
whole priority order. It is not. Every count was decomposed from `asm/funcs/func_80041188.s`
under E-s28-2's hosting rule (a set = 1 reference, a set-that-also-reads = 2, prologue `sw` /
epilogue `lw` of callee-saved registers are save/restore and not pseudo references). Full
decomposition with asm line numbers: `tmp/grind/func_80041188/s29/census.md`.

| hard reg | local | target hosts | honest chassis (H3) model | V15a model |
|---|---|---|---|---|
| $s1 | a1 | **16** | 16 ✓ | 16 ✓ |
| $s2 | a2 | **16** | 16 ✓ | 16 ✓ |
| $s3 | stptr | **5** | 5 ✓ | 7 (F1 chain-extender, +2 FAKE) |
| $s4 | i | **8** | 8 ✓ | 10 (split increment, +2 FAKE) |
| $s5 | tbl | **4** | 4 ✓ | 6 (split increment, +2 FAKE) |
| $s0 | stptr2 | **6** | 6 ✓ | 6 ✓ |
| $fp | a3 | **4** | 4 ✓ | 4 ✓ |
| $s7 | pa4 | **7** | 6 ✗ (−1) | 7 ✓ |
| $s6 | out2 | **3** | 4 ✗ (+1) | 3 ✓ |
| $s3 | out3 | **3** | 3 ✓ | 3 ✓ |

The census kills the mis-count hypothesis outright and replaces it with a sharper statement of
the residual. On the honest chassis **exactly one pair disagrees with target's bytes, and it is a
transfer, not an excess**: we spend on `out2` (4 refs) the reference target spends on `pa4`
(7 refs). Target's 7th `pa4` reference is `addiu $s3,$s7,0x20` at line 74 — `out3 = pa4 + 0x20`.
Ours is `out3 = out2`, which moves that reference from pa4 to out2 and emits `move $s3,$s6`.
That single transfer IS the floor-1 residual, now expressed as a conservation law rather than as
a missing instruction.

### E-s29-2 — THE HARD CONTRADICTION: target's own emitted reference/live profile does not produce target's own register assignment under the validated model.

Plug target's hosted counts into the exact priority formula (E-s26-3,
`pri = floor_log2(nrefs) * nrefs * 10000 / live_length`) with the live lengths this grind has
measured for the V15a chassis (the only chassis that emits target's block-2 `addiu $s3,$s7,0x20`,
hence the only chassis whose pseudo set matches target's):

    pa4  7 refs / live 95 = 1473
    a3   4 refs / live 99 =  808
    out2 3 refs / live 42 =  714

Descending priority seats those three in `$s6, $s7, $fp` **in that order**, i.e.
pa4 → $s6, a3 → $s7, out2 → $fp. Target's bytes show **out2 → $s6, pa4 → $s7, a3 → $fp**.
The seat-order rule itself has been validated in both directions on this function (E-s14-1), and
E-s29-1 now shows the reference counts fed into it are target's own. Therefore **target's source
contains at least one reference (or live-length effect) on `out2` that target's own emitted bytes
do not host.** E-s28-2 already enumerated the only two shapes combine can delete here (a reg-reg
copy with a single-use destination; `(set p (plus R c))` merged into its single use) and showed
target's bytes host neither for `$s6`. Together these two results promote E-s28-3(c) from a
suspicion to a derivation: **the two-locals goto chassis is not the original's shape, and no
spelling of it can be, because the contradiction is a property of the pseudo set rather than of
the statement order.** Any further work on this chassis is bounded above by floor 1.

### E-s29-3 — the `i` live-length ladder is fully enumerated, and requirement (A) has exactly one honest delivery

Requirement (A) (E-s14-1) is `stptr > i`. With target's hosted counts, stptr = 5 / 41 = 2439 and
i = 8 / L, so (A) needs `240000 / L < 2439`, i.e. **L ≥ 99**. Measured this session on the
un-split (honest, no F1 chain-extender) chassis with `BB2_ALLOC_DEBUG=1`:

| form | `i = 0x12;` source position in block 2 | `s32 i = 1;` decl position | livelen(i) | pri(i) | seats |
|---|---|---|---|---|---|
| H4 | 3 (target's own emission order) | 1st | **97** | 2474 | i → $s3, stptr → $s4 ✗ |
| candidate-unsplit | 3 | 2nd | **97** | 2474 | ✗ |
| H1 | 2 | 1st | **98** | 2448 | ✗ |
| H2 | 1 | 1st | **99** | 2424 | **ALL-TARGET** |
| H3 | 1 | 2nd | **99** | 2424 | **ALL-TARGET** |

Two closures. (1) **The declaration position of `i` in block 0 is completely inert** — H4 and the
unmodified un-split body both give livelen 97, so there is no block-0 dial and the only dial is
the source position of `i = 0x12;` in the between-loops block, worth +1 per statement it is moved
ahead of. (2) **L = 98 is not enough** (2448 > 2439), so `i = 0x12;` must be the FIRST statement
of block 2 — which is precisely the source order that costs the two sched1 emission-order diffs
(`li $s4,0x12` at slot 67 instead of 69), the cost s15's sched_solver already proved is repairable
only by moving `i = 0x12;` back (30 single atoms + all pairs, one vector, E-s15-1). Emission order
and requirement (A) are the same variable, now bounded from both sides by arithmetic instead of by
sampling. H2 ≡ H3 ≡ `alt_fakefree_floor3_s14.c`: **sandbox 3 at 132/132, re-measured this session
in both spellings**, ALL-TARGET seats, zero FAKE constructs.

Corollary, from E-s25-3 read together with this ladder: **there is no byte-free live-length lever
in the F1 / split-increment class.** A split increment lifts `reg_n_refs` (+2) but leaves
`reg_live_length` unchanged (i stayed 97, tbl stayed 47 in E-s25-3), because `reg_n_refs` is fixed
by flow.c before combine while live lengths are recomputed by sched1 *after* combine has folded the
split back. Any insn that lengthens a live range must therefore survive combine — i.e. it must be
emitted — unless it is removed by a pass that runs *after* sched1's recount (reload's no-op-move
deletion, jump2 cross-jumping). That is the one live-length class this grind has never tested.

### s29 disposition and artifacts

- Artifacts: `tmp/grind/func_80041188/s29/census.md` (the reference census with asm line numbers),
  `H1.c` / `H2.c` / `H3.c` / `H4.c` (the four ladder forms), `H1/`, `H2/`, `H3/`, `H4/`
  (`red.i`, `red.s`, `cc1.err` with the ALLOCDBG tables for each), `apply.py`, `dump.sh`,
  `text1a_pre.HEAD.c`.
- Rejected forms banked: `rejected/i-declaration-position-inert-for-live-length.c`,
  `rejected/i-live-98-insufficient-needs-99.c`.
- `candidate.c` is UNCHANGED and remains the floor (sandbox 1). No new candidate was produced;
  s29's product is a derivation that bounds the current chassis above at floor 1.

- [s29] Chassis re-measured at the start of s29: memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c gives sandbox func_80041188 --disable all = score 1 at 132 build / 132 target insns, rules_dropped 16, cheat_asm_stripped 0. HEAD itself measures 27. The ledger floor is intact and candidate.c remains the shipping chassis.

- [s29] TARGET-HOSTED REFERENCE CENSUS (new, complete; artifact tmp/grind/func_80041188/s29/census.md): $s1 a1 = 16, $s2 a2 = 16, $s3 stptr = 5, $s3 out3 = 3, $s4 i = 8, $s5 tbl = 4, $s6 out2 = 3, $s7 pa4 = 7, $fp a3 = 4, $s0 stptr2 = 6. Every count decomposed to asm/funcs/func_80041188.s line numbers under E-s28-2's hosting rule (prologue sw / epilogue lw of callee-saved registers are save/restore, not pseudo references).

- [s29] The census matches the honest-chassis ALLOCDBG model on eight of nine allocnos. The one disagreement is a TRANSFER, not an excess: our out2 carries 4 references and our pa4 6, where target carries 3 and 7. Target's seventh pa4 reference is `addiu $s3,$s7,0x20` (asm/funcs/func_80041188.s:74) = `out3 = pa4 + 0x20`; ours is `out3 = out2`, which relocates that reference onto out2 and emits `move $s3,$s6`. That transfer IS the floor-1 residual, now stated as a conservation law rather than as a missing instruction.

- [s29] V15a's three surplus counts over target (stptr 7 vs 5, i 10 vs 8, tbl 6 vs 4) are exactly its three FAKE lifts (the F1 chain-extender plus the two owner-sanctioned split increments), so V15a requires four references target's bytes cannot host while the honest chassis requires exactly one.

- [s29] HARD CONTRADICTION (E-s29-2): target's own hosted counts with the V15a live lengths give pa4 7/95 = 1473, a3 4/99 = 808, out2 3/42 = 714, which the validated descending-priority seat rule assigns as pa4 -> $s6, a3 -> $s7, out2 -> $fp. Target's bytes are out2 -> $s6, pa4 -> $s7, a3 -> $fp. Since E-s28-2 excluded both combine-deletable shapes for a hidden $s6 reference, the original's source is not this pseudo set, and no spelling of the two-locals goto chassis can reach distance 0. E-s28-3(c) is promoted from suspicion to derivation.

- [s29] reg_live_length ladder for i on the zero-FAKE chassis, measured with BB2_ALLOC_DEBUG=1: `i = 0x12;` at block-2 source position 3 -> 97, position 2 -> 98, position 1 -> 99; i's DECLARATION position in block 0 is inert (97 either way). Requirement (A) needs L >= 99 exactly, so emission order and requirement (A) are the same variable, bounded arithmetically from both sides rather than by sampling.

- [s29] Corollary drawn from E-s25-3 together with this ladder: there is NO byte-free live-length lever in the F1 / split-increment class. A split increment lifts reg_n_refs by 2 but leaves reg_live_length unchanged, because reg_n_refs is frozen by flow.c:2081 pre-combine while live lengths are recomputed by sched1 post-combine, after combine has folded the split away. A live-range extender must survive combine -- i.e. be emitted -- unless it is removed by a pass running after sched1's recount (reload's no-op-move deletion, jump2 cross-jumping). That class has never been tested here.

- [s29] alt_fakefree_floor3_s14.c re-measured this session in two spellings (H2, H3): sandbox 3 at 132 build / 132 target insns, ALL-TARGET callee-saved seats, zero FAKE constructs. Its three diffs are the two `li $s4,0x12` emission-order slots plus the out2/pa4 transfer.

- [s29] Correction for the next session: s28's frontier item 3 stated that D6's body was not banked in s27. It was -- memory/grind/func_80041188/alt_D6_132insns_schedresidual_s27.c exists alongside alt_D5_alltargetseats_score2_s27.c. It does not need re-deriving.

- [s29] src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched, no commit was made, no permuter campaign was launched, and no background process is running.

## s30 (structural, 2026-08-27) — the post-flow lever surface is now ENUMERATED from the compiler sources, the 30-session-old "a4 live 190 vs a3 live 99" is EXPLAINED, and the whole pa4-free chassis family is FORECLOSED in closed form

Chassis re-measured at the START of this session: `memory/grind/func_80041188/candidate.c`
applied to `src/text1a_pre.c` = `sandbox func_80041188 --disable all` **score 1, 132 build /
132 target insns, rules_dropped 16, cheat_asm_stripped 0**. `src/text1a_pre.c` was restored to
HEAD at the end of the session; no build-pipeline file was touched, nothing was committed, and
no permuter campaign was launched.

(A previous run of this same session index was discarded by the driver validator for a scope
violation, not for a measurement error. Its two struct-walker rejected forms and its
`tmp/grind/func_80041188/s30/` artifacts were left on disk; its headline kill is RE-MEASURED
FIRST-HAND below as E-s30-4 and its `find_reg` instrumentation is re-stated as E-s30-1 with the
artifact paths, so nothing in this entry rests on the discarded run's unverified claims.)

### E-s30-1 — INHERITED, artifacts on disk: `find_reg` has no preference dial in this function

`BB2_FINDREG_DEBUG` dumps at `tmp/grind/func_80041188/s30/CAND/fr_86.err`, `fr_77.err`,
`fr_75.err` (candidate chassis) show `hard_reg_copy_preferences`, `hard_reg_full_preferences`
and `regs_someone_prefers` ALL EMPTY for out2 (86), pa4 (77) and a3 (75), and `pass0_used` =
the full register file, so GCC 2.7.2's two-pass `find_reg` reduces exactly to "descending
allocno priority, then lowest-numbered non-conflicting register of {$s0..$s7,$fp}" — the rule
every session inferred black-box (E-s14-1) is instrumented, and there is no preference lever a
C spelling can reach.

### E-s30-2 — MECHANISM FOUND: `local-alloc.c:1064` doubles `reg_live_length` for any pseudo carrying a REG_EQUIV note — this is the whole of "a4 = 190 vs a3 = 99"

Every session since s5 has carried the unexplained pair "a3 4 refs / live 99" versus "a4 7 refs
/ live 190" for two parameters with nearly identical live ranges, and the standing frontier
called it "the load-bearing constant of the whole residual [that] has never been explained".
The mechanism is now named and verified end to end:

  * `flow.c`'s per-insn accounting (instrumented via `BB2_FLOW_DEBUG`, artifacts
    `s30/A1/flow_75.err`, `flow_76.err`) gives a3 **105** (b0:11 b1:41 b2:5 b3:48) and a4
    **104** (b0:10 b1:41 b2:5 b3:48) — nearly equal, no doubling.
  * `sched.c`'s post-combine recount (`BB2_SLL_DEBUG`, `s30/A1/sll_75.err`, `sll_76.err`)
    overwrites both at `sched.c:5106`: a3 = 8+40+5+46 = **99**, a4 = 40+5+46 = **95**. Still no
    doubling.
  * `tools/gcc-2.7.2/local-alloc.c:1058-1064`, inside `update_equiv_regs`, runs AFTER sched1:
    `if (note && reg_live_length[regno] >= 0) { ... reg_live_length[regno] *= 2; }` where
    `note` is a REG_EQUIV note. a4 is the FIFTH parameter, so expand sets its pseudo from the
    incoming stack slot and attaches a REG_EQUIV note to that MEM — verified in
    `s30/A1/red.i.sched` insn 12: `(set (reg/v:SI 76) (mem:SI (plus (reg $0) (const_int 16))))`
    carrying `(expr_list:REG_EQUIV (mem:SI (plus (reg $0) (const_int 16))))`. Hence 95 * 2 =
    **190**, exactly what `.lreg` and ALLOCDBG report.
  * a3 is the FOURTH parameter (a hard register), so its pseudo is set from a REG, not a MEM,
    and gets no note. The auto-created-note path (`local-alloc.c:1051-1055`) additionally
    requires `reg_basic_block[regno] >= 0`, i.e. the pseudo used in ONE basic block, which no
    contested allocno in this function satisfies. **The doubling is available to the stack
    parameter and to nothing else a C spelling of this function can reach.**
  * COROLLARY, and the reason candidate.c's chassis is the right one: introducing the `pa4`
    carrier local makes cse1 merge carrier and parameter into ONE pseudo whose SET is a REG
    copy rather than a MEM load, so no REG_EQUIV note is attached and the live length is NOT
    doubled (95, pri 1473 — re-measured on the candidate chassis this session).

### E-s30-3 — FORECLOSED IN CLOSED FORM: the pa4-free chassis family (s21 N1/CANDF, s22/s23 A1, s21 Q1) at target-hosted reference counts

Form A1 (no `pa4` carrier; `out3 = (s32 *)((u8 *)a4 + 0x20)` honest in block 2) re-measured
this session (`s30/A1/`): a1 16/99=6464 $s1, a2 16/99=6464 $s2, stptr 7/41=3414 $s3,
stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 4/47=1702 $s5, **a3 4/99=808 $s6, a4 7/190=736 $s7,
out2 3/42=714 $fp** — target's three contested seats inverted, model 142 insns.

With the doubling named, the inversion is an identity, not an accident:

    pri(a4) = floor_log2(7)*7*10000 / (2 * L4raw) = 70000 / L4raw
    pri(a3) = floor_log2(4)*4*10000 /      L3raw  = 80000 / L3raw

and `L4raw <= L3raw` on every spelling (measured 95 vs 99: a3's range starts one insn EARLIER
in block 0, both die at the same `func_800523E0` in loop2, and there is no region of this
function where a3 is live and a4 is not). Therefore **pri(a4) < pri(a3) unconditionally**, a3
is seated first and takes $s6 — the seat target gives out2.

Measured exchange rate confirming the doubling operationally (form **A1I** = A1 plus one extra
real store `*((s16 *)(stptr2 + 8)) = 1;` inside loop2, `s30/A1I/`): **a4 190 -> 192 (+2, i.e.
doubled), a3 99 -> 100 (+1), out2 42 -> 42 (unchanged — it is dead in loop2)**, priorities
736 -> 729 and 808 -> 800. Solving `140000/(190+2y) > 80000/(99+y)` gives `y < -67`: every insn
added where a4 is live pushes a4 DOWN twice as fast as a3, so no amount of loop2 work can
invert the pair. s21's Q1/N1 inverted it only by giving a4 an EIGHTH reference (the block-0
do-while(0) wrap), which crosses `floor_log2`'s 7->8 step and yields 3*8*10000/190 = 1263 —
one more reference than target's own bytes host (census E-s29-1: target hosts the matrix
pointer at 7). The family is dead at honest counts. Banked
`rejected/pa4-free-chassis-a4-regequiv-doubled-a3-always-outranks.c`.

**Structural conclusion about the ORIGINAL:** the original source carries a carrier local for
the fifth parameter. candidate.c's chassis is the original's, and the two-chassis ambiguity
that has run since s21 is resolved.

### E-s30-4 — RE-MEASURED FIRST-HAND: the struct-walker pseudo set (s29 frontier item 2) is dead

`rejected/struct-walker-member-makes-no-pseudo-rematerialised-4-sites.c` (a function-local
`typedef struct { s32 a[8]; s32 b[8]; } PAIR;` with `PAIR *pp = (PAIR *)a4;` and all four call
sites written `pp->a` / `pp->b`, `out2` and `out3` deleted) applied to `src/text1a_pre.c` and
measured this session: **sandbox `--disable all` = 55 at 132 build / 132 target insns**. The
ALLOCDBG table (`s30/S1/`) has NINE global allocnos instead of eleven: a struct-member address
expression forms no pseudo in GCC 2.7.2 and is rematerialised into a call-clobbered temp at
each use, so the hoped-for "the references currently attributed to out2 are attributed to the
walker" is false in this compiler. The mixed spelling (out2 kept for loop1, loop2's second
argument written `pp->b`) measures **48 at 134 build insns** — with no block-2 definition and
no loop notes on loop2 nothing hoists the address, so it is rematerialised at BOTH loop2 use
sites where target has exactly one insn, in block 2; banked
`rejected/struct-member-in-loop2-rematerialises-twice-no-block2-addiu.c`. Same family as
E-s12's `&m[0]` / `&m[1]` MATRIX spelling, reached from the other direction.

### E-s30-5 — THE STANDING "reg_n_refs IS FROZEN BY flow.c:2081" LAW IS FALSE, and the complete post-flow lever surface is now enumerated

s28, s29 and candidate.c's own FAKE annotation all rest on "flow.c fixes reg_n_refs during life
analysis and nothing recomputes it, so combine is the only pass that can host a byte-free
reference". Grepping every writer of `reg_n_refs` and `reg_live_length` in
`tools/gcc-2.7.2/*.c` shows that is not true. The COMPLETE set of sites that can change either
quantity after flow and before `global_alloc` reads them is:

| site | effect | gate |
|---|---|---|
| `combine.c:2313,2336` | `reg_n_refs[regno] = 0` for a register whose insns combine deleted | the two deletable shapes enumerated in E-s28-2 |
| `sched.c:5106` | `reg_live_length` OVERWRITTEN by sched1's own post-combine recount | always (this is why ALLOCDBG live lengths differ from flow's) |
| `local-alloc.c:781,783` (`optimize_reg_copy_1`) | TRANSFERS `reg_n_refs` from a copy's SRC to its DEST (`-= loop_depth` / `+= loop_depth` per replaced use) and adjusts both live lengths (`820`, `831`) | needs a REG_DEAD note for SRC on a later insn in the SAME basic block; the forward scan breaks at any CODE_LABEL, JUMP_INSN or loop note |
| `local-alloc.c:913,914` (`optimize_reg_copy_2`) | TRANSFERS `reg_n_refs` from DEST back to SRC across a `dest = src` ... `src = dest` round trip | same block-local scan, plus SRC must die at the first copy |
| `local-alloc.c:1064` (`update_equiv_regs`) | `reg_live_length[regno] *= 2` | a REG_EQUIV note (E-s30-2) |
| `local-alloc.c:1110` | `reg_n_refs[regno] = 0` when a 2-reference pseudo is replaced by its equivalence | REG_EQUIV note + exactly 2 references |

**Neither copy optimisation can be aimed at out2, for two independent structural reasons.**
(i) Both require a REG_DEAD note on the copy's source AT a later insn of the same basic block.
`out2` is defined in block 0 and is loop-carried through loop1 — it is live-out of loop1's body
on the back edge, so flow places no REG_DEAD note for it anywhere inside loop1, and it is
therefore never an eligible SRC. (ii) A copy sited in block 0 cannot reach out2's uses at all:
the forward scan of both functions breaks at `CODE_LABEL`, and loop1's label sits between block
0 and every use of out2. A copy sited INSIDE loop1 would be an in-loop1 DEFINITION, which
E-s23-1's cse-EBB law folds block 2's `out3 = pa4 + 0x20` into the residual `move`. So the
newly-discovered pass EXTENDS E-s28-2's enumeration rather than reopening it: out2's missing
fourth flow-counted reference cannot come from combine, and cannot come from local-alloc
either.

### s30 artifacts

`tmp/grind/func_80041188/s30/` — `apply.py`, `dump.sh`, `fr.sh`, `flow.sh` (new: per-block
`BB2_FLOW_DEBUG` live-length histogram), `sll.sh` (new: `BB2_SLL_DEBUG` sched1 per-segment
live-length dump), the variant bodies `A1.c` / `A1I.c` / `S1.c` / `S2.c`, and the per-tag dump
directories `CAND/` `A1/` `A1I/` `S1/` holding `red.i.*` pass dumps, `cc1.err` (ALLOCDBG),
`flow_*.err`, `sll_*.err`, `fr_*.err`.

- [s30] Chassis re-measured this session: candidate.c applied to src/text1a_pre.c = sandbox 1 at 132 build / 132 target insns, rules_dropped 16. src restored to HEAD at end of session; no build-pipeline file touched, nothing committed, no permuter campaign launched.
- [s30] MECHANISM: local-alloc.c:1064 (update_equiv_regs) doubles reg_live_length for any pseudo carrying a REG_EQUIV note. The fifth (stack) parameter a4 gets such a note from expand (verified: s30/A1/red.i.sched insn 12), so its 95 becomes 190. a3, a register parameter, gets none. This explains the 30-session-old unexplained "a4 190 vs a3 99" and it is the ONLY live-length multiplier in the compiler.
- [s30] flow.c's own per-insn counts are a3 105 / a4 104 (s30/A1/flow_*.err) and sched1's recount is a3 99 / a4 95 (s30/A1/sll_*.err) — so the ALLOCDBG live lengths are sched1's values, times two for REG_EQUIV pseudos. Neither flow nor sched1 doubles anything.
- [s30] FORECLOSED: the whole pa4-free chassis family (s21 N1/CANDF, s22/s23 A1, s21 Q1) at target-hosted reference counts. pri(a4) = 70000/L4raw versus pri(a3) = 80000/L3raw with L4raw <= L3raw (95 vs 99), so a3 outranks a4 unconditionally and takes $s6. Measured exchange rate (A1I, one extra loop2 insn): a4 +2, a3 +1, out2 +0 — every added insn makes it worse; the inversion needs y < -67 insns. s21's Q1/N1 worked only by giving a4 an EIGHTH reference, one more than target's bytes host.
- [s30] STRUCTURAL FACT ABOUT THE ORIGINAL: the original source carries a carrier local for the fifth parameter (candidate.c's `pa4`), because the carrier makes cse1 merge parameter and carrier into a pseudo whose SET is a REG copy, so no REG_EQUIV note is attached and the live length is not doubled (95, pri 1473). The s21 two-chassis ambiguity is resolved in favour of candidate.c's chassis.
- [s30] RE-MEASURED FIRST-HAND: the struct-walker pseudo set is dead — S1 (all four call sites as `pp->a`/`pp->b`) = sandbox 55 at 132/132 with NINE allocnos instead of eleven (a struct-member address expression forms no pseudo and is rematerialised per use); S2 (loop2 only) = 48 at 134 insns.
- [s30] THE "reg_n_refs IS FROZEN BY flow" LAW IS FALSE: local-alloc.c's optimize_reg_copy_1 (781/783) and optimize_reg_copy_2 (913/914) TRANSFER reg_n_refs between pseudos after flow and before global_alloc, and also adjust reg_live_length (820/831). The complete post-flow surface is now enumerated (combine 2313/2336, sched 5106, local-alloc 781/783/820/831/913/914/1064/1110).
- [s30] Neither copy optimisation can host out2's missing fourth reference: both require a REG_DEAD note on the copy's SOURCE at a later insn of the same basic block, and out2 — defined in block 0, loop-carried through loop1 — has no REG_DEAD note anywhere in loop1; the forward scan also breaks at loop1's CODE_LABEL, so a block-0 copy cannot reach out2's uses. E-s28-2's enumeration is extended, not reopened.

- [s30] Chassis re-measured at session start: memory/grind/func_80041188/candidate.c applied to src/text1a_pre.c = sandbox func_80041188 --disable all score 1, 132 build / 132 target insns, rules_dropped 16, cheat_asm_stripped 0. src/text1a_pre.c restored to HEAD at the end of the session; no build-pipeline file touched, nothing committed, no permuter campaign launched.

- [s30] MECHANISM (new, 30 sessions old as a mystery): local-alloc.c:1058-1064 doubles reg_live_length for any pseudo carrying a REG_EQUIV note. The fifth (stack) parameter a4 gets such a note from expand (verified in tmp/grind/func_80041188/s30/A1/red.i.sched insn 12), so its sched1 value 95 becomes 190; a3, a register parameter, gets none and stays 99.

- [s30] flow.c's own per-insn counts are a3 105 / a4 104 (s30/A1/flow_75.err, flow_76.err) and sched1's post-combine recount is a3 99 / a4 95 (s30/A1/sll_75.err, sll_76.err), so ALLOCDBG live lengths are sched1's values, doubled for REG_EQUIV pseudos. Neither flow nor sched1 doubles anything.

- [s30] FORECLOSED: the whole pa4-free chassis family (s21 N1/CANDF/Q1, s22/s23 A1) at target-hosted reference counts. pri(a4) = 70000/L4raw versus pri(a3) = 80000/L3raw with L4raw <= L3raw (95 versus 99), so a3 outranks a4 unconditionally and takes $s6, the seat target gives out2. Measured exchange rate (form A1I, one extra real store in loop2): a4 +2, a3 +1, out2 +0; the inversion would need y < -67 added insns. s21's Q1/N1 worked only by giving a4 an EIGHTH reference, one more than target's bytes host.

- [s30] STRUCTURAL FACT ABOUT THE ORIGINAL: the original source carries a carrier local for the fifth parameter (candidate.c's pa4). The carrier makes cse1 merge parameter and carrier into one pseudo whose SET is a REG copy rather than a MEM load, so no REG_EQUIV note is attached and the live length is not doubled (95, pri 1473). The two-chassis ambiguity open since s21 is resolved in favour of candidate.c's chassis.

- [s30] RE-MEASURED FIRST-HAND: the struct-walker pseudo set is dead. S1 (all four call sites as pp->a / pp->b) = sandbox 55 at 132/132 with nine global allocnos instead of eleven; S2 (loop2 only) = sandbox 48 at 134 insns. A struct-member address expression forms no pseudo in GCC 2.7.2 and is rematerialised at every use.

- [s30] THE STANDING 'reg_n_refs IS FROZEN BY flow' LAW IS FALSE: local-alloc.c's optimize_reg_copy_1 (781/783) and optimize_reg_copy_2 (913/914) transfer reg_n_refs between pseudos after flow and before global_alloc, and adjust reg_live_length at 820/831. The complete post-flow surface is now enumerated: combine.c 2313/2336, sched.c 5106, local-alloc.c 781/783/820/831/913/914/1064/1110.

- [s30] Neither copy optimisation can host out2's missing fourth reference: both require a REG_DEAD note on the copy's SOURCE at a later insn of the same basic block; out2 has no REG_DEAD note anywhere inside loop1 (it is loop-carried and live-out on the back edge), and the forward scan breaks at loop1's CODE_LABEL so a block-0 copy cannot reach out2's uses. E-s28-2's enumeration is extended, not reopened.

- [s30] INHERITED WITH ARTIFACTS ON DISK (from the driver-discarded previous run of this session index, not re-measured): BB2_FINDREG_DEBUG dumps at s30/CAND/fr_86.err, fr_77.err, fr_75.err show hard_reg_copy_preferences, hard_reg_full_preferences and regs_someone_prefers all EMPTY for out2/pa4/a3 with pass0_used the full register file, so find_reg reduces exactly to descending allocno priority then lowest-numbered non-conflicting register of {$s0..$s7,$fp}.

## s31 (structural, 2026-08-27) — the D-family's block-2 claim is CORRECTED, the fold is named as a `cse_end_of_basic_block` CODE_LABEL effect and proven breakable, and the Z1 real-loop chassis is re-measured into a ONE-REFERENCE residual with target's whole seat table minus one swap

Chassis re-measured at the start of this session, all with `sandbox func_80041188
--disable all` and the edit in place in `src/text1a_pre.c`: **HEAD = score 27** (132 build /
132 target insns, `rules_dropped: 16`, `cheat_asm_stripped: 2`), **candidate.c = score 1**
(132/132, `rules_dropped: 16`), `alt_V15a_targetorder_purera_s25.c` = **15** (132/132),
`alt_D5_alltargetseats_score2_s27.c` = **2** (133 build / 132 target),
`alt_D6_132insns_schedresidual_s27.c` = **8** (132/132), D7 = **2** (133), D1 = **3** (133),
`alt_Z1_realloop_honest_s26.c` = **13** (132/132). `src/text1a_pre.c` was restored to HEAD at
the end of the session; no build-pipeline file was touched, nothing was committed, and no
permuter campaign was launched.

### E-s31-1 — CORRECTION OF THE LEDGER: the D-family does NOT deliver target's block-2 `addiu $s3,$s7,0x20`; D5 is candidate.c's residual PLUS a surplus instruction

s27's E-s27-1 recorded that D1/D5/D7 give "ALL SEVEN target callee-saved seats AND target's
block-2 `addiu $s3,$s7,0x20`". Objdump of D5's own sandbox object this session
(`mipsel-linux-gnu-objdump -d tmp/sandbox/func_80041188/text1a_pre.o`, diffed with
`tmp/grind/func_80041188/s23/odiff.py`) shows the block-2 insn is **`move $s3,$s6`**, i.e.
exactly candidate.c's residual, and the two scored positions are (i) the surplus in-loop1
`addiu $s6,$s7,0x20` of the re-store itself and (ii) that `move`. So the D family at 133 insns
is **strictly worse than candidate.c**, not a step past it: it buys the seat table candidate.c
already has and pays one extra instruction for it. Every D-chassis conclusion that assumed the
block-2 addiu was present (s27 frontier item 3, the "one surplus instruction" framing) must be
re-read with this correction. Banked
`rejected/d5-restore-block2-folds-to-move-strictly-worse-than-candidate.c`.

### E-s31-2 — MECHANISM NAMED AND CONFIRMED TWICE: the block-2 fold is `cse_end_of_basic_block`'s CODE_LABEL scan, and a CODE_LABEL at block 2's start breaks it

`tools/gcc-2.7.2/cse.c:8038` scans `while (p && GET_CODE (p) != CODE_LABEL)`, so cse1's basic
block runs from loop1's label through loop1's body AND STRAIGHT ON THROUGH BLOCK 2 (there is no
CODE_LABEL between loop1's conditional back-branch and block 2; the next label is `loop2:`).
An in-loop1 re-store of `out2` therefore enters `(plus pa4 32)` into cse's hash table with out2
in its class, and block 2's `out3 = (s32 *)((u8 *)pa4 + 0x20)` is rewritten to the cheaper
register — the `move`. Two independent measurements confirm the CODE_LABEL is the whole gate:

  * **G2** (D5 plus an extra early-exit `if (i >= 0x12) goto blk2;` inside loop1 and `blk2:`
    at block 2's start — a pure instrument): block 2 emits **`addiu $s5,$s4,32`**, an addiu.
    136 insns, sandbox 60 (the extra branch permutes everything else).
  * **H1** (D5 with loop1 rewritten as a PRE-TEST goto loop, so `blk2:` is a genuine
    jump.c-proof CODE_LABEL preceded by a BARRIER): block 2 emits **`addiu $s3,$s5,32`**.
    135 insns, sandbox 22.

This is the first time in the grind that the block-2 `move`/`addiu` choice has been attributed
to a named cse routine and then switched by measurement.

### E-s31-3 — KILLED: the honest source of that CODE_LABEL (a pre-test loop1) costs two instructions target does not have

Rewriting loop1 as `loop1: if (i >= 0x12) goto blk2; <body> goto loop1; blk2:` is ordinary C
(it is a `while` loop spelled with gotos) and jump.c does NOT rotate it back into a bottom test
— the label survives — but the shape emits an unconditional `j` plus its delay slot at the loop
bottom in ADDITION to the exit test: **H2** (V15a + pre-test) = sandbox 21 at **134** insns,
**H3** (candidate.c + pre-test) = sandbox 7 at **134** insns, **H1** (D5 + pre-test) = 22 at
**135**. Target is 132. The label itself is free; the loop shape that produces it is not.
Banked `rejected/pretest-loop1-label-breaks-cse-fold-but-costs-two-insns.c` and
`rejected/pretest-loop1-on-candidate-chassis-134-insns.c`.

### E-s31-4 — KILLED: a same-value re-store of the DONOR (`pa4 = pa4;`) is not a cse-table invalidator

**G1** = D5 plus `pa4 = pa4;` as loop1's last statement, on the theory that setting pa4 would
make cse `invalidate` every expression containing pa4 and so remove the `(plus pa4 32)` entry
before block 2. Measured: sandbox **2 at 133 insns**, byte-identical to D5, block 2 still
`move $s3,$s6`. cse recognises the no-op move and neither emits nor invalidates. Banked
`rejected/pa4-self-assign-does-not-invalidate-cse-plus-table.c`.

### E-s31-5 — KILLED: the block-2 dead-store pair, in both orders

**G3** = candidate.c with block 2 written `out3 = out2; out3 = (s32 *)((u8 *)pa4 + 0x20);`
(the reference to out2 first, target's spelling second): sandbox **15 at 132 insns** — bit-for-bit
V15a's result, i.e. the dead store is deleted before flow counts and out2 falls back to three
references and loses $s6. **G4** = the same pair in the opposite order: sandbox **1 at 132**,
i.e. candidate.c exactly (the first assignment is the dead one and the surviving statement is
the `move`). A dead store in block 2 is therefore not a reference-lift site in either order,
which closes the block-2 half of the "byte-free fourth reference" question by measurement
rather than by derivation. Banked
`rejected/block2-dead-store-out3-from-out2-dropped-before-flow.c`.

### E-s31-6 — THE LOAD-BEARING RESULT: on the Z1 real-loop chassis every seat is target's except ONE SWAP, and the whole gap is a single UNWEIGHTED reference to out2

`alt_Z1_realloop_honest_s26.c` (loop1 as a plain `do { ... } while (i < 0x12);`, loop2 still a
goto loop, no split increments, no chain extender, block 2 spelled `out3 = (s32 *)((u8 *)pa4 +
0x20)`) re-measured this session at **sandbox 13, 132 build / 132 target insns**. Its full
ALLOCDBG table (`tmp/grind/func_80041188/s31/Z1/fr_86.err`) is:

| pseudo | local | nrefs / livelen | pri | seat | target |
|---|---|---|---|---|---|
| 73 | a1 | 17 / 99 | 6868 | $s1 | $s1 |
| 74 | a2 | 17 / 99 | 6868 | $s2 | $s2 |
| 125 | stptr | 9 / 40 | 6750 | $s3 | $s3 |
| 78 | i | 11 / 97 | 3402 | $s4 | $s4 |
| 79 | tbl | 7 / 47 | 2978 | $s5 | $s5 |
| **77** | **pa4** | **9 / 94** | **2872** | **$s6** | **$s7** |
| 91 | stptr2 | 6 / 48 | 2500 | $s0 | $s0 |
| **86** | **out2** | **5 / 41** | **2439** | **$s7** | **$s6** |
| 75 | a3 | 5 / 99 | 1010 | $fp | $fp |
| 87 | out3 | 3 / 47 | 638 | $s3 | $s3 |

Two consequences, both new:

1. **flow.c's loop_depth weighting delivers `stptr` HONESTLY on this chassis.** Z1 spells stptr
   as the single statement `stptr = base + 0xFC;` and still reaches 9 references / live 40 =
   6750, comfortably above `i`. candidate.c needs its `/* FAKE */` F1 chain-extender
   (`stptr = base; stptr += 0xFC;`) to reach 7 / 41 = 3414 for the same ordering. **The
   real-loop chassis removes candidate.c's only FAKE construct.**
2. **The residual is now ONE unweighted reference.** out2 needs to outrank pa4's 2872. At live
   41 and six references the priority is `floor_log2(6)*6*10000/41 = 2926 > 2872`, and the
   `BB2_FINDREG_DEBUG` conflict dump for pseudo 86 lists hard reg 16 ($s0) among out2's
   conflicts, so promoting out2 above stptr2 cannot cost stptr2 its seat — the promotion swaps
   exactly out2 and pa4 and leaves the other eight allocnos where they are. Because loop1 is a
   REAL loop, an in-loop1 reference is weighted x2 (that is E-s27-6's overshoot to 8 refs /
   5714); the +1 the chassis needs must therefore be an **unweighted (block-0 or block-2)**
   reference to out2 that emits no byte.

### E-s31-7 — the Z1 chassis's OWN residual, read from the object diff

Z1's 13 scored positions decompose as: the out2/pa4 swap (five `addu $a1,$s6/$s7,$zero` and
`addu $a0,...` positions plus the two block-0 `addiu`s), and **three positions caused by
loop.c's strength reduction of `stptr`**: our block 0 emits `addiu $s3,$v0,0x134` where target
emits `addiu $s3,$v0,0xFC`, and the two in-loop uses become `addu $a3,$s3,$zero` and
`sh $t0,-0x32($s3)` where target has `addiu $a3,$s3,0x38` and `sh $v0,6($s3)`. That is loop.c
replacing `stptr` with the giv `stptr + 0x38` — the pass that only exists because loop1 is a
real loop. So the real-loop chassis buys stptr's priority honestly but pays for it with a giv
that shifts stptr's base; defeating that giv (`.claude/rules/strength-reduce-defeat.md`) is a
separate, ordinary-C problem worth three of the thirteen positions.

### s31 artifacts

`tmp/grind/func_80041188/s31/` — `ap.sh` (pin HEAD + apply a variant body), `apply.py`,
`text1a_pre.HEAD.c`, the variant bodies `CAND.c` `V15a.c` `D1.c` `D5.c` `D6.c` `D7.c` `Z1.c`
`G1.c` `G2.c` `G3.c` `G4.c` `H1.c` `H2.c` `H3.c`, and `Z1/` (cpp + reduced TU + `fr_86.err`
carrying the ALLOCDBG table and the FINDREGDBG conflict dump quoted in E-s31-6).

- [s31] Chassis re-measured this session: HEAD = 27, candidate.c = 1, V15a = 15, D5 = 2 (133 insns), D6 = 8 (132), D7 = 2 (133), D1 = 3 (133), Z1 = 13 (132) — all against 132 target insns with rules_dropped 16. src/text1a_pre.c restored to HEAD at the end; no build-pipeline file touched, nothing committed, no permuter campaign launched.
- [s31] LEDGER CORRECTION: D5's block-2 insn is `move $s3,$s6`, NOT target's `addiu $s3,$s7,0x20` as E-s27-1 recorded. D5 = candidate.c's residual plus one surplus instruction, i.e. the whole D family is strictly worse than candidate.c.
- [s31] MECHANISM: the block-2 fold is cse_end_of_basic_block (cse.c:8038, `while (p && GET_CODE (p) != CODE_LABEL)`). loop1's body and block 2 are ONE cse basic block, so an in-loop1 re-store puts (plus pa4 32) in the hash table and block 2's addiu is rewritten to the cheaper register. Confirmed by switching it twice: with a CODE_LABEL at block 2's start (G2, H1) block 2 emits an addiu.
- [s31] KILLED: the honest CODE_LABEL source (pre-test loop1, `while` spelled with gotos) costs +2 insns — H2 = 21 at 134, H3 = 7 at 134, H1 = 22 at 135. jump.c does not rotate the pre-test away, so the label survives, but the shape emits an unconditional j + delay slot the target does not have.
- [s31] KILLED: `pa4 = pa4;` does not invalidate cse's (plus pa4 32) entry (G1 = 2 at 133, block 2 still `move`).
- [s31] KILLED: the block-2 dead-store pair in both orders — G3 (`out3 = out2;` then target's spelling) = 15 at 132, identical to V15a because the dead store is dropped before flow counts; G4 (reverse) = 1 at 132, identical to candidate.c.
- [s31] LOAD-BEARING: on the Z1 real-loop chassis EVERY allocno except out2 and pa4 is on target's seat, and the two are swapped by a 433-unit priority gap (pa4 9/94 = 2872 vs out2 5/41 = 2439). out2 at SIX references and live 41 gives 2926 > 2872, and out2's FINDREGDBG conflict set already contains $s0, so the promotion swaps exactly those two seats.
- [s31] LOAD-BEARING: the real-loop chassis reaches stptr 9 refs / live 40 = 6750 from the single honest statement `stptr = base + 0xFC;`, because flow.c weights in-loop references by loop_depth. candidate.c needs its /* FAKE */ F1 chain-extender to reach 3414 for the same ordering — so the Z1 chassis REMOVES candidate.c's only FAKE construct.
- [s31] On the Z1 chassis loop1 is a real loop, so an in-loop reference is weighted x2 (E-s27-6's overshoot). The +1 reference the chassis needs must be an UNWEIGHTED block-0 or block-2 reference to out2 that emits no byte.
- [s31] Three of Z1's thirteen scored positions are loop.c strength reduction of stptr (our `addiu $s3,$v0,0x134` + `addu $a3,$s3,$zero` + `sh $t0,-0x32($s3)` versus target's `addiu $s3,$v0,0xFC` + `addiu $a3,$s3,0x38` + `sh $v0,6($s3)`) — a giv that only exists because loop1 is a real loop, and an ordinary-C problem (.claude/rules/strength-reduce-defeat.md) separate from the seat swap.

- [s31] Chassis re-measured this session with the edit in place in src/text1a_pre.c and `sandbox func_80041188 --disable all`: HEAD = 27, candidate.c = 1, V15a = 15, D5 = 2 (133 build insns), D6 = 8 (132), D7 = 2 (133), D1 = 3 (133), Z1 = 13 (132) — all against 132 target insns with rules_dropped 16. src/text1a_pre.c was restored to HEAD at the end of the session; no build-pipeline file was touched, nothing was committed, and no permuter campaign was launched.

- [s31] LEDGER CORRECTION: D5's block-2 instruction is `move $s3,$s6`, not target's `addiu $s3,$s7,0x20` as s27's E-s27-1 recorded. D5 is candidate.c's residual PLUS one surplus instruction, so the entire D family (D1/D5/D6/D7) is strictly worse than candidate.c and the 's27 collapsed the residual to one surplus instruction' framing is withdrawn.

- [s31] MECHANISM: the block-2 move/addiu choice is cse_end_of_basic_block (tools/gcc-2.7.2/cse.c:8038, `while (p && GET_CODE (p) != CODE_LABEL)`). loop1's body and block 2 are ONE cse basic block, so an in-loop1 re-store of out2 puts (plus pa4 32) into cse's hash table with out2 in its class and block 2's own pa4+0x20 is rewritten to the cheaper register.

- [s31] The fold is switchable from C: with a CODE_LABEL at block 2's start, block 2 emits an addiu again — G2 (instrument, extra in-loop early exit) gives `addiu $s5,$s4,32` at 136 insns / sandbox 60, H1 (pre-test loop1) gives `addiu $s3,$s5,32` at 135 insns / sandbox 22.

- [s31] KILLED: the honest source of that CODE_LABEL. A pre-test (`while`-shaped) loop1 survives jump.c but costs an unconditional `j` plus its delay slot: H2 (V15a) = 21 at 134 insns, H3 (candidate.c) = 7 at 134, H1 (D5) = 22 at 135, versus 132 target insns.

- [s31] KILLED: `pa4 = pa4;` at loop1's bottom is not a cse-table invalidator (G1 = 2 at 133, byte-identical to D5, block 2 still `move`).

- [s31] KILLED: the block-2 dead-store pair in both orders. G3 (`out3 = out2;` then target's spelling) = 15 at 132, identical to V15a because the dead store is dropped before flow counts; G4 (reverse) = 1 at 132, identical to candidate.c. The block-2 half of the byte-free-extra-reference question is now closed by measurement.

- [s31] LOAD-BEARING: on the Z1 real-loop chassis (sandbox 13 at 132/132) EVERY allocno except out2 and pa4 sits on target's seat, and those two are swapped by a 433-unit priority gap — pa4 9 refs / live 94 = 2872 -> $s6 versus out2 5 / 41 = 2439 -> $s7. out2 at six references and live 41 gives 2926 > 2872 and flips exactly that pair.

- [s31] LOAD-BEARING: the Z1 chassis reaches stptr 9 refs / live 40 = 6750 from the single honest statement `stptr = base + 0xFC;`, because flow.c weights in-loop references by loop_depth — so the real-loop chassis REMOVES candidate.c's only /* FAKE */ construct (the F1 chain-extender).

- [s31] Because loop1 is a real loop on the Z1 chassis, an in-loop reference is weighted x2 (E-s27-6's overshoot to 8 refs / 5714), so the +1 that chassis needs must be an UNWEIGHTED block-0 or block-2 reference to out2 that emits no byte.

- [s31] Three of Z1's thirteen scored positions are loop.c strength reduction of stptr: our `addiu $s3,$v0,0x134` + `addu $a3,$s3,$zero` + `sh $t0,-0x32($s3)` versus target's `addiu $s3,$v0,0xFC` + `addiu $a3,$s3,0x38` + `sh $v0,6($s3)`. That giv exists only because loop1 is a real loop, and defeating it is an ordinary-C problem (.claude/rules/strength-reduce-defeat.md) separate from the seat swap.

- [s31] The owner's 2026-08-27 split-increment directive was executed indirectly: V15a (which carries both sanctioned split increments) was re-measured at 15/132 and used as one of the three chassis for the pre-test probe, and the honest out2 lift the directive asks for is now located on a chassis that needs NO split increment at all (Z1's lift comes from loop_depth weighting, not from a construct).
