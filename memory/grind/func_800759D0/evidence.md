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
Session 4 (2026-09-25, manual lane): the owner answered it as Ruling 9 amendment (b′)
(bcdc1648e). The same body, with only the `cells` declaration comment rewritten for (b′)(4),
is re-submitted; the (b′)(1)-(3) record is the section "Ruling 9 (b′)" below.

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
  12-byte header-like records follows (from 0xCA0: tp 0x1A, count 2, CLUT (16,505); session 4
  correction, this line said 0xC9C).
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

## Ruling 9 (b′) (session 4, 2026-09-25; amendment bcdc1648e)
**Line anchor:** every `text1b.c:N` below is a line of src/text1b.c AT COMMIT bcdc1648e
(func_800759D0 as INCLUDE_ASM at :11986). With the body spliced in, lines after :11986 shift
by +146 (e.g. the 0x14 store :12127 becomes :12273). `:N` alone = asm/funcs/func_800759D0.s.
Status: layer-2 FAILED this re-submission 2026-09-25 as UNDECIDED on (b′)(3) (does "lands
inside a record" mean BASE's own record or any record? 0xCA8 is byte 8 of the unreferenced
record at 0xCA0). (1), (2), prongs (5)/(6) and `zero` held. Owner question:
docs/grind/borderline.md 2026-09-25 "func_800759D0 — (b′)(3) past-the-end into
unreferenced data". Body banked as pending-bprime-0.c; candidate.c stays the per-site 25.
Census script: tmp/f759d0/bprime_census.py (output tmp/f759d0/bprime_census.txt). It reads
D_8009BCF8 from the original EXE (disc/SLUS_006.63, file offset 0x8C4F8) and every sheet from
disc/TIM2D/D_SEL.BIN, and classifies BASE + K per site as first cell / inside the record on
something else / outside the record. Record extent = 12 x (header count) + 8 x (header 0 count).

### (1) The assumed layout, from accesses other than the `cells` writes
- **Header block, 12-byte stride, headers 1 and 2 addressed.** The highlight step moves
  sp18 (`.header`) from header 0 to header 1 + arg3, i.e. +12 or +24:
  loop 1 asm/funcs/func_800759D0.s:109-112 (`$s7 = arg3*12 + 0xC`), applied :150
  (`addu $v0,$a2,$s7`, `sw 0x18($sp)`); loop 2 :228-234 (`(arg3*2 + arg3)*4 + 0xC`, then
  `sw 0x18($sp)`); loop 3 :274-277 (`$s6 = arg3*12 + 0xC`), applied :298-299. So on the
  sheets loops 1-3 draw, the code reads SprtHdrA records at +0, +12 and +24 (header 2
  occupies +24..+35).
- **Each addressed header is read as a SprtHdrA.** Loop 2 reads the moved header's `count`
  at +2 (:243 `lbu $v0,0x2($a0)`, $a0 = sp18); the head reads header 0's `count` (:43).
  func_8007352C (src/text1b.c:10945-10987) reads `.header` as SprtHdrA (text1b.c:10929:
  count +2, cx/cy +4/+6, ubase +8, vbase +10) and func_8006E480 reads header 0 at the head
  and tail (:55, :335, :355).
- **Cells are 8-byte SprtEntA records walked from `.table`.** func_8007352C walks
  `(SprtEntA *)env->table + i` for i < count (text1b.c:10955-10957; SprtEntA text1b.c:10939,
  8 bytes). This function advances `.table` by `count * 8` (:45 and :245, `sll 3`).
- **Other readers of the same records** (not writes under judgment): func_8007636C's loops
  over root+0x14 (via its pick list) and root+0x20/+0x24/+0x28 apply the same highlight step
  (text1b.c:12212-12213, 12257-12258) and read the moved header's count (12216, 12261);
  func_800753D8 draws root+0x14[0] from header 0 only and advances by `count << 3`
  (text1b.c:11861-11869), the same one-header use the head makes of that record.
- **Assumed layout.** Head: the table[0] page sheet is header 0 then cells (no header past 0
  is ever addressed on it, here or in func_800753D8), so the first cell is at +0xC.
  Loops 1-3: the sheet is headers 0, 1, 2 (normal, then one highlight per player) then
  cells, so the first cell is at +0x24. In both cases BASE + K is the sheet's first SprtEntA
  cell: K = 12 x (headers the code addresses on that sheet).

### (2) Every normal path reaches the first cell (census, every value each index can take)
Value sets, from every writer of each index (all writers are C in src/text1b.c):
- Head: table = root+0x14, fixed index [0].
- Loop 1: `entry = D_8009BCF8[i].unk0`, i in [arg1*10, arg1*10+9]. arg1 = work+0x68+player,
  which is only ever 0 or 1 (init :12540 `= t0`; func_80075670 :11912-11914 `+1` then `&= 1`,
  :11926 `(x+1) & 1`), so i in 0..19. D_8009BCF8 is read-only (no store in src/ or
  asm/funcs/; only readers func_800759D0, func_80075F80, func_80076D74) and its 20 unk0
  bytes in the EXE are a permutation of 0..19. So table[entry+1] is [1..20].
- Loop 2: `arg2[i]`, i in [0, f3C]. arg2 = rows[player] (work+0x6A+player*10, 5 s16). Its
  only writers: func_800770B8 :12546 (-1), func_80075F80 :12023 (-1), :12103 (`entry`, a
  D_8009BCF8 grid byte, index `(f1C*5+f20)*2 + arg1*20` <= 38, so 0..19) and :12127 (0x14).
  f3C in state 2 is 0..f65+2 <= 4 (entered as f38 via func_8007526C :11762/:11771, f38 in
  {0, f65+2}; func_80075F80 :12109-12113 caps it at f65+2; :12024 decrements only from
  != 0), so i stays inside the 5-entry row. The guard `arg2[i] >= 0` drops -1. So
  table[arg2[i]+1] is [1..20] or [21].
- Loop 3: table = root[0x20 + f65*4], i < f65+3. f65 is written only at :12600 (0) and in
  func_800747D8 :11448-11468 (cycles 0..f64); f64 <= 2 (:12594-12595). So f65 in {0,1,2}.

| site (K) | records reached | result |
|---|---|---|
| head (0xC) | root+0x14[0] 0xD90 (1 header, count 2, ends 0xDAC) | +0xC = first cell |
| loop 1 (0x24) | root+0x14[1..20] | all 3 headers; +0x24 = first cell (20/20) |
| loop 2 (0x24) | root+0x14[1..20] | all 3 headers; +0x24 = first cell (20/20) |
| loop 2 (0x24) | root+0x14[21] 0xC84 (arg2[i] == 0x14) | OUTSIDE the record: the anomaly below |
| loop 3 (0x24) | root+0x20 [0..2], +0x24 [0..3], +0x28 [0..4] | all 3 headers; +0x24 = first cell (12/12) |
No reachable write lands INSIDE a record on anything other than its first cell.

### (3) The one anomaly path (documented; a latent bug in the original)
- **Trigger.** The cursor of a player in select state 2 sits on a cell that is not
  selectable (D_8009BCE4[entry] bit 0 clear) or that this player already took (bit 4<<p).
  func_80075F80 then stores the placeholder `arg2[f3C] = 0x14` (text1b.c:12127). In the same
  frame func_80077374 case 2 (text1b.c:12649-12655) calls func_800759D0, whose loop 2 draws
  slot i == f3C: `s.sp18 = table[0x14 + 1]`, `cells = s.sp18 + 0x24`. (After a confirm the
  cursor stays on the character just taken, so the next frame already takes this path.)
- **Measured bytes (disc/TIM2D/D_SEL.BIN).** root+0x14 table at 0x280; entry [21] (0x2D4)
  = 0xC84. The record at 0xC84: header `0a000200 2000e001 b000f400` (count 2, CLUT
  (32,480)), then cells 0xC90 `62002d00 00003f0c` and 0xC98 `a1002d00 0000240c`; it ends at
  0xCA0. BASE + K = 0xC84 + 0x24 = **0xCA8**, 8 bytes past the end. 0xCA8 holds `d000b400`,
  the ubase/vbase word of the 12-byte header-shaped record at 0xCA0 (`1a000200 1000f901
  d000b400`), the first of a run no table in the file points to (no u32 in 0x0-0x1000
  equals 0xCA0/0xCAC/0xCB8/0xCC4).
- **Why it is malformed under the assumed layout.** Loop 2 imposes the three-header layout
  on every sheet it draws, but the code's own other reader of this record, func_80075830
  (text1b.c:11960-11962: `*(tbl + 0x54)` = [21], `.table = +0xC`), draws [21] as ONE header
  followed by cells, which matches the data. Under the three-header view the headers would
  be 0xC84/0xC90/0xC9C, but 0xC90 and 0xC98 are cells, and the "cell array" at +0x24 starts
  past the record. The highlight step on this path moves sp18 to 0xC90 (arg3 0: cell 0 read
  as a header, count byte 0x2D) or 0xC9C (arg3 1: count byte 0x24), so func_8007352C also
  walks 45 or 36 "cells" from 0xCA8 + count*8 (0xE10 / 0xDC8), all outside the record.
- **Classification.** BASE + K lies outside BASE's record (past its end), as the original
  data lays it out: an anomaly under (b′)(3), not a second meaning. It is the only such
  path (census above). func_8007636C is not affected: its pick lists hold confirmed entries
  only (its ledger, 71b14499d).

### (4) The comment
The `cells` declaration comment states what the code assumes ("The code treats each sheet
it draws here as ...", "loops 1-3 treat their sheets as three headers"), names the
placeholder path and where it runs past the sheet, and points here. It makes no claim about
all the data.

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
