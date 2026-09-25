# func_80073C78 — evidence (manual lane, 2026-09-25)

Rotated POLY_FT4 sprite-sheet drawer: sibling of func_80073728 (text1b.c, COMPLETED-C) —
same EnvF / Ft4Sheet / Ft4Cell / POLY_FT4 types; adds a bounding-box pass (min/max of the
scaled cells -> centre cx/cy), then per cell RotMatrix(angle) + ScaleMatrixL(D_8009BCD4 =
VECTOR {0x1000,0x800,0x1000}) + SetRotMatrix, 4x ApplyRotMatrix on the corner offsets
(negated per mode 1/3 = x flip, 2/3 = y flip), then the sibling's UV / shade / semitrans /
AddPrim tail.

Floor: 362 (INCLUDE_ASM) -> 55 (first spelling) -> **2** (honest `+` form = candidate.c).
The `|` spelling reached 0 / SHA1 == oracle but was **FAILED by layer-2 (2026-09-25)**:
banked as rejected/ior-spelling-patch-workaround.c (reviewer reasoning in its header).
Status: owner question answered by ruling bcdc1648e; narrow condition adopted in 9bc64b751.
candidate.c now reaches 0 / oracle SHA1 as-is — see "Landing after the narrow PLUS->IOR
adoption" at the end.

## What closed it to 2 (all ordinary C)
- tpage/clut `u32` (target spills them with `sw`, clut with `andi 0xffff`).
- env->x / env->y read once into locals `x`/`y` before the 8 vertex stores (target loads
  +0x18/+0x1C once; the stores to p would otherwise force reloads).
- UV offsets exactly as the sibling: `du0 = 0; dv0 = 0;` before the loop,
  `du1 = e->w; dv1 = e->h;` at loop top (target spills du1/dv1 as `sh` to 0xC0/0xC8).
- first loop compares written `if (minx > e->x)` / `if (miny > e->y)` (minx sign-extend
  expanded before the load -> target sched order + reorg fill of the a3 giv increment).
- declaration order sets the spill-slot layout (slots allocated in pseudo order):
  `s16 i; u32 tpage; u32 clut; s16 du0,dv0; s16 du1,dv1; u16 ub,vb; Ft4Sheet *hdr;`
  then the stack arrays `vec[4][4] (0x10), ang[4] (0x30), out[4][4] (0x38), mtx[8] (0x78)`.
- K&R definition (text1b.c declares it unprototyped `extern s32 func_80073C78();`; a
  prototype with an s16 param is a hard error against that decl; angle is spilled `sh`).
  Precedent: func_800485EC (text1b.c), func_800433E4 (text1a_c.c).

## The last 2 insns: a compiler-identity artifact, PROVEN
Target `ori a1,v1,0x0` / `ori a0,v0,0x0` = (ior u du0) after reload substituted du0's
REG_EQUIV const 0. The `ior` comes from combine.c's PLUS->IOR rewrite (combine.c:3618,
"adding two things that have no bits in common"). The project's oracle compiler carries
tools/cc1-no-plus-to-ior.patch, which deletes exactly that rewrite (docs/ORACLE-COMPILER.md).

Proof (2026-09-25): built a research-only STOCK cc1 in ~/cc1stock (WSL home, never
installed) from the pinned upstream 43d1cdb6 with the documented recipe (all -O0 -g,
combine.o -O, -fgnu89-inline; simplify_rtx size 0x3a11 = stock) plus ONE host-ABI fix
(reorg.c `rtx negate_rtx ();` declaration — without it the 64-bit host truncates the
pointer and cc1 segfaults in fill_slots_from_thread on this very function). Compiling
text1b.c truncated after this function with the `+` form: stock vs oracle compiler differ
in exactly two lines in the whole TU, both in func_80073C78: `addu $5,$0,$3` -> `ori
$5,$3,0x0000` and `addu $4,$0,$2` -> `ori $4,$2,0x0000` = the target. Scripts:
tmp/f73c78/mkstock.sh, fixstock.sh, trunc.sh (tmp, gitignored).

So the natural `+` spelling (sibling-identical) IS the original source under stock GCC;
it is unreachable under the patched oracle compiler except by spelling the operator `|`.
This is new evidence for docs/ORACLE-COMPILER.md's OPEN QUESTION ("did the original
compiler perform this conversion?"): here the shipped binary contains the conversion.

## Ruled out for `+` under the oracle compiler
du0/dv0 types (s16/u16/u8/s8/s32/u32), u/v types, init forms, s32/u8 intermediate UV
locals, chain assignments, du0=0 inside the loop, inline helper: none can produce `ori`
because the patched combine never rewrites PLUS to IOR (no other IOR source applies).

## Stock-cc1 reproduction recipe (research only; never installed)
1. `bash tools/wsl.sh 'bash tmp/f73c78/mkstock.sh'` — copies tools/gcc-2.7.2 to ~/cc1stock,
   `git checkout -- .` (pristine upstream 43d1cdb67ed1...), recipe build: `make combine.o
   CFLAGS="-O -g -fgnu89-inline"` then `make cc1 CFLAGS="-g -fgnu89-inline"`; no patch.
   simplify_rtx size 0x3a11 (= stock; oracle is 0x39ab).
2. `tmp/f73c78/fixstock.sh` — adds `rtx negate_rtx ();` after `#include "insn-attr.h"` in
   ~/cc1stock/reorg.c and relinks. Without it the 64-bit host truncates negate_rtx's
   implicit-int return and cc1 segfaults in reorg.c fill_slots_from_thread (the
   `addiu a3,a3,-8` reverse-adjust path) on this function.
3. `tmp/f73c78/trunc.sh` — text1b.c with candidate.c spliced, truncated after this
   function; cpp once, both compilers on the same .i. Entire diff (oracle < > stock):
   ```
   28406c28406
   < 	addu	$5,$0,$3
   ---
   > 	ori	$5,$3,0x0000
   28411c28411
   < 	addu	$4,$0,$2
   ---
   > 	ori	$4,$2,0x0000
   ```
   Both lines are in func_80073C78 and equal the target (`ori a1,v1,0x0` / `ori a0,v0,0x0`).

## Bounded check for another honest spelling (2026-09-25, after the FAIL)
None exists under the oracle compiler. The patch deletes combine.c's only PLUS->IOR
rewrite; the remaining IOR producers in combine.c (bit-field insertion, (a&b)|(a&c)
distribution, De Morgan) need an IOR/AND already in the source. `ori rX,rY,0` needs an
IOR whose second operand reload rewrites to 0, so it needs `|` in the C. `^` gives xori,
and `+` gives addu/addiu (the `+` variants in the ruled-out list below all measured addu).

## Landing after the narrow PLUS->IOR adoption (2026-09-25)
Owner ruling bcdc1648e (F) + adoption 9bc64b751: the build compiler now keeps combine.c's
PLUS->IOR rewrite except for (plus REG CONST_INT). candidate.c (the `+` body, unchanged)
spliced into src/text1b.c: sandbox --disable all = 0/362 (15 not-scored branch-target
hunks only); verify-oracle --rebuild = 62efab4f73f992798c43e8c730aa43baa10bb4fa.
Self-check measurement (tmp/f73c78_selfcheck.sh, all 34 TUs, spliced tree):
narrow(aa04d761) vs retired no-rewrite(0f438e42) differ in {text1b} only — exactly the
two UV `addu`->`ori` lines; stock(~/cc1stock) vs no-rewrite differ in {ings text1b}.
tools/build_oracle_cc1.sh ORACLE_EXPECT_DIFF / STOCK_EXPECT_DIFF updated to match.
The `|` spelling (rejected/ior-spelling-patch-workaround.c) stays rejected.
