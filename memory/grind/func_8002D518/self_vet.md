# SELF-VET — func_8002D518

Session s8 (synthesis, 2026-08-19). This vet SUPERSEDES the previous one, which
cited the `ud = disc;` re-store under **duplicated-statement-into-arms** — the
2026-08-19 07:16 layer-1 review FAILed that citation (the family's scope does not
describe this code shape) and the 2026-08-19 07:34 Judge ruling narrowed the ban
to exactly that citation, ordering re-derivation under **dead-store-fake-exception**
and resubmission. Every duplicated-statement-into-arms claim is dropped here and
in the in-source annotation.

Diff surface: `src/code6cac_b.c`, function body of `func_8002D518` only.
Honest measurement with the diff in place, THIS session, on the current chassis:
`& tools/wteng.ps1 main sandbox func_8002D518 --disable all` =>
`"score": 0`, `target_insns` 144 == `build_insns` 144, `rules_dropped: 33`,
`cheat_asm_stripped: 289`, `scorable: true`.
No regfix.txt / asmfix.txt / .claude/rules/ / engine/ / tools/ / Makefile / *.ld
file is touched; `git status` shows only `src/code6cac_b.c` plus ledger/scratch.

CONSTRUCTS: (1) redundant same-value re-store `ud = disc;` inside the
`if (disc >= 0)` LZCS guard arm; (2) named intermediate `u32 ud` for the LZCS
input / slow-path table index; (3) named intermediate `s32 flag` for the
comparison result, copied into `result`; (4) named intermediate `s32 num1` for
the first quotient's numerator; (5) variable reuse — `disc` carries the
discriminant, then its square root, then the `<<9` result; (6) the GTE LZCS
canonical inline-asm island (`mtc2 $t4,$30` / `swc2 $31`).

## T1 semantic purpose: per construct
1. `ud = disc;` (the re-store): NO independent observable effect. `ud` was
   assigned `disc` immediately above the guard and neither is modified in
   between, so at this point the statement is semantically `ud = ud;` — the
   value it stores is indistinguishable from the value already there, and the
   store does not survive to final output (build_insns 144 == target_insns 144,
   i.e. it materialises ZERO bytes). This is the one construct in the diff that
   fails T1 standing alone; it is therefore claimed under the sanctioned
   dead-store-fake-exception carve-out and carries a `/* FAKE: ... */`
   annotation on the statement (family A below).
2. `u32 ud`: real. It is the value whose leading zeros the island counts
   (`"r"(ud)`) and the value the slow path re-indexes
   (`(&D_8008D118)[ud >> shift]`). Both reads are live.
3. `s32 flag`: real. It is the boolean `t2_val < 0x101` guarded by
   `t1_val >= 0`; `result` is the function's return value, also written by the
   `disc < 0` arm. Two distinct quantities, distinct lifetimes, both consumed.
4. `s32 num1`: real. It is `(neg_b + disc) << 8`, the numerator of `t1_val`,
   written once and divided once.
5. `disc` reuse: real. Every assignment stores a value the following code reads;
   no store is dead. It mirrors the arithmetic (discriminant -> sqrt of the
   discriminant -> sqrt<<9), which is one physical quantity refined in place.
6. The island: real and irreplaceable. `mtc2 $t4,$30` (LZCS) / `swc2 $31`
   (LZCR) are GTE cop2 register accesses with no C form.

## T2 human-programmer: partly, and the one exception is declared
2,3,4,5,6: yes — a human writing this collision/quadratic routine from spec
would naturally name a numerator, a boolean flag and an LZCS input, reuse one
variable across successive refinements of the same physical quantity, and drop
to inline asm for a cop2 sequence. 1: NO — a reader would ask "why is `ud = disc`
written twice?". That admission is precisely why construct 1 is annotated
`/* FAKE */` and claimed under a named sanctioned family instead of being
presented as ordinary code. No other construct draws that question.

## T3 GCC-internals justification: only construct 1, as the family requires
Constructs 2-6 are justified by PROGRAM LOGIC alone; no GCC internal is needed
to explain why they are written that way, and none is cited for them.
Construct 1 IS justified by a GCC internal — cse.c's `make_regs_eqv` — which is
the REQUIRED shape here, not the cheat signal: dead-store-fake-exception
prerequisite 2 states verbatim "**The GCC-pass interaction is named.** The worker
states which analysis the dead store influences (allocno priority / nrefs,
scheduling, DCE-flow) and why the natural form can't reach it. A dead store with
no articulated mechanism is noise, not a lever." The mechanism was MEASURED, not
guessed, by reading the cc1 `-da` dumps
(`tmp/grind/func_8002D518/s8/dumps_v6/code6cac_b.cse` vs
`tmp/grind/func_8002D518/s8/dumps_nodup/code6cac_b.cse`): with one def, cse puts
`ud` and `disc` in a single quantity, rewrites the post-join `ud >> shift` read
to `disc`'s register and deletes the copy insn entirely (control v6_nodup:
score 3, no `addu $a0,$a2` in the output); with the second def the pseudo is
multiply-defined across the join, the equivalence is invalidated, both copies
survive cse, and global_alloc lands them in target's $a2/$a0 pair.

## T4 permuter/search provenance: none — hand-derived and predicted
None of the six constructs came from a permuter or an automated search, and none
of them passes a detector merely because the detector misses that spelling —
construct 1 is affirmatively DECLARED here and annotated in-source precisely so
the detectors and both review layers see it. The s4 permuter campaign (33,881
random iterations plus an exhaustive 24-point directed cross-product) found
nothing and is banked as exhausted; the floor moved 30 -> 7 -> 4 -> 0 afterwards
by hand-derived structural edits. Constructs 1 and 3 were derived by reading the
target's register assignment and the cc1 RTL dumps, PREDICTED, and then
confirmed by measurement, not sampled.

## T5 family check: one carve-out claim, two frozen-list entries, no analogies
1. Matches the sanctioned **dead-store-fake-exception** family: a redundant
   same-value store (`ud = disc;` where `ud` already equals `disc`, i.e. the
   rule's `x = x;` self-assignment shape) to a **LOCAL**, `/* FAKE */`-annotated,
   emitting zero bytes. Claimed below with verbatim scope + precedent. It is NOT
   a store to a global (out of scope per that rule) and is NOT paired with any
   `register T x asm("$N")` pin — the diff contains no pin at all; the only asm
   is the GTE island, which binds `"r"(ud)` by constraint and lets GCC choose the
   register, and whose `$t4` references are INSIDE the template of a canonical
   cop2 sequence in the exact form already accepted in-tree for siblings.
2,3,4. Match the frozen SOTN entry **named-intermediate declaration order**.
5. Matches the frozen SOTN entry **variable reuse for codegen control**.
6. Canonical cop2 inline asm — the `canonical` gate's category, not a coercion
   family; the identical island form is already shipped in accepted siblings in
   the same file and the function is listed in the cop2 cluster rule
   (`.claude/rules/cop2-addressing-preamble-cluster.md:74`).
No construct in the diff is a register pin, a hardcoded-`$N` lost-codegen
injection, a scheduling barrier, a volatile coercion or alias rename, a dead
local array or frame-coercion declaration, a dead-param-assign, an `if (1)`
wrapper, a dead goto pad, a DImode chain, a combine-foldable chain-extender, or
a build-time rewrite. NO claim is made under duplicated-statement-into-arms.

## T6 naming-announces-intent: no coercion-announcing names
Names in the diff: `ud`, `flag`, `num1`, `denom`, `t1_val`, `t2_val`, `shift`,
`half`, `tval`, `lzcr`, `sp_tmp`, `disc`, `result`, `ax`, `az`, `x1r`, `z1r`,
`ax_sq`, `az_sq`, `cx`, `cz`, `x1_sq`, `z1_sq`, `dot2`, `dot2_9`, `c_val`,
`dist_sq`, `neg_b`. None is `pad`, `_pad`, `dummy`, `unused`, `spill`, `_buf`,
`tail`, `slack`, or `_frame_pad`. `sp_tmp` is not an `sp_*` frame pad: it is the
`"=m"` output that the island's `swc2 $31, 0($t4)` writes and which is read back
on the next line as `lzcr`, a live value with a real consumer. Every named local
in the diff has at least one real read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-store-fake-exception
  SCOPE: "Ordinary-C assignment statements inside a function body whose stored value is never read (GCC DCEs the store; its existence influences RA / scheduling / flow analysis upstream of DCE):"
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:24
  (sub-scope bullet, verbatim: "- self-assignment: `x = x;`" at
  .claude/rules/dead-store-fake-exception.md:28 — `ud = disc;` at this point IS
  `ud = ud;`, since `ud` was set from `disc` immediately above the guard and
  neither is modified in between.)
  FROZEN-LIST CONFIRMATION: .claude/rules/no-new-park-categories.md:260
  JUDGE PRECEDENT FOR THIS EXACT CONSTRUCT: docs/grind/decisions.md:6521
  (2026-08-19 07:34 PASS — "the construct (redundant same-value re-store of a
  LOCAL, `/* FAKE */`-annotated, dropped before final output) sits inside
  dead-store-fake-exception's stated scope"; authorises resubmission under this
  family only.)
  PREREQUISITES, one by one:
    1. Lever-exhaustion documented — memory/grind/func_8002D518/hypotheses.md +
       evidence.md + 32 files in memory/grind/func_8002D518/rejected/, across
       s1-s9: every plain-C placement of the copy measured (base if/else, init
       guard, hoisted above the 0x400 test, defined only inside the guard arm,
       assigned in both arms, static-inline helper param copy, srlv-reads-disc)
       — each either folds the copy away or costs insns; plus 33,881 permuter
       iterations with zero finds. This was the LAST lever reached, not the first.
    2. GCC-pass interaction named — cse.c `make_regs_eqv`, attributed from the
       cc1 `-da` `.cse` dumps of the with/without controls (see T3), not guessed.
    3. Mandatory annotation — present ON the statement, quoted below.
    4. Layer-1 + layer-2 cheat-reviewer — owed; this vet is their input.
  BYTE-NEUTRALITY: the re-store materialises zero instructions —
  build_insns 144 == target_insns 144 at honest sandbox score 0.

  FAMILY: named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]]"
  PRECEDENT: .claude/rules/no-new-park-categories.md:189
  (Claimed for `num1` only — the entry's stated qualification is "once-written,
  once-read ... real value", and `num1` is written once and read once, holding
  the real numerator `(neg_b + disc) << 8`. `ud` and `flag` are NOT claimed under
  this entry: `ud` has two live reads and `flag` has two writes, so neither fits
  the once-written/once-read shape — they stand as ORDINARY C on their own
  semantics (T1.2 / T1.3), not as matching levers.)

  FAMILY: variable reuse for codegen control
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: .claude/rules/no-new-park-categories.md:170
  (Quoted across the rule file's line wrap at :170-:172. Claimed for `disc`,
  which successively carries the discriminant, its square root and the `<<9`
  result — mirroring target's $a2. Every one of those stores is live and read.)

ANNOTATION-CONFORMANCE:
/* FAKE: redundant same-value re-store of the LOCAL `ud`
 * (it already holds `disc` on entry to this arm), mechanism:
 * cse.c make_regs_eqv - with a SINGLE def cse puts `ud` and
 * `disc` into one quantity, rewrites the post-join
 * `ud >> shift` read to disc's register and DELETES the copy
 * insn outright (measured control v6_nodup: score 3, no
 * `addu $a0,$a2`). The second def makes the pseudo
 * multiply-defined across the join, invalidating that
 * equivalence, so both copies survive cse and global_alloc
 * lands them in target's $a2/$a0 pair; the redundant store
 * itself is dropped before final output (build_insns 144 ==
 * target_insns 144, zero emitted bytes).
 * lever-exhaustion: memory/grind/func_8002D518/hypotheses.md
 * + rejected/ (s1-s9: 32 rejected forms, 33,881 permuter
 * iterations; every plain-C placement of the copy measured
 * folding or costing insns). */
Emitted verbatim in src/code6cac_b.c immediately above the `ud = disc;` statement
inside the `if (disc >= 0)` arm. It carries all three required elements: WHAT
(the redundant same-value re-store of a local), MECHANISM (a named GCC pass,
cse.c `make_regs_eqv`, attributed from the cc1 dumps), and LEVER-EXHAUSTION (the
ledger path). No other construct in the diff carries or needs a FAKE annotation,
and no duplicated-statement-into-arms claim appears anywhere in the source or in
this vet.
