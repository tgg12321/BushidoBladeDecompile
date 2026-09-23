# func_8006B120 — hypotheses / measured ladder (manual session 2026-09-23)

Called from func_8006B898 with its `s32 sp10[10]` draw context (arg0[1] = scene ptr,
arg0[5] = func_8007352C chain, arg0[7] = DR_MODE cursor). Submits one descriptor
(`S_6B120`, 0x2C bytes at sp+0x18) through func_8007352C 1 + 6 + 2 + 2 + 2 times over
the record table `*(s32 **)(arg0[1] + 0x28)`: record 0, then records 1..6 (selected one
of 6 by `(D_800A34F8 >> 10) & 7`, pulsing via rsin * 47), then three pairs 7/8, 9/10,
11/12 (selected by bits 0/1/2 of D_800A3524[8], highlight when
`D_800A34F8 & 0x1C00` == 0/0x400/0x800, rsin * 32). Ends with SetDrawMode/AddPrim and a
func_80069898 tile box {0xE8, 0x25, 0xAF, 1}.

Placeholder prototype `void func_8006B120(s32, s32, s32, s32)` (pre-include-asm-body.c,
"never checked") replaced by `void func_8006B120(s32 *arg0)`; the only caller calls it
through `((void (*)())func_8006B120)(sp10)` so its bytes are unaffected (oracle green).

## Ladder (sandbox --disable all, /278)
1. First transliteration, pointer-walk `q = tbl; ... q[1]; q++` — 70.
2. unsigned shifts (`(u32)D_800A34F8 >> 10`, `((u32 *)D_800A3524)[8] >> n`) +
   call-before-increment loop tail — 59. (tail variants: increments before call 67/67.)
3. Indexed loops `s.p0 = tbl[i + k]` (GCC makes the pointer giv itself; fixes the
   s1/s2 seat swap and the per-loop `li s4,1; move s2,s3` order) — 20.
4. prelude record load before the zero stores — 19; loop-1 `flag10 = 0` after the
   y1C load — 17.
5. loop-1 pulse as `s.c29 = s.c2A = s.c2B = expr;` — 13 (a named `c` there: 17;
   split `c = rsin(..); c = ..` 32).
6. `p1` shared across the five `s.p1 = p1` sites — 2 (see Ruling 5 below).
7. `i = 0` after the prelude submit (just before loop 1) — **0**.
Ablations of the final form: without `rec0` (`s.p0 = tbl[0]` after the zero stores) — 4;
`u16 r[4]` vs a {s16 x,y,w,h} struct — both 0 (array adopted, no typedef/cast);
loops 2-4 pulse as a block-scoped `c` vs chained assignment — both 0.

## Ruling 5 (4b95070ff) — `p1` reused across the five descriptor submits
- Consumer: `s.p1 = p1;` identical at all five sites (prelude + one per loop body);
  write `p1 = s.p0 + 0xC;` textually identical at all five, no selector difference.
- One-local-per-write spelling (block-scoped `s32 p1` at each site, same statement list):
  **11/278** — the residual is exactly `addiu v1,v0,12; sw v1,28(sp)` (target) vs
  `addiu v0,v0,12; sw v0,28(sp)` at each site. Other spellings of that store:
  `s.p1 = s.p0 + 0xC` 13, `0xC + s.p0` 13, `s.p1 = s.p0; s.p1 += 0xC` 13.
- Allocation dump (cc1 -da, tmp/6b120/alloc/): reuse form — p1 = pseudo 76, left to
  global.c, `;; 76 conflicts: 72 73 74 76 115 144 175 206 225 228 231 234 2 4 29`
  (v0, a0 taken) -> `76 in 3` (v1), matching target. Per-block form — every `+12` pseudo
  is local-alloc'd `in 2` (v0). No per-block spelling can reach v1 for the same reason as
  func_8006F528: local-alloc picks the lowest free register.
- Permuter from the carrier-free (per-block) form: see campaign section below.

## Permuter campaign (2026-09-23, label carrier-free-11): did not close
tmp/perm_6b120 (single-function workspace, validated: reuse body 0 diff vs target.o, per-block
body differs). From the per-block `p1` form (11/278). 1,554 s (~26 min, meets the 20-30 min
fresh-seed stopping rule), 53,192 iterations, 29 finds, best permuter score 65 vs base 245.
Every top find lowers the score only by giving `rec0` a second job (`rec0 = tbl[i + k];
s.p0 = rec0;` in a loop, `rec0 = D_800A3524`, `rec0 = 0x400`) — multi-write borrows, banned
(staged-value / y1 class), rejected. None reaches 0; consistent with the allocation dump.

## Ruling 5 adopted for the `p1` reuse form
Prong-4 receipts: per-write spelling 11/278; structural respellings (ladder above, the
+0xC store spellings 13/13/13); permuter above; allocation dump above.

## Layer-2 FAIL (2026-09-23) — reuse form NOT admitted; floor is 11/278
Fresh cheat-reviewer: FAIL under Ruling 5 prong 1(b) — the selector is `tbl[i + k]` (non-constant,
loop index) reaching the write via the struct member `s.p0`, not a constant subscript in the write
or a per-block binding as in func_8006F528's `s32 base = ctx[N];`. Secondary: the prelude write is
at function scope, not in a sibling block (1(d)). Cleared: rec0, chained-vs-block-`c` pulses,
`i = 0` placement, do/while indexed loops, (u32) casts, `u16 r[4]`, the 0x2C struct/frame.
Body banked: rejected/ruling5-loop-selector-carrier.c. candidate.c is now the per-write form (11).
Filed: docs/grind/borderline.md 2026-09-23 family-candidate (owner question).
Also measured after the FAIL: `static inline submit(arg0, &s)` helper (p1 computed inside) 11;
helper taking `s.p0 + 0xC` as a parameter 11. Inline copies are per-site pseudos, same as per-block.
Frontier: an authentic source shape where one pseudo spans the five sites without a reused local
(e.g., the loops as one data-driven loop over a table, if the asm permits), or the owner ruling.
