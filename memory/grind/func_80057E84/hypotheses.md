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

## Session 2 (manual, 2026-09-25) — honest floor still 44 (candidate.c = n0)
Receipts are single edits from n0 unless marked.
| variant | change | score |
|---|---|---|
| pending-va-borrow-22.c | edge-start `va` also carries endpoint b + both corner reads (vb/cv removed) | 22 (all vertex sites match; residual = node only) |
| rejected/buf-carries-vertex-pun-22.c | `buf` cast-carries the vertex pointer at all 4 sites (PUN, evidence only) | 22 |
| rejected/node-split-init-36-445insns.c | Ruling 4 split `node = buf->node; node += c;` | 36, 445 insns: CSE folds buf->node to sp+44 (`addiu a0,sp,44`), target keeps buf in t0 and folds +4 into the offset. Dead end, not a floor |
| bc_n1split (tmp) | pun + node split | 14, 445 insns |
| rejected/hoisted-vtab-dn-44.c | permuter 630: vertex-table load hoisted above `if (go_dn)` | 44: tie just moves to operand 2 (`addu a0,v1,a0`) |
| p580 (tmp) | permuter 580: `hit_z = cv[0]` (hit_z is addressable) | 448 insns, worse |

Permuter campaign s2-honest-n0-long (tmp/perm_e84b, 3 workers, ~30 min, ~5.5k
iterations, stopped + harvested): finds 420/485/630/665/680 (base 680). All are
value-staging through unrelated or addressable locals (`c`, `next`, `hit_x`,
`new_var = up_x`) except 420 = the `va` borrow above. None touches the node pointer.

### Why no per-site spelling can close (tie mechanics, local-alloc.c:1211-1300 + combine_regs:1784-1945)
The tie is attempted at the DEST's birth insn: combine_regs refuses when the dest
pseudo already has a qty or is global (`reg_qty[sreg] >= -1`), otherwise it ties
the dest to the first dying LOCAL input (operand 1, then operand 2). The target's
`addu t0,v1,v0` / `addu a1,t0,v0` tie to neither input, so the dest is global (or
born earlier, which needs a dead first store: banned). A single write cannot make it
global: reg_n_deaths is 1 and there is no branch between address and loads at any
site. So each target register needs ONE pseudo across several sites:
- vertex ($t0): one pointer across the edge pair AND both corner blocks. The only
  REAL pointer variables are poly (live), arg0 (live, tail), arg1 (`lw a1,4(a1)`
  at entry needs it in a1), buf (PathBuf*, pun). The one same-type real variable
  is n0's own `va` -> pending-va-borrow-22.c.
- node ($a1): one PathNode* across both corner blocks. No real carrier exists.
  The tail source is local ($v0, tied), the tail dest is $a0, buf is $t0, every
  other variable crosses calls (callee-saved) or is addressable (hit_x/z, ofs*,
  dn/up). Writing node through buf gives $t0. Ruling 5 fails (3 reads per write,
  1(c); consumers dn_x vs up_x, ext (A)). Ruling 6 fails (both blocks run in one
  iteration).

## Frontier after s2
1. Layer-2 question A: is pending-va-borrow-22.c a staged-value borrow (real `va`,
   same type and kind of value, dead at each staging point, like `buf` which
   passed), or the FAILED `vtx` under another name? It only matters together with (2).
2. Node ($a1, 22): needs an owner ruling admitting one per-direction "node being
   appended" pointer written once in each of the two sequential corner blocks, or a
   structural discovery that makes the node address a global pseudo from ONE
   write. Neither exists today. With (1) and (2) both admitted the body is
   rejected/vtx-node-multiwrite-0.c (0, 447/447).

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated (floor 44/447). `candidate.c` (n0, every var single-role) = 44. `rejected/vtx-node-multiwrite-0.c` = 0 but failed layer-2 on `vtx`/`node`. `pending-va-borrow-22.c` = 22. Layer-2 already rated PathWalker, the `i` copy-loop reuse and `buf` sound.
- RE-OPENABLE: rotated ~2026-09-25 20:27 CDT; Ruling 11 (reused local admitted on dump proof that no split matches) landed 262db111c 2026-09-26 13:30 CDT. The rotation cites Ruling 5/6/9 failures, but `.claude/rules/ordinary-c-judge-decidable.md:40-41` lets Ruling 11 admit what those fail. Never tried under Ruling 11.
- CONSTRAINTS: `vtx`/`node` on the Ruling 9 banned list (`.claude/rules/reused-local-meaning-source.md:59-61`); Rulings 5/6 fail them (hypotheses.md verdict section). That list does not block Ruling 11.
- BLOCKER [F]: dump-proven allocator effect - local-alloc.c:472-477 + combine_regs (:1784-1945) tie a block-local address pseudo to its dying input; the target's $t0/$a1 are reachable only by one global pseudo spanning several sites; no single-write spelling produces that.
- PLAN:
  1. Re-measure the 0-body on HEAD.
  2. Retype the polygon record (`s16 (*vtx)[2]`) to drop the `*(s16 **)(poly + 4)` casts (struct view measured identical).
  3. Ruling 11 package for vtx/node: bank .lreg/.greg for 0-body and n0 with command lines; name the mechanism above; bank every split measured (16, 22, 28, 36/445, 44, 125); the n0 permuter campaigns (~5.5k iters) count as the search; replace the staged-value FAKE text with a Ruling 11 (F) comment (names are kind-names true of every value).
  4. Fresh manual layer-2.
  5. Operator returns it with `queue auto-return`/`unpark` (not an owner ruling).
- DEPENDS: PathWalker typedef must be visible to func_80058580's call - land this before func_80058580.
- ODDS/LANE: manual, ~1 session of paperwork, ~65% [I].

## Session 3 (laneB, 2026-10-01) — re-baselined on main 632e2fe2c; Ruling 11 route
- Data model: arg0 is `PracticeMenuRec *` (the prototype func_80058580 already declares,
  text1b.c extern); +0x360/+0x361 split out of `unk_352` as `u8 unk_360` / `u8 unk_361` (the bytes
  func_80057ACC writes); waypoints are `CpuWaypoint` (kind <- poly byte 1); the polygon record is
  `NavPoly` {flags, kind, margin, nvtx, s16 (*vtx)[2]} reached through `NavPolySet` (the D_8009A658
  row: count, polygon array) — no `*(s16 **)(poly + 4)` / s32-held pointers left; `(u8 *)poly` only at
  the func_80057CC8 calls (its completed prototype). The local route buffers are `RouteBuf`.
- Scores (sandbox --disable all): 0-body re-typed 0 (tmp v0/v1/v2); vtx/node declared in the iter-loop
  body (Ruling 11 (A)) 0; annotated candidate.c 0 (447/447).
- `buf` renamed `route` (no-semantic-purpose name list); `i` kept under Q51 (SOTN DebugCaptureScreen,
  src/dra/42398.c:75 @aa53500), fresh copy counter 79.
- Ruling 11 package for vtx/node/route: r11/README.md (twins, ablations, structural respellings,
  dumps, find_reg traces, permuter from the full split).
