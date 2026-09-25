# func_80063E10 — evidence (manual lane, 2026-09-25)

Queue distance 431 (whole-body INCLUDE_ASM, no prior C; the old
pre-include-asm-body.c is a placeholder with a wrong signature). Canonical gate:
ASM-PARTIAL, 58/431 insns GTE/cop2 (c2/ctc2/lwc2/mfc2/mtc2/swc2).

## What the function is
Per-lane billboard draw, called by func_80063B78 (lane 0) / func_80063BA4
(lane 1) after they store the lane's texture-rect index to *D_800A3480. Work
area D_800A34EC (scratchpad 0x1F8000B8): MATRIX mats[10] at +0x28 (filled by
func_800644FC = RotMatrix per live slot), composite MATRIX cm at +0x168,
sxy[4] at +0x188, u16 zbuf[10] at +0x19C, u16 zn at +0x1B0.
count = min(D_800A344C[lane], 10); func_800644FC(&count, mats, lane);
texture rect D_8009B920[*D_800A3480] (u16[4]: tpage x/y, u, v) -> the
D_800A3490..D_800A34E0 scratch words (same pattern as func_8006295C, w=7
h=15). zn = 0; *D_800A34B0 = ReadGeomScreen(). Per live slot i (bit i of
D_800A3454[lane]): POLY_FT4 header (len 9, code 0x2F, uv/clut/tpage words),
mats[i].t = D_800F0EC8[lane][i] - *D_800A3470, then GTE: gte_CompMatrix
(D_800A3474, &mats[i], cm) written out (PsyQ gtemac.h :323-352 =
MulMatrix0 [SetRotMatrix; 3x ldclmv/rtir/stclmv] + SetTransMatrix/ldlv0/rt/
stlvnl), SetRotMatrix(cm)+SetTransMatrix(cm), ldv3(D_8009BBE4/BBEC/BBF4)
rtpt stsxy3(sxy..sxy+2), stsz(D_800A34D0). z>0 -> z = func_80052C28(z-50,0);
z < 0x1005 and (geomscreen>>4) < z -> zbuf[zn++] = z, ldv0(D_8009BBFC) rtps
stsxy(sxy+3), copy sxy to x0..x3, prim++ while the buffer has room
(prim - D_800A3720 < 0x1C1). Tail: addPrim each new quad at OT[zbuf[k]]
through the D_800A34E8/D_800A34E4 globals, D_800A37D4 = end, return 1.

## Closing form: memory/grind/func_80063E10/candidate.c — sandbox 0 (431/431)
Measured on the spliced src 2026-09-25: sandbox --disable all 0, 0 scored
hunks (11 not-scored branch-displacement hunks); verify-oracle --rebuild
--allow-dirty build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa. 23 GTE
islands, region hashes in tools/canonical_asm_regions.json.

## Score ladder (sandbox --disable all, each measured)
- v1 (ternary count, inline `& (1 << i)`, walking zp tail) 73 -> tmp/f3e10/v1-73.c
- if/else count + `bit` statement + tail zbuf[k] 50
- + base staged through prim (a) 41; + `bits` read first (b) 40
- live pointer `s32 *live = &D_800A3454[lane]` before the loop (c) 14:
  right code, but computed BEFORE the loop guard (target: after, in the
  preheader = loop.c hoist)
- `*(D_800A3454 + lane) & (bit = 1 << i)` (h) 4 — only residual: giv init
  order (move s1,s0 / move s6,zero) swapped
- t stores through `mats[i].t[k]`, `m = &mats[i]` after them (n) 0
- `D_800A3454[lane] & (bit = 1 << i)` (r1) 0; drop `m`, operands `&mats[i]` (x2) 0
  = candidate.

## Findings (mechanisms, from -dL/-dl dumps via tmp/f3e10/dump.sh)
- loop.c move_movables in the 141-real-insn slot loop: `la D_800A3454` (life 3)
  is moved, then the lane<<2 shift (life 2, savings 2) is "not desirable"
  (threshold*savings*lifetime < 141) unless its life is stretched. The
  embedded `(bit = 1 << i)` does that: expand_binop expands the MEM address,
  then the assignment operand (li 1 / sllv into bit), then loads.
- Inline `1 << i` in the test: combine folds to srav/andi (429 insns, 44).
- Giv init order = reverse insn order of the givs: the i*12 giv
  (D_800F0EC8[lane][i]) must precede the i*32 giv (&mats[i]) in the loop body;
  writing the t stores as `mats[i].t[k] = <D_800F0EC8 ...>` puts the RHS
  giv first (expand_assignment of an ARRAY_REF LHS), `m = &mats[i]` before them: 4.
- base: a fresh local is a single-block pseudo (lreg: "used 6 times across 16
  insns in block 0") -> local-alloc v0; target holds it in s2 = prim's reg.
- gte_rt (not gte_rtv0tr) is the macro gtemac.h gte_CompMatrix uses; both map
  to 0x4A480012 in the independent no-DMPSX headers.
