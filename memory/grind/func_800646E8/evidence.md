# func_800646E8 — evidence (manual lane, slotB, 2026-09-26)

Draws the 16 effect slots func_800645B0 spawns (bit i of D_800A3444, counter
D_800F0BCC[i], world pos = 12-byte record at D_800F0D78/D_800F0D7C/videoDec +
i*12). Work area D_800A34EC (scratchpad): w s16 +0x10, h s16 +0x12, frame s32
+0x14, trans VECTOR +0x18, pos VECTOR +0x28, SVECTOR +0x38, RotTransPers p
+0x40, end-of-fill POLY_FT4* +0x44, u32 z list +0x48.

## Result: candidate.c = sandbox 0/490 (0 scored hunks, 22 not-scored branch displacement)

candidate.c needs ONE decl change at landing: src/text1b.c file-scope
`extern s16 D_800F0BCC;` -> `extern s16 D_800F0BCC[];` (the 16-entry counter
array; D_800F0BCC + 0x20 = D_800F0BEC, the next array). Whole-file .s with
that decl is byte-identical to the sandbox-0 variant (tmp/func_800646E8/dump,
g1_full vs final_full: only the .file line differs), so func_800645B0's
`(s32)(&D_800F0BCC)` spelling is unaffected.

GTE islands: gte_SetRotMatrix(D_800A3474) and gte_stsz(D_800A34D0), verbatim
from the siblings func_80063E10 / func_8006295C (inline_c.h 4.3 :297-310,
:1042-1046). Needs auth: row + region hashes (COMPLETED-INLINE-ASM-CANONICAL).

## Measured levers (each is what the target's bytes demand)

- Tail OT link: `D_800A34E4 = ot + *zbuf*4;` BEFORE `D_800A34E8 = prim;`
  (target loads zbuf/ot before `sw s1,D_800A34E8`); E8-first = 59->32 gap.
  zbuf[k] (in-struct) instead fixes order but spills zbuf (zbuf loses to trans
  in global.c priority) — 59.
- Sprite y: `(*(s32 *)D_800A34B8 >> 16)` — target reads the packed sxy word
  with `lh 2(a0)` BEFORE the /32 division (combine narrows the >>16 of the lw);
  `((s16 *)D_800A34B8)[1]` is shortened to a lazy lhu after the division.
- Size clamp: `*w = prod > 0x800 ? prod >> 8 : 8;` with the product written
  twice and NO local. expr.c COND_EXPR: condition reads memory, so
  safe_from_p(mem target) fails -> HImode temp -> combine force_to_mode turns
  the ashiftrt into the target's srl, and ONE store sits in the join block
  (sched places it after the counter-address calc, as target). A local
  (`sw = prod; if/?:`) gives sra and a store per arm (e1 = 2, d1/d3 = 4-6).
- `*h += (*h * (cnt << 2)) >> 8` (cnt*4 reassociates onto h: +2).
- `if (D_8009BD44[0] & 1)`: in-struct load must follow the in-struct
  prim->clut store (true_dependence); scalar D_8009BD44 lets sched hoist it (+5).
  D_8009BD44 is a 5-word dlabel (asm/data/7D920.data.s:23871); the dossier's
  SPLIT-AGGREGATE "+1 of g_menu_screen_live_byte_c (byte @8009BD43)" cannot be
  right — the target does an aligned `lw` of the word.
- TexRec struct view `((TexRec *)D_800A3488)->clut_x`: clut_x load must be
  in-struct so it schedules before `sw D_800A3488` (target: lhu p0, sw, lhu p1).
  `((u16 *)D_800A3488)[0]` folds the +0 -> not in-struct (15 worse); the
  `(*(u16(*)[4])...)[0]` view did not move it.
- `for (i = 0, zp = zbuf; ...)` (zp init after i=0 as target; before loop = 2).
- prim re-walks the quads in the tail (fresh cursor = 34, rejected/).
- `SetTransMatrix((u8 *)trans - 0x14)`: target keeps base+0x18 in a spill slot
  and computes -0x14; a MATRIX view at base+4 = 5 (rejected/matrix-view-trans-5.c).
