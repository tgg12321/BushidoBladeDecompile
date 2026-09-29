# Hypothesis ledger — func_80029454

Scores below marked "harness" are `probes/q.py` (prefix of src/code6cac_b.c + candidate, Makefile
recipe for code6cac_b, compared against build/src/code6cac_b.o; "structdiff" = differing lines
with register names normalized, "rawdiff" = with registers). The 5 residual lines at the final
body are the splat pseudo-relocation artifact described in evidence.md [s2] (linked bytes equal).

## s2 (manual laneC, 2026-09-29)

- H1 CONFIRMED — the 0x1F800000 point tables are reached as members of one struct view through a
  constant pointer (`SPAD->unk00[i][k]`, `SPAD->unk48[i][k]`, `SPAD->unkA8[0][k]`). Mechanism:
  expr.c's COMPONENT_REF/ARRAY_REF path (get_inner_reference) keeps the constant base in the
  address as `(plus (reg i*36) (const 0x1F80000C))`, which loop.c records as DEST_ADDR givs and
  combines/reduces to the target's `s4 = 0x1F800000 + 36i` / `s6 = 0x1F800054 + 24i` pointers
  (and leaves the one unreduced `lui at; addu at,$fp,at; lw 0xC(at)` site exactly as the target
  does). The pointer-arithmetic spelling `((LeafPos *)0x1F800000)[i * 3 + k]` forces the
  constant into a register first (tmp dump d1: insn 184 `(set r134 0x1F800000)`), so no address
  givs are recorded. Measured: first draft (constant casts) harness 339 -> struct view 260;
  final-body ablation a7_constcast 215 vs 5. Save loop in isolation (probes/m1.c): only the
  member form `SPAD->limb[0][k]` gives the target's `li 0x1f800000` + 168/172/176 displacements.
- H2 CONFIRMED — halving loop needs the byte offset summed before it is added to `ws`:
  `(u8 *)ws + (i * 0x60 + j * 0x18)` (target `addu v1,s2,a1; addiu a1,a1,24`). Ablations:
  a2_noparen 125, a3_index (`&ws[i * 8 + j * 2]`) 131.
- H3 CONFIRMED — bounds loops re-read the pointer slot per component
  (`((LeafPos **)(scr + 0x60))[k]->x`), no cached point pointer: a11_cached_ptr 177.
- H4 CONFIRMED — the box test is a materialized 0/1 value (target `move a0,zero ... xori
  a0,v0,1; beqz a0`), written as the `box_overlap` static inline helper used by both passes.
  Folding the conjunction straight into the `if` (a4_direct) = 120. A named local written at
  both sites gives the same bytes as the helper but is a two-write local; the helper was kept.
- H5 CONFIRMED — `mask` is `u32`. With `s32`, fold-const.c:4443-4457 rewrites
  `mask & (1 << n)` (compared with 0) into `((mask >> n) & 1)` (`srav; andi`); the target has
  `sllv v0,s3,s1; and v0,s5,v0` with the constant 1 hoisted into $s3 and shared by the switch's
  `beq s4,s3`. Under `u32` the shift operand is a NOP_EXPR conversion of the int shift, so the
  LSHIFT_EXPR test at :4458 does not fire. a1_s32mask = 12 (20 raw).
- H6 CONFIRMED — overlap operands read the first box first (`amin <= bmax && amax >= bmin`):
  a10_overlap_order (`bmax >= amin`) = 17.
- H7 CONFIRMED — loop counters: `i` counts the two record loops and the halving outer loop, `j`
  the halving inner loop and both passes' outer loops, `n` both passes' inner loops, `k` the
  save/restore and bounds loops. Target evidence: the halving loop makes no call, yet its counters
  sit in callee-saved $s3/$s4 — the registers of the call-making loops' counters (record loop
  $s3, pass outer $s4). a8_halving_own_var (separate inner counter) = 5 struct / 36 raw;
  a9_save_on_i = 13 raw.
- H8 CONFIRMED — `*(s16 *)(rec + 0x286) = ... != 0 ? 0x19 : 0xB;` (target `beqz; li v1,11` /
  `li v1,25` / `sh v1`); the if/else store form a12 = 140.
- H9 NEUTRAL — `ws` initialised from `SPAD->unkA8[0]` or from `(LeafPos *)0x1F8000A8`: identical
  bytes (a5). Save/restore through `ws[k]` instead of `SPAD->unkA8[0][k]` = 165 (a6).
