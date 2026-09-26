# func_800620B8 — evidence (manual lane, slotC, 2026-09-26)

## Status (2026-09-26)
`candidate.c` measures **0 / 501** on `sandbox --disable all --diff` (every remaining hunk
`not-scored`: branch displacements / section addends). Canonical gate: ASM-PARTIAL, 1/501 insns =
the `swc2 $19` gte_stsz(r0) island (same verbatim inline_c.h macro island func_8006295C landed with,
c3eff5ec6). Floor path: 144 (first natural spelling) -> 127 -> 124 -> 83 -> 61 -> 52 -> 3 -> 0.

## Proven constructs and their receipts (all measured on the final chassis, 2026-09-26)
Each row = candidate with ONLY that construct respelled.

| construct in candidate | alternative measured | score |
|---|---|---|
| switch: case 3 / case 2 set sizes then `goto` into case 0 / case 1's shared tail | each case carries its own copy of the tail (order 3,0,2,1) | 49 |
| one 12-record table `D_8009BA00[12][4]` ([0..5] frames A, [6..9] frames B, [10]/[11] alt-CLUT records) | four split splat symbols D_8009BA00/BA30/BA50/BA58 | 47 |
| `SetTransMatrix((u8 *)tv - 0x14)` | `SetTransMatrix(base)` | 65 |
| rotation-matrix pointer read into `rot` before the dst32 copies | read `D_800A3474` at the func_80061FAC call | 3 |
| `rot` also used for SetRotMatrix | (i.e. `SetRotMatrix(rot)`) | 71 |
| tag link through the D_800A34E8 / D_800A34E4 scratch words | first half through `prim->tag` | 22 |
| `width`/`height` s16 locals for the clamped projected size | inline ternaries | 5 |
| (height via if/else stores) | | 1 |
| `prim` read after dst16[0] | prim read first | 3 |

Why each is what the original did (mechanism, from dumps in tmp/func_800620B8/dump):
- **Table merge.** Target holds 0x8009BA00 in callee-saved `$fp` across the loop (hoisted by
  loop.c) while 0x8009BA30/50/58 are rematerialised in `$t0` at each use. loop.c only hoists the
  BA00 load (`life 9, savings 2, moved`) when cse relates the other three addresses to it as
  `(const (plus D_8009BA00 N))` (use_related_value) — i.e. one symbol. With four separate symbols
  the BA00 load has life 1-2 and is "not desirable" -> not hoisted -> 47. Data-layout evidence
  (independent of codegen): 0x8009BA00..0x8009BA5F is 12 contiguous 8-byte records of one shape
  {clut x, clut y, u, v} (asm/data/7D920.data.s:23262-23304): [0..5] clut(0x3C0,0xFD) u=0..0xA0
  step 0x20 v=0x94 (32px strip, +0x1F), [6..9] clut(0x3F0,0xFC) u=0..0x30 step 0x10 v=0x80 (16px,
  +0xF/+0x13), [10] clut(0x3C0,0xFE) same uv as [0] = alternate CLUT for A, [11] clut(0x3F0,0xFD)
  = alternate CLUT for B. The original indexes both groups with stride 8 (`(n%6)*8 + $fp`,
  `(n&3)*8 + base`). Only func_800620B8 references any of the four labels.
- **SetTransMatrix(tv - 0x14).** Same idiom in the ORIGINAL binary in two siblings:
  func_80063084 (0x800632CC `addiu $a0,$t1,-0x14`) and func_800646E8 (0x800648AC
  `addiu $a0,$t0,-0x14`). base+0x10/0x12 are the w/h halfwords (would be m[2][2]/pad), so there is
  no MATRIX at base: SetTransMatrix reads only m->t, and the code points it 0x14 below the
  translation vector. With `SetTransMatrix(base)` base stays live through the loop and the whole
  allocation shifts (65).
- **goto-shared case tails.** Target case 3 = `lw v1,A8; li 0x151; sh; lw v1,AC; j <case 0's AC sh>;
  li 0xA8` and case 2 likewise into case 1: jump2 cross-jumps the AC store into the goto target.
  Duplicated tails are NOT cross-jumped by our cc1 (identical tails stay, +33 insns).
- **Tag link.** Target reloads D_800A34E4/D_800A34E8 after the `sw` into the prim tag: the store was
  not MEM_IN_STRUCT, so it went through the scratch word, not a struct member.
- **rot.** Target loads D_800A3474 into `$a2` between the dst16 and dst32 stores and re-reads the
  global for SetRotMatrix after the call. Same shape as matched sibling func_80060B70 (same TU,
  `last_arg`).

## Open
- Landing needs: header declaration of the 12-record table (aggregate-merge prong (d)), POLY_FT4
  typedef moved above the function (it is defined after it in text1b.c), auth: row for the gte_stsz
  island (inline_asm_canonical.txt + tools/canonical_asm_regions.json), oracle rebuild.
