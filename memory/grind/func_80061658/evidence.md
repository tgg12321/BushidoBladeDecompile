# Evidence bank — func_80061658

## Floor
- Honest sandbox `--disable all` = **9** (46 target insns, 46 build insns). 0 regfix/asmfix.
  HEAD byte-matches only via two register pins: `register s32 t asm("$2")`,
  `register s32 mask asm("$3")` (cheat; stripped by sandbox).

## The gap is a PURE v0<->v1 REGISTER SWAP (s1 2026-07-22, RTL-confirmed) — corrects prior WIP
The prior WIP claimed "register-cycle + schedule coupling / interleaving not
reproduced." That is WRONG. Objdump of the honest (pin-stripped) build vs
`asm/funcs/func_80061658.s` shows the tail instruction ORDER and interleaving
are **byte-identical** to target (the mask `lui` is scheduled between load2 and
store2 in BOTH). The ONLY difference is register assignment in the 12-insn tail:
  - TARGET:   load-temp `t` (arg0[0..2]) -> **v0**;  mask 0x10FFFF -> **v1**
  - UNPINNED: load-temp `t` -> **v1**;               mask 0x10FFFF -> **v0**
Every tail insn mentioning v0/v1 differs -> exactly 9 differing insns = the floor.
It is a pure register-allocation choice, NOT scheduling.

## Mechanism (global.c allocno priority) — why unpinned GCC swaps
RTL greg dump (tmp/grind/func_80061658/s1/dumps/np.i.greg, unpinned): the load
temp lands in reg 3 (v1), the mask const in reg 2 (v0). MIPS `reg_alloc_order`
puts $2(v0) before $3(v1), so whichever allocno global.c processes first wins v0.
`t` is ONE reused C variable => one long-lived allocno spanning all three
load/store pairs (~9 insns, 6 refs); `mask` has a short scheduler-fixed live
range (~5 insns, ~3 refs). global.c priority = log2(n_refs)*freq/live_length:
mask's shorter live_length gives it the higher priority, so mask is allocated
FIRST and grabs v0; t falls to v1. The pins override this ordering.

## KILLED this session
- **mask-before-call** (put `mask=0x10FFFF;` before func_80060A68() to cross the
  call and force mask off v0 into v1): floor 9. Disasm shows GCC rematerializes
  the constant AFTER the call (never crosses it), so allocation unchanged.
  => Corollary: ANY source reordering of a *constant* mask is inert (its live
  range is scheduler-pinned to just before its single use). Explains why the 3
  prior WIP mask-move variants were all 9. See rejected/mask-before-call.c.

## Prior WIP rejected forms (banked; do NOT re-run)
- split `t` into t0/t1/t2 single-def temps — floor 11 (WORSE)
- move `mask=` after the 3rd load — floor 9
- compute `mask=` early (after call, before loads) — floor 9
- inline loads directly into global stores (no `t` temp) — floor 11 (WORSE)

## Family
Siblings share the t=$2 / mask=$3 pin pattern: func_80061710, 617C8, 618B4,
611A4, 6133C. A pure-C form that flips 61658 likely flips all. (NOT a park
proposal — no-new-park-categories forbids register-rotation infrastructure.)

- [s1] Honest sandbox --disable all = 9 (46/46 insns); 0 regfix/asmfix; byte-match only via cheat pins t->$2, mask->$3.

- [s1] Pin-stripped build tail is byte-identical in ORDER/interleaving to target; the entire 9-diff residual is the v0<->v1 swap between load-temp t and mask const.

- [s1] Unpinned RTL greg: load-temp -> reg 3 (v1), mask 0x10FFFF -> reg 2 (v0); target wants t->v0, mask->v1.

- [s1] Mechanism: global.c allocno priority — mask's short scheduler-fixed live range outranks t's one long reused-variable allocno, so mask wins v0 first.

- [s1] KILLED: mask-before-call — constant is rematerialized after the call, never crosses it; floor unchanged 9. Reordering a constant mask is inert in general.

- [s1] Prior banked negatives (do not re-run): split t into t0/t1/t2 = 11; move mask after 3rd load = 9; compute mask early = 9; inline loads no-temp = 11.

- [s1] Family sharing t=$2/mask=$3 pins: func_80061710, 617C8, 618B4, 611A4, 6133C — a real pure-C fix should generalize.

## s2 — floor lowered 9 -> 7 (Lever A), root cause = local-vs-global RMW asymmetry
- [s2] RTL root cause (cur.i.lreg/greg, current src): mask (pseudo 76) has two sets
  li(insn105)+ori(insn106) that are RMW-CHAINED -> ONE contiguous quantity -> allocated
  by local_alloc, which runs BEFORE global_alloc and grabs v0. Load-temp t (pseudo 75)
  has THREE disjoint load ranges -> local_alloc can't combine -> t goes to global_alloc
  -> gets leftover v1. local-before-global is why mask seats @v0. This is the mechanism
  behind the s1 "pure v0<->v1 swap".
- [s2] Lever A (register-alloc-pure-c.md, block-local split) LOWERS the floor:
  splitting the MIDDLE load into `{ s32 u = arg0[1]; D_800F1144 = u; }` makes `u` a local
  pseudo born before mask => u@v0, mask correctly @v1. Floor 9 -> 7. Emitted: load1@a0,
  load2(u)@v0, mask@v1(target!), load3@a0, with load2 hoisted (interleave distorted).
- [s2] Split-load3 = 8; split-load2 = 7 (best); split-load1 = 11; split loads1&3 = 11;
  3 scoped block-locals = 11; 3 named temps t0/t1/t2 = 11 (re-confirmed). Only the
  MIDDLE-load split reaches 7; outer/multi splits scatter temps to v0/v1/a0/a1 and break
  the interleave (scheduler hoists all no-anti-dep loads).
- [s2] Measured INERT (all floor 9, grouping-preserving reorders of single-t form):
  mask between load1/load2 (variant A); declaration-order swap (mask decl before t);
  [s1 banked] mask-early, mask-late, mask-before-call. Confirms pure reorder/decl-order
  of the single-temp form cannot flip the tiebreak.
- [s2] THE WALL to 0: single-t gives the TARGET SCHEDULE but wrong RA (mask@v0); every
  split gives target RA (mask@v1) but breaks the SCHEDULE. Reaching 0 needs BOTH: the
  single reused load-temp @v0 (for the interleave) AND mask@v1 — i.e. the disjoint-range
  shared load-temp must win v0 over the RMW-chained LOCAL mask, which local-before-global
  allocation forbids for these value shapes. No grouping-preserving structural transform
  changes the local/global classification. Next lever is the directed permuter (frontier
  #2), a different modality — NOT more structural splitting.

- [s2] Honest sandbox --disable all floor lowered 9 -> 7 this session (clean pure C, 0 rules/pins) via Lever A block-local split of the middle load.

- [s2] RTL root cause: mask (pseudo 76) is RMW-chained (li+ori) -> one contiguous LOCAL quantity -> local_alloc grabs v0 before global_alloc places the disjoint-range load-temp t (pseudo 75) into v1. local-before-global is the swap mechanism.

- [s2] Split-load sweep: load1=11, load2=7(best), load3=8, loads1&3=11, 3-scoped=11, t0/t1/t2=11. Only the middle-load split keeps the shared t=$loads1&3 and improves.

- [s2] Reorder/decl sweep (single-temp form): mask-mid=9, decl-order-swap=9, [banked] mask-early/late/before-call=9 - pure reorder cannot flip the tiebreak.

- [s2] The wall to 0: single-t gives the TARGET SCHEDULE but wrong RA (mask@v0); any split gives target RA (mask@v1) but breaks the SCHEDULE. Reaching 0 needs the disjoint-range shared load-temp to win v0 over the RMW-chained LOCAL mask - which local-before-global forbids for these value shapes; no grouping-preserving structural transform changes that classification.

- [s2] Both floor forms are clean pure C: the floor-9 form is a pure RA swap with the correct target schedule (cleaner permuter base); the floor-7 form has mask@v1 but a distorted interleave.

## s3 — GCC-SOURCE-LEVEL confirmation of the local/global wall (floor unchanged 7)
- [s3] Regenerated the greg/lreg dumps (s1/s2 dumps were gitignored & gone) from the
  floor-9 pure form. greg: "6 regs to allocate: 77 75 78 73 72 74" — pseudo 75
  (load-temp t) IS in the global list -> disposition 75->reg3(v1); pseudo 76 (mask)
  is ABSENT from the global list -> disposition 76->reg2(v0), i.e. allocated by
  LOCAL alloc. Exactly the s1/s2 v0<->v1 swap, now re-confirmed on fresh dumps.
  Artifacts: tmp/grind/func_80061658/s3/dumps/{text1b.i.greg,text1b.i.lreg,func_greg.txt}.
- [s3] lreg: "Register 75 used 6 times across 9 insns in block 5; dies in 3 places";
  "Register 76 used 2 times across 5 insns in block 5" (dies once). Both in the SAME
  block 5 (post-call).
- [s3] ROOT CAUSE pinned to GCC source: tools/gcc-2.7.2/local-alloc.c:472 gates a
  pseudo to LOCAL alloc iff `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`. The
  load-temp loads 3 DISTINCT values -> reg_n_deaths==3 -> reg_qty=-1 -> DEFERRED to
  global_alloc (unconditional; not a priority tie). mask is a single constant ->
  reg_n_deaths==1 -> local-allocatable.
- [s3] Register CHOICE pinned: MIPS backend defines NO REG_ALLOC_ORDER (grep of
  config/mips = 0 matches) -> find_free_reg iterates hard regs in ASCENDING number.
  In block 5 the ONLY local qty is mask; find_free_reg gives it the lowest free
  caller-saved reg = v0(2). No copy-suggestion exists (constant has no copy src).
  So the lone local mask qty DETERMINISTICALLY takes v0; the global load-temp gets
  leftover v1. This is mechanical, not a tiebreak that C reordering can flip.
- [s3] To reach 0 you must EITHER (a) make the load-temp reg_n_deaths==1 (one
  contiguous live range) so it becomes local & born-first -> impossible: 3 distinct
  loaded values = 3 deaths regardless of C spelling; OR (b) make mask reg_n_deaths>=2
  so it goes global where the higher-ref (6 vs 2) load-temp outranks it for v0 ->
  no byte-neutral pure C exists (mask's store is on the single post-call path; a 2nd
  use = an extra emitted insn); OR (c) introduce a v0-blocking single-death LOCAL
  across mask's range -> that is a load SPLIT, and every split was measured 7-11
  because the split temp has no anti-dep and the scheduler hoists it (distorting the
  interleave) and/or steals v0 from the shared global load-temp (scatters it to a0).
- [s3] KILLED frontier #2 (chain-loads-into-one-quantity): 3-word struct block copy
  `*(struct{s32 a,b,c;}*)&D_800F1140 = *(struct{...}*)arg0;` scored 22 (build_insns
  34 vs 46). GCC lowers it to lw/sw pairs that STILL die 3x AND drops the mask
  interleave / folds addressing -> wholly different shape. No block-copy spelling
  creates a single contiguous load-temp live range. rejected/struct-block-copy.c.
- [s3] CONCLUSION: the structural modality is exhausted with a source-level proof.
  The residual 7 (best) / 9 (pure-swap) is a local-vs-global allocation-CLASS wall
  (reg_n_deaths==1 gate + ascending REG_ALLOC_ORDER), not a priority tie any
  grouping-preserving structural transform can flip. Floor still potentially movable
  via the directed PERMUTER (frontier #1, different modality) — NOT owner-gated
  (still grindable). No new structural lever remains to derive.

- [s3] greg (fresh, s3): '6 regs to allocate: 77 75 78 73 72 74' includes pseudo 75 (load-temp t) -> disposition 75 in reg3(v1); pseudo 76 (mask) ABSENT from the global list -> 76 in reg2(v0), i.e. allocated by local_alloc. Re-confirms the s1/s2 pure v0<->v1 swap on fresh dumps.

- [s3] lreg (fresh, s3): 'Register 75 used 6 times across 9 insns in block 5; dies in 3 places'; 'Register 76 used 2 times across 5 insns in block 5' (dies once). Both live only in block 5 (post-call).

- [s3] ROOT CAUSE at GCC source: local-alloc.c:472 gates local alloc on `reg_basic_block>=0 && reg_n_deaths==1`. Load-temp reg_n_deaths==3 -> global (unconditional); mask reg_n_deaths==1 -> local.

- [s3] REGISTER CHOICE at GCC source: config/mips has NO REG_ALLOC_ORDER (grep = 0 matches) -> find_free_reg iterates ascending hard-reg numbers; the lone local mask qty gets the lowest free caller-saved reg = v0(2). No copy-suggestion exists (constant has no copy source). Deterministic, not a tiebreak.

- [s3] KILLED frontier #2: 3-word struct block copy scores 22 (build_insns 34 vs 46) -- lw/sw pairs still die 3x and the mask interleave is dropped; no block-copy spelling makes the load-temp one contiguous live range.

- [s3] Floor unchanged this session (best 7 = s2 middle-load block-local split; pure-swap base 9). Structural axis measured-dead; permuter (frontier #1) remains open, so still grindable / NOT owner-gated.

## s4 — PERMUTER FOUND A BYTES-PROVEN pure-C form (floor 7 -> 0) — PENDING OWNER RULING
- [s4] Built a clean single-fn permuter workspace (tmp/grind/func_80061658/s4):
  base.c = full text1b.c TU with func_80061658 in the floor-9 pure-swap form (pins
  stripped); target.o from asm/funcs/func_80061658.s + r3k prelude at offset 0;
  compile.sh = real cc1|prologue_fix|maspsx|multu_pad pipeline, extract func region.
  Validated base 46 insns / target 46 insns, diff = exactly the 9-insn v0<->v1 tail
  swap. Permuter base_score 50 (weighted). Campaign s4-floor9-pureswap, -j8.
- [s4] Campaign yielded SEVEN independent score-0 finds within ~161s (output-0-1..7).
  ALL SEVEN share ONE lever: stage the mask constant through the reused local `val`
  (`val = 0x10FFFF; mask = val;` instead of `mask = 0x10FFFF;`). Placement of the
  `val = 0x10FFFF;` among the 3 loads varies; the copy chain `mask = val` is invariant.
  This is the copy-suggestion escape s2/s3 named as the unmet need.
- [s4] ENGINE-VERIFIED (authoritative): with output-0-2's placement applied to
  src/text1b.c (pins removed), `sandbox func_80061658 --disable all` = 0,
  target_insns 46, build_insns 46, rules_dropped 0, func carries zero pins/__asm__.
  The honest pure-C distance is 0. Mechanism confirmed: staging through `val` gives
  `mask` a copy source; mask -> $v1, reused 3-death load-temp -> $v0 = target RA,
  flipping the s1-s3 local-alloc.c:472 class wall via a copy-preference.
- [s4] LAYER-1 cheat-reviewer verdict = FAIL (recorded, not overturned). Grounds:
  the construct changes local-alloc register CHOICE (copy-preference) on STRAIGHT-LINE
  code; the sanctioned families it resembles are scoped elsewhere — staged-value-
  reused-variable is a sched.c adjust_priority mechanism staging a LOAD;
  defeat-licm-hoist-var-reuse is loop-scoped ("do not cite for straight-line code").
  Staging a CONSTANT through a reused local for RA choice on straight-line code is
  arguably a NEW technique-family application needing an SOTN evidence pass + owner
  sign-off per review-discipline-before-commit. Verified the rule-scoping claims are
  accurate (read both rule files).
- [s4] DISPOSITION: ruling-request (never self-approve a SOTN exception; prime
  directive). src reverted to HEAD pinned form. Bytes-proven form saved to
  candidate.c and tmp/grind/func_80061658/s4/output-0-*/source.c. Structural axis
  was already dead (s3); the residual is NOT a wall — a pure-C sandbox-0 form EXISTS
  and is proven. The only open question is policy classification of the construct.
