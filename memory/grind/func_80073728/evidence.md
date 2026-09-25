# func_80073728 — evidence (manual session 2026-09-24)

Scaled POLY_FT4 sprite-sheet walker: the FT4 sibling of func_8007352C (SPRT).
func_80073200 builds the 0x2C-byte descriptor on its stack (S73200) and calls
this function 4x with mode 0..3; mode 4..6 select semi-trans rate in tpage.

## Result
COMPLETED-C candidate: `sandbox --disable all` 0 at 340/340, full build SHA1 ==
oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild).

## Floor trajectory (all measured this session, HEAD 5934a1377)
- 137 (367 insns): first transcription; uv stores written as
  `p->u0 = du0 + (ub + e->u)` — narrowed to QImode arithmetic, the du reloads
  became `lbu` and e->u was re-read after every store.
- 50 (340): named s16 u0/v0/u1/v1 sums computed before the stores.
  Register allocation became identical; remaining diff = stack-slot order.
- 9 (340): declaration order `i, tpage, clut, du0..dv1, ub, vb, ox, oy`
  (reload assigns spill slots in pseudo-number order = declaration order;
  target slots sp10 i, sp18 tpage, sp20 clut, sp28..40 du0/du1/dv0/dv1,
  sp48/50 ub/vb, sp58/60 ox/oy).
- 0 (340): `s32 u = ub + e->u; s32 v = vb + e->v;` and each byte store
  written as its own expression (`p->u0 = u + du0; ... p->v3 = v + dv1;`) in
  POLY_FT4 field order (setUV4 order).

## Mechanism of the last 9 (why the s16-local spelling cannot close)
Target interleaves adds and byte stores
(`addu a1; sb 6; ...; addu v0; sb 23; sb 31`), ours put all adds first.
- sched1 (tools/gcc-2.7.2/sched.c) schedules backward; stores and 1-cycle
  adds get EQUAL priority (priority() subtracts one per edge), and within an
  equal-priority group schedule_select's potential_hazard pick always prefers
  the store (memory unit). So adds can only interleave if adjust_priority's
  birthing boost fires: birthing_insn_p needs a plain REG dest, set once.
- With `s16 u0` locals, combine folds the SImode sum temp into the HImode var
  as `(set (subreg:SI (reg/v:HI u0)) (plus ...))` — not a REG dest, no boost.
- The `(u16)` cast / int-local forms get the boost (interleave appears) but add
  an `andi 0xffff`, or reload du into the output register (zero_extend
  operand) instead of the t0/a3 spill regs.
- The closing form: each store's value `u + du0` is its own expression; CSE
  shares the two identical expressions (u0/u2 etc.) into one SImode temp with
  two uses, so combine cannot fold it, the add keeps a REG dest, gets the
  birthing boost, and the target schedule falls out. The du operands stay
  paradoxical HImode subregs (lhu into spill regs t0/a3) exactly as target.

## Ablations (simplest-form)
- drop the `sx`/`sy` locals (read env->scale_x inline): 101 — KEPT.
- `s16 ox = e->x, oy = e->y;` initializers vs separate statements: both 0;
  initializer form adopted.
- declaration order a.c (natural first-draft order): 50 → the order is
  load-bearing (stack-slot layout), but it is ordinary C declaration order.

## Tooling left in tmp/f73728 (scratch)
mini.sh (standalone cc1 compile, 0.05 s), region.py + target_region.txt
(uv-block comparator), runsw.sh (batch), rtldump.sh / rtl.py (RTL dumps).
