# marionation_Exec — HONEST SCORE 17 (was 56); printf block SOLVED

## SESSION 5 — PRINTF BLOCK CRACKED (w24) — resume HERE
**w24 composite (u12/u13 + w9 + the w24 printf form) scores HONEST 17**
(sandbox --disable all, 42 rules dropped, 20 cheat-asm stripped;
build 177 vs target 179). The w24 printf block:
`{ s32 arg5; s32 t0; void **pp; t0 = idx_1494[0];
pp = (void **)&D_800F19C0; t0 *= 4;
t0 = (s32)((u8 *)tbl_125c + t0); arg5 = tbl_125c[idx_1494[1]];
debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5); }`
→ **ALL FOUR block registers TARGET: chain4=a0 (lbu a0/sll a0,a0/addu
a0,a0/lw a3,0(a0)), chain5=v0, val5=v1 (`lw v1,0(v0)`), 11D5=v0-reuse**;
la last, a1-lw early. Mechanisms (all dump-verified):
- pp in-block pointer (combine substitutes symbol in place) kills the
  a1-move → fmt-la wins backward clock 5 → a0 hard-free.
- **t0 MULTI-SET accumulator** (`t0=idx[0]; t0*=4; t0=(s32)((u8*)tbl+t0)`)
  — reg_n_sets>1 → birthing_insn_p FALSE → chain DE-LAUNCHED from the
  backward-sched storm → lbu drifts to pos 1, chain qty spans the block
  (sparse, allocated last) → walk lands a0. Same-reg chain = target's
  exact byte form. Sanctioned split-init family.
- **t0-stmts BEFORE the arg5 stmt (w24) flips the val5/chain4 density
  tie** (w21 = same but arg5-stmt-before → chain4=v1/val5=a0 swapped;
  w22 = arg5-inline → chain4=a0 but val5/11D5 swapped). RAZOR TIES —
  keep w24's exact stmt order.
**Remaining 17 (normalized diff tmp/mar_diff2.sh, sandbox .o kept):**
1. TRIO s5↔s7 (arg0/tbl swap): current pris arg0 256(2/78) >
   i1495 202(3/148) > tbl 197(3/152). Need tbl livelen ≤147 or arg0
   ≥ +2. t1-chain overshoots (526 > arg1 365) — livelen route only.
2. Block micro-perm (3 lines): ours [lbu4, a1lw, lbu5, sll4..] vs
   target [lbu4, lbu5, a1lw, sll5, addu5, sll4..]; addu-tbl uses the
   trio reg — may move WITH the trio fix.
3. ARM-1 shape: target = ORIGINAL param-test (`src=..; if (a1) {
   dst=a1; i=7; do..}`; bytes: nop; beqz s4; move a1,s4 in delay +
   arm-2's own move) = +2 insns. **w25 (arm-1 reverted) REGRESSED to
   33**: arg1 refs 3→4 → pri 975 > saved 952 → arg1 steals s1,
   i1494→s3 rotation. Need arg1 livelen 82→85+ (-23 pri) BEFORE the
   arm-1 revert lands. Session-2 note "t3 arg1 refs 4→3" documents
   this exact trade.
Session 5b (w26/w27): arm-2 stmt moves grew arg1 livelen 84→85→86;
saved-tie broken ✓ (saved=s1, i1494=s2 with arm-1 reverted; w27 = 26).
BUT target needs arg1 BELOW i1496 (666): 4 refs ⟹ livelen ≥ 121 — a
+35 jump = the printf-region span. **The flow live_length CLASS
mystery is now THE blocker**: cycle-live pseudos split into a ~150
class (i1494/95/96/tbl — used in the timeout/poll regions) and a
~78-86 class (arg0/arg1 — used only in the check-arms tail), despite
identical live-at-start block membership (verified session 4). Target's
arg1 (4 refs, s4) and arg0 (s7 lowest) must both sit in the LONG class.
**Session 5c/5d — THE LIVELEN CLASS MECHANISM FOUND**: the ~150-vs-80
split is **local-alloc.c:1064 `reg_live_length[regno] *= 2`** — fires
for single-set pseudos carrying a REG_EQUIV note (REG_EQUAL constant
promoted at :1031, incl. symbol arithmetic; or the single-block
unchanging-MEM path :1051). Verified by SLLDBG segments: raw sched
totals are 74(i1495)/76(tbl)/78(arg0)/86(arg1) — near-identical — and
the lreg 148/152 = EXACTLY 2× for the equiv-init pointers, while the
param copies (hard-reg src → no note) stay raw. flow.c's own counts
are overwritten by sched.c's recomputation (update_flow_info); the
sched raw totals ARE the substrate the doubling applies to.
Hooks live (gitignored tree, env-gated): BB2_FLOW_DEBUG (flow.c:1684),
BB2_SLL_DEBUG (sched.c finish_sometimes_live :3121 — NOTE the 2nd
accumulation site :3903 contributes nothing here). Rebuild via
tmp/build_gccdbg2.sh (i386 triplet + parser-touch; CRLF guard: edit
gcc sources then `python3 tools/normalize_lf.py <file>`).
**Target-consistency equations (w27 numbers)**: saved 952(21 raw,
undoubled) > i1494 933(75×2) > i1496 666(75×2) > arg1[NEEDS (197,666):
4 refs ⟹ L ≥ 121; ours 86 raw, undoubled, param-copy can't gain an
equiv note ⟹ target's arg1 raw-L ≥ 121] > tbl 197(76×2) > i1495
202(74×2)?? [tbl/i1495 also need swapping: tbl > i1495 > arg0 256??
arg0 must drop to LAST: arg0 needs L > 78... recheck vs doubled tbl]
— arg1's +35 raw clocks ≈ the copy-loop blocks (~27) + success (~3)
where arg1 is currently dead (dies at `dst=a1` before each loop):
NEXT LEVER CANDIDATES: keep a1 live through the loops (a real post-
loop use exists? arm-1's `return check` — no; investigate whether
TARGET's liveness has a1 live-through via the do-while backedge
semantics vs our early-kill), or find the +35 in entry-block/success
scheduling. Measure with SLLDBG per-segment diffs on shape variants.
THEN assemble → retire 42 → full SHA1 → dual review → queue done.
Kit: tags w20-w27 (tools/mar_probe_kit.py committed), mar_sandbox_
test.sh (HONEST sandbox), mar_diff2.sh, mar_flowdbg.sh/mar_slldbg.sh
(per-block/per-segment livelen traces, marionation-isolated).
**Best-known = w24 (17)**; w27 = arm-1-revert branch (26; arm bytes +
saved s1/i1494 s2 correct; arg1 misplaced s3 at 86-raw/930).

## Composite recipe (other pieces, all probe-verified byte-neutral 142)
- u12/u13: masks (new_var/new_var3 opaque consts; fold via
  update_equiv_regs refs==2 UNWEIGHTED, local-alloc.c:1079) + csmd4
  arm shape + BLOCK-LOCAL `u8 bb` per copy loop → tail cascade
  (bb v0, src a0, i v1, dst a1, check a2).
- w9: poll as REAL `do { status=f(); if (status==0) break; {arms} }
  while (1);` → loop_depth weights (flow.c: depth PLUS ONE) → status
  8 refs, i1494 909→933 (under saved 952), i1495 → s6. **Outer cycle
  MUST stay goto-loop** (real loop → mask refs 4 ≠ 2 → fold dies).

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
