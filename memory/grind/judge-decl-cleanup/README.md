# judge-decl-cleanup: the retro-audit table-declaration cleanup (laneH, 2026-09-30)

Start here. This ledger covers the combined cheat-cleanup landing that started as "36 scalar `&Judge`
uses" (docs/audits/RETRO-AUDIT-2026-09-29.md § Follow-ups) and grew, through layer-2 rounds 1-5, into
truthful declarations for five tables, Rec44, and the practice-table reads in 36 functions.
The g_sqrt_table_u8 part is also summarized in pre-slim-2026-10-01:memory/grind/sqrt-table-decl-cleanup/evidence.md.

## What landed
Landing commit: 8e007927d (subject "cheat-cleanup: Judge, g_sqrt_table_u8, D_8008EB40, ...").
The full commit message is in git and in tmp/laneH/msg_final.txt at landing time.
- **include/code6cac.h:**
  - `extern s16 Judge[0x1000];` (sine table)
  - `extern u8 g_sqrt_table_u8[0x400];` (floor(8*sqrt(i)))
  - `extern s16 D_8008EB40[3][3];`
  - `extern s16 D_800A310C[4];`
  - `extern u8 D_8008DA08[0x48];`
  - `extern u8 D_800F5F68[];`
  - PracticeMenuRec gains `u16 unk_6A` (the same member as laneG's L1)
  - Rec44's h30..h3E become `s16 h30[2][4]`
  - Every scalar file-scope duplicate of these is removed.
- **Element/member access in 36 functions**, all sandbox 0, full build SHA1 == oracle:
  - Judge: 800283D0, 8003032C, 80030580, 80030D7C, 80032064, 800325E0, 8001BAE4, 80021DB0,
    800233AC, 80023648.
  - sqrt: 80018094, 80018300, 800187F4, 800274BC, 8002A458, 8002BC68, 8002BEA0, 8002D320,
    8002D518, 8002D780, 8002DAD0, 8002E838, 8002EA24, 8002EBDC, 8002F2D0, 8002F770, 80032314,
    800325E0, 8001A67C, 8001A820, 8001F2E4.
  - D_800A310C/D_8008DA08/D_8008EB40: 80023648, 800233AC.
  - Rec44: 8001A820, 8001B294, 8001B3C0, 8001B748.
  - Practice table: 8002BC68, 8002BEA0, 80032314, 8001F888, 8003CE18.
  - D_800F6608 members: 8003C9A4.
- **Second handles removed:** judge_ptr (800233AC), new_var (800274BC).
- **Kept with FAKE paperwork:**
  - the D_8008EB40 row pointer `tbl` (800233AC, 80023648): eb40-pointer-alias.md;
  - the practice-table record pointers t2_base/t3_base (8002BC68, 8002BEA0):
    pre-slim-2026-10-01:memory/grind/func_8002BC68/q73-practice-reads.md;
  - `v1_v` in 80032314 (named intermediate): pre-slim-2026-10-01:memory/grind/func_80032314/v1v-named-intermediate.md.
- **Symbol rows:** D_80101FA0/FA8/801023EC/23F4 retired (their only reader was 8002BC68).
  D_80101FBC/FC4/80102408/10 stay declared, because func_8001C8DC, func_8001E878 and
  func_8001EA84 (HEAD bodies, outside this landing) still read them.
- **Restored from history**, because re-certified bodies cite them: hypotheses.md of 80023648,
  800283D0, 80030580, 8002D320, 8002D518 and 80032314; 8002D518/self_vet.md.
- **Banked or regenerated scratch evidence:** 80032314/allocdbg-2026-09-30.txt;
  80018094/tmp-evidence/; 8002D518/s8-dumps/.

## Measurement method
- evidence.md: the Judge object, plus the 33-TU object-identical harness (tmp/laneH/harness.py,
  edits.py), with a negative control.
- Rounds 3+ change relocation symbols, so their proof is the full-build SHA1 + sandbox.

## Q73 (practice-menu per-word reads): LAPSED
Owner ruling Q73 (rule commit 321a8529f) would have kept the eight per-word reads in
8002BC68/8002BEA0 only if every single-object spelling failed. One does not: record pointers set
first and reads through them (form (b)) score sandbox 0 with an oracle SHA1. Q73 therefore lapses
for both functions (its own text), and no per-word name is read there. Measurements:
pre-slim-2026-10-01:memory/grind/func_8002BC68/q73-practice-reads.md and q73/ (s1 18/12, s2 0, s3 0, s4 14 with +1
insn, s5 18/12).

## D_8008EB40 pointer alias
eb40-pointer-alias.md, with dumps/ (expand RTL: expr.c:5245 keeps the array base as a constant
term until after the row multiply).

## Disclosure scan
scan-2026-09-30/: scan.txt (590 hits), access.txt, disposition.tsv (one row per hit, plus manual_extra.tsv rows:
object or "not debt"),
manual.tsv (hand-classified rows), manual_extra.tsv, summarize.py (groups disposition.tsv by object), and the
self-contained scripts (span.py, scan.py, scan2.py, dispo.py). Re-run them from the repo root.

## Layer-2 history
Per-function layer2.jsonl rows:
- r1 rev-judge 7 PASS / 3 FAIL;
- r2 rev-tables 25/5;
- r3 rev-tables-r3 32/7;
- r4, r5, r6 and r7: message-only FAILs;
- final: r8 rev-tables-r8 PASS (36 functions, 2026-10-01).

## Not-certified debt (verbatim from the landing message)
NOT certified by this commit: remaining debt, out of scope.
The definitive list is every row of
memory/grind/judge-decl-cleanup/scan-2026-09-30/disposition.tsv that is not marked "not debt":
566 of the 590 hits from the systematic scan of all 36 changed bodies, plus 7 rows from
manual_extra.tsv for named debt the scan patterns cannot see (the D_800A37E8/EA/EC split vec3,
func_8003CE18's two-write `addr`, and the names still used outside the bodies). Each row has its function,
source line, construct and object. summarize.py groups them; scan.py / scan2.py / dispo.py re-run
it from that directory alone. Summary by object ([T] = an object this landing touches):

[T] PracticeMenuRec records (g_practice_menu_table, stride 0x44C), reached by byte offset through a
`u8 *` / `s32 *` parameter or local:
- func_8001A820: arg2/arg3 +0x6A (unk_6A) and +0xB8 (unk_B8, as CamVec); arg0/arg1 read as
  `((s32 *)argN)[0..2]` / `*(CamVec *)arg0` (the record's unk_168 vector).
- func_8001B294: a0/a1 +0xF4/+0xF8/+0xFC. func_8001B3C0: a0 +0x180/+0x184/+0x188.
- func_8001B748: `base = (u8 *)&D_80101EC8 + D_800A3748 * 0x44C`, read at +0x184/+0x19C/+0x1A8.
- func_8001F2E4: obj and its unk_00 record, at +0xC..+0x26E.
- func_800233AC: arg0 at +0x2C/+0x98/+0xB8..+0xC0/+0x1D8.
- func_80023648: arg0 at +6..+0x1D8.
- func_800283D0: arg0 and its unk_00 record `temp_s4` (+4/+0xC/+0xE/+0x6A/+0x8C/+0x1CA/+0x286/
  +0x288 + idx*2, and `tail`).
- func_8002A458: obj (+4/+0xF4..+0xFC/+0x26C) and its unk_00 record (+0x1D8).
- func_80030580: src at +4/+0x1A/+0xF4..+0xFC/+0x1CA.
- func_80032064: src at +4/+0x1A/+0xB2/+0xBC/+0xF4/+0xFC/+0x1CA.
- D_80101EC8, the byte-base name, is still used by other functions' byte-offset walks.
[T] Rec44:
- func_8001A820: `func_8001A538((s32 *)cam, …)` (3 calls).
- func_800325E0: `D_800A36B4` is declared `s32` but holds a Rec44 pointer. It is read at
  +0x12/+0x20/+0x24/+0x28.
- func_8003C9A4: `func_80046BF4((s16 *)a0, …)` under code6cac_c2.c's own prototype.
- func_8001BAE4:1077: `func_8001B748(&D_800F6608, arg0, arg1, (s32 *)arg2, …)` passes `s32 *`
  arguments into Rec1C * parameters and casts the int arg2 to a pointer.
- func_8001B748: only its h30..h3C stores are respelled; its other accesses are unchanged.
[T] D_800F5F68 (0x1B8-byte records): func_8002A458 `rec = &D_800F5F68[id * 0x1B8]`, read at
  +0/+0xC..+0x12.
Rec1C: func_8001BAE4 reads its `s32 *` arg0/arg1 at +4/+8 as s16.
Call-boundary `(s32 *)` casts of typed locals:
- func_80021DB0 nrm (4);
- func_800233AC out1 (3, plus one `*(SVec8_233AC *)out1` view);
- func_80030D7C nrm;
- func_8001A820 scr->nrm / scr->eye / scr->head.
func_8002F2D0: `((s16 *)a1)[0..2]` (an s16 triple held in an `s32 *` parameter) and
  `(MATRIX *)a0`.
D_80106A78 (0x64-byte records): func_80030580 and func_80030D7C `obj = (u8 *)&D_80106A78`.
func_8003032C: its `s32 *a0` is read at +0x44/+0x48/+0x4C. Nothing calls it directly, so its object
  is untraced; the offsets match the D_80106A78 velocity fields.
D_80104E88 (0x2C-byte records): func_80032064 (ptr/s0) and func_80032314 (t0/a3).
D_800A37E8/EA/EC: a split s16 vec3 written as three scalars (func_8002A458).
Scratchpad (0x1F80xxxx) views:
- func_80018300;
- func_8001A820's CamScratch view of 0x1F800000;
- func_8002A458 (scr 0x1F8002B8, pos 0x1F8000A8);
- func_8002D320 / D780 / DAD0 / E838 / EA24 (obj = the 0x1F8002B8 block their callers pass);
- func_8002EBDC / F2D0 / F770;
- func_80030D7C.
Model/geometry data: func_80018094, func_80018300 and func_800187F4.
Other: func_80021DB0 `(s16 *)stage_GetDataPtr()`; func_8001F2E4's a/b are the caller's stack
  buffers (func_80023F08 sp+0x18 / sp+0x9C), read at +0xC..+0x7E.
func_8003CE18's `addr` keeps its two writes (record 0, then record 1 if D_800A3748 == 0).
