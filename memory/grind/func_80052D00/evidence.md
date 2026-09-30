# func_80052D00 — evidence

## Manual session 2026-09-25 (first real C; ledger previously held only a placeholder)

Function: XZ grid walk (DDA) over the 32x32 grid of 2000-unit cells used by
func_80053E9C / func_80053754 (the per-cell tests stored at work+0x5C by
func_80053304 / func_8005344C / func_80053584 / func_80053614). Work area =
`Work_80053E9C` (the struct already on main for func_80053E9C), with its
`u8 unk60[0x48]` blob split into the members this function uses
(0x60..0x90) and `unk5C` typed as the per-cell test pointer.

Floor trail (sandbox --disable all, --candidate):
- first hand-written body: 30 (387/385)
- `W->unk74 > W->unk70` (operand order of the swap test; target loads +0x74 first): 11
- slope statement moved after the +0x90 count, `-=` before the `+0x78` store: 0 (385/385)
- `*(s32 *)&cell` compare replaced by a `Cell_80052D00` union (s16 x/z + s32 w): still 0
- if/else for all three direction flags (xdir/zdir/swapped): still 0

Sandbox 0 but full-build SHA1 mismatched on the first landing attempt: 2 words,
the two in-loop `bnez v0` hit-breaks (0x800530FC, 0x800531E0) branched to the
func_80053694 call instead of the post-loop `W->unk80 == 0` retest. Sandbox masks
branch displacement, so this was invisible to the score. Diagnosed with -da dumps
(tools/grinder/dump.ps1): the retarget happens in jump.c `thread_jumps` (run
before cse, toplev.c:2861). `W->unk80 = f(); if (W->unk80 != 0) break;` expands to
a reload of D_800A33F4 + a load of +0x80 + compare, pattern-identical to the
post-loop test, so thread_jumps proves the post-loop test and redirects. Spelling
the break as `if ((W->unk80 = f(...)) != 0) break;` compares the call value, the
patterns differ, no threading: branch check (tmp/f52d00/brcheck.py, unmasked
function-relative targets vs asm/funcs/func_80052D00.s) = 0 diffs.

Landing (2026-09-25): spliced into src/text1b.c, typedef + `#define W` hoisted
above func_80052D00 (existing `#undef W` after func_80053E9C unchanged).
verify-oracle --rebuild --allow-dirty = 62efab4f73f992798c43e8c730aa43baa10bb4fa;
sandbox against the spliced src = 0 (385/385, 0 source-level, 0 operand-only,
33 not-scored branch-displacement hunks).

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 1ea98419d landing FAILed the 2026-09-29 retro-audit: union word view `Cell_80052D00 { struct {s16 x, z;} c; s32 w; }` landed 2026-09-25, before Q33, and outside Q33 scope (a member of a TU-local typedef reached through `#define W ((Work_80053E9C *)D_800A33F4)`). Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_80052D00);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. LEFT IN PLACE: typedef Cell_80052D00 and Work_80053E9C (members unk88/unk8C), the `#define W` view, and the D_800A33F4 / func_80053694 externs. Work_80053E9C is still used by func_80053754 and func_80053E9C; after this reopen nothing reads unk88/unk8C through the union.

## 2026-09-30 -- laneC, ledger-only (src/text1b.c is peer-reserved): READY TO LAND, no union

The retro-audit objection was the `Cell_80052D00` union word view (`.w` compared as one word).
It is not needed. `W->unk88.x == W->unk8C.x && W->unk88.z == W->unk8C.z` (and the `!=`/`||`
form for the post-loop test) compiles to the target's single `lw`/`lw` compare: fold-const.c
fold_truthop merges two comparisons of adjacent fields into one wider field reference
(tools/gcc-2.7.2/fold-const.c:2973-2985, "If both pairs of fields being compared are adjacent,
we may be able to make a wider field containing them both"). Target word loads it produces:
0x80052DDC `lw $a1,0x88($t1)` / 0x80052E00 `lw $a0,0x8C($t1)` (first test) and 0x800532A8
`lw $v1,0x88($a2)` / 0x800532AC `lw $v0,0x8C($a2)` (post-loop test).

With no word view left, Cell_80052D00 becomes a plain `struct { s16 x; s16 z; }` (the halves'
own accesses: lh/lhu/sh at +0x88/+0x8A and +0x8C/+0x8E, x and z passed as the two arguments of
the per-cell test at +0x5C). No other function names unk88/unk8C or Cell_80052D00 (grep of src/,
include/).

Measured (scratch full builds of HEAD 231895b18 with tmp/c8dc/mktree.sh + buildtree.sh, i.e.
engine.pipeline.build_all() in an exported tree; the unmodified export builds the oracle):
- retro-audit body verbatim (union, `.w`): sandbox 0 (385/385), full build == oracle.
- same body, field compares instead of `.w` (union typedef unchanged): sandbox 0, full build == oracle.
- landing form = candidate.c (plain struct typedef + field compares, `.x`/`.z` accesses):
  full build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
Everything else in the body is the retro-audit body, which that audit found ordinary
(xdir/zdir/swapped per-branch constants, the `(W->unk80 = f()) != 0` break, the struct split).

Landing: landing.patch (this directory) against src/text1b.c at 231895b18: two hunks, the
Cell_80052D00 typedef (union -> plain struct) and the INCLUDE_ASM line -> candidate.c. Nothing
else changes (no header, no symbol file).

## 2026-09-30 -- LANDED COMPLETED-C (0eb256779, queue a1b2eccaa)
Layer-2 PASS, reviewer l2-80052D00-r1, round 1, body_hash 25ebf3eade31d99c, scope match
(memory/grind/func_80052D00/layer2.jsonl). Landed text = candidate.c + the Cell_80052D00 plain-struct
typedef (landing.patch). Reviewer: fold_truthop word compare confirmed by probe (plain field
compares give lw 136/140 + xor); W casts the value of a pointer-holding global, not a pun;
per-branch flags all read; the assignment-in-condition break is a real store. Commit-message
wording fix before commit ("raised no objection to (independently re-reviewed at layer-2)").
Rebuild SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa; check_completion_integrity OK.
