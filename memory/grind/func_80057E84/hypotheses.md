# func_80057E84 — hypotheses / ladder (manual lane, 2026-09-25)

All scores `sandbox --disable all` (cheat-invisible), 447-insn target.
Scratch variants tmp/e84/ (not banked); receipts in rejected/.

## Ladder (447 -> 0)
| variant | change | score |
|---|---|---|
| v1 | first transcription (u8* args, raw casts, separate j/copy counter) | 137 |
| v2 | copy loop reuses edge counter j; `pos0 = pos1 = ...` store order | 78 |
| v4 | + one vertex cursor for the edge/corner reads | 57 (with a bad `c` form) |
| v6 | + `c = count; count = c + 1` (s32 c) + node pointer | 39 |
| v8 | + `hit_up = 0; hit_dn = 0;` order | 36 |
| v13 | + route-buffer pointer shared (block staging + tail) | 28 |
| s2 | + arg0 as struct (x/z loads MEM_IN_STRUCT_P), pos init before counts | **0** |
| c4 | cleanup: block-local c, idx_dn/up passed directly, s16 ax.., `*(s16 **)` | **0** |
| candidate.c | c4 + annotations + names | **0** (447/447, 0 scored hunks) |

## Load-bearing constructs — receipts (single ablation from c4)
- copy counter reuses `i`: fresh 79 (rejected/fresh-copy-counter-79.c).
  Fresh counter is call-free -> $t0; spill set becomes t2/t3 and every reload
  register rotates (v1 dump: `move t0,v0` in the copy loop, `lw t3,168(sp)`).
- `vtx` single cursor: edge pair split 16, corner split 22 (fresh-edge-vtx-pair-16.c,
  fresh-corner-vtx-22.c). Target address temp is $t0 at all four sites.
- `buf` shared: block-local 4 (fresh-block-buf-4.c; diff = t0 -> a1 only).
- `node` shared: block-local 22 (fresh-block-node-22.c).
- all fresh: 125 (all-fresh-125.c).
- struct walker: cast reads 26 (walker-cast-reads-26.c). RTL (tmp/rtl/v13
  f.sched, cast chassis): load FC has true deps on the two pos stores
  (insn_list 812/548), so sched1 cannot hoist it; target has both loads first.
- PathBuf[2] array vs two locals: 122 (separate-buffers-122.c, frame 296).

## Ruled out / neutral
- `c` split per block, corner index staged through a local `v`: neutral (0).
- poly entry-order variants (poly first/last, go_up placement, `*(u8 **)`):
  28 on the v13 chassis — the entry was the struct/alias issue, not order.
- `c = count++` with s32 c: +andi (worse); u8 c: sltiu (worse).
- node written without a pointer (`buf->node[c].x = ...`): 28, 445 insns.
- dn/up as function-scope pointers `dn = &path[0]; up = &path[1]`: 71 (spilled
  to real stack slots, frame grows).

## Open question for layer-2
Four multiply-written locals (i, vtx, buf, node) are load-bearing, each only
through global.c seating (register-only diffs when split). Framed as
staged-value-reused-variable FAKEs (real value, consumed next, prior value
dead). No permuter campaign was run from all-fresh-125.c (the carrier-free
form) — needed if layer-2 judges any of them under Ruling 5 prong 4 instead.

## Layer-2 verdict 2026-09-25 on the 0 body (rejected/vtx-node-multiwrite-0.c): FAIL
SOUND (keep): PathWalker struct (truthful type, Ruling 1(3)); copy loop reusing
`i` (SOTN fake-reuse-of-i, staged-value bounds 1-6); `buf` shared (real route
buffer pointer, same role, dead at each staging point).
FAIL: `vtx` — fresh local whose only job is these writes (not a staged-value
borrow, bound 2); under R5/R6: edge pair feeds ax/az then bx/bz (1(a)), four
selector vars (1(b)), each value read 2-4x (1(c)), regions not exclusive
(R6(A)). `node` — fresh local written twice for allocator effect; fails R5
ext (A) (consumers dn_x vs up_x), (B) (no member-store selection), 1(c)
(read 3x), R6(A) (both blocks run in one iteration).

## Honest chassis after the verdict: candidate.c = n0, floor 44
n0 = 0-body with per-site `va`/`vb` (edge endpoints) and block-local `cv`
(corner vertex) + block-local `node`; i/buf/PathWalker kept. 447/447, score 44,
every scored hunk register-only (address temp v1/a0 vs target t0; node v0 vs
target a1, and the v0/v1/a0 cascade around them).

Mechanism (tools/gcc-2.7.2/local-alloc.c:472-477 + combine_regs:1784ff): a
pseudo used in ONE basic block with one death gets a local qty, and
combine_regs ties the address result to the dying base/index input (`lw v1;
addu v1,v1,v0` / `addu v0,t0,v0`). Target's t0 / a1 are only reachable for a
GLOBAL pseudo (reg_qty -1: multi-block or multi-death) whose conflict set
covers v0..a3 — at the post sites a1..a3 are never live, so t0 there is only
reachable if the same allocno also spans an inner-loop site (where a0..a3 hold
call args). A per-site pointer is block-local at every site (no branch between
address and loads), so no per-site spelling reaches it.

Killed on the n0 chassis (reduced TU tmp/perm_e84, validated: the 0 body
compiles to 0 diffs there):
- node computed before the `c >= 7` test (to make it multi-block): 445 insns, worse.
- permuter campaign honest-n0 (2 workers, ~2 min, 3 finds 680->580): every find
  stages through another local (hit_z = cv[0]; hit_x = cv[0] - up_x;
  bx = poly[1] then node->poly = bx; new_var = &arg0->path) — borrow-type
  re-shares, evidence only, none closes.
Frontier: a legitimately-real variable (staged-value bound 2) that can carry
the vertex pointer at all four sites and the node pointer in both blocks —
none found (buf/node are PathBuf*/PathNode*, a pun for s16*); or an owner
ruling on a one-role cursor shared across non-exclusive blocks.
- struct view of the vertex table (`Vtx *va = &(*(Vtx **)(poly + 4))[i]`): identical
  to n0 (reduced-TU diff 84 lines, same as n0) — tmp/e84/sv.c.
- `static inline s16 *vtx_at(poly, i)` helper at all four sites: identical to n0
  (inlining makes fresh block-local pseudos) — tmp/e84/inl.c.
