# SELF-VET — func_8002D518

Diff surface: `src/code6cac_b.c`, function body of `func_8002D518` only.
Honest measurement with the diff in place: `sandbox func_8002D518 --disable all`
=> `"score": 0`, `target_insns` 144 == `build_insns` 144, `rules_dropped: 33`,
`cheat_asm_stripped: 289`. No regfix/asmfix/engine/tools/Makefile/*.ld touched.

CONSTRUCTS: (1) duplicated `ud = disc;` inside the `if (disc >= 0)` LZCS-guard arm; (2) named intermediate `u32 ud` for the LZCS input / slow-path table index; (3) named intermediate `s32 flag` for the comparison result, copied into `result`; (4) named intermediate `s32 num1` for the first quotient's numerator; (5) variable reuse — `disc` carries the discriminant, then the square root, then the `<<9` result; (6) the GTE LZCS canonical inline-asm island (`mtc2 $t4,$30` / `swc2 $31`).

## T1 semantic purpose (per construct)
1. `ud = disc;` (duplicate): NO independent observable effect — `ud` already holds
   `disc` when the arm is entered. This is the one construct in the diff that
   fails T1 on its own and is therefore claimed under a sanctioned family with a
   `/* FAKE */` annotation (see SANCTIONED-FAMILY-CLAIMS, family A).
2. `u32 ud`: real. It is the value whose leading zeros the island counts and the
   value the slow path re-indexes (`(&D_8008D118)[ud >> shift]`). Both reads are
   live; deleting it changes what the code says.
3. `s32 flag`: real. It is the boolean `t2_val < 0x101` guarded by
   `t1_val >= 0`; `result` is the function's return value. Two distinct
   quantities with distinct lifetimes; the value is computed and consumed.
4. `s32 num1`: real. It is `(neg_b + disc) << 8`, the numerator of `t1_val`,
   written once and divided once.
5. `disc` reuse: real. Every assignment stores a value the following code reads;
   no store is dead. It mirrors the arithmetic the function performs
   (discriminant -> sqrt(discriminant) -> sqrt<<9).
6. The island: real and irreplaceable. `mtc2 $t4,$30` (LZCS) and `swc2 $31`
   (LZCR) are GTE cop2 register accesses with no C form.

## T2 human-programmer
2,3,4,5,6: yes. Named intermediates for a numerator, a boolean flag, and an
LZCS input are ordinary C; reusing one variable across successive refinements of
the same physical quantity (discriminant -> its square root) is ordinary C; a
cop2 GTE sequence must be inline asm. 1: NO — a reader would ask why `ud = disc`
is written twice. That is exactly why it is annotated `/* FAKE */` and claimed
under a named family rather than presented as ordinary code.

## T3 GCC-internals justification
Constructs 2-6 are justified by program logic alone; no GCC internal is needed
to explain why they are written that way. Construct 1 IS justified by a GCC
internal (cse.c `make_regs_eqv`) — measured, not guessed, from
`tmp/grind/func_8002D518/s8/dumps_v6/code6cac_b.cse` vs
`tmp/grind/func_8002D518/s8/dumps_nodup/code6cac_b.cse`. Per the
duplicated-statement-into-arms rule that is the expected shape for this family
(the rule's own confirmed closure is a `reg_n_refs`/global-RA argument), and the
rule requires the mechanism be named in the annotation. It is.

## T4 permuter/search provenance
None of the six constructs came from a permuter or an automated search. The s4
permuter campaign (33,881 iterations) found nothing and is banked as exhausted.
Constructs 1 and 3 were derived this session by reading the target's register
assignment and the cc1 RTL dumps, then predicted and confirmed by measurement:
the duplicate was predicted to keep the pseudo alive across the join before it
was compiled, and `flag` was predicted from target's `addu $v0,$a1,$zero` at
0x8002D770 (i.e. two distinct locals) before it was compiled. Both hypotheses
are recorded in the outcome JSON with their probes.

## T5 family check
1. Matches the sanctioned **duplicated-statement-into-arms** family exactly (a
   real statement written into a control-flow arm instead of relying on the one
   copy before the branch). Claimed below with scope + precedent.
2,3,4. Match the frozen SOTN entry **named-intermediate declaration order**.
5. Matches the frozen SOTN entry **variable reuse for codegen control**.
6. Canonical cop2 inline asm — the `canonical` gate's category, not a coercion
   family; the identical island is already shipped in accepted siblings
   `func_8002BC68` / `func_8002BEA0` (src/code6cac_b.c:760, :1414) and the
   function is listed in the cop2 cluster rule.
None of the six is a register pin, a hardcoded-`$N` lost-codegen injection, a
scheduling barrier, a volatile coercion, an alias rename, a dead local/array, a
dead-param-assign, a dead-conditional store, an `if (1)` wrapper, a dead goto
pad, a DImode chain, or a build-time rewrite. No `regfix.txt`/`asmfix.txt`/
`.claude/rules/`/`engine/`/`tools/`/`Makefile`/`*.ld` file is touched.

## T6 naming-announces-intent
Names in the diff: `ud`, `flag`, `num1`, `denom`, `t1_val`, `t2_val`, `shift`,
`half`, `tval`, `lzcr`, `sp_tmp`, `disc`, `result`. None is `pad`, `_pad`,
`dummy`, `unused`, `spill`, `sp_*` in the frame-padding sense, `_buf`, `tail`,
`slack`, or `_frame_pad`. `sp_tmp` is the `"=m"` output the island's
`swc2 $31, 0($t4)` writes and is read back as `lzcr` — a live value, not a pad.
Every named local in the diff has at least one real read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, **including** when:"
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:63
  (Prerequisites, per that rule: (1) the statement is a REAL def whose value is
  read on the path — `ud` is read by the island operand and by
  `(&D_8008D118)[ud >> shift]`, it is not a dead store; (2) byte-neutrality —
  the duplicated copy does NOT materialise an extra instruction: build_insns is
  144, identical to target_insns 144, and the honest sandbox distance is 0;
  (3) lever-exhaustion — memory/grind/func_8002D518/hypotheses.md, 7 prior
  sessions, 22 banked rejected forms, 33,881 permuter iterations, every plain-C
  placement of the copy measured folding; (4) `/* FAKE */` annotation present;
  (5) layer-1 + layer-2 review — pending, this vet is the input to it.)

  FAMILY: named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: .claude/rules/no-new-park-categories.md:188

  FAMILY: variable reuse for codegen control
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: .claude/rules/no-new-park-categories.md:170

ANNOTATION-CONFORMANCE:
/* FAKE: duplicated `ud = disc;` into the LZCS-guard arm, mechanism: cse.c
   make_regs_eqv - the second def makes pseudo `ud` multiply-defined, so the
   equivalence ud == disc established at the first copy is invalidated and cse
   can no longer rewrite the post-join `ud >> shift` read to disc's register;
   without it cse DELETES the copy outright (measured: v6_nodup, score 3, no
   `addu $a0,$a2`). lever-exhaustion:
   memory/grind/func_8002D518/hypotheses.md (s1-s7: 22 rejected forms, 33,881
   permuter iterations, every plain-C copy placement measured folding). */
Emitted verbatim in src/code6cac_b.c at the duplicated copy; carries what
(the duplicated statement), mechanism (a named GCC pass, cse.c make_regs_eqv,
attributed from the cc1 -da dumps, not guessed), and lever-exhaustion (the
ledger path). No other construct in the diff carries or needs a FAKE
annotation.
