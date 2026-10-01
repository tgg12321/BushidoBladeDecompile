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

## laneA manual 2026-10-01 — table model, 0 (manual-2026-10-01/scores.txt)
Data model: PracticeMenuRec gains `LeafPos unk_210[3]` and `LeafPos unk_234[2]` (func_8002C61C copies
SPAD->unk00[k][0..2] / SPAD->unk48[k][0..1] there; LeafPos moved above the struct) and `u8 unk_AD`
(func_8002C61C's flag pair, target lbu/sb). The per-word externs D_801020D8..D_80102110 (include/code6cac.h,
src/code6cac_b_tu2.c, src/code6cac_b.c), D_80101F75 / D_801023C1 and their undefined_syms_auto.txt rows
(plus D_80102314) retire: after the landing no INCLUDE_ASM .s names them (asm/6CAC.s is not built).
func_8002C22C: record 0 through g_practice_menu_table[0], record 1 through `rec1` (pointer-alias FAKE:
direct 26, one table-base alias 13), scratch reads as SPAD members, the 0x1F8002B8 record as an s32 view.
The 2026-09-22 doc note 1 (u8 *scr needed against MEM_IN_STRUCT_P scheduling) no longer holds: with
SPAD member reads the s32-indexed and u8-displaced spellings both score 0 (p1s_s32 / p1s), so the
cast-free one lands. func_8002C61C respelled in the same landing (it held the last C uses of the
retired externs): s1/s0 FAKE aliases, per-site measurements in scores.txt.

## LANDED 2026-10-01 — COMPLETED-C (Match 18cb49eea, queue 1afc6ce98)
Layer-2 round 1 FAIL rev-C22C (body 0dd69c0386ac246b): prong (c) — 15 named_syms.txt census rows over the
merged addresses (g_char_vec3_*, g_practice_menu_table_p2, g_char_p1/p2_field_AD) were left; retired, message
fixed. Round 2 PASS rev-C22C-r2 on the same body (no surviving alias rows/addresses; rec1 pointer-alias FAKE
complete; data-model prongs a/b/d hold). func_8002C61C's respelled body: PASS rev-C61C (6a997acd5282edc7,
cheat-cleanup). Non-blocking notes left for later: C61C annotation wording / s1,s0 names; `(u16)...unk_6A`
on the s16 member is a header type-correction candidate. Not changed: func_80029454 (L4) and func_80021A98
(L1) still walk +0x210..+0x248 / +0xAD by byte offset (memory/grind/func_80021424/HANDOFF.md series).
check_completion_integrity OK; SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
