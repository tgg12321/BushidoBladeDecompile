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
**Open lead:** find_cross_jump requires REG_DEAD-note equality post-
reload (`cross_jump_death_matters`) — the two sites may differ in the
original by surrounding register liveness. Next session: dump the
death notes at both [set s0,13] sites (instrumented cc1 or -da .jump2
dump) and find a C shape that makes the death sets differ (e.g. a value
live across one site but not the other) with zero byte delta.
Also untried: making ONE site fall through to sel_dispatch (suffix < 2
insns → no merge) — requires the case-block emission order to place it
last; check GCC's case-ordering vs target layout first.

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
