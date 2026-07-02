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
- Final block order is dictated by RA + sched2 anti-deps; sched1 order
  (what local-alloc sees) is the real substrate. sched.c = BACKWARD
  list scheduling: pick sequence = reverse layout; ties at equal pri
  break by class-vs-last-scheduled then luid.
- **u8 t0 fails**: expand emits QI-load + zext-at-use; combine merges
  AT THE USE → lbu sinks (the w10/w11 lbu4-late mystery). `s32 t0`
  keeps the merged zext-load at stmt-1 (uservar not combine-sunk) but
  sched1 still slaves the lbu to its consumer's position — the fix is
  the ADDRESS-LOCAL p4 (sll/addu at stmt-3 luids) + `*p4` inline (lw
  at call luid, AFTER the sw in expand order — a tbl-based (reg-addr)
  load cannot cross an sp-store (memrefs can't prove distinct), while
  symbol-based loads (F19C0/11D5/11DC) cross freely).
- **Local-alloc**: block_alloc ties chains through dying srcs into one
  qty; qty order = copy-suggested first (a3/a1 feeders), then density
  `floor_log2(refs)*refs*size/(death-birth)` DESC (qty_compare,
  local-alloc.c:~1578). Sparse chain4 allocates LAST → gets a0 IFF
  v0+v1 busy across its range AND a0 not hard-blocked.
- **THE a0 BLOCKER (w15 trace, log line 4350)**: fmt-la `(set a0
  (symbol D_800161C8))` (combine-merged direct-a0) has backward pri 1
  — loses every pick to the huge-pri chain insns → drifts to sched1
  pos-7 → hard-a0 live [7..16] → chain4 can never take a0. Target
  needs fmt-la picked by backward clock ~5 (ready set {a1-move(p2),
  fmt-la(p1)} at clock 5 — it must WIN there or the deep insns arrive).
- **val5→v1 condition**: val5 (arg5 value) takes v1 (target) instead
  of v0 iff its range overlaps the 11D5 qty in SCHED1 coords (i.e. the
  sw5 sits AFTER 11D5's lbu in sched1 order). w13 (arg4-local) had
  that ✓; w15 doesn't. With val5=v1 AND fmt-la late, chain4 (sparse,
  last) → v0✗v1✗ → a0 ✓ and the whole tail order follows via sched2
  anti-deps (lw4 sinks past 11D5's v0 ops; fmt-la after lw4 by a0
  anti-dep; lbu4(a0)/lbu5(v0) order follows).

## Next session — exact resume
1. **x3 sweep (highest EV)**: extend tmp/sweep_mar_printf.py classify
   to also print chain4's disposition (grep greg for the p4/t0
   pseudos). Sweep stmt-order micro-perms around w15: {t0,arg5,p4}
   orders × {F19C0 as early local a1v} × {11D5 as local placed 2nd/3rd/
   4th} × {arg5 addr-local p5 variant}. LOOK for chain4=a0. The two
   target conditions: fmt-la late in sched1 + val5/11D5 overlap.
2. If a0 lands: re-run probe_mar_disp → expect tail micro-perm to
   snap to target; then re-measure the TRIO (arg0/tbl livelens shift
   with the block reshape; need tbl<152, arg0>102-ish livelen or
   tbl+1 byte-free ref via the t1 F19C0-chain (folds, verified)).
3. Assemble final form in src/system.c: u13 masks/arms/loops + w9 poll
   do-while + winning printf shape + DECLARE `int new_var; int
   new_var3;` (new_var is currently UNDECLARED at line 555 — fix in
   the final form only). Then retire all 42 rules + full build SHA1 +
   dual review + queue done.
4. Fallback if x3 dry: instrument adjust_priority (sched.c) to print
   fmt-la's pri computation; find what legitimately raises it (e.g. a
   REAL dependence shape); or accept chain4=v1 and check whether ANY
   rules can still retire (prologue offsets etc. depend only on the
   s-reg trio).

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
- `new_var` UNDECLARED at src/system.c:555 (Makefile pipe swallows
  cc1's error-recovery exit; bytes fine). DON'T fix mid-derivation
  (shifts pseudo numbering); fix in the final committed form.
- The 42 rules are index-anchored: mid-derivation full builds are
  meaningless. Iterate at cc1-dump level; the only end gate is
  retire-all-42 + full SHA1. Masked sandbox is rotation-blind.
- Twin-function contamination: csmd4 (line ~399) shares text with
  marionation (499) — ALL probe anchors must be marionation-unique
  (the `_2` suffix on D_800A147C_2, or split at "s32 marionation_Exec").
- w9 pris: arg1:357 saved:952 i1494:909 i1496:684 arg0:250 tbl:192
  i1495:197. **csmd4 payoff**: its remaining 5 rules are THIS block
  (reg-half = chain4=a0); the w15+x3 answer transfers, but respect its
  committed ref-count balance (WIP rejected_forms) before applying.

## Pointers
- tmp/probe_mar.py (tags t0..w15) + probe_mar_disp.py + probe_mar_
  sched.py + mar_sched_section2.py + mar_rtl_order.py + mar_block_
  regs.py + sweep_mar_printf.py — the probe kit (all restore src).
- tmp/gccdbg/cc1 = instrumented cc1 (BB2_SCHED_DEBUG/BB2_PRIO_DEBUG/
  BB2_ALLOC_DEBUG in-tree env-gated; build/cc1 is canonical May-18).
- memory/wip/cpu_side_move_dir_4/notes.md — the twin's ledger.
- [[duplicated-statement-into-arms]], [[register-alloc-pure-c]].
