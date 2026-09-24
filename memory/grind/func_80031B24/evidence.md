# func_80031B24 — evidence (manual session 2026-09-24)

Per-frame hit test: 12 thrown objects (0x64-byte records at D_80106A78) against
the opponent's 22 body spheres (scratchpad SCR[other].j[j + 4], radii from
D_800F5F68 record, stride 0x14). The object's segment (prev pos +0x38 -> cur pos
+0x2C) is staged at scratchpad 0x1F8002B8 for func_8002E838 / func_8002EA24.

## Floor trajectory (sandbox --disable all, --candidate)
- first draft (p = obj + 4 carrier, byte-stride scratch index): 158
- pure obj-relative fields + Vec3i struct copy for the segment: 179 (structure aligned; stack slots off)
- scratch address `&SCR[other].j[j + 4]` (the existing ProbeScr macro, as func_8002CA8C uses): 29
  (hp local `Vec3i (*)[22]`, `u8 *hp + other*0x108 + j*0xC`: 170; the j*12 giv + other*0x108
  split in the target only appears with the constant-base macro)
- declaration order scr, i, deep, obj, seg, ch (spill-slot order 0x28..0x50): 9
  (natural order scr, seg, obj, i, ch, deep: 20 — ablated on the final body)
- `u16 st` + a plain `==` chain over all ten states: GCC folds the adjacent pairs
  (0x1C/0x1D, 0x1E/0x1F, 0x20/0x21) into `sltiu` range tests and shorten_compare
  emits the `andi a0,v1,0xffff` the target has for the single-value tests: 4
  (split `if`s with explicit `(u32)(st - 0x1C) < 2`: 9; `s16 st`: 11/68; switch: 15/19)
- the remaining 4 was the stand-in probe symbol for the table byte at +0xD; with the
  real record declaration the scorer resolves `D_8008E194+13` == `D_8008E1A1`: 0.

## Object-model changes landed with it
- include/code6cac.h: `Tbl8008E194 D_8008E194[]` (14-byte record) replaces the per-word
  scalars D_8008E194 / D_8008E19E / D_8008E1A1 (aggregate-merge family, owner ruling
  2026-08-17). Evidence independent of this session: func_80030580 (COMPLETED-C) indexes
  the table with stride 14 (`sll 3; subu` then *2), func_80030D7C (asm) reads +0x0 and
  +0xA of a record; the data at 0x8008E194 repeats with period 14.
- func_80030580: `tbl = &D_8008E194[arg1]`, fields by name — byte-neutral (oracle).
- undefined_syms_auto.txt: D_8008E1A1 row retired (only referrer was this function's .s);
  D_8008E19E stays, suffixed `alias of D_8008E194+0xA; retire with func_80030D7C`.
- func_8002FF20 `u8 arg1` -> `s16 arg1`: the only caller passes an s16 record field with
  no `andi 0xff` (lh a1,2(fp) straight into the call); callee bytes unchanged (sandbox 0
  for both s16 and s32 before landing; oracle after).
- g_disp_fade (not the D_800A36A8 alias) so sdata_exclude.txt's existing
  `func_80031B24: g_disp_fade` row applies (target uses lui/sb, not gp).

## Ablations on the final body
- `Vec3i *seg = (Vec3i *)scr;` vs a second `(Vec3i *)0x1F8002B8` constant: both 0 -> kept the alias.
- `j >= 6 && j <= 9` vs `(u32)(j - 6) < 4`: both 0 -> kept the plain range.
- declaration order: 20 without it (see above).

Final: oracle GREEN 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild --allow-dirty).

## Layer-2 review (2026-09-24): PASS
Non-blocking notes: (1) ProbeScr `Vec3i j[22]` is under-modelled — `j + 4` reaches j[25];
`((Vec3i (*)[22])0x1F8000A8)[other][j]` also scores 0, a byte-neutral model fix is open.
(2) the func_80027AD8 prototype's 6th param (Tbl8008E194 *) fits this caller only;
func_8002AB08 (asm) passes a 0/1 boolean there.
