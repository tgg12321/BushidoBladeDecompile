# func_8007636C evidence

## 2026-09-24 manual session — CLOSED at sandbox 0 (348/348)

candidate.c is the body landed in src/text1b.c (spliced after the
SelWork_800768DC typedef/macro; SelWork gains `u16 f34` at 0x34).
Oracle SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa with --rebuild.

Constructs: ordinary C plus ONE FAKE-annotated constant-holder (`s32 mode = 0`,
func_8006E480's second argument) under named-local-fake-exception; lever
exhaustion in hypotheses.md. Scripts/variants: tmp/e636/ (gen*.py sweeps,
dump.sh RTL dumps).

## Ruling 9 re-audit (2026-09-25, manual lane)

Directive: ordinary-c-judge-decidable.md Ruling 9 (c3e7a0b9e) "func_8007636C directive":
re-audit the landed function-scope `q` under Ruling 9 with it renamed, not reverted. The
rename is `q` -> `cells` (byte-neutral: sandbox 0/348, verify-oracle --rebuild SHA1 == oracle).
The sites: head `cells = s.sp18 + 0xC` (root+0x30 [12]); loop A `+0x24` (root+0x14
`[arg2[i] + 1]`); loop B `+0x24` (root+0x30 `[idx*2]`) then `+0xC` (`[idx*2 + 1]`), two sites in
one loop body; loop C `+0x24` (root+0x20+f65*4 `[i]`). Each is read once by `s.sp1C = cells;`.

**(b) what +0xC and +0x24 reach.** The record and layout evidence is shared with the case-2
sibling: memory/grind/func_800759D0/evidence.md "Ruling 9 (b)" (committed 23045f51f). In brief:
`s.sp18`/`s.sp1C` are the `.header`/`.table` of func_8007352C's descriptor (text1b.c:10945),
which reads a 12-byte SprtHdrA and walks 8-byte SprtEntA cells. A sheet is N headers followed
by its cells. Header stride 12 is in this function's own bytes (`s.sp18 + 12 + arg3 * 12`,
asm/funcs/func_8007636C.s:93/168/275 `addiu $sX,$v0,0xC`), and cell stride 8 in
`s.sp1C += count * 8`. The file is disc/TIM2D/D_SEL.BIN. Census (tmp/f759d0/sheet_census.py)
for every table this function indexes, with the index ranges the code can reach:
| site | table slots reached | header count | K |
|---|---|---|---|
| head | root+0x30 [12] | 1 | 0xC |
| loop A | root+0x14 [1..20]: arg2[i] holds confirmed picks only in cases 3-5 (func_80075F80 sets the state to 3 right after the (f65+3)th confirm writes `arg2[f3C] = entry`, a D_8009BCF8 byte 0..19; func_800768DC never writes arg2) | 3 each | 0x24 |
| loop B, 1st | root+0x30 [0,2,..,10]: idx is a slot number 0..5 (func_80075F80 fills `slots[arg3][i] = i`, i < f60, and slot 4 = 5) | 3 each | 0x24 |
| loop B, 2nd | root+0x30 [1,3,..,11] | 1 each | 0xC |
| loop C | root+0x20/+0x24/+0x28 [0..f65+2] (3/4/5 sheets, exactly the loop bound) | 3 each | 0x24 |
K = 12 x (header count) at every site, so every write reaches the same sub-object: the first
SprtEntA cell of the sheet's cell array, stored to `.table` and walked by func_8007352C. The
+0x30 table is laid out as 6 (3-header, 1-header) pairs plus [12], which matches the
`idx*2` / `idx*2 + 1` indexing.

**Prong walk (src/text1b.c func_8007636C, renamed body).**
- (a) all 5 writes feed `s.sp1C = cells;` (identical text), the descriptor's `.table` member.
- (b) every write is `cells = s.sp18 + K;`, K in {0xC, 0x24}. BASE is `s.sp18`, member of the
  local `s`, just assigned the sheet address at each site (loops A/B/C write before the
  highlight step moves sp18 to header 1/2). Sub-object: see above.
- (c) head: adjacent, in the `f14 < 4` then-block. Loop A: the for body, with the highlight
  if between. Loop B 1st: the for body, with the highlight if and `s.sp30 =` between. Loop B
  2nd: the for body, adjacent. Loop C: the for body, with the highlight if between. No
  return/break/continue/goto between. `cells` has no other reader.
- (d) target adds: 800763EC `addiu a2,v1,0xC`; 8007651C `addiu a2,a0,0x24`; 80076680 `addiu
  a2,a1,0x24`; 800766F8 `addiu a2,v0,0xC`; 800767B8 `addiu a2,a0,0x24`.
- (e) every consumer store is followed in its own block, unconditionally, by
  `arg0[4] = func_8007352C((s32)&s);` before the next consumer store or the exit. This holds
  for both loop-B sites.
- (f) `cells`: true of all five writes.
- (g) same statement list as the per-site spelling rejected/per-site-locals-6.c (diff:
  declarations and identifiers only). Every write is consumed before the next. `s.sp18` is
  re-written before every write, so no write re-stores a held value. No computation is split.
- (h) one declaration at function scope, the innermost scope enclosing all five writes. No
  other declaration moved.
- (i) receipts, measured 2026-09-25 (the 2026-09-24 session adopted `q` without them):
  - per-site cells0..cells4 = 6/348, the same whether function-scope or block-local
    (rejected/per-site-locals-6.c). Hunks: only the two +0xC sites, target
    `addiu a2,vX,12; sw vX,0x18(sp); sw a2,0x1C(sp)`, ours `addiu vX,vX,12; sw vX,0x1C(sp)`.
  - structural respellings at both +0xC sites, all 6/348 (tmp/e636/r9_ladder.py): no local;
    block-scoped initialized local; sheet-pointer local; sum from the table value before the
    sp18 store; sp1C store moved ahead of `s.sp40 = 0` (loop B) or `s.sp40`/`s.sp30` (head);
    `u8 *` pointer arithmetic.
  - allocation dump (tmp/e636/rtl.py, instrumented cc1 -da, tmp/e636/rtl/{shared,persite}).
    Shared: `cells` is ONE pseudo (78), allocated by global.c to $6/$a2 at all five sites.
    Hard conflicts {v0, v1, a0, sp}. Per-site: the +0x24 pseudos (79/80/82) span blocks
    (computed before the highlight if, stored after), go to global.c and get $a2 as in the
    target. The two +0xC pseudos (78 head, 81 loop B) are single-block, so local-alloc ties
    each to the dying sheet value ($v1/$v0). The target's $a2 at the +0xC sites needs the one
    multi-block pseudo. This is the same mechanism as func_800759D0 (evidence.md
    "Allocation provenance").
  - permuter from the carrier-free chassis (r9_persite.c, tmp/perm_636r9, 3 workers): 9,716 iterations (615 s), base 410 (permuter-weighted) -> best 215, no
    zero. Every find below base either reuses a +0xC local (cells0/cells3) as a cross-block
    carrier for an unrelated value (header pointer, `i`, `count*8`, `table[12]`, `arg3`, the OT
    base, an rsin argument), which is banned multi-role reuse, or narrows its type (`char`/`u16`,
    which truncates a pointer). They confirm that the target needs the +0xC sums multi-block and
    offer no legal per-site form. Stopped and harvested; tmp/e636/permdiff.py lists the finds.

## Earlier (Codex, 2026-09-24)

Fresh m2c 248/348; descriptor aggregate 184/348 (superseded).
