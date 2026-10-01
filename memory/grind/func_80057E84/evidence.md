# func_80057E84 — evidence (manual lane, 2026-09-25)

Queue distance 447 (whole-body INCLUDE_ASM, no prior C; pre-include-asm-body.c
is a placeholder stub, not logic). Canonical gate: C (hand_coded_tier LOW).
First manual session: 447 -> sandbox 0 (447/447, 0 scored hunks, 44
not-scored branch-displacement hunks).

## What the function is
Corner router called once from func_80058580 (0x80059510, args s0 = walker,
s4 = polygon set, s6/s7 = goal x/z) right after it seeds the walker's route
(+0x364 node 0, +0x362 count = 1 at 0x800594E0). arg1: +4 -> 8-byte polygon
records (u8 flags 0x80 = open chain, u8 id, u8 margin (func_80057CC8 scales
it by 40), u8 nvtx, s16 (*vtx)[2]) — same layout func_80057ACC walks.
Walker: +0xF4/+0xFC s32 x/z, +0x360 poly idx, +0x361 vertex idx, +0x362
count, +0x364 6-byte nodes {s16 x, s16 z, u8 poly}. Two 0x34-byte route
buffers on the stack (sp+0x28 / sp+0x5C, adjacent => one PathBuf[2] array;
measured: two separate PathBuf locals pad to 8 bytes, frame 296 vs 288,
score 122, rejected/separate-buffers-122.c). For each direction (dn =
vertex idx--, up = idx++): if the segment pos->goal crosses any edge
(func_8005763C, skipping edges whose corner offset point is pos), step to the
next corner (func_80057CC8 offset point), add SquareRoot0 distance, append a
node (stop direction after 8); otherwise add the straight distance and stop.
Open chains end with dist = 100000. Cheaper buffer is appended to the walker
route in reverse.

## Frame / spill model (measured)
Spill slots (8-byte aligned, reload alter_reg) in pseudo order: arg0 0xA8,
goal_x 0xB0, goal_z 0xB8, go_dn 0xC0, idx_dn 0xC8, idx_up 0xD0, iter 0xD8,
nedges 0xE0, dist_dn 0xE8, dist_up 0xF0 — the declaration order in
candidate.c reproduces it. go_up lives in $fp. Reload spill regs t1/t2
(round-robin); anything that seats a pseudo in $t0 elsewhere shifts it.

## 2026-09-25 layer-2: vtx/node sharing FAILED (see hypotheses.md); honest
## floor 44 = candidate.c (n0). The 0-body constructs below are the record.
## Closing constructs (each measured; receipts in rejected/, all from the
## un-annotated candidate tmp/e84/c4.c which is byte-identical in code)
- PathWalker struct for arg0 (MEM_IN_STRUCT_P loads hoist over the pos
  stores in sched1): cast reads 26 (walker-cast-reads-26.c, 448 insns).
- copy loop reuses the edge counter `i` ($s2): fresh counter 79.
- one `vtx` cursor for all four vertex reads ($t0): edge pair split 16,
  corner split 22.
- `buf` shared by both corner blocks + pick/copy tail ($t0): block-local 4.
- `node` shared by both corner blocks ($a1): block-local 22.
- all four fresh at once: 125 (all-fresh-125.c).
- `c` and the corner vertex index are block-local / direct (neutral, 0).
- ordering facts: `hit_up = 0;` before `hit_dn = 0;` (reorg steals the first
  loop-top insn into the latch delay slot: target steals s7), stores
  `dn_x = up_x = ...` (up slot 0xA4 stored first), `c = buf->count; buf->count
  = c + 1;` with s32 c (slti on the old count; `c = count++` emits andi),
  open-chain arm spelled `go = 0; dist = 100000;` in both blocks (form in
  use at the closing measurement; its isolated effect was not re-measured).

## Session 2 (2026-09-25) — measured facts
- Target corner block: `addiu t0,sp,0x28` (buf) sits in the delay slot of the
  `c >= 7` test (fill_simple_delay_slots from before the branch); node =
  `addu a1,t0,v0` with v0 = c*6+4, meaning buf is not folded into sp. n0 does the
  same except the dest register (v0, tied).
- buf and the vertex pointer both being $t0 is consistent with global.c seating two
  separate call-free global pseudos in the first free register after a0-a3. The pun
  receipt shows one pseudo would also match; that is evidence, not a spelling.
- The staged-value rule does not mention type. Layer-2's s1 verdict treats a
  PathBuf*/PathNode* carrying an s16* as a pun, and Ruling 5 1(b) bans
  cast-laundered bases. Treat a cast-typed borrow as disqualifying.

## Session 3 (laneB, 2026-10-01) — landing prepared (round 1)
Body = candidate.c (83327e250) spliced into src/text1b.c + include/code6cac.h unk_360/unk_361; full build
SHA1 == oracle; sandbox --disable all on the spliced src 0 (447/447, 0 source-level / operand-only);
layer2 hash 4c0dc0cccbcea75a; reviewer_precheck clean. Rulings relied on: Ruling 11 (vtx, node, route;
r11/README.md), Q51/Q53 (i; SOTN src/dra/42398.c:75 @aa53500). Data model: hypotheses.md Session 3.
## Round-1 layer-2 (2026-10-01) on 4c0dc0cccbcea75a: FAIL (rejected/r1-local-navpoly-dm-fail-0.c)
rev-57E84-r11 PASS (vtx/node/route Ruling 11 package, i Q51, scores reproduced). rev-57E84-dm FAIL on the
data model. Frontier (the orchestrator's fix list): (1) NavPoly/NavPolySet in include/code6cac.h,
`extern NavPolySet D_8009A658[];`, func_80058580's pois typed NavPolySet *, prototypes of E84/ACC
`(PracticeMenuRec *, NavPolySet *, s32, s32)`, no `(NavPolySet *)arg1`; (2) func_80057ACC respelled on
members, no `(s32)p` at its call; (3) the route record declared once in the header and embedded in
PracticeMenuRec at +0x360 (replacing unk_352's tail, unk_362/363, unk_364[8]), used for path[2], consumers
(E84, ACC, 58580, func_800571C0) respelled byte-neutrally; (4) func_80057CC8's arg0 retyped NavPoly *,
the four `(u8 *)poly` casts dropped. Landing order after Q65: (A) data-model cheat-cleanup, (B) Match.
src/include reverted, build/ back to the oracle.
## Round 2 landing prepared (2026-10-01, post-Q65)
(A) data model landed bdd726a8f (rev-57E84-A PASS on 5 bodies). (B) body = candidate.c spliced over the
INCLUDE_ASM line; full build SHA1 == oracle; sandbox 0 (447/447, 0 source-level / operand-only); layer2
hash 18f12977fa847c9f; precheck clean; Ruling 11 package re-run on this body (r11/README.md, round 2);
lane_checklist_1001c run (no casts, no int-held pointers, header types, Q51 tag form).

## LANDED COMPLETED-C (2026-10-01)
Match 8bd07dcea (body hash 18f12977fa847c9f), queue 618265ec9; data model bdd726a8f. Layer-2 round 2:
rev-57E84-B PASS (full scope: Ruling 11 package re-verified on this body, i/SOTN 42398.c verified, zero
casts, lh/lhu as cc1's own choice, no read without a write). Round 1: rev-57E84-r11 PASS / rev-57E84-dm
FAIL (data model). check_completion_integrity OK. Ledger closed.
