# func_80074E08 — Ruling 9 record for `cells`

laneB, 2026-10-01. Landing body: memory/grind/func_80074E08/candidate.c. It replaces the retro-audit FAIL
body (rejected/retro-audit-2026-09-29.c: `s8 *table = (s8 *)s.header + 0xC` x4, claimed under the Ruling 5
extension, failing its prong (C)). Changes from that body:
- the descriptor's `.header` / `.table` are `s32` (EnvA, used only by this function in text1b_tu2.c), the
  form the file's sibling descriptors use (S_80074488 in func_8007636C, DescF97C in text1b_tu1d.c), so
  every write is `cells = s.header + 0xC;` with no cast; `records` is `s32 *`;
- the four sites lose their bare `{ }` (inert: ff-c-2026-09-30/v4 measured 0);
- work-area reads go through SelWork members (`SELWORK->f0C[arg1]`, `f08[arg1]`, `f24`): 4fd37366e made
  SelWork the one declaration of those bytes. `f24` is the current draw buffer (`&g_gpu_db + idx *
  0x4090`, src/text1b_tu2.c func_80077724), whose first member is the DRAWENV (src/ings.c:267-268
  SetDefDrawEnv on `base` and `base + 0x4090`; src/ings.c:465/488 PutDrawEnv on the same
  `&g_gpu_db + idx * 0x4090`), so its fields are read as `((DRAWENV *)SELWORK->f24)->clip.x` etc.
  (include/gpu.h:71-80; text1b_tu2.c gains `#include "gpu.h"`).
Every score is `sandbox func_80074E08 --disable all --candidate <file>` on main (281/281 insns unless
noted); r9/scores.txt.

## Ruling 9 prongs
- (a) one consumer: every write is read by `s.table = cells;`, identical text, the descriptor's
  `.table` member.
- (b) every write is `cells = s.header + 0xC;` — ONE identical BASE `s.header` (member of the local
  descriptor `s`, read without side effect), K = 0xC, no cast/call/other operator. Sub-object: the
  first SprtEntA cell of the sheet `.header` points at. Evidence independent of the byte-chasing:
  func_8007352C (src/text1b_tu1e.c:123), the consumer of this descriptor, reads `.header` as a 12-byte
  SprtHdrA and walks `.table` as `count` 8-byte SprtEntA cells (src/text1b_tu1e.c:107-121); this
  function's own bytes advance `.table` by `header.count * 8` (asm/funcs/func_80074E08.s:89 `lbu
  $v1,0x2($v0)`, then `sll $v1,$v1,3`). A sheet is N headers followed by its cells, so +0xC is the
  first cell exactly when N = 1. D_SEL.BIN census of the four sheets reached (root+0x18 entries
  [0..3], the only indices this function uses): r9/sheet_census_18.py -> r9/sheet_census_18.txt, each
  with 1 header, cells at +0xC. Same census method as memory/grind/func_800759D0/s0930/sheet_census.py
  (the func_8007636C / func_800759D0 Ruling 9 records).
- (c) each write is read once, by the consumer on the next line, in the same compound statement (the
  do-while body for records[3] and [2]; the function body for [0] and [1]); nothing between.
- (d) the additions in the target: asm/funcs/func_80074E08.s:69 (0x80074F0C), :79 (0x80074F34), :116
  (0x80074FC8), :133 (0x80075004), each `addiu $v1,$v0,0xC`.
- (e) every `.table` store is read by a non-consumer statement before the next one: the
  `func_8007352C((s32)&s)` call that follows each site (records[2]'s site also by `s.table +=`).
- (f) `cells`, true of every write (the sheet's cell array), the name the sibling records use.
- (g) Ruling 5 prong 2: every write is used before the next; the one-local-per-write spelling has the
  same statement list (r9/variants/pv_fn.c vs reuse.c); no write re-stores a held value (each follows
  a new `s.header =`; in the loop's second pass records[3]'s write follows records[2]'s); no
  computation split.
- (h) prong 3: no other job, no other declaration moved, declared once at function scope (the
  innermost scope enclosing the loop sites and the function-level sites).
- (i) receipts below.

## Receipts
| spelling | score |
|---|---|
| reuse body = candidate.c (variants/reuse.c) | **0** |
| one local per write, function scope (pv_fn.c; declarations reversed pv_fn_rev.c) | 12 / 12 |
| one local per write, each in its own block (pv_block.c) | 12 |
| no local, `s.table = s.header + 0xC;` (nolocal.c) | 12 |
| no local, from the record (nolocal_rec.c) / table stored first (nolocal_tablefirst.c) | 31 (289) / 30 (288) |
| per-block sheet local `hdr` (hdr_block.c; table first: hdr_block_tablefirst.c) | 12 / 16 |
| per-block `hdr` + `cells` (hdr_cells_block.c) | 12 |
| per-write locals computed from the record (pv_fn_from_rec.c) | 12 |
| 2026-09-30 ff-c set on the old chassis (ff-c-2026-09-30/v1-v5) | 12 each |

Allocation dump (r9/alloc_dump.txt, r9/tools/dumps_all.sh): in the reuse body `cells` is one pseudo
(78) set in four blocks, `dies in 4 places`, so local-alloc leaves it to global.c, which seats it in $v1
(`78 in 3`) beside the header value in $v0 — the target's `addiu $v1,$v0,0xC`. In every one-local-per-
write spelling each value is a single-block, single-death pseudo (`used 4 times across 2 insns in block
3`), local-alloc seats it in $v0 after the header value dies, and the add moves after the header
store: the 12 instructions of the residual.

Permuter: campaign record appended below.
