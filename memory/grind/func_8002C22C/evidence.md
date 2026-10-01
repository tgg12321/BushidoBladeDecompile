# func_8002C22C ledger

(The per-session ledger of the 2026-09-22 landing 8a3843d5e was deleted at completion; tombstone:
memory/grind/_completed/func_8002C22C.json.)

## ff-b 2026-09-30 — retro-audit FAIL (class A), reopened per owner Q37
Finding (tmp/audit-2026-09-29/review/batch_00.md): `extern s32 D_80102314; s32 *d_tbl =
&D_80102314; ... d_tbl[0x234/4] .. d_tbl[0x248/4]` — an unannotated C pointer alias, load-bearing, that
indexes up to +0x248 past a scalar splat symbol to reach record 1 of the 2 x 0x44C table based at
D_80101EC8 (0x80102314 = D_80101EC8 + 0x44C), while record 0 in the same function is read through
per-word scalars (D_801020D8.., D_801020FC..). Split-scalars-hide-aggregate; the audit names the table
declaration as the honest fix.

The target (asm/funcs/func_8002C22C.s:5-6) holds record 1's base (%hi/%lo 0x80102314) in $t1 and
reads the second block's fields at displacements off it; record 0's fields are absolute.

Spellings of the record-1 reads (sandbox --disable all, target 252 insns; ff-b-2026-09-30/):
| spelling | score | insns | admissible? |
|---|---|---|---|
| landed `s32 *d_tbl = &D_80102314; d_tbl[off/4]` | 0 | 252 | no: indexes past a scalar, unannotated |
| `(&D_80102314)[off/4]` | 26 | 262 | no: same indexing, and no match |
| `*(s32 *)((u8 *)&D_80102314 + off)` | 26 | 262 | no: same |
| `*(s32 *)((u8 *)&D_80101EC8 + 0x44C + off)` | 26 | 262 | no: cast past the u8 table-base scalar |
| `u8 *rec1 = (u8 *)&D_80101EC8 + 0x44C; *(s32 *)(rec1 + off)` | 0 | 252 | no: the `((s32*)&D_80101EC8)[i]` cast-on-scalar family banned in pre-slim-2026-10-01:docs/grind/decisions.md:83,91 |
| `s32 *rec1 = (s32 *)((u8 *)&D_80101EC8 + 0x44C); rec1[off/4]` | 0 | 252 | no: same |
Every form that reaches 0 reaches record 1 through arithmetic past a symbol declared as a scalar. The
honest spelling needs the table declared as what it is: a 2-element array of a 0x44C-byte record at
D_80101EC8. That merge covers 89 per-word names inside 0x80101EC8..0x80102760 used from 9 files
(include/code6cac.h, include/m2c_context.h, src/code6cac*.c, src/text1b.c; tmp/ff-b/tblsyms.py) and is
an aggregate-merge package in its own right, outside a fix-forward.

Q37 applied: landed body banked verbatim in rejected/retro-audit-2026-09-30.c (its doc comment's two
load-bearing codegen notes — `u8 *scr` displaced casts vs MEM_IN_STRUCT_P, and cse's find_best_addr
folding only the first block — stay valid for any future body); src back to
`INCLUDE_ASM("asm/funcs", func_8002C22C);` with a forward prototype for the call in func_8002C61C
(same TU); the now-unused `extern s32 D_80102314;` removed (the symbol stays in undefined_syms_auto.txt for
the assembly). Rebuild SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
Next worker: start from the table declaration (record struct, 0x44C stride), not from the alias.
