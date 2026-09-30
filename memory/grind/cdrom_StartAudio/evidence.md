# cdrom_StartAudio — CdState D_80101E58 object evidence (owner ruling Q43, 2026-09-30)

Owner ruling Q43 (docs/grind/owner-rulings-2026-09-26.md, twenty-third batch): keep the CdState merge
if the cdrom_StartAudio proof holds, and shrink the object to the span actually proven. A fresh
layer-2 (q2-review) judged the proof HOLDS-WITH-CONDITIONS; this ledger banks the evidence against
the FINAL layout on main (CdState: proven span 0x80101E58..0x80101E99, sizeof 0x44; E9A..9B lies
inside the object and holds a halfword only func_80036140's asm accesses, via the D_80101E9A alias
row, so it has no member (q2-review condition) until that function lands in C — the owner has not
ruled on E9A; 0x80101E9C / E9E / EA0 / EA4 are declared separately because no proof places them in
the object — not proven separate: the span sweep's `58/A4` row, E58..EA3 as one object, also
scores 0; C declares only `u16 D_80101E9E` and `s32 g_cdread_expected_pos`). Header text:
include/code6cac.h, CdState / ReplayCamRec comments.

## The three links (original binary)
1. **E58..E62 one object** (base+offset): 800370AC `addiu a1,s0,-0xA` / 800370B0 `sb v0,-0xA(s0)`
   with s0 = &0x80101E62 (80036FDC/E0) — the (file, chan) CdlSetfilter parameter handed to
   CdControlB; C spells it `CdControlB(0xD, &D_80101E58.file, 0)`. cse
   relates two constant addresses only as offsets of one symbol.
2. **E6C..E99 one object** (base+offset): func_80036940 80036A74 `addiu s0,s0,-0x20` with
   s0 = &0x80101E8C, and 80036B08 `addiu a1,s0,-0x2C` with s0 = &0x80101E98 (both -> &pair).
   Span sweep (runs/span_sweep/span_cc1.txt): cutting inside it costs func_80036940 13/13/11
   (cuts at 74/78/98) and 14 (a cut at 6C with the head split), so its object is E6C..E99.
3. **E60/E62 and E6C one object** (prong (a), (a1)/(a2)) — the Q2 case. cdrom_StartAudio stores
   `rec.unk00` (8003701C sh), copies the CamPair (80037020..34, sw 8003702C / 80037034), and only
   THEN reloads `rec.unk00` (8003703C lh). In sched1, true_dependence (tools/gcc-2.7.2/sched.c:817)
   -> memrefs_conflict_p (sched.c:614): SIZE_FOR_MODE(BLKmode) == 0, so the BLKmode store at
   `pair` conflicts with the HImode load at `unk00` only when both addresses reduce to the same
   symbol_ref (sched.c:777). Separate objects -> no dependence -> the load is hoisted above the copy
   (runs/split6C.cc1psx.diff shows exactly that: `lh 0x1e60` moved above the two `lw`/`sw`).

## Dumps (dumps/, final layout; instrumented cc1 proven IDENTICAL to the build cc1 on each .i)
- merged.sched.txt / split6C.sched.txt — cdrom_StartAudio's section of the sched1 (-da .sched) dump.
- merged.sched1.trace.txt / split6C.sched1.trace.txt — BB2_SCHED_DEBUG pass=1 trace. The
  difference is the dependence list: merged has `dep insn=66 pred=55` (66 = the `lh unk00` reload,
  55 = the movstrsi_internal BLKmode copy into pair) and `dep insn=55 pred=34 kind=15` (34 = the
  `sh unk00` store); split has neither memory dependence (55<-34 only as kind 0).
- merged.combine.txt / split6C.combine.txt — the RTL: insn 55 `(set (mem:BLK (const (plus
  (symbol_ref "D_80101E58") 20))) ...)` vs `(set (mem:BLK (symbol_ref "g_cd_loc")) ...)`.
- cmdline.txt — the cpp/cc1 command lines.

## Scores (runs/; candidates/ hold the bodies; each candidate is substituted into a copy of main's
code6cac_b5_post.c, main's header unchanged)
`python3 -m engine.cli sandbox cdrom_StartAudio --disable all --candidate <c>` (our cc1) and
`python3 -m engine.cli cc1psx-check cdrom_StartAudio --candidate <c>` (original PsyQ cc1psx):

| candidate | layout / body | cc1 (sandbox) | cc1psx |
|---|---|---|---|
| merged.c | main (one object), plain body | **0** | **0** |
| split6C.c | cut at 0x80101E6C, same body — the split floor | **8** | 12 |
| split68.c | cut at 0x80101E68 | 8 | 12 |
| split64.c | cut at 0x80101E64 | 8 | 12 |
| split6C_R.c | cut at 6C, structural respelling `((CamPair *)&g_cd_file_table)[unk00]` | 8 | 12 |
| split6C_X.c | cut at 6C, cdrom_StartRead's `sval = (a0<<16)>>13` spelling | 10 | 14 |
| split6C_W.c | cut at 6C, word-by-word copy through `CamPair *e` | 17 | 17 |

Per-run files: `<c>.sandbox.txt` (with --diff), `<c>.cc1psx.txt`, and function listings + unified
diffs against the target for both compilers (`<c>.cc1.lst/.diff`, `<c>.cc1psx.lst/.diff`,
target.lst). Cuts at E66/E6A cannot be spelled (the tail's 4-byte members would be misaligned).
No FAKE construct and no pointer local in any body (layer-2 re-review 2026-09-30): the old FAKE
read-back (rec/idx/cam/entry locals) and the single-use `s16 *s0 = &rec.unk02` are gone; main's body
tests `if (D_80101E58.rec.unk02 != 0)` directly and copies with `rec.unk00 = arg0; rec.pair =
*(CamPair *)(&g_cd_file_table + rec.unk00 * 8);` (the reviewer's I form; the same unk00 read already
appears in the next statement). All runs above were re-banked on this body; the numbers did not move.

Harness caveat: tools/cc1psx_wrapper.sh does not surface dosemu errors. These runs are unaffected:
runs/cc1psx_banner.txt shows the wrapper's output for the merged and split TUs carries the
`GNU C 2.7.2.SN.1 ... Sony Playstation` banner and cdrom_StartAudio, and every cc1psx score differs
from cc1 on the split runs (so the object came from cc1psx, not a stale file).

## Every consumer on the final layout
runs/consumers_sandbox.txt: `sandbox <f> --disable all` = 0 for all 17 C consumers (cdrom_Init,
cdrom_ReadyCallback, cdrom_IsIdle, cdrom_StartRead, cdrom_StartReadAt, cdrom_Pause, game_FrameLoop,
cdrom_StartAudio, func_80036940, func_80037110, func_800371AC, func_800371E8, func_800371F8,
func_80037234, func_80037250, func_80037260, func_800372C0). Full build == oracle
62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild).
runs/span_sweep/: the q2-startaudio span sweep over all three TUs (sa.py, span.sh, results). Its
`58/9C/9E/A0/A4` row (E58..E9B one object, E9C/E9E/EA0/EA4 separate) is the landed layout: 0 for
every function under cc1; under cc1psx only the TU's baseline differences (cdrom_StartRead 7,
func_80037260 2, func_80037540 4, cdrom_ReadyCallback 12 — identical in the LANDED row).
`58/9C` (one separate E9C..EA7 record) costs cdrom_ReadyCallback 12, so E9C..EA7 is not one record
either; nothing in C needs it to be.

## (a4') member table — member_table.md (generated by tools/memtable.py)
Span 0x80101E58..0x80101E99 = lowest..highest byte the two judged functions (cdrom_StartAudio,
func_80036940) access: file E58 (StartAudio 800370B0) .. rec.unk38 E98..99 (func_80036940
lhu/sh). Typing of each member (every access listed in member_table.md):
- Accessed by a judged function — width from those accesses, one declared type under which all are
  ordinary C: file/chan u8 (sb); unk04 s32 (lw/sw, func_80036940); rec.unk00 s16 (lh
  8003703C); unk02 s16 (lh 80036FEC); unk04 s16 (sh; lh at func_80036140 80036458); unk06 s16 (sh
  only, func_80036940 800369C8; width 2, type as landed 2026-09-26); unk08 s16 (lh 80036994);
  unk0A s16 (sh; lh at func_80036140); pair CamPair (sw a/b 8003702C/34; the aggregate copy);
  unk14 s32 (sw); unk18/unk1C/sectors_remaining/dest_buffer s32 (lw/sw, func_80036940);
  unk2C s32 (lw/sw); unk30 u8 (sb 800370CC); unk34 s32 (sw); unk38 s16 (lhu/sh read-modify-writes;
  signed by the `++unk38 > 0x3C` compare: sll/sra 16 sign-extension then slti, 80036A38..40).
- **Forced in** (neither judged function touches the bytes; Q13): rec.unk28 E88..8B `s32` — its only
  original accessor is func_80036140 (INCLUDE_ASM), width 4 (lw/sw 800364C8/D8, 80036504, 80036810/
  1C), signed by `slti v0,v0,0x5` at 8003650C and 80036820 (ordered compare, admissible evidence).
- **Padding, no member**: E5A..5B (alignment before s32 unk04; no function accesses it), E91..93
  (alignment after u8 unk30).
- **Inside the object, no member**: E9A..9B. The object's size is a multiple of 4, so it covers
  these bytes; they hold a halfword that only func_80036140's asm accesses (sh 80036548, lhu
  80036778, sh 80036788, via the D_80101E9A alias row). No member until that function lands in C
  (q2-review condition 2); the owner has not ruled on E9A.
- filter.pad (layer-2 re-review 2026-09-30): the earlier embedded libcd CdlFILTER member is gone.
  No function accesses E5A..5B and SOTN has no CdlFILTER (Q50/Q55 do not apply), so the bytes are
  not admissible as a member under (a4')(2)/(3) + Q13; CdState now declares `u8 file; u8 chan;`
  directly and E5A..5B is compiler alignment before `s32 unk04`.
