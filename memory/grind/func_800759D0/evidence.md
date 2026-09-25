# func_800759D0 — evidence (manual lane, 2026-09-25)

Character-select grid renderer, select-screen case 2 of func_80077374 (the draw half of
func_80075F80). Same descriptor (S_80074488) and draw calls as the case-3 sibling
func_8007636C (COMPLETED-C d844de59a) and func_800753D8.

Floor: 364 (INCLUDE_ASM, no prior C) -> 66 (first transcription) -> 39 -> 32 -> **25**
(candidate.c, per-site locals, the honest floor under current rules) -> 0 with ONE
function-scope `q` reused at 4 sites, which layer-2 FAILED 2026-09-25 (multi-write carrier,
Ruling 5 1(b)/1(f), extension (A), Ruling 6): rejected/function-scope-q-multiwrite-0.c.
Owner question: docs/grind/borderline.md 2026-09-25 "one role, differing constant offsets".
All measured with `sandbox --disable all --diff`. The q-step below describes the rejected 0.
Session 3 (2026-09-25, manual lane): the owner answered it as Ruling 9 (c3e7a0b9e,
ordinary-c-judge-decidable.md). The 0 body with `q` renamed `cells` landed as 23045f51f
(layer-2 PASS), then a combined layer-2 re-review the same day FAILED it on Ruling 9
(b)/(f) (section "Ruling 9 FAIL" below), and it was reverted to INCLUDE_ASM. The body is
rejected/ruling9-cells-placeholder-overrun-0.c. candidate.c is back to the honest per-site 25
form (also kept as rejected/per-site-locals-25.c, the Ruling 9 (i) receipt).
Owner question: docs/grind/borderline.md 2026-09-25 "func_800759D0 — Ruling 9 (b) on a
latent-bug path".

## Ruling 9 (b) — what +0xC and +0x24 reach (session 3, 2026-09-25)
**The record.** `s.sp18` is S_80074488 / EnvA `.header` and `s.sp1C` is `.table` of the draw
descriptor func_8007352C consumes (src/text1b.c:10945-10988): it reads a 12-byte `SprtHdrA`
(text1b.c:10929; tp, `count` at +2, clut cx/cy at +4/+6, ubase, vbase) at `.header` and walks
`count` 8-byte `SprtEntA` cells (text1b.c:10939; s16 x,y; u8 u,v,w,h) from `.table`. A sprite
sheet is N SprtHdrA headers followed by its SprtEntA cell array. Both strides are in the target's
own bytes: header stride 12 (`s.sp18 + 12 + arg3 * 12`, asm/funcs/func_800759D0.s:112
`addiu $s7,$v0,0xC` and the loops' copies), cell stride 8 (`s.sp1C += count * 8`).
func_80073728 (text1b.c:10995-11008, `Ft4Sheet` 12 bytes / `Ft4Cell` 8 bytes) declares the same
layout for the POLY_FT4 walker.

**The data (original disc, not byte-chasing).** arg0[0] = *(D_800A36A0 + 4) = the buffer
func_8006E950(6, p_old) loads (text1b.c:12489-12498); func_80076FF8 (text1b.c:12433) relocates
its root slots [5]..[14] (+0x14..+0x38). That file is disc/TIM2D/D_SEL.BIN (sha256 032e85c7…
451df12): its root word [2] is the image func_8006E950 LoadImages (0x180x0x1DC + 0x170x0x24
16-bit = 0x5FB80 bytes) and [3]-[2] = 0x60AF0-0xF70 = 0x5FB80 exactly; and root+0x20/+0x24/
+0x28 hold 3/4/5 sheets, exactly loop 3's `f65 + 3` bound for f65 = 0/1/2. Census
(tmp/f759d0/sheet_census.py, output tmp/f759d0/sheet_census.txt; a slot is header-shaped iff
tp1 == 0, pad == 0, count >= 1 and CLUT row 480 <= cy < 512):
| root slot (reader, K) | sheets | header count |
|---|---|---|
| +0x14 [0] (this fn head, func_800753D8:11861/11864, K=0xC) | 1 | 1 (count 2) |
| +0x14 [1..20] (this fn loops 1-2 `table[entry+1]`/`table[arg2[i]+1]`, func_8007636C:12205-12206, K=0x24) | 20 | 3 each |
| +0x20/+0x24/+0x28 [i] (this fn loop 3, func_8007636C:12250-12251, K=0x24) | 3+4+5 | 3 each |
| +0x30 [0,2,..,10] (func_8007636C:12228-12229 `table[idx*2]`, K=0x24) | 6 | 3 each |
| +0x30 [1,3,..,11] (func_8007636C:12239-12241 `table[idx*2+1]`, K=0xC) and [12] (func_8007636C:12185-12188 `table[12]`, K=0xC) | 7 | 1 each |
| +0x2C (func_800753D8:11845-11847/11877-11880, K=0xC) | 4 | 1 each |
| +0x34 (func_80074488:11317-11319, K=0xC) | 15 | 1 each |
(CORRECTED 2026-09-25. The session-3 text here claimed that no +0x24 reader reaches
+0x14 [21]. That is FALSE for this function; see "Ruling 9 FAIL". Loop 1 indexes
`entry + 1` with `entry` a D_8009BCF8 byte (0..19 in the EXE), so it reaches [1..20] only.
Loop 2's `arg2[i]` can hold the 0x14 placeholder, which reaches [21].)
Every sheet any reader reaches with +0xC has exactly one header. Every sheet reached with +0x24
(except this function's loop-2 placeholder path to [21]) has exactly three (normal + one cursor highlight per player; the three headers share tpage and
count and differ in CLUT x). So K = 12 x (header count) in every case: both offsets reach the
SAME sub-object, the first SprtEntA of the sheet's cell array, which every reader stores to
`.table` (sp1C) and func_8007352C walks the same way. The +0xC/+0x24 choice is the sheet's
header count, not a different kind of object. First cells: +0x14[0] {x0,y0,u0,v0,w64,h64};
+0x14[1] {x117,y45,u0,v0,w64,h11} (tmp/f759d0/sheet_census.txt).

## Ruling 9 FAIL (combined layer-2 re-review, 2026-09-25) — reverted to INCLUDE_ASM
- **The path.** When the cursor sits on an unavailable cell, func_80075F80 (text1b.c, the
  `arg2[state[0x3C / 2]] = 0x14;` statement after its confirm block) writes the placeholder 0x14
  into the current pick slot f3C. In the same frame, func_80077374 case 2 calls
  func_800759D0. Loop 2 (`i < f3C + 1`, guard `arg2[i] >= 0`) then draws slot f3C:
  `s.sp18 = table[0x14 + 1]`, `cells = s.sp18 + 0x24`, and (i == f3C) the highlight step
  `s.sp18 += 12 + arg3 * 12`.
- **The data.** D_SEL.BIN root+0x14 [21] is the sheet at 0xC84: 1 header (count 2, CLUT
  (32,480)), cells {98,45,63x12} and {161,45,36x12}. The record ends at 0xCA0, and a run of
  12-byte header-like records follows (0xC9C: tp 0x1A, count 2, CLUT (16,505)).
  `+0x24` = 0xCA8 is past the end of the record, inside that next run. The highlight step
  reads 0xC90 (arg3 = 0, which is cell data read as a header, `count` 0x2D) or 0xC9C (arg3 = 1).
  This looks like a latent bug in the original game: loop 2 treats every sheet as 3-header.
- **Why it fails.** Ruling 9 (b) requires BASE + K to be a sub-object of BASE's record at
  every write. At this reachable write it is not. (f): the declaration comment's "three
  headers" claim is false on that path. The reviewer measured a block-local loop-2 site at
  30/364, so the shared variable is not byte-neutral to split and falls back to Ruling 5/6,
  which FAILed it earlier.
- **Status.** INCLUDE_ASM on main. Honest floor 25/364 (candidate.c, per-site). The owner
  question is in docs/grind/borderline.md: should (b) be judged by the layout the code
  assumes rather than by the data on a buggy path? func_8007636C is NOT affected: in cases
  3-5 its pick list holds only confirmed entries (evidence in its ledger, 71b14499d).

## Ruling 9 prong walk (the rejected cells body, session 3; (b) FAILED, see above)
- (a) all 4 writes feed `s.sp1C = cells;` (identical text), the descriptor's `.table` member.
- (b) every write is `cells = s.sp18 + K;`, K in {0xC, 0x24}; BASE `s.sp18` (member of local
  `s`), just assigned the sheet address at each site (loops 2/3 compute before the highlight
  step moves sp18 to header 1/2); sub-object: the cell array (section above).
- (c) head: lines adjacent at function scope; loop 1: both in the `& 1` then-block; loop 2: both
  in the `arg2[i] >= 0` block with only the highlight if/else between; loop 3: both in the for
  body with only the highlight if between. No return/break/continue/goto; `cells` has no other
  reader.
- (d) target adds: 80075A40 `addiu a1,v1,0xC`; 80075BB4 `addiu a1,a2,0x24`; 80075D14 and
  80075E24 `addiu a1,v1,0x24`.
- (e) each consumer store is followed, unconditionally in its own block, by
  `arg0[4] = func_8007352C((s32)&s);` before any later consumer store or exit.
- (f) `cells`: true of all four writes.
- (g) statement list identical to rejected/per-site-locals-25.c (diff: declarations and
  identifiers only); every write consumed before the next; `s.sp18` is re-written before every
  write, so no write re-stores a held value; no computation split.
- (h) one declaration, function scope (the innermost scope enclosing the head and loop writes);
  no other declaration moved.
- (i) per-site 25/364 (rejected/per-site-locals-25.c); structural ladder (below), permuter from
  the carrier-free chassis (hypotheses.md "Permuter"), greg allocation dump (hypotheses.md
  "Session 2").

## What closed it (each step measured)
- Loop 2 highlight test written `if (i != f3C) {sp40 = 0} else {sp40 = 1; sp18 += ...}` with
  `q = s.sp18 + 0x24` computed before it and `s.sp1C = q; s.sp1C += ...` after: 66 -> 39.
- D_8009BCF8[i] read as `(D_8009BCF8 + i)->unk0` (pointer arithmetic, address formed as its
  own value) at all three sites: the address `sym + i*2` is then a pseudo, cse reuses it for
  the second read (target `t0`) and loop.c hoists the symbol into `$s6`, which the target
  uses for all three reads. `D_8009BCF8[i].unk0` folds the symbol into each MEM
  (`lui at; addu at; lbu %lo`): 41 at the third site alone, 44 with it at all sites.
- Grid lookup `((u8 *)D_8009BCF8)[index]` with `index` a local, exactly func_80075F80's
  (main) spelling: at-form lbu as target. Inline index 36; the struct-index spelling
  `D_8009BCF8[row*5+col + arg1*10].unk0` 67.
- Head order `s.sp18 = table[0];` before the sp30/sp34 stores: 32 -> 25.
- ONE function-scope `q` for the sprite-image pointer written at the head (+0xC) and in
  loops 1 and 2 (+0x24) (and loop 3, byte-neutral there): 25 -> 0. Same construct as the
  sibling func_8007636C on main (function-scope `q`, +0xC / +0x24 writes).
  Mechanism: local-alloc only takes a pseudo whose every reference is in one basic block;
  a q referenced in several blocks goes to global.c, which seats it in `$a1` at every site
  as the target does (head `addiu a1,v1,12`, loop 1 `addiu a1,a2,36`, loop 2/3
  `addiu a1,v1,36`), and that also re-seats the neighbouring loop-1 values
  (table value a2, state v0, arg1*4 a3, cell address t0).
- `s32 zero` constant-holder (FAKE, named-local-fake-exception) for func_8006E480's 2nd arg
  (target `$fp`): literal 0 = 30/364 at 362 insns.

## Ablations of the final form (all from candidate.c, one change each)
| change | score |
|---|---|
| per-site q0..q3 (one local per write) — = candidate.c (honest floor) | 25 |
| head without q (`s.sp1C = s.sp18 + 0xC`) | 3 |
| loop 1 without q | 22 |
| loop 2 block-local q | 30 |
| loop 3 block-local q | 0 (byte-neutral; kept on the one q for one role) |
| literal 0 instead of `zero` — rejected/literal-zero-no-holder-30.c | 30 |
| third cell read as `D_8009BCF8[i].unk0` | 41 |
| `rec = &D_8009BCF8[i]` held across the func_8007352C call | 123 |
| `(&D_8009BCF8[i])->unk0` at all three sites | 0 (same tree as `+ i`) |

## Allocation provenance (session 2, 2026-09-25)
greg dump of the shared-q form: q = pseudo 80 -> $a1, hard conflicts {v0, v1, a0, sp}. In the
target's loop-1 and head q ranges, $v1/$a0 are free. So the $a1 seat comes from the union of
conflicts across sites (loop 2 holds the table value in $v1 and arg3*2 in $a0 live across its
q). One variable was shared in the original source. Details and measurements: hypotheses.md
"Session 2".

## Tools
tmp/f759d0/gen.py (variant generator), tmp/f759d0/rtl.py (splice + cc1 -da, per-function
dumps), tmp/f759d0/hk.py (scored hunks). tmp is gitignored.
