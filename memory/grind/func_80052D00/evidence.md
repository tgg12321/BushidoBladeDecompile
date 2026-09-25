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
