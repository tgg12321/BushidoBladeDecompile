# motion_SetMotion (code6cac_c_mid.c) — WIP, Wall 1 SOLVED, Wall 2 open

## TL;DR (2026-07-01)
The coupled wall split and half fell. **Wall 1 (RA priority) is SOLVED
in pure C** — both pins retired (commit: cheat-cleanup 2026-07-01); full
SHA1 == oracle with the 1 remaining rule applied. **Wall 2 (cross-jump
merge) remains**: 1 regfix rule (case-9/11 sel 12→13 subst @
regfix.txt:2484). Masked sandbox is BLIND here — full SHA1 only.

## Wall 1 recipe (landed — reusable for the cluster)
Duplicate load_sel2's `sel2 = D_800A3350;` into case 0 (FAKE-annotated)
and move the label to case 13/17's copy. Flow counts the REAL second def
(sel2 reg_n_refs 3→4 → pri 851 > result 412) → RA lands sel2→$s2 /
result→$s3 naturally; jump2 cross-jump re-merges the identical
[lbu; j] tails → zero byte delta. **Label placement steers merge
direction**: label on case 13/17 ⇒ cross-jump rewrites case 0's copy ⇒
target layout exactly. Mirror labeling (m6) emits a 4-diff flipped
layout. Measured this session: dead stores are INERT for global RA
(m1: cse folds const re-stores; m3: `sel2 = v0` deleted by flow WITHOUT
being counted — 2.7.2 counts refs post-deletion). Real duplication is
the working spelling.

## Wall 2 — cross-jump no-merge (open)
Target keeps BOTH `[li s0,13; j sel_dispatch]` sites (==3 arm at ea0 as
j-delay form; case 9/11 at fc0). Writing 0xD in C merges them (SHA1
82e50f76). Ruled out this session:
- consecutive-label aliasing (sel_dispatch2:) — jump1 unifies
  same-address labels, then merge fires ANYWAY and deletes 2 insns (m7).
- ==3 arm reshaped to the two-goto form (`sel=0xF; if(==10) goto disp;
  sel=0xD; goto disp;`) matching target's delay-slot structure — still
  merges (m8).
**REG_DEAD lead REFUTED (2026-07-02):** the death-note comparison in
find_cross_jump is `#ifdef STACK_REGS` only (jump.c:2436-2467) — MIPS
never compares them. **Pairing algorithm fully mapped** (jump.c:
1966-2001 + 2371-2533): for each simplejump, (a) minimum=1 attempt vs
code-before-its-own-label, (b) minimum=2 attempts vs every other jump
in the SAME label's jump_chain. Stream-1 hitting a CODE_LABEL does
`--minimum; break` — so a [jtbl-label; set13; j] block as stream 1
gets 1 match + label bonus = merge GUARANTEED against any same-label
[set13; j] partner. ⇒ the original compile must have had the two 0xD
jumps targeting DIFFERENT CODE_LABELs at jump2 time (labels are free
in bytes — they coalesce at assembly). Remaining lever family: a
second label for case 9/11's goto that is NOT adjacent to sel_dispatch
(adjacent → m7 showed unification/merge). Known cost problem: any
real-code separation between the labels either emits bytes or gets
cross-jump-merged itself leaving a thunk `j` (+1 insn). UNSOLVED
puzzle: what C makes GCC keep two labels apart through jump1's
tensioning yet emit them at the same final address. Also note: in the
CURRENT source's cc1 output the ==3 arm's 0xD/0xF sets are NOT visible
as separate li's near the dispatch (only case-10's 13 at a bne delay)
— map the arm's actual emission before theorizing further.

## Ruled out (session 1-2, do not re-derive)
- Declaration-order swap (masked noise, swap persisted).
- `func_8006BEC4(0xA, sel2)` at early exit (bytes: li a1,-1 vs move).
- Real 4th-ref forms that add bytes; livelen restructures (init-later).

## Pointers
- tmp/probe_msm.py (m0-m9 probe harness; regenerate from git if gone).
- Wall-1 mechanics: dead-store-fake-exception.md (chain-extender note),
  register-alloc-pure-c.md, register-alloc-deep-dive.md.
- Cluster siblings to apply the Wall-1 recipe to: saEft00Add,
  marionation_Exec, cpu_side_move_dir_4 (local-alloc variant — see its
  WIP), func_8007C97C.
