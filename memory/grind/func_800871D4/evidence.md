> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

# Evidence bank — func_800871D4

- WIP rejected_form: one-expr var_v1 = temp_a0 & 0xFFFF: GCC FOLDS the redundant andi (higher score)

- WIP rejected_form: two-stmt var_v1 = temp_a0; var_v1 &= 0xFFFF: andi appears but with extra move

- WIP rejected_form: declaration-order swap (var_a1 before var_a2): no effect on register assignment (score stays 10)

- == imported from memory/wip notes.md ==
# func_800871D4 — BLOCKED (cc1-vs-cc1psx andi-fold divergence CONFIRMED)

## TL;DR
Bit-flag setter in main.c: reads D_8010280A (u16), builds two shift masks, clears a
table slot, RMWs four flag globals. HEAD has 2 cheat-asm andi blocks + empty scheduling
barrier. cc1psx calibration (2026-06-16) CONFIRMS: cc1psx generates `andi $v1,$a0,0xffff`
from `var_v1 = temp_a0 & 0xFFFF` (no fold); our cc1 folds it. Best honest floor: score=10.

## Confirmed gap (10 Levenshtein diffs, oracle=52 insns, candidate=50 insns)
1. Oracle: `lui a0; lhu a0` — candidate: `lui v1; lhu v1` (2 load-register diffs)
2. Oracle has `andi v1,a0,0xffff` at pos 3 (missing from candidate, 1 delete)
3. Oracle has `andi v1,a0,0xffff` at pos 13 (restore after else, missing, 1 delete)
4. a1/a2 register swap throughout (6 diffs: var_a2 in a1, var_a1 in a2 in candidate vs oracle)

## cc1psx calibration results (2026-06-16)
- func_testA (temp_a0 + `& 0xFFFF`): cc1psx generates `andi $3,$4,0xffff` (NOT folded)
- func_testB (direct `var_v1 = D_8010280A`): cc1psx generates `lhu $3` (no andi, same as cc1)
- Divergence CONFIRMED: cc1psx does not fold andi from u16-bounded source; our cc1 does.

## Rejected forms
- `var_v1 = temp_a0 & 0xFFFF` (one-expr): our cc1 folds the andi completely
- `var_v1 = temp_a0; var_v1 &= 0xFFFF` (two-stmt): generates move+andi, not single andi
- declaration-order swap (var_a1 before var_a2): no effect on register assignment, score=10

## Best candidate (candidate.c, score=10, cheat-reviewer PASS)
Direct form: `var_v1 = D_8010280A` (no temp_a0, no mask). Semantics differ from oracle
in else-path (oracle restores var_v1 to original D_8010280A via second andi; direct form
leaves var_v1 = D_8010280A-16 for the post-if computation). Score is 10 (honest floor).

## Next step (Adjudicator)
Park with confirmed cc1psx divergence. The oracle requires cc1psx-specific behavior
(no andi fold from u16-bounded lhu) that our cc1 cannot reproduce in pure C.


- [s1] canonical func_800871D4 -> verdict=C, distance=10, total=52

- [s1] sandbox --disable all (candidate.c/direct form) -> score=10, target_insns=52, build_insns=50 (missing 2 insns = both andi $v1,$a0,0xFFFF)

- [s1] sandbox --disable all (m2c-shape with explicit &0xFFFF twice) -> score=12 (rejected/m2c-shape-explicit-andi-restore.c)

- [s1] Oracle structure: lhu $a0,%lo(D_8010280A); nop; andi $v1,$a0,0xFFFF; ...; else-arm ends with andi $v1,$a0,0xFFFF (re-materialize / restore). GCC keeps $a0 as master copy of loaded u16, $v1 as the masked working copy destroyed by -=0x10.

- [s1] m2c reconstruction shows explicit `var_v1 = D_8010280A & 0xFFFF` in the else arm (restore) — but this fold-defeats fail in our cc1 because RTL nonzero_bits proves the source is u16-clean.

- [s1] WIP inheritance already killed: (a) one-expr `temp_a0 & 0xFFFF` (folds), (b) two-stmt split (adds extra move), (c) decl-order swap (no effect).

- [s1] WIP cc1psx calibration (2026-06-16): cc1psx does NOT fold `temp_a0 & 0xFFFF` from u16 source — this is a fork-vs-cc1psx divergence. Per [[no-compiler-divergence]] this is informational only; cannot patch cc1.

- [s1] Cross-tree census (5 other BB2 funcs with the same shape, ALL INCOMPLETE) elevates this from single-function difficulty to a shared unresolved family across the codebase.

- [s1] Sibling func_80086BFC (main.c:1143, COMPLETED-C, PsyQ verbatim-linked note2pitch2) uses the same <16 / -=16 split-shift on a *s16* var_a3 with a local shift subexpression — no re-mask needed because s16 semantics differ.

- [s1] Function is a bitmask-flag setter with trivial control flow (single if/else + linear stores). Zero call sites in the function body. NO hand-coded-asm signals (S1/S2/S6 tier per scan_hand_coded); the construct is pure C in intent.

- [s2] [s2] sandbox --disable all with candidate.c (u32 var_v1, in-place -=0x10, zero cheat-asm) = score=10 target_insns=52 build_insns=50 (missing 2 andi insns) — re-confirmed s1's honest floor after applying candidate.c to src/main.c.

- [s2] [s2] sandbox --disable all with u16 var_v1 narrow-type variant = score=11 build_insns=52 (WORSE); banked rejected/u16-var-v1-narrow-type.c.

- [s2] [s2] scan_hand_coded --single func_800871D4 = tier=LOW score=0/8, no S1/S2/S6 strong signals; canonical-asm authorization decisively unavailable (Gate 1 FAIL).

- [s2] [s2] Cross-tree BB2 census (from s1): 5 sibling functions share the exact `lhu $r,GLOBAL; nop; andi $r2,$r,0xFFFF` shape, all INCOMPLETE; zero COMPLETED-C precedent in-tree (Gate 2 FAIL under standing 2026-07-27 ruling).

- [s2] [s2] cc1 combine.c folds `& 0xFFFF` on a u16-typed lhu result because nonzero_bits proves the source is 16-bit-clean; 2026-06-16 WIP calibration recorded cc1psx does NOT fold, but per no-compiler-divergence.md this is informational only (the compiler is frozen).

- [s2] [s2] Every non-cheat structural spelling of the initial read has been measured: direct u32 assign (floor=10, folds both andis), one-expr `temp_a0 & 0xFFFF` (folds), two-stmt split `var_v1=temp_a0; var_v1&=0xFFFF` (extra move, worse), m2c-shape explicit-andi twice (score=12), decl-order swap (no effect), u16 narrow-type (score=11).

- [s2] [s2] The remaining hypothesized levers from s1 frontier (H1 bitfield struct pun of &D_80102808; H2 alternative first-andi source crossed with in-place -=0x10) both reduce to a no-new-park-categories cheat-by-spelling: no semantic purpose in a bit-flag setter, GCC-internals-only justification, would not be written by a human programmer from the function's spec — not surfaced.

- [s2] [s2] Filed docs/grind/decisions.md entry `2026-07-28 — func_800871D4 — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE` documenting both-gate failure and terminal disposition.
