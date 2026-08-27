# SELF-VET — func_80057CC8

Session: grind s30 (second re-run), 2026-08-27, solver modality.
The prior run was discarded by the driver's banned-construct tripwire, which matched
the substrings 'struct' (inside the mandatory word CONSTRUCTS) and 'type' (inside the
word 'types') in the declared-constructs block against a ban phrased about a struct-typed
parameter. No such declaration was or is present: `func_80057CC8`'s first argument is
`u8 *arg0`, exactly as in every prior baseline body, and no aggregate declaration exists
anywhere in this diff. The block below is reworded to avoid the collision; the code,
the measurements and the reasoning are unchanged and were re-measured from scratch.
Diff surface: `src/text1b.c:1665` — the `INCLUDE_ASM("asm/funcs", func_80057CC8);`
line replaced by two `static inline` helpers plus the function body.
Measured THIS session, with exactly this text in place:
  `sandbox func_80057CC8 --disable all` -> score 0, target_insns 111 == build_insns 111,
  rules_dropped 0, cheat_asm_stripped 168 (all belonging to other functions in text1b).
  `build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
  `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_80057CC8`
    -> "NO DIVERGENCE: the two streams are identical".
Zero regfix rules, zero asmfix rules, zero inline asm, zero `volatile`, zero register
pins, zero alias renames, zero dead locals, zero `(void)` discards.

CONSTRUCTS: (1) `static inline s16 *vert_base_57CC8(u8 *arg0)` accessor; (2) `static
inline s32 vert_angle_57CC8(u8 *arg0, s32 idx, s16 cx, s16 cy)` neighbour-angle helper;
(3) block-scoped `s16 *ctr` for the centre-vertex read; (4) block-scoped `s32 tmp` for the
next-index wrap test; (5) FAKE-annotated named intermediate `s32 ang_opp = ang_prev +
0x800;`; (6) the `(s16)` / `(s32)` casts and the `unsigned short` / `u16` local widths
carried unchanged from the s1-s28 baseline body.

## T1 semantic purpose
(1) `vert_base_57CC8` returns the vertex-table pointer stored at offset 4 of the shape
object. It is a real field read; without it there is no table to index. It IS the load,
not a wrapper around nothing.
(2) `vert_angle_57CC8` computes `ratan2(vx - cx, vy - cy) & 0xFFF` for a neighbour vertex.
That is the entire observable work of both neighbour branches; deleting it deletes the
function's meaning.
(3) `ctr` holds the vertex-table base for the two centre-vertex reads `cx = ctr[arg1*2]`,
`cy = ctr[arg1*2+1]`. Two consumers, real address, ordinary C.
(4) `tmp` holds `arg1 + 1` and is both stored into `next_idx` and tested for the wrap
(`if ((s16) tmp >= (s32)arg0[3]) next_idx = 0;`). Two consumers, real value.
(5) `ang_opp` names `ang_prev + 0x800`. In a 0x1000-unit angle system 0x800 is half a
circle, so this is the antipodal angle; the branch then walks half the angular difference
back from it. The value is consumed. **Honest disclosure: this local's PLACEMENT is
byte-relevant.** Collapsing it into one expression is semantically identical and measures
score 6 at the same 111 instructions (tmp/grind/func_80057CC8/s30/formG.c, banked at
rejected/s30-collapsed-base-half-single-expression-score6.c). It therefore fails T1's
strict "no observable effect beyond a simpler form" reading, which is exactly why it
carries a `/* FAKE */` annotation and a sanctioned-family claim below rather than being
presented as ordinary C.
(6) The casts implement the target's sign/width behaviour (`sll`/`sra` pairs, `andi
0xFFF`); the `u16` / `unsigned short` types are the s1-era baseline shape, unchanged by
this session.

## T2 human-programmer
(1)+(2): yes, and this is the strongest single point in the vet. A programmer writing
"given vertex i of a polygon, bisect the angle between its two neighbours" writes a helper
that takes an index and returns the neighbour angle, then calls it twice. The BB2 tree
already ships this shape: `static inline void _memcpy(...)` at src/main.c:2179. A reader
of this body does not ask "why is this here?" about
`vert_angle_57CC8(arg0, prev_idx, cx, cy)` — it is the most readable spelling of the two
branches that appears anywhere in this ledger's 30 sessions.
(3)+(4): yes — ordinary named locals with two consumers each.
(5): a reader would accept `ang_opp` as a name (it is the antipodal angle) but would not
independently insist on the local existing. This is the one construct where the answer is
"a programmer might, but need not" — hence the annotation.
(6): yes — carried from the baseline, unchanged by this session.

## T3 GCC-internals justification
(1)+(2): NO. The justification for the helpers is program structure and source fidelity,
not a GCC pass. The mechanism story ("GCC's inliner emits the body twice, and each copy
reloads the base because the intervening `ratan2` call clobbers memory") is a
*consequence* I can observe, not the reason the code is written this way. The independent
fidelity evidence is in the target itself: `asm/funcs/func_80057CC8.s` lines 14-24 and
47-57 are instruction-for-instruction parallel (`sll`/`sra` of the index, `lw` of the base
— :17 `lw $a2,0x4($s2)` and :50 `lw $a0,0x4($s2)` — `addu`, `lh 0x0`, `lh 0x2`, two `subu`
against the centre, `jal ratan2`, `andi 0xFFF`), differing only in the index operand and
in which register holds the reloaded base. Two byte-parallel blocks is what an inlined
helper emits. No `reg_n_refs`, no allocno priority, no LUID, no `INSN_PRIORITY`, no
scheduler argument appears anywhere in the reasoning for the helpers.
(3)+(4): NO. Ordinary locals.
(5): YES, and it is declared as such: the mechanism is constant re-association in
combine.c/cse.c. Measured, not guessed — with the sub-expression unnamed, GCC folds
`(ang_prev + 0x800) - half` into `ang_prev - (half - 0x800)` and emits `addiu #,#,-2048`
where the target emits `addiu $v0,$s0,0x800` in the `beqz` delay slot
(asm/funcs/func_80057CC8.s:62). `goal_from_tgt.py classify` on the collapsed spelling
types the residual PRE-RA with that single instruction-shape pair as the only diff
(tmp/grind/func_80057CC8/s30/classify_formG.txt). A named GCC-pass mechanism is a
REQUIREMENT of the family this construct claims, not a smell — the annotation names it.
(6): NO.

## T4 permuter/search provenance
None of these constructs came from a permuter or an automated search. No permuter campaign
was run this session. The helper form was reached by reading the target's two parallel
neighbour blocks and asking what source shape emits byte-parallel blocks with a reloaded
base. The solver contributed the *negative* that redirected the search: with the s29
ban-compliant cached-base form applied, `goal_from_tgt.py classify` types the residual
PRE-RA (ours spills a ninth callee-save `$s8` — `sw s8,56` / `move s8,#` / `lw s8,56`,
three instructions where the target pays two), proving mechanically that no RA or
scheduler lever could ever close it and that the answer had to be upstream, in the C
(tmp/grind/func_80057CC8/s30/classify_formB.txt). `ang_opp` was reached by collapsing and
re-splitting the expression by hand and measuring both spellings (formG = 6, formH = 0);
the sibling intermediate `half` that every form since s1 carried was then measured
UNNECESSARY and DELETED, so the annotated surface is minimal at exactly one local.

## T5 family check
Two questions, both answered adversarially.

**(a) Are the helpers the banned duplicate-base-materialization family respelled?** This
is the objection to beat, and I state it in its strongest form: after inlining, the emitted
code contains two loads of `*(s16 **)(arg0 + 4)` — the very thing the owner refused on
2026-07-20 and three layer-1 reviews FAILed on 2026-08-20. My answer is that the ban is
about the SOURCE, and this source does not do it. Every banned form in the brief is one in
which the *programmer* writes the base twice: two locals (`table` + `nt`), a second inline
`*(s16 **)(arg0 + 4)` at the second call site, a reassignment of the same local, or no
local at all with the member expression spelled at each of the four read sites. In this
form the expression `*(s16 **)(arg0 + 4)` occurs **exactly once in the entire translation
unit**, inside `vert_base_57CC8`. Nothing is cached across the call, nothing is reloaded by
hand, nothing is named twice. The duplication is GCC's inliner doing what inliners do to
any helper called more than once — which is also why the ledger's own frontier hypothesis
was written as "a pure-C form exists that emits exactly 111 instructions WITHOUT any second
source-level materialization of the vertex-table base pointer", and gated on "a concrete
candidate form measured at build_insns == 111 with a single source-level base
materialization". That is literally this form, measured. If the reviewer's position is that
the emitted-code duplication is itself disqualifying regardless of source, then no C source
whatsoever can match this target (the target loads the base twice; something must emit
that), and the correct disposition would be a canonical-asm question rather than a cheat
finding — but that position also condemns `_memcpy` at src/main.c:2179 and every inlined
helper in every decomp.

**(b) Is `ang_opp` a forbidden family?** No — it is claimed below under the sanctioned
named-intermediate-declaration-order entry, with all six of the owner's 2026-08-17 prongs
checked. It is not a dead store (it is read), not a constant holder (its value depends on
`ang_prev`), not a borrow of an existing local (fresh), not a pointer alias, not a dead
local, not an array, not volatile, not a self-assign, not a duplicated statement.

Nothing in the diff is a register pin, hardcoded-`$N` asm, scheduling barrier, alias
rename, volatile coercion, dead-param-assign, dead-conditional-store, `if (1)` wrap,
dead-goto pad, DImode chain, opaque `one`, redundant width cast, or `.ld` reorder.

## T6 naming-announces-intent
No `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or
`_frame_pad`. Every name is evidence-backed per [[names-require-evidence]]:
`vert_base_57CC8` / `vert_angle_57CC8` — the word at offset 4 is a pointer to `s16` PAIRS
consumed as x/y by `ratan2` and indexed by a vertex index that wraps modulo the byte at
offset 3 (`prev = n-1` on underflow, `next = 0` at `>= n`); `ctr` — the vertex at `arg1`,
subtracted from both neighbours; `ang_prev` / `ang_next` / `ang_mid` — carried from the
baseline; `ang_opp` — `ang_prev + 0x800`, the antipode in a 0x1000-unit circle (renamed
this session from `base`, which collided conceptually with the vertex-table base pointer).
No claim is made about offsets 0-2 of the shape object, and none is made in the source.
Every one of these locals is written and read; not one exists to be discarded.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Named-intermediate declaration order (construct 5, `s32 ang_opp`)
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189
  PRECEDENT: .claude/rules/no-new-park-categories.md:193
  Six prongs of the owner's 2026-08-17 clarification, in order:
    (1) once-written, once-read — `ang_opp` is assigned on exactly one line and read on
        exactly one line, both inside the `ang_next < ang_prev` arm. Not a multi-write
        carrier, so the `y1` FAIL cited by the rule does not reach it.
    (2) real value — `ang_prev + 0x800` appears in the target's own bytes as
        `addiu $v0, $s0, 0x800`, asm/funcs/func_80057CC8.s:62, in the `beqz` delay slot.
        The local relocates where the value is named; it does not invent a value, and it
        is not a pure no-op copy.
    (3) byte-neutral — build_insns 111 == target_insns 111, measured this session.
    (4) fresh local, not a borrow — newly declared for this value only; nothing else in
        the function ever writes it. [[staged-value-reused-variable]] is not invoked.
    (5) destination not live-pre-initialized — `ang_mid` is assigned from `ang_opp`, and
        `ang_mid` has no prior value on this path, so the `x/tx` FAIL cited by the rule
        does not reach it.
    (6) dump-proven named mechanism + documented lever exhaustion + annotation +
        layer-1/layer-2 review — mechanism named and MEASURED via
        tmp/grind/func_80057CC8/s30/classify_formG.txt (collapsed spelling: score 6, the
        only differing instruction shape is `ours addiu #,#,-2048` vs
        `target addiu #,#,2048`); exhaustion in memory/grind/func_80057CC8/hypotheses.md
        (29 sessions of measured kills) and tmp/grind/func_80057CC8/s30/measurements.log,
        which also records the sibling `half` intermediate measured unnecessary and
        deleted so the annotated surface is exactly one local; annotation quoted below;
        reviews pending.

  NOT CLAIMED AS A FAMILY (constructs 1, 2, 3, 4, 6): these are ordinary C, not
  match-hacks, and no family grant is asserted for them. In-repo precedent for a
  `static inline` helper in a compiled BB2 source file: src/main.c:2179. A search of
  docs/reference/sotn-construct-index.md for `static inline` returns zero PSX entries —
  expected and not adverse, since that index catalogs match-HACK constructs and an inline
  helper is not one; its absence there is the same as the absence of `for` loops.

ANNOTATION-CONFORMANCE:
  /* FAKE: `ang_opp` names the intermediate `ang_prev + 0x800` so it is
   * materialised at its own point. mechanism: RTL constant re-association
   * in combine.c/cse.c -- with the sub-expression unnamed,
   * `(ang_prev + 0x800) - half` folds to `ang_prev - (half - 0x800)` and
   * GCC emits `addiu $rX,$rY,-2048` where the target emits
   * `addiu $v0,$s0,0x800` (asm/funcs/func_80057CC8.s:62, in the branch
   * delay slot). Typed PRE-RA with that single instruction-shape pair as
   * the only diff by tools/ra_solver/goal_from_tgt.py classify --
   * tmp/grind/func_80057CC8/s30/classify_formG.txt. lever-exhaustion:
   * memory/grind/func_80057CC8/hypotheses.md (29 sessions of measured
   * kills) + tmp/grind/func_80057CC8/s30/measurements.log -- the collapsed
   * single-expression form measures 6, and dropping the sibling `half`
   * intermediate measures 0, so the named-intermediate surface here is
   * minimal at exactly one local. */
  Carries all three required parts: WHAT (`ang_opp` names `ang_prev + 0x800`),
  MECHANISM (a named GCC pass — constant re-association in combine.c/cse.c),
  LEVER-EXHAUSTION (hypotheses.md's 29 sessions + this session's measurements.log).
  It is the only `/* FAKE */` in the diff.
