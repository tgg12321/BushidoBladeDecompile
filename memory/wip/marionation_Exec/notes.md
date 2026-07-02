# marionation_Exec (system.c) — WIP, 6/8 regs + printf-block mechanism decoded

## TL;DR (2026-07-02, session 4)
**w9+u13+w15 composite = 6/8 registers locked** (status s0, saved s1,
i1494 s2, i1496 s3, arg1 s4, **i1495 s6** ✓) + both masks fold + tail
cascade (check→a2, both `move v0,a2` returns) + printf-block structure
12/14. Residuals: (1) trio tail {arg0 s5/tbl s7 must swap to s7/s5} —
small livelen deltas, expect movement from the final printf shape;
(2) printf-block chain4 colored v1 not a0 — blocker EXACTLY identified
(fmt-la sched1 drift, below); lbu-pair order + tail micro-perm follow
the chain4=a0 flip automatically (sched2 anti-deps).

## Composite recipe (probe tags in tmp/probe_mar.py)
- **u12/u13**: masks `check = *idx_1496 & new_var;` / `& new_var3` (two
  single-use opaque constants, declared after idx_1496 decl anchor) +
  csmd4 arm shape + BLOCK-LOCAL `u8 bb` per copy loop (u13) → tail
  cascades: bb→v0, src→a0, i→v1, dst→a1, check→a2, `move v0,a2` ×2.
  insns 142 = byte-neutral. Masks fold via local-alloc
  update_equiv_regs (reg_n_refs==2 UNWEIGHTED, local-alloc.c:1079).
- **w9**: poll cycle as REAL `do { status=f(); if (status==0) break;
  {arms} } while (1);` → loop_depth (flow.c: depth PLUS ONE is the ref
  weight) double-counts poll-resident refs: status 4→8 refs (pri
  24000), i1494 6→7 (909, stays under saved 952), i1495 2→3 (197 →
  lands s6 ✓). **Outer cycle MUST stay goto-loop** — a real outer
  do-while would make mask refs 4 (≠2) and kill the andi fold.
- **w15 printf block** (current best):
  `s32 t0; s32 *p4; t0 = idx_1494[0]; arg5 = tbl_125c[idx_1494[1]];
  p4 = &tbl_125c[t0]; debug_printf(fmt, D_800F19C0, 11DC[11D5], *p4, arg5);`
  → chain4 stretched (lbu@2, sll@6, addu@8, lw@11), lw4 crosses the sw
  (source-after ⇒ no anti-dep), fmt-la last, chain5=v0 ✓, 11D5=v0 ✓.

## Printf-block mechanism ledger (session-4 dumps; tmp/rtl/marS.*)
- Final order = RA + sched2 anti-deps; sched1 order (what local-alloc
  sees) is the substrate. sched.c = BACKWARD list scheduling: pick
  sequence = reverse layout; ties by class-vs-last then luid.
- **u8 t0 fails**: QI-load + zext-at-use; combine merges AT THE USE →
  lbu sinks. `s32 t0` keeps the merged load at stmt-1, and the
  ADDRESS-LOCAL p4 + `*p4` inline puts sll/addu at stmt-3 luids and
  the lw at call luid AFTER the sw (reg-addr loads can't cross an
  sp-store — memrefs can't prove distinct; symbol loads cross freely).
- **Local-alloc**: chains tie through dying srcs into one qty; order =
  copy-suggested first, then density floor_log2(refs)*refs*size/span
  DESC (qty_compare ~:1578). Sparse chain4 allocates LAST → a0 IFF
  v0+v1 busy across its range AND a0 not hard-blocked.
- **val5→v1 condition**: val5 takes v1 (target) iff its range overlaps
  the 11D5 qty in sched1 coords (sw5 AFTER 11D5's lbu). w13 had it;
  w15 doesn't. With val5=v1 AND fmt-la late → chain4 → a0 → the whole
  tail order + lbu order follow via sched2 anti-deps.

## Next session — exact resume (x3 ran: ALL p4-shapes converge c4=v1)
The block reduces to ONE clock-5 race, fully traced (session 4b):
- sched1 = BACKWARD list sched; reg-KILLING insns get LAUNCH_PRIORITY
  (0x7f000001, sched.c:3963 + adjust_priority) and jump the queue;
  non-killing leaves (fmt-la, symbol-addressed a1-load) lose every
  contested pick and drift to the block FRONT (early-forward).
- At backward clock 5 (call+1) ready = {a1-MOVE (pri 2), fmt-la (pri
  1)}: the move wins, storm loads arrive clock 6 → la drifts to pos
  ~7 → hard-a0 [7..16] → chain4 can never color a0. **If the a1-move
  did not exist** (F19C0 load merged to `set a1 (mem sym)` — a LOAD,
  ready clock 6, non-killing so it drifts early-forward to pos 3-4 =
  target!), la is ready ALONE at clock 5 → placed pos ~14-15 → a0
  free → chain4 (sparse-last) → a0 → sched2 anti-deps reproduce the
  ENTIRE target permutation incl. the lbu order. Everything hinges on
  killing the a1-move.
- Why ours keeps the move: expand's reg-parm precompute
  (calls.c:1618+1653, preserve_subexpressions_p()==1 at -O2 via
  flag_expensive_optimizations) copies any arg with rtx_cost>2 →
  symbol-mem F19C0 → pseudo 109 + `move a1,109`; combine does NOT
  merge 117+133 (the sp-store sw5 @129 sits between in luid order;
  suspected mem-conservatism — NOT yet verified in combine.c). The
  a2-arg escapes because ARRAY_REF returns an unloaded (mem (reg))
  (cost≤2, no copy) → direct late load.
- w13-vs-w15 ANTICORRELATION (both dumped): w13 (arg4 plain local)
  → la at sched1 pos 14 ✓ but chain4 dense [5..9] → v0 ✗. w15 (p4)
  → chain sparse [4..14] ✓ but la pos 6-7 ✗. Target needs BOTH.
1. **Decisive next probe**: verify the combine blocker — read
   combine.c can_combine_p/try_combine for MEM-src into hard-reg-dest
   across an intervening MEM store; then find a legitimate C spelling
   that either (a) makes F19C0's arg escape the precompute copy, or
   (b) places the sw5 outside [117..133] in expand order, or (c)
   drops the a1-move's pri to 1 / raises la's to 2 (clock-5 tie →
   luid favors la? check rank direction first). cc1psx CONFIRMED
   identical on w15 (parity holds; the C is the variable).
2. If the move dies: expect full cascade; re-run probe_mar_disp; then
   re-measure the TRIO (need tbl<152 livelen or arg0 pri<197; the t1
   F19C0-chain (+1 tbl ref) is verified byte-free and available).
3. Assemble final form in src/system.c: u13 masks/arms/loops + w9
   poll do-while + winning printf shape + DECLARE `int new_var; int
   new_var3;` (new_var UNDECLARED at :555 — fix in final form only).
   Then retire all 42 rules + full SHA1 + dual review + queue done.

## Target ground truth (from asm/funcs/marionation_Exec.s)
- Register map: status→s0, saved→s1 (`andi $s1,$v0,3`), idx_1494→s2,
  idx_1496→s3, arg1→s4, tbl→s5, idx_1495→s6, arg0→s7. Tail: check→a2,
  src→a0, i→v1, b→v0. Printf block (718FC-7194C): lbu a0(idx0), lbu
  v0(idx1), lui/lw a1(F19C0), sll v0, addu v0+s5, sll a0, lw v1 0(v0),
  lbu v0 11D5, addu a0+s5, sll v0, sw v1 0x10(sp), lw a2 11DC(at+v0),
  lw a3 0(a0), la a0 fmt, jal. tslTm2 delay slot = NOP (lbu a0
  arg-conflicts; lbu v0 return-conflicts — follows from the coloring).
- Poll arms + after_blocks: `beqz s7 → loop` w/ `move v0,zero` delay.
  Both null-paths share one [j; move v0,a2] at .L2BC; arm-1 copy-exit
  has its OWN copy at 71A6C (unmerged pair — same as our compiles ✓).

## Known gotchas
- `new_var` UNDECLARED at :555 (pipe swallows cc1's error-recovery
  exit; bytes fine). Fix only in the final committed form.
- The 42 rules are index-anchored: mid-derivation full builds are
  meaningless; end gate = retire-all-42 + full SHA1. Sandbox is
  rotation-blind. Twin csmd4 (~:399) shares text — probe anchors must
  be marionation-unique (`_2` suffix / split at "s32 marionation_").
- w9 pris: arg1:357 saved:952 i1494:909 i1496:684 arg0:250 tbl:192
  i1495:197. **csmd4 payoff**: its remaining 5 rules are THIS block;
  the answer transfers (respect its committed ref-count balance).

## Pointers
- tmp/probe_mar.py (tags t0..w15) + probe_mar_{disp,sched}.py +
  mar_{sched_section2,rtl_order,block_regs}.py + sweep_mar_printf.py
  (all restore src). tmp/gccdbg/cc1 = instrumented cc1 (env-gated
  SCHED/PRIO/ALLOC debug; build/cc1 = canonical May-18).
- memory/wip/cpu_side_move_dir_4/notes.md; [[register-alloc-pure-c]].
