# marionation_Exec — HONEST SCORE 8; final-4 sched ties fully decoded

## SESSION 7 — score 8; the last-mile dependency web (READ FIRST)
candidate.c = the score-8 form (restore-verify before iterating:
`bash tools/mar_test_candidate.sh` needs tmp/mar_candidate.c +
tmp/mar_apply_candidate.py — committed copies in tools/). The final 8
masked diffs + 1 insn = 4 regions; each is now MEASURED to the exact
tie (hooks: BB2_QTY_DEBUG in local-alloc block_alloc (both phases),
BB2_SCHED/SLL/FLOW as before; tools/mar_qtydbg.sh prints per-block qty
allocations, marionation-isolated):
- **The RA structure that WORKS (QTYDBG, blk=3)**: chain5-qty(6refs)
  →v0; 11D5(4refs)→v0-reuse; val5(2refs,[20..26])→v1 (v0 blocked by
  chain5 death-touch AT the lw5 boundary!); sll4-TEMP(2refs,[14..24])
  →a0; t0-var = MULTI-DEATH ⟹ NO local qty ⟹ GLOBAL-alloc, LAST
  ⟹ a0 (fits the temp's gap [lbu..sll]+[addu..lw4]). The *=4 temp
  split (t0 dies at the sll, reborn at addu) is what makes t0
  global-class; `t0 <<= 2` has no mid-death ⟹ one LOCAL qty(6refs,
  5000-pri) ⟹ beats val5 ⟹ steals v1 ⟹ FAILS.
- **Region-1 (printf lbu5/sll4 order)**: needs sll4-luid > sll5-group
  (SCHEDDBG: flips picks c13-c17 to target exactly) BUT every such
  form (v2/a5i/<<=) narrows the temp span or unifies the qty ⟹ temp
  beats val5 (3333-vs-2500-3333 razor, qty-birth tie-break) ⟹ v1
  stolen ⟹ chain4=v1. CONSTRAINT: temp must allocate AFTER val5 —
  needs val5-span < temp-span strictly (target's own geometry gives
  temp[8..11]=3 < val5[9..13]=4 ⟹ ??? target contradiction unresolved
  — its sll a0,a0 = in-place no-temp + val5 v1 ⟹ its chain must be
  global-class AND slotless: UNFOUND spelling. Next: enumerate 2-death
  no-temp spellings; or QTYDBG the exact-tie margins per variant).
- **Region-2 (arm-1 sb/la)**: the head is a 3-insn block; sched1 tie
  {sb(l0), la(l1)} both pri-1 → emits [sb, la, beqz] ALREADY-target;
  then DBR hoists the sb (1-word, from-before, past the 2-word la
  macro) into the beqz slot ⟹ [la, beqz, sb-slot] ✗. Target's dbr
  did NOT hoist ⟹ its sb was slot-ineligible (may_trap on reg-based
  store? but ours hoisted...) — UNRESOLVED: read dbr fill_simple's
  from-before scan conditions (reorg.c) for what blocks a store hoist.
  C-side sb/la stmt swap does NOT reach sched1 (canonicalized) but
  costs +1 elsewhere.
- **Region-3/4 (check2 move-slot vs nop, +1 insn)**: ours' dbr steals
  the arm-2 move (depth-4 fall-through, independent of sb/la/li);
  target = nop ⟹ its move was ineligible (a1 liveness at the taken
  target per mark_target_live_regs? our steal says a1-dead) —
  UNRESOLVED same-code contradiction; likely coupled to region-2's
  dbr pass ordering (earlier fills change later liveness!). If
  regions 2+3 fix, +1 insn lands (179 ✓).
- arg1's L: [i;dst] arm-interiors + [store;src;i;dst] arm-2 = L 87 =
  919 < i1494/96's 933 ✓ (2-pt margin); every dst-early form costs
  2 → rotation. The 933-tie between i1494/i1496 (both 7 weighted refs
  after the F19C0-chain) breaks by ascending allocno ✓ correct.

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5,
  i1495 s6, arg0 s7; tail check a2, src a0, i v1, b v0.
- Printf block (718FC-7194C): lbu a0(idx0), lbu v0(idx1), lui/lw a1,
  sll v0, addu v0+s5, sll a0, lw v1 0(v0), lbu v0 11D5, addu a0+s5,
  sll v0, sw v1 0x10(sp), lw a2 11DC(at), lw a3 0(a0), la a0 fmt, jal.
  tslTm2 slot = NOP (lbu a0 arg-conflicts, lbu v0 return-conflicts —
  both follow from the coloring).
- after_blocks: `beqz s7 → loop`, `move v0,zero` delay. The two
  null-path [j; move v0,a2] sites stay unmerged (ours too ✓).

## Known gotchas
- `new_var` UNDECLARED at :555 (pipe swallows cc1 error-recovery exit;
  bytes fine). Declare new_var/new_var3 only in the FINAL form.
- 42 rules are index-anchored: mid-derivation full builds meaningless;
  end gate = retire-all-42 + full SHA1. Sandbox rotation-blind. Twin
  csmd4 (~:399) shares text — anchors must be marionation-unique
  (`_2` suffix / partition at "s32 marionation_Exec").
- csmd4 payoff: its last 5 rules are THIS block; answer transfers.

## Pointers
- tmp/probe_mar.py (tags t0..w17) + probe_mar_{disp,sched}.py +
  mar_{sched_section2,rtl_order,block_regs}.py + sweep_mar_printf.py
  (all restore src). tmp/gccdbg/cc1 = instrumented (env-gated
  SCHED/PRIO debug; build/cc1 = canonical May-18).
- memory/wip/cpu_side_move_dir_4/notes.md; [[register-alloc-pure-c]].
