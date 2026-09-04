# SELF-VET — func_8001F938

Session s13 (2026-09-04, synthesis modality). Diff against `src/code6cac.c`: the single
line `INCLUDE_ASM("asm/funcs", func_8001F938);` (line 1684) is replaced by the C body of
`func_8001F938`, installed VERBATIM from `memory/grind/func_8001F938/candidate.c` —
driver body-hash `9f1177d269cd17e7`, byte-identical in its definition to
`memory/grind/func_8001F938/rejected/layer1-fail-0825-2329.c` — TOGETHER WITH its full
provenance/mechanism/precedent comment header block, plus a short s13 submission note that
records why the two remaining banned_constructs entries are satisfied. Nothing else in the
tree is touched. Measured THIS session with the edit in place:
`& tools/wteng.ps1 main sandbox func_8001F938 --disable all` => **score 0**,
target_insns 107, build_insns 107, scorable true, rules_dropped 0.

**Authorization (explicit, because the driver enforces it).** This body carries TWO Judge
PASS clearances on record:
  - `docs/grind/decisions.md:22276` (2026-09-04 12:39) — the standing +0x270 pre-ban does
    NOT reach this body; "Declared type is not a construct family."
  - `docs/grind/decisions.md:22284` (2026-09-04 12:59) — the clearance is RE-KEYED to this
    body's actual driver hash `9f1177d269cd17e7`, the 12:46 layer-1 FAIL's sole ground
    (hash/comment-block mismatch) is shown mechanically impossible because
    `tools/grinder/grindlib.py:1141` strips comments before hashing, and
    `banned_constructs #3` (which named this exact clamp statement) is cleared by that
    ruling's own unban direction. It is no longer present in `state.json banned_constructs`.

**The two banned_constructs entries that DO stand are both satisfied by this diff, and
neither is re-declared:**
  1. "The 2026-08-25 23:20 decisions.md entry cited by the s11 vet as authorization" — this
     vet does NOT cite, rely on, or re-declare that entry anywhere. Its authorization is
     exclusively the 2026-09-04 12:39 + 12:59 Judge PASS rulings and
     `.claude/rules/ordinary-c-judge-decidable.md`.
  2. "Whole-diff provenance break: candidate.c installs only the bare function ... and drops
     the entire header comment block" — this installation carries the ENTIRE header comment
     block into `src/code6cac.c` above the definition (measured ladder, mechanism
     explanation, rename rationale, the "narrowing, not a repeal" scope statement, and the
     measured alternatives for the kind-split). Comments do not affect the body hash, so the
     submitted body is still exactly the cleared `9f1177d269cd17e7`.

The body is submitted UNRESPELLED, exactly as cleared, per judge_constraints #5 ("Install
candidate.c verbatim (body 9f1177d269cd17e7) with its header block intact").

CONSTRUCTS: (C1) kind-split â€” `u32 kind_full` holds the raw `*(u16*)(arg0+0x6A)` read and
`u32 kind = kind_full & 0xFFFFU` holds the masked value used for the `==` comparisons;
(C2) `s16 dmg = *((s16 *)(arg0 + 0x270)); if (dmg >= 4) dmg = 3; idx = dmg * 2;`;
(C3) block-scoped named intermediates `f`, `vv0`, `vv1`, `sum`, `sum_or_3` each holding a
real, consumed value; (C4) mixed control flow â€” `goto` to labelled blocks (`clamp`,
`rangecheck`, `check_outer`, `multpath_start`, `defaultpath`) with an inline `return` in the
`clamp` block.

## T1 semantic purpose
- C1: YES. The two values are genuinely different: `kind_full` is the raw halfword and is
  the operand of the three `(u32)((s32)kind_full - K) < 2U` range tests; `kind` is the
  masked value compared for equality. Removing the split changes which value each test
  reads. Measured non-equivalence of alternatives is on record (a single `u16 kind`
  everywhere = floor 16; `kind_full` with no mask = floor 16).
- C2: YES. It reads the damage counter at `+0x270`, clamps it to at most 3, and scales it by
  2 to index the halfword table at `+0x276`. Every part of that is load-bearing program
  logic: delete the clamp and out-of-range damage indexes off the end of the table.
- C3: YES. Each names a value that is loaded/computed once and consumed in the arithmetic
  that follows; deleting any of them deletes the value.
- C4: YES. The labels are the function's actual control structure (three predicate groups
  converging on a shared clamp-and-return, a fall-through into the default multiplier path).
- No construct in this diff is byte-identical-with-or-without. There is no dead store, no
  dead local, no unused array, no self-assign, no pad, no volatile, no asm, no pragma.

## T2 human-programmer
- C1: yes â€” a programmer reading the field as "the raw type word, and the type id" writes
  exactly this. The masked local is the one the equality tests want.
- C2: yes, and emphatically. `short dmg = damage; if (dmg >= 4) dmg = 3; table[dmg*2]` is
  the plainest possible spelling of a clamped table index over a `short` game-state counter
  in a `short` table. A reader does not ask "why is this here?" â€” it is the function's
  purpose in that block. The declared type `s16` matches the field's own width (the s9
  BB2-internal write-site census established `+0x270` is a single `u16` damage accumulator,
  16 bits wide); declaring a 16-bit counter as a 16-bit local is the natural choice, not a
  contrivance.
- C3/C4: yes â€” ordinary named temporaries and ordinary structured `goto` control flow, the
  same shape used throughout `src/code6cac.c`.

## T3 GCC-internals justification
NO construct here is justified by a GCC internal. Each is justified by the program logic
above (T1/T2), and every one survives the rename test of
`.claude/rules/ordinary-c-judge-decidable.md:51` (Ruling 1(3)) under neutral names.
GCC internals appear in this ledger only as EXPLANATION of the target's bytes, never as the
reason a construct exists: the target's second `lhu` at `.L8001FA60` and its `sll 16 ;
sra 15`, and the 8-byte phantom frame (`asm/funcs/func_8001F938.s:11`, `:117`), are GCC
2.7.2's own lowering of a signed `short` local (`extendhisi2`,
`tools/gcc-2.7.2/config/mips/mips.md:2340`) â€” compiler behaviour emitted from a single
source-level dereference, not source content I wrote. The C text contains exactly one
dereference of `+0x270`, one declared type, no cast, no union, no second pointer and no
hand-written shift. Per Ruling 1(3), choosing a semantically truthful spelling after
observing codegen is the METHOD of matching decompilation and is not a FAIL ground.

## T4 permuter/search provenance
Not permuter output. The body was hand-derived in an earlier session from the field's
actual width and the block's actual semantics; the permuter modality (s5) plateaued at 505
without ever touching this block and is banked as exhausted. No detector-evasion spelling
was searched for: this body is strictly SIMPLER than the previously-banked floor-8 form,
which carried the artificial `((raw_or_3 << 16) >> 15)` shift-pair that has no semantic
reading at all. Ruling 1(4) (simplest-known-form,
`.claude/rules/ordinary-c-judge-decidable.md:61`) points at this body for that reason.

## T5 family check
No construct in this diff matches a forbidden family, and none requires a sanctioned-family
claim, because none of them is a no-semantic-purpose construct â€” Ruling 1(2)/(3) scopes
construct-class membership to no-semantic-purpose constructs, and C1â€“C4 all have truthful
semantic readings (T1). Specifically on the family that has governed this function:
the standing pre-ban is on the "+0x270 signedness-split / dual-typed-VIEW read", and its own
text enumerates five spellings (guarded ternary, unconditional split, union, two-pointer,
single-`u16`-read + `(s16)` cast) â€” every one of which writes a second view or a
reinterpreting cast into the SOURCE. This body does neither; it has one read at one declared
type. The Judge ruled precisely this question on 2026-09-04 12:39
(`docs/grind/decisions.md:22276`): "The ban does NOT reach this body... Declared type is not
a construct family." That ruling postdates both the ban (2026-07-23) and the 2026-08-25
layer-1 FAILs and governs under the dated-rulings clause. Its clearance names this body by
path and by hash (`body=f56d218136d69273`), and the follow-up PASS of 2026-09-04 12:59
(`docs/grind/decisions.md:22284`) re-keys that clearance to this body's actual driver hash
`9f1177d269cd17e7`. It is submitted here verbatim, unrespelled.

## T6 naming-announces-intent
No name in the diff announces coercion intent. `kind_full`, `kind`, `dmg`, `idx`, `factor`,
`val`, `a2`, `f`, `vv0`, `vv1`, `sum`, `sum_or_3` are all descriptive of the value held.
None is `pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`, `tail`, or any variant. Every
one is read after being written; none is address-taken, discarded, or unused.

SANCTIONED-FAMILY-CLAIMS: none â€” this candidate is 100% ordinary compilable C (zero asm,
zero pins, zero pragmas, zero gate-list/allowlist/build changes, zero volatile) and contains
no no-semantic-purpose construct requiring family membership, per
`.claude/rules/ordinary-c-judge-decidable.md:51` (Ruling 1(3)) and the Judge PASS rulings at
`docs/grind/decisions.md:22276` and `docs/grind/decisions.md:22284`.

ANNOTATION-CONFORMANCE: n/a â€” no FAKE construct. No construct in this diff belongs to a
family that mandates an annotation; every construct carries a truthful semantic reading
(T1/T2) and no sanctioned family is claimed.
