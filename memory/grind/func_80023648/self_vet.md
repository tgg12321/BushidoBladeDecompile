# SELF-VET — func_80023648

Diff under vet: `src/code6cac.c`'s `INCLUDE_ASM("asm/funcs", func_80023648);`
replaced by the pure-C body in `memory/grind/func_80023648/candidate.c`.
Measured this session: `sandbox func_80023648 --disable all` → **score 0,
target_insns 159, build_insns 159, rules_dropped 0**, with the FAKE annotations
in place in the source.

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
4 — YES. `a1[row]` is ordinary C, cast-free, no hand-scaled index, and the
reversed-subscript spelling is a long-standing idiom. It was chosen over the
equivalent `*(s16 *)(a1 * 2 + (s32)row)` (also measured 0) precisely because it
introduces no manual `* 2` scaling and no casts.
5 — YES; naming a table base and a row pointer is normal.

## T3 GCC-internals justification
Disclosed honestly rather than hidden: constructs 1-3 are justified by GCC 2.7.2
`global.c` — a multiply-set C variable is ONE pseudo, hence ONE allocno spanning
all of its live ranges, so `global_alloc` necessarily seats every staged value in
a single hard register. That is the mechanism named in each annotation and it is
the reason the exception is claimed under a sanctioned family with a FAKE
annotation instead of being passed off as ordinary C. Construct 4's mechanism is
front-end/RTL operand order for a commutative PLUS (`addu $d,$s,$t` emits the RTL
operands in order); this is a C-source-visible property, not an allocator hack,
and it is the addition analogue of the project's already-ordinary
`compare-operand-order-register` technique. Construct 5 needs no internals story.

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
4 — ordinary C operand order; the closest catalogued technique is
`.claude/rules/compare-operand-order-register.md`, which the project treats as
ordinary C requiring no annotation. Not a member of any forbidden family.
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

  FAMILY: operand-order choice (construct 4) — ordinary C, no exception claimed,
    listed here only for completeness
  SCOPE: "Reversing a comparison's operand order (write `local > GLOBAL` instead of `GLOBAL < local`) can flip which register cc1 picks for the local; one source edit retires a `$X <-> $Y` regfix rule cluster"
  PRECEDENT: .claude/rules/compare-operand-order-register.md:3

ANNOTATION-CONFORMANCE:
  /* FAKE: the clamped |*(s16*)(arg0+0x150)| is staged through the existing `a2` (its D_8008EB40 table-entry value is dead here - it was consumed by the func_8001F860 call above and is never read again), mechanism: GCC 2.7.2 global.c - a multiply-set pseudo is ONE allocno spanning all of its live ranges, so global_alloc seats every staged value in a single hard reg ($a2) exactly as target does; separate locals form separate allocnos that find_reg seats in $a2/$a0/$a1, lever-exhaustion: memory/grind/func_80023648/hypotheses.md (s1-s5: structural axis, named-intermediate axis, two permuter basins) */
  /* FAKE: the second read of *(s16*)(arg0+0x1A) is staged through the existing `sub_result` (its 0x14E difference is dead here - consumed by the store above and by new_14e), mechanism: GCC 2.7.2 global.c multiply-set pseudo / single allocno as above, lever-exhaustion: memory/grind/func_80023648/hypotheses.md */
  /* FAKE: the >>12 speed is staged through the existing `a2` (its clamped-|0x150| value is dead here - consumed by sub_result above), mechanism: GCC 2.7.2 global.c multiply-set pseudo / single allocno as above, lever-exhaustion: memory/grind/func_80023648/hypotheses.md */
  All three carry what + named GCC pass + lever-exhaustion pointer.
