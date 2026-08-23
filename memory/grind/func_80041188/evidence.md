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
