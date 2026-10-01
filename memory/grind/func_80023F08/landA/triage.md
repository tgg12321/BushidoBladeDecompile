# Checklist 1001c item 4 triage for the func_80023F08 data model (laneC 2026-10-01)
scan.txt: every raw `*(T *)(p + 0xNN)` in src/ at an offset this data model newly types in PracticeMenuRec (342 sites;
handles unresolved). scan_nonagree_ctx.txt: the WIDTH / SIGN rows outside the scratchpad `scr` handle, with their function.
Triage of the non-AGREE rows by handle:
- Not a PracticeMenuRec: scratchpad record `scr` (code6cac_b_tu2.c, 117); func_8002CD58 / func_8002D780 / func_8002DAD0 /
  func_8002E838 `obj` (pointer fields at +0x60/+0x64/+0x68, s32 pairs at +0xA8/+0xAC: a different record);
  func_80040D48 s1..s4 (vectors &rec->unk_F4 / &rec->unk_1C8 and their callees' data); func_80049718 obj/part;
  game_GetPlayerData data (func_80027A58 v0); D_800A34FC / D_800A35A8 / D_800A33F4 tables; func_80037F40 `bp`
  (save-file records); gpu / spu / text1b_tu1c tile code; func_8001F2E4 `a`/`b` (MotionFrame: +0x26 is unk_0C[13], u16 —
  agrees); func_8003F824 scene; func_80017FA0 p68.
- PracticeMenuRec handles among the non-AGREE rows (two):
  * func_80039680 `a0` (src/code6cac_c_mid.c:1375): `*(u16 *)(a0 + 0x68)` copied into an s16 field — an lhu copy of
    the s16 member unk_68, which is how GCC copies an s16; agrees in width.
  * func_800233AC `arg0` (src/code6cac_tu2.c:4117; the same function reads arg0 + 0x1D8, a PracticeMenuRec):
    `*(SVec8_233AC *)(arg0 + 0x98)` stores a file-local struct of 4 x s16 — agrees with SVec4i16 unk_98 in width and
    signedness; the file-local type over the record is pre-existing debt (checklist 1001c item 3).
  No contradicting width or signedness found.
Disposition: no consumer contradicts the new types; the PracticeMenuRec raw-offset sites that agree (u8 *-typed bodies in
code6cac_tu2.c, code6cac_b_tu2.c, code6cac_c_mid.c, text1b.c, ...) stay as disclosed pre-existing debt (checklist 1001c
item 4; precedent b3843cc02 / rev-pmr-split) — the func_80021424 L1-L4 handle series (memory/grind/func_80021424/HANDOFF.md).
Original-access check (asm_width_check.py -> asm_width.txt): every absolute access in asm/funcs/*.s to records 0/1 at a
newly typed offset (`%lo(D_<0x80101EC8 + k*0x44C + off>)`, k = 0, 1) — 9 sites, all AGREE with the member width and
signedness. (Records "2/3" at 0x80102760+ are other globals: D_80102760, the PadState at 0x80102788.) The two
INCLUDE_ASM callees handed the record (func_800204C0, func_800207C8) touch none of the new offsets.
Members cite only consumers this landing respells or func_80023F08 itself; MoveScript +0x04..0x06 stays padding
(its reader func_80021A98 is a u8 * body outside this landing).
