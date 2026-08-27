# SELF-VET — func_80023648

Diff under vet: `src/code6cac.c`'s `INCLUDE_ASM("asm/funcs", func_80023648);`
replaced by the pure-C body in `memory/grind/func_80023648/candidate.c`.
Measured (s6, and RE-MEASURED unchanged in the s6b annotation-fix session
2026-08-26 with the corrected annotation set in place): `sandbox func_80023648
--disable all` → **score 0, target_insns 159, build_insns 159, rules_dropped 0**,
with all FOUR FAKE annotations in place in the source.

CONSTRUCTS: (1) staged value through the existing `a2` local — clamped
`|*(s16 *)(arg0 + 0x150)|`; (2) staged value through the existing `a2` local —
the `>>12` speed; (3) staged value through the existing `sub_result` local — the
second read of `*(s16 *)(arg0 + 0x1A)`; (4) index-first spelling of the
D_8008EB40 element address (`a2 = a1[row];` rather than `a2 = row[a1];`);
(5) the `new_var` → `row` two-step address split (pre-existing, inherited).

## T1 semantic purpose
1/2/3 — each staging assignment writes a REAL value that is READ by nearby code:
(1) the clamped absolute value is read by `sub_result = *(u16 *)(arg0 + 0x14E) - a2;`
on the next statement; (2) the `>>12` speed is read twice, by the 0xD8 and 0xE0
accumulates; (3) the second `0x1A` read is read by `mult_res = sub_result * tbl_val;`
on the next statement. Delete any of them and the function computes a different
result — none is a dead store, a self-assign, or a no-op. What the construct
chooses is only WHICH variable carries the value, not whether the value exists.
4 — `a1[row]` IS the load; removing it removes the table read. Standard C defines
`E1[E2]` as `*(E1 + E2)`, so `a1[row]` and `row[a1]` are the same value by
definition; the choice is operand order inside the address addition.
5 — `row` is dereferenced; `new_var` is the table base. Both hold real addresses.
No construct in this diff is behaviour-neutral filler.

## T2 human-programmer
1/2/3 — NO, a programmer writing from the spec would declare `abs_val`, `speed`
and a second scratch instead of restaging through one variable. This is why they
carry `/* FAKE: ... */` and are claimed under a sanctioned exception rather than
presented as ordinary code. The exception exists precisely because the original
1998 authors' compiler bookkeeping, not their intent, is what we are matching;
SOTN ships the identical shape (see PRECEDENT below).
4 — PARTLY. `a1[row]` is ordinary, cast-free, semantically-true C and the
reversed-subscript spelling is a long-standing idiom (it was chosen over the
equivalent `*(s16 *)((a1 << 1) + (s32)row)`, also measured 0, precisely because it
introduces no manual scaling and no casts) — but a programmer writing from the
spec would have written `row[a1]`, and this spelling is chosen purely for matching
(`&row[a1]` = 8 vs `a1[row]` = 0, hypotheses.md s6). It therefore carries a
`/* FAKE: ... */` mark, as the 2026-07-06 construct-honesty line requires of
purely-for-matching spellings of true C.
5 — YES; naming a table base and a row pointer is normal.

## T3 GCC-internals justification
Disclosed honestly rather than hidden: constructs 1-3 are justified by GCC 2.7.2
`global.c` — a multiply-set C variable is ONE pseudo, hence ONE allocno spanning
all of its live ranges, so `global_alloc` necessarily seats every staged value in
a single hard register. That is the mechanism named in each annotation and it is
the reason the exception is claimed under a sanctioned family with a FAKE
annotation instead of being passed off as ordinary C. Construct 4's mechanism is
front-end/RTL operand order for a commutative PLUS (`addu $d,$s,$t` emits the RTL
operands in order; GCC 2.7.2's `pointer_int_sum` canonicalises pointer+integer to
pointer-first, so the order is only C-visible via the reversed subscript). That
mechanism is named in its FAKE annotation; the construct is a C-source-visible
property of semantically-true C on the ALLOWED side of the 2026-07-06
construct-honesty line, not an allocator hack. Construct 5 needs no internals story.

## T4 permuter/search provenance
No permuter ran this session. Every construct was derived from a measured,
aligned instruction-level seat diff of the current residual
(`tmp/grind/func_80023648/s6/cmp.py` output, banked in the ledger), which named
the three register clusters and their exact target seats before any C was
written. Constructs 1-3 were then hand-written to that diff and each was measured
SEPARATELY (15 → 8 → 3 → 0). Two of the previous sessions' permuter-found
constructs (s4's `varA` div16 reuse and s4's `varL` `ent` pointer local) were
REMOVED this session after re-measuring them as no longer load-bearing — the diff
is strictly smaller than the inherited candidate, not larger.

## T5 family check
1/2/3 — "Variable reuse for codegen control", the first entry on the FROZEN
SOTN-accepted list, in its live-code form gated by
`.claude/rules/staged-value-reused-variable.md`. All six of that rule's bounds
hold: (i) each staged value is real and read by adjacent code; (ii) `a2` and
`sub_result` both already exist for real jobs (the D_8008EB40 table entry, and
the `0x14E − abs` difference) — neither was invented to be borrowed; (iii) the
borrow is provably safe — `a2`'s table-entry value is consumed by the
`func_8001F860` call before the first restage and never read again, `a2`'s
clamped-abs value is consumed by `sub_result = ... - a2;` before the second
restage, and `sub_result`'s difference is consumed by the `0x14E` store and by
`new_14e` before its restage; (iv) each carries a full FAKE annotation with what,
mechanism and lever-exhaustion; (v) last resort with receipts — five prior
sessions of measured lever exhaustion in `hypotheses.md`; (vi) adversarial review
still applies. No forbidden family is touched: no asm, no register pins, no
volatile, no alias renames, no dead stores, no dead locals, no scheduling
barriers, no `if (1)`, no `do{}while(0)`, no rule-file edits.
4 — semantically-true C whose spelling is purely-for-matching: the ALLOWED list
of the 2026-07-06 construct-honesty ruling (`.claude/rules/do-while-zero-exception.md:46`),
which sanctions "any spelling of semantically-TRUE C, whatever pass it nudges"
and requires the FAKE mark when the spelling is purely-for-matching — the mark is
present. It asserts no false program fact (no cross-symbol address derivation, no
cast, no volatile/alias handle), so it is not the 2026-07-05 semantic-lie shape,
and it is a member of no forbidden family.
5 — ordinary named intermediates over real address values.

## T6 naming-announces-intent
No `pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`, `tail` or similar. The
names present are `kind`, `new_var`, `row`, `bits`, `a0`, `a1`, `a2`,
`sub_result`, `div16`, `new_14e`, `tbl_val`, `mult_res`, `limit`, `speed_prod`,
`sin_val`, `cos_val` — all inherited from prior sessions and all naming real
values. `a0`/`a1`/`a2` are register-derived decompilation names, not coercion
markers; every one of them is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: variable reuse for codegen control — staged value through a reused
    variable (constructs 1, 2 and 3)
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: docs/reference/sotn-construct-index.md:51

  FAMILY: purely-for-matching spelling of semantically-TRUE C — the index-first
    subscript `a1[row]` (construct 4), FAKE-marked per the 2026-07-06
    construct-honesty ruling
  SCOPE: "**ALLOWED — any spelling of semantically-TRUE C**, whatever pass it nudges: do-while(0) wraps, split/redundant arithmetic (the SOTN-wiki `+ 1 - 1` class — note this places the 2026-07-05 double-split rejection's SPELLING half under the allowed side; its cross-symbol half stays forbidden under #5), variable reuse/staging, named intermediates, statement order, mixed exit forms. Marked with the FAKE convention (below) when purely-for-matching."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:46

ANNOTATION-CONFORMANCE:
  /* FAKE: the clamped |*(s16*)(arg0+0x150)| is staged through the existing `a2` (its D_8008EB40 table-entry value is dead here - it was consumed by the func_8001F860 call above and is never read again), mechanism: GCC 2.7.2 global.c - a multiply-set pseudo is ONE allocno spanning all of its live ranges, so global_alloc seats every staged value in a single hard reg ($a2) exactly as target does; separate locals form separate allocnos that find_reg seats in $a2/$a0/$a1, lever-exhaustion: memory/grind/func_80023648/hypotheses.md (s1-s5: structural axis, named-intermediate axis, two permuter basins) */
  /* FAKE: the second read of *(s16*)(arg0+0x1A) is staged through the existing `sub_result` (its 0x14E difference is dead here - consumed by the store above and by new_14e), mechanism: GCC 2.7.2 global.c multiply-set pseudo / single allocno as above, lever-exhaustion: memory/grind/func_80023648/hypotheses.md */
  /* FAKE: the >>12 speed is staged through the existing `a2` (its clamped-|0x150| value is dead here - consumed by sub_result above), mechanism: GCC 2.7.2 global.c multiply-set pseudo / single allocno as above, lever-exhaustion: memory/grind/func_80023648/hypotheses.md */
  /* FAKE: index-first element address `a1[row]` (identical value to `row[a1]` - C defines E1[E2] as *(E1+E2), so this is the same load), mechanism: GCC 2.7.2 RTL expansion emits the operands of the commutative PLUS in source order, so index-first flips the addu operand order and the element pointer lands in $a2 as target does - measured 8 -> 0, lever-exhaustion: memory/grind/func_80023648/hypotheses.md s6 */
  All four carry what + named GCC pass/mechanism + lever-exhaustion pointer. The
  fourth was added in the s6b annotation-fix session to cure the 2026-08-26 18:56
  Judge FAIL (annotation-format ground only); the code is byte-identical to the
  Judge-verified body and the floor was re-measured at 0 with it in place.
