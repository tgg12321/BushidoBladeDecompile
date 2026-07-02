# marionation_Exec (system.c) — 6/8 regs + printf block ONE condition out

## TL;DR (2026-07-02, session 4c) — resume HERE
Composite w9+u13+w16 (probe tags, tmp/probe_mar.py) = 6/8 s-regs locked
(status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, i1495 s6) + masks
fold + tail cascade (check→a2, `move v0,a2` ×2) + printf block: **fmt-la
wins backward clock 5** (w16's in-block pp kills the a1-move → a0
hard-free), chain sparse-tied, a1-lw pos 3, la last. **ONE condition
left: v1-occupancy** — local-alloc packs chain5→v0, 11D5→v0, val5→v0,
leaving v1 empty until chain4's turn → chain4 takes v1 (numeric order)
before a0. Target needs val5→v1 (`lw v1,0(v0)`), i.e. v0 blocked over
val5's range ⟺ **the 11D5-lbu forward-BEFORE the sw5 in SCHED1 coords**.
The launch cascade (birthing_insn_p: bump iff dest-live + function-wide
reg_n_sets==1 — a2/a3 hard-loads bump; moves/sw/la never) packs clocks
6-11 rigid → sw@12, lw5@15 → sw always lands before the lbu ✗.
Then: trio {arg0 s5 / tbl s7 swap} — small livelen deltas (t1 tbl-chain
+1 ref verified byte-free, available); then assemble + retire 42.

## w16 printf block (current best; c4=v1, all else target)
`{ s32 arg5; s32 t0; s32 *p4; void **pp; t0 = idx_1494[0];
pp = (void **)&D_800F19C0; arg5 = tbl_125c[idx_1494[1]];
p4 = &tbl_125c[t0];
debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *p4, arg5); }`
- pp (IN-BLOCK) → combine substitutes symbol into the mem IN PLACE
  (symbol-SET moves nothing across the sw) → direct `(set a1 (mem
  sym))`, no a1-move → la ready ALONE at clock 5 (SCHEDDBG `clock=5
  picked=132`). A plain `D_800F19C0` arg CANNOT do this: expand
  precopies it (calls.c:1618+1653, rtx_cost>2 + -O2's
  flag_expensive_optimizations) and combine won't merge the load
  across the sw. The a2 arg escapes precopy (ARRAY_REF → unloaded
  (mem (reg)), cost≤2 → direct late load).
- s32 t0 (NOT u8 — u8's zext merges at the USE and the lbu sinks) +
  addr-local p4 + `*p4` inline: sll/addu at stmt-3 luids, lw at call
  luid AFTER the sw (reg-addr loads can't cross sp-stores; symbol
  loads cross freely) → chain tied sparse [8..15], allocated last.
- pp/p4/t0 are sanctioned C (pointer-alias-fake-exception / plain
  locals); FAKE-annotate pp in the final form.

## v1-occupancy — enumerated-dead + next ideas
DEAD: val5 (needs the sw/lbu flip), 11D5 via chain5-overlap (chain5
dies at lw5@7 always), untied chain fragments (wrong bytes),
global-class merges (local-alloc runs first; chain4 beats them to v1),
a2-set-count unbump (no byte-free 2nd a2 set), v0-hard segments (no
byte-free source), cross-block pointer alias w17 (`u8 *pi=&D_800A11D5`
head-init: pi SURVIVES to RA — no equiv-deletion in this compiler —
colored fp, +3 insns ✗; only in-block aliases vanish, via combine).
w18/w19 (i5 = D_800A11D5; i5s = i5*4 stmt locals, byte-indexed a2 arg):
insns 142 ✓, **the a0-flip machinery WORKS** — but the i5-qty became
the sparser chain and TOOK a0 (11D5→a0, chain4→v1 ✗ target needs 11D5
in v0). Competition rule confirmed: the two sparsest qtys take {v1,a0}
in density order; val5→v1 requires the INLINE-dense 11D5 (v0, 32000)
covering val5's range ⟺ STILL the sw/lbu ordering. Also proven: moving
`v0 = -1;` early is bytes-fatal (74's range would block v0 at check).
UNTESTED next:
(a) luid-tie reshuffles of the launch cascade with INLINE 11D5 (w16
    base): the cascade packing is tie-driven (126@8 vs 115 by luid);
    arg-order/spelling changes that lower 126/121's luids below 115/
    113's may open the pre-12 gap for the sw → lbu before sw;
(b) exhaustive stmt-permutation sweep w/ chain-reg flags (harness
    tmp/sweep_mar_printf.py, ~25s/variant) on the w16 base;
(c) assemble the REST first (arms/trio final form), re-probe the block
    LAST — the cascade is luid-sensitive; full-function shape shifts
    ties (cheapest remaining unknown);
(d) cc1psx CONFIRMED identical on w15 (parity holds; C is the variable).

## Composite recipe (other pieces, all probe-verified byte-neutral 142)
- u12/u13: masks `check = *idx_1496 & new_var;` / `& new_var3` (two
  single-use opaque constants; fold via update_equiv_regs refs==2
  UNWEIGHTED, local-alloc.c:1079) + csmd4 arm shape + BLOCK-LOCAL
  `u8 bb` per copy loop → tail cascade (bb v0, src a0, i v1, dst a1,
  check a2).
- w9: poll as REAL `do { status=f(); if (status==0) break; {arms} }
  while (1);` → loop_depth weights (flow.c: depth PLUS ONE) → status
  8 refs, i1494 909 (under saved 952), i1495 197 → s6. **Outer cycle
  MUST stay goto-loop** (real loop → mask refs 4 ≠ 2 → fold dies).
- w9 pris: arg1:357 saved:952 i1494:909 i1496:684 arg0:250 tbl:192
  i1495:197. Trio needs tbl>197 (t1 chain +1 → 512? recheck vs arg1
  357 — overshoot risk; livelen route: tbl<152) and arg0 lowest.

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
