NOT certified by this commit: remaining debt, out of scope. This is the complete list from a
systematic scan of all 36 changed bodies (memory/grind/judge-decl-cleanup/scan-2026-09-30/:
scan.txt has every hit, access.txt the base/offset extraction, classification.md the per-object
trace). Grouped by object; [T] = an object this landing touches.

[T] PracticeMenuRec records (g_practice_menu_table, stride 0x44C), reached by byte offset through a
`u8 *` / `s32 *` parameter or local:
- func_8001A820: arg2/arg3 +0x6A (unk_6A, 15 reads) and +0xB8 (unk_B8, as CamVec); arg0/arg1 read
  as `((s32 *)argN)[0..2]` / `*(CamVec *)arg0` (the record's unk_168 position vector, passed by
  the caller).
- func_8001B294: a0/a1 +0xF4/+0xF8/+0xFC (unk_F4).
- func_8001B3C0: a0 +0x180/+0x184/+0x188 (unk_180).
- func_8001B748: `base = (u8 *)&D_80101EC8 + D_800A3748 * 0x44C`, reads +0x184/+0x19C/+0x1A8.
- func_8001F2E4: obj, and the other record `*(u8 **)obj` (unk_00): +0xC/+0xE/+0x6A/+0x8C/+0x96/
  +0xF4..0xFC/+0x180..0x188/+0x1CA/+0x1D8/+0x1E6..0x1EA/+0x25C..0x268/+0x26C/+0x26E. Its a/b
  are the caller's stack buffers (func_80023F08 sp+0x18 / sp+0x9C), read at +0xC..+0x7E.
- func_800233AC: arg0 +0x2C/+0x98 (SVec8_233AC view)/+0xB8/+0xBC/+0xC0/+0x1D8.
- func_80023648: arg0 +6/+0xA/+0x1A/+0x2C/+0x44/+0x6A/+0xD8/+0xE0/+0x14C/+0x14E/+0x150/
  +0x1CA/+0x1D8.
- func_800283D0: arg0 and its unk_00 record `temp_s4 = *(u8 **)arg0`: +4/+0xC/+0xE/+0x6A/+0x8C/
  +0x1CA/+0x286/+0x288 + idx*2; `tail = (s32 *)(temp_s4 + temp_s5 * 0x10)`.
- func_8002A458: obj +4/+0xF4..0xFC/+0x26C; the other record `*(u8 **)obj` +0x1D8.
- func_80030580: src (= arg0) +4/+0x1A/+0xF4..0xFC/+0x1CA.
- func_80032064: src +4/+0x1A/+0xB2/+0xBC/+0xF4/+0xFC/+0x1CA.
- D_80101EC8, the byte-base name of g_practice_menu_table, is still used by other functions'
  byte-offset walks.
[T] Rec44 (D_800F5328 / D_800F6608):
- func_8001A820: `func_8001A538((s32 *)cam, …)` call-boundary cast (the callee walks Rec44 by
  byte offset).
- func_800325E0: `D_800A36B4` is declared `s32` but holds a Rec44 pointer (func_8001E404 stores
  it). It is read at +0x12 (h12) and +0x20/+0x24/+0x28 (w20..w28) through
  `*(T *)((u8 *)D_800A36B4 + off)`.
- func_8003C9A4: `func_80046BF4((s16 *)a0, a1, …)` under code6cac_c2.c's own prototype
  (s16 *, s16 *, s32); the definition, in sound.c, takes (s32 *, u16 *, s32).
- func_8001B748: only its h30..h3C stores are respelled; its other accesses are unchanged.
[T] D_800F5F68 (u8 records of 0x1B8): func_8002A458 `rec = &D_800F5F68[id * 0x1B8]`, read at
  +0/+0xC/+0xE/+0x10/+0x12.
Rec1C: func_8001BAE4 reads its `s32 *` arg0/arg1 (Rec1C records from func_8003993C) at +4/+8 as
  s16 (h4/h8).
D_80106A78 (0x64-byte records): func_80030580 and func_80030D7C `obj` (+0..+0x60).
func_8003032C: its `s32 *a0` is read at +0x44/+0x48/+0x4C. Nothing in the binary calls it
  directly, so its object is untraced; the offsets match the D_80106A78 velocity fields.
D_80104E88 (0x2C-byte records): func_80032064 `ptr` / s0 (+1..+0x28) and func_80032314 t0/a3
  (+1/+2/+6/+0xA).
D_800A37E8/EA/EC: a split s16 vec3 written as three scalars (func_8002A458).
Scratchpad (0x1F80xxxx) views through `u8 *` / `s32 *` casts and offsets:
- func_80018300;
- func_8002A458 (scr 0x1F8002B8, pos 0x1F8000A8);
- func_8002D320, func_8002D780, func_8002DAD0, func_8002E838, func_8002EA24 (obj = the
  0x1F8002B8 scratch block passed by their callers);
- func_8002EBDC, func_8002F2D0, func_8002F770 (scr + 0xA8/+0xD8);
- func_80030D7C (scr).
Model/geometry data: func_80018094 (`(MATRIX *)arg0[1]`, dst), func_80018300 (arg0 +6/+0xC/
  +0x10), func_800187F4 (arg0 +0xC, arg1 +4 / [3]), func_8002F2D0 (`(MATRIX *)a0`).
Other casts: func_80021DB0 `(s16 *)stage_GetDataPtr()`.
func_8003CE18's `addr` keeps its two writes: record 0, then (if D_800A3748 == 0) record 1. The
  record-pointer respellings score 2.
