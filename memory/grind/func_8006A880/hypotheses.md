# Hypothesis ledger — func_8006A880

## s2 (manual lane, 2026-09-26) — receipts table (sandbox --disable all, /552)

Chassis g2 = memory/grind/func_8006A880/candidate.c (0/552). Every row changes ONE thing
from g2 unless stated. Variant files: tmp/func_8006A880/v/*.c.

| spelling | score | note |
|---|---|---|
| g2 (shared `mask`, shared `tbl`, `cells`, shared `i`) | 0 | sandbox-0 form |
| `alive` split from `mask` (g2_sep) | 46 (551 insns) | scan mask to $s2/$s3, cascade |
| B/F/G tables in fresh locals (t1) | 6 | loads land in v1/a0, target $s2 |
| + Phase C/D table in a fresh local (t2) | 12 (553) | |
| all three split = ordinary body (ord) | 32 | |
| ord with per-site `s.p1 = s.p0 + 0xC` (ord0, carrier-free) | 47 | permuter base |
| `cells` direct `s.p1 = s.p0 + 0xC` (g2_direct) | 15 | Ruling 9 (i) receipt |
| `cells` one block-local per site (g2_block) | 15 | Ruling 9 (i) receipt |
| scan counter `bit` split from `i` (g2_bit) | 7 (553) | func_8003800C counter-reuse shape needed |
| explicit `cursor`/`y += 0x18` bivs instead of GIVs (base) | 25 | pre-loop order wrong |
| s32 mask + literal 1 (fold rewrites to srav/andi) | 86-era | use u32 |
| s32 *arg0 `arg0[N]` (in-struct) instead of u8* cast-offset | 56 -> 39 | Phase D load hoisted into nop |
| Phase G `s.h` before `s.x`/`s.y` (g1) | 2 | |

- CONFIRMED (dump): target seats BC08-mask and BC04-alive in ONE callee-save ($s6);
  as two pseudos global.c cannot (see evidence.md s2 allocation fact).
- CONFIRMED: target loads the Phase B / F / G sheet tables straight into $s2 with no call
  before their use — only a pseudo that also lives across calls (the loop's table) is
  global-allocated there; a fresh block-local is local-alloc'd to v1/a0 (t1 = 6).
- KILLED: pre-loop statement orders p1-p9 (25-35) with explicit bivs — the GIV form is
  what orders the preheader.

## Policy status (s2, updated): Ruling 11 (owner 2026-09-26) is the admission route for
`sheets` and `row_mask` — proof in ruling11.md; `cells` under Ruling 9; the scan counter is
its own `bit` (initialised at the top, 0/552), so no loop-counter reuse is needed. The
paragraph below is the pre-Ruling-11 reading, kept for the record.

## Policy status (s2, before Ruling 11 was found in the tree)
The 0 form needs two multi-write locals that no ruling admits as written:
`mask` (BC08 scan mask, then BC04 alive mask: two meanings) and `tbl` (five loads of
five different sheet tables, one meaning, consumers `s.p0 = tbl[idx]` with differing idx,
first write read twice). Ruling 5 fails 1(b)/(c) (tbl) and 1(a)/(b)/(f) (mask); Ruling 9
needs `VAR = BASE + K` (tbl's writes are loads); Rulings 6/8/10 do not apply;
staged-value-reused-variable needs an immediately-consumed staged value (mask's BC08 read
is consumed after four calls). Filed as a policy-question in docs/grind/borderline.md.
