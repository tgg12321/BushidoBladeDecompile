# SELF-VET — func_80070C70  (s17, structural; sandbox distance 0/194, full-build SHA1 == oracle)

CONSTRUCTS: `s32 c60 = 0x60;` (constant-holder local, FAKE-annotated); the second
`prim.zero1C = 0; prim.mode = 0;` pair (same-value re-store of two LOCAL struct fields, byte-
materializing, NOT annotated - see T1); `((GameObj *)arg0)->field_18` struct-member spelling of
the three 0x18 accesses (ordinary C); `prim.p_static = t; prim.p_static += D_800A3590[i] << 4;`
(split-init accumulation, ordinary C); `u16 rect[4]` (honest declaration, all four elements
written and passed to func_80069898); companion declaration edits (`extern s16 D_800A3558;`,
`extern u8 D_800A3560[];`, `extern s16 D_800A3590[];`, removal of a wrong m2c prototype).

## T1 semantic purpose
- `c60`: no observable effect vs the inline literal `0x60` - it exists to hold the constant in a
  callee-saved register across the three `func_8006E480` calls. This IS a codegen construct;
  it is declared as such and FAKE-annotated under the sanctioned constant-holder family.
- second `prim.zero1C = 0; prim.mode = 0;` pair: these stores MATERIALIZE BYTES THAT ARE IN THE
  TARGET. asm/funcs/func_80070C70.s stores zero to 48(sp) and 52(sp) BEFORE the `lw s2,100(v1)`
  context fetch and AGAIN after it (target listing lines 12-13 and 20-21; both pairs are inside
  the matched 194 insns). The original source clears those two fields twice; this body
  reproduces the original program, it does not coerce the compiler. Deleting one pair costs 2
  insns of distance, deleting half of it costs 1 - i.e. the bytes are the program.
- struct-member spelling of `arg0->field_18`: same loads/stores, same semantics; it is the
  correctly TYPED spelling of an access this function already makes through a `GameObj *`
  (it passes `(GameObj *)arg0` to func_80069898 on the next line).
- `prim.p_static = t; prim.p_static += ...;`: both stores are real and both are in the target.
- `u16 rect[4]`: every element is written and the array is passed to func_80069898, whose
  definition (src/text1b.c:5413) takes `u16 *` and reads [0..3]. Not a pad, not oversized.

## T2 human-programmer
Yes for everything except `c60`. A programmer writing this function writes `obj->field_18 +=
0xC;` (typed member access), `for (i = 0; i < 6; i++)`, `p_static = base; p_static += offset;`,
and a `u16 rect[4]` it fills and passes. The duplicated zero-init pair is the kind of redundancy
real game code contains, and here it is not a guess: the original binary contains both stores.
`c60` is the one construct a reader would ask "why is this a variable?" about - which is exactly
why it is annotated and claimed under its family rather than presented as ordinary C.

## T3 GCC-internals justification
The 7->2 step is justified by a GCC internal (sched.c:817 `true_dependence`'s MEM_IN_STRUCT_P
disambiguation), but the CONSTRUCT it selects is not a coercion: it is the ordinary typed
spelling of the access, and the mechanism explains WHY the honest spelling was not found for 16
sessions. No barrier, no pin, no dummy operand is involved. The 2->0 step is a loop written in
the ordinary `for` form. Only `c60` has a mechanism-as-purpose (local-alloc keeps a
live-across-call pseudo in a callee-saved register) and it is annotated accordingly.

## T4 permuter/search provenance
No permuter was run this session. Every form was hand-derived from the sched.c source read and
then measured with tools/sweep_variants.py (73 spellings, tmp/grind/func_80070C70/s17/v1-v6 +
the .json score tables). The winning spelling was predicted from the dependence predicate
BEFORE it was measured; it is not a detector-evading survivor of a random search.

## T5 family check
- `c60` -> constant-holder / dead scalar local (sanctioned; claimed below).
- duplicated zero-init pair -> shape matches the dead-store family, but the family is for stores
  that do NOT survive to the bytes ("GCC DCEs the store; its existence influences RA/scheduling/
  flow analysis upstream of DCE"). These stores survive and are present in the target, so they
  are original code, not a lever. No FAKE claimed and none applicable.
- struct-member access, `+=` accumulation, `for` loop, `u16 rect[4]`, corrected extern
  declarations: none of these match any forbidden family. Array declarations for
  D_800A3560/D_800A3590 are the remedy the brief's INDEXED-ACCESS data-model signal mandates
  (declare the indexed symbol as an array), applied at the DECLARATION, not as a per-use pun.

## T6 naming-announces-intent
No `pad`, `dummy`, `unused`, `spill`, `slack`, `_buf` or `_frame_pad` names. `c60` names the
constant it carries (0x60); `rect`, `prim`, `code`, `g`, `t`, `var_s0` are all consumed values.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: constant-holder / dead scalar local (`s32 c60 = 0x60;`)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): constant-holder locals (a local existing only to carry a constant in a register across calls) and dead/unused SCALAR locals that bias RA are allowed as LAST-RESORT levers with `/* FAKE: ... */` annotation + documented lever-exhaustion. SOTN ships `s16 three = 3;`, `s32 zero = 0; // needed for PSP`, `fake = 8;`, `new_var` in 9 files. Arrays/frame coercion stay forbidden."
  PRECEDENT: docs/reference/sotn-construct-index.md:859

ANNOTATION-CONFORMANCE:
  /* FAKE: constant-holder local, mechanism: local-alloc/global.c keeps a
   * live-across-call pseudo in a callee-saved register (the target's
   * `li s4,96` + three `move a1,s4`); the inline literal re-materializes
   * `li a1,0x60` at each call site and measures 7/191 vs 0/194.
   * lever-exhaustion: memory/grind/func_80070C70/hypotheses.md (s11-s17;
   * literal re-measured on every chassis, 9 declaration slots inert) */
  (what = constant-holder local; mechanism = local-alloc/global.c allocation of a
  live-across-call pseudo to a callee-saved register; lever-exhaustion = the named ledger file,
  which records the literal spelling re-measured on the do/while chassis (s12), the top-test
  chassis (s15: 14/191), and this chassis (s17 v3/v5: 7/191), plus 9 inert declaration slots.)
