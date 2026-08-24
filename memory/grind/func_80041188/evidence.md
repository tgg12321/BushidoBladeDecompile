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
