# SELF-VET — func_800645B0

CONSTRUCTS: (1) stride/offset arithmetic spelled as expressions at each store
site, no named intermediates; (2) named local `bit` holding the 1 that is
shifted; (3) `val` reused — it receives D_800A3444 and then receives the OR
result before the write-back; (4) named local `last` receiving the leading
rand() result; (5) `bit = 0;` — a dead store to a local, /* FAKE */-annotated;
(6) explicit pointer-cast stores of the shape `*(s32 *)((s32)&SYM + offset)`.

## T1 semantic purpose: (1) yes — those expressions compute the three addresses
the stores need, and the halfword address; the array is one array of 3-word
structs plus a parallel s16 array. (2) yes — `bit` holds the value 1 that gets
shifted into the mask; a variable shift on MIPS needs the shifted value in a
register, so this is a real datum. (3) yes — `val` stages a read-modify-write of
D_800A3444: it receives the global, receives the OR against the mask, and is
stored back; every one of its values is read. (4) yes — `last` is read twice (one
word store and the halfword store), so it must be a named datum. (6) yes — those
are the stores the function exists to perform. (5) NO — the value stored by
`bit = 0;` is never read, so it has no observable effect. That is exactly the
construct the sanctioned dead-store carve-out describes, and it carries the
annotation that carve-out mandates; the claim block below cites it.

## T2 human-programmer: (1)-(4) and (6) yes — ordinary C: an address expression,
a named datum, a read-modify-write staged in a local, three stores. A reader asks
no question about any of them. (5) no — a programmer writing only to the
specification would not store 0 into `bit`, and a reader WOULD ask why it is
there. That is precisely why the sanctioned carve-out requires the /* FAKE */
annotation on it, naming both the GCC mechanism and where the lever-exhaustion
trail lives, and why this vet claims that family explicitly rather than
presenting the statement as ordinary logic.

## T3 GCC-internals justification: For (1)-(4) and (6) the justification is
program logic — the data model, the addresses, the read-modify-write. For (5) the
justification IS a named GCC mechanism, and stating it is prerequisite 2 of the
carve-out being claimed, not an evasion of test 3: loop.c's count_loop_regs_set
(loop.c:3036-3047) marks may_not_move for a register set in two basic blocks of a
loop, so the gate at loop.c:649 never treats the const-1 set as movable and it
stays put; flow.c's life_analysis deletes this store as it walks and therefore
never counts it, leaving reg_n_sets[bit] == 1; sched.c's adjust_priority then
lifts the const-1 set to max_priority alongside the index `addu`, and the
INSN_LUID tie-break yields the target's emission order. The carve-out exists for
exactly this case: a statement whose only effect is on analyses that run upstream
of DCE.

## T4 permuter/search provenance: No permuter, campaign or auto-search produced
any part of this body. Construct (5) was PREDICTED in writing — by reading
loop.c, flow.c and sched.c — before any measurement, together with two negative
controls; the s9 measurement table then confirmed the prediction and both
controls (a single-set spelling of `bit` costs two insns; a dead store spelled
`bit = 1;` also costs two insns, since cse1 runs ahead of loop.c and folds out a
store of the value already held). Two further spellings (store at the head of the arm,
and `bit = 2;`) also reach 0, so no placement or value tuning is involved.

## T5 family check: (5) is a dead store to a local — the family claimed below,
with its scope sentence and precedent. (3) is a read-modify-write staged in one
local, which the layer-1 reviewer already ruled ordinary C for this function in
session 1 (its constructs 1-4 were ruled legitimate, var-reuse included). (4) is
a named intermediate. (1), (2) and (6) are ordinary C address arithmetic, a named
datum and three stores. No register pin, no `__asm__`, no volatile, no alias, no
statement reordering lever, no wrapper, no goto, no array, no cast-width trick.

## T6 naming-announces-intent: the locals are `i`, `j`, `idx`, `mask`, `val`,
`last`, `bit` — every name describes the value it holds. None of `pad`, `_pad`,
`dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, `_frame_pad`
appears. `bit` is genuinely read (the shift reads it), so it is not a
declaration-only or address-only datum.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Dead stores / self-assignments to locals+params (FAKE-annotated
    last-resort carve-out, owner ruling 2026-07-01)
  SCOPE: "Ordinary-C assignment statements inside a function body whose stored value is never read (GCC DCEs the store; its existence influences RA / scheduling / flow analysis upstream of DCE)"
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:103

  Prerequisites of that rule, addressed in order:
  1. Lever-exhaustion documented — memory/grind/func_800645B0/hypotheses.md,
     sessions 1-8: H10 (foldable second sets inert), H15, H16, H18 (every
     single-set spelling of the const-1 costs two insns), H24/H25 (the
     expand_binop wall), H30/H31 (the local-alloc wall), H37/H38, H39 (the
     reg_n_sets impossibility argument), H40/H41, plus two permuter campaigns
     across four chassis (~102k iterations) and this session's own controls.
  2. GCC-pass interaction named — see T3 (loop.c:3036-3047 / loop.c:649,
     flow.c life_analysis, sched.c adjust_priority).
  3. Annotation present — see below.
  4. Layer-1 + layer-2 review — this vet is written for them.

ANNOTATION-CONFORMANCE: one /* FAKE */ construct, annotated in src/text1b.c as:
  `bit = 0; /* FAKE: dead store to a local; mechanism: loop.c`
  ` * count_loop_regs_set sets may_not_move for a reg set in two basic`
  ` * blocks of the loop, which keeps the const-1 set inside the inner`
  ` * loop (loop.c:649), while flow.c life_analysis deletes this store`
  ` * and never counts it, so reg_n_sets == 1 and sched.c`
  ` * adjust_priority lifts the const-1 set alongside the index addu;`
  ` * lever-exhaustion: memory/grind/func_800645B0/hypotheses.md`
  ` * (sessions 1-8, H10/H15/H16/H18/H24/H30/H37/H39) */`
carrying what (a dead store to a local), the mechanism (named GCC passes) and
the lever-exhaustion pointer, as the carve-out's prerequisite 3 requires.
