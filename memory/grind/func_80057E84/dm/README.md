# func_80057E84 — data-model fix after round-1 layer-2 (rev-57E84-dm FAIL), laneB 2026-10-01

Objection (evidence.md, round 1): local NavPoly/NavPolySet and the `(NavPolySet *)arg1` cast;
func_80057ACC's offset accesses; the route record not declared once / not embedded in
PracticeMenuRec; the `(u8 *)poly` casts into func_80057CC8.

## The edits (`edits.py <text1b.c> <code6cac.h>`, anchor-based, re-appliable after Q65)
- include/code6cac.h: `CpuRoute` {u8 poly; u8 vtx; u8 count; CpuWaypoint node[8];} (0x34 bytes),
  `NavPoly`, `NavPolySet` declared after CpuWaypoint; PracticeMenuRec's `unk_352[0x362 - 0x352]`,
  `unk_362`, `unk_363`, `unk_364[8]` become `unk_352[0x360 - 0x352]` + `CpuRoute cpu_route` (+0x360).
- text1b.c: every `->unk_362` -> `->cpu_route.count` (14), `->unk_364[` -> `->cpu_route.node[` (21)
  (func_80058580 and the record reset at the `p->unk_362 = 0;` site); `extern NavPolySet
  D_8009A658[];` (data: asm/data/7D920.data.s D_8009A658, rows of count word / polygon array / zero
  word); func_80058580's `pois` is `NavPolySet *` (`pois = &D_8009A658[D_800A36A4];`), its calls drop
  `(s32)p` (func_80057ACC, func_800571C0); prototype `func_80057E84(PracticeMenuRec *, NavPolySet *, s32, s32)`.
- func_80057ACC (acc.c): `(PracticeMenuRec *arg0, NavPolySet *arg1, ...)`, members throughout.
- func_800571C0 (c0.c): `PracticeMenuRec *obj`, `p` = `obj->unk_00`, `unk_B8.vx/vy/vz`, `unk_1D8`; the
  waypoint loop writes `obj->cpu_route.node[nl]` (the integer `e = obj + nl * 6` is gone), then
  unk_398 / unk_3A0 / unk_3A2 / unk_39E from node[0].
- func_80057CC8 (cc8c.c): `NavPoly *arg0`, `arg0->vtx[k][0/1]`, `arg0->nvtx`, `arg0->margin`; the F3
  comment now names `arg0->vtx`. One line keeps its pre-existing integer form (below).
- func_80057E84 = ../candidate.c: no local typedefs, no casts; `CpuRoute path[2]`, `NavPolySet *arg1`.

## Measurement (`build.py`: copy src/text1b.c + include/code6cac.h to tmp, apply edits.py, build
text1b.o with the build pipeline, compare with build/src/text1b.o; main at 43e73e46b, pre-Q65)
- .text / .rodata / .data IDENTICAL (102804 / 88 / 0 bytes) with every edit and E84 as C.
- Per function (engine score): func_800571C0 0 (287), func_80057ACC 0 (127), func_80057CC8 0 (111),
  func_80057E84 0 (447), func_80058580 0 (2991).
- func_80057CC8's next-vertex address, all pointer spellings score 4 (differing words at +0xB8..+0xC8:
  `lw a1,4(s2)` / `addu a1,a1,v1` vs the target's `lw a0,4(s2)` / `addu v1,v1,a0`, 0x80057D80 /
  0x80057D88): cc8.c `arg0->vtx[(s16)next_idx]`, cc8a.c `*((s16)next_idx + arg0->vtx)`, cc8b.c
  `&arg0->vtx[(s16)next_idx][0]`, cc8d.c `(u8 *)arg0->vtx + ((s16)next_idx << 2)`, cc8e.c
  `arg0->vtx[(s32)(next_idx << 16) >> 16]`, cc8f.c `*(arg0->vtx + (s16)next_idx)`. C's pointer
  arithmetic always puts the pointer first (c-typeck pointer_int_sum), so the sum is born from the
  table load and local-alloc's combine_regs ties it there; the target's sum is index-first, the form
  the landed body already had: `(s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)arg0->vtx)`,
  kept with a comment (cc8c.c).

## Landing order (orchestrator): after Q65 applies, (A) data-model cheat-cleanup (header, ACC, CC8,
571C0, 58580 respellings) then (B) the Match; the Ruling 11 package (../r11/) is re-run on the new
E84 body once (A) is on main (scripts updated for CpuRoute).
