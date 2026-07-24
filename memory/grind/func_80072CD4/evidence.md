# Evidence bank — func_80072CD4

- s1 [recon] CONFIRMED: distinct `0xFC` literal at @0x14 (vs shared fc_const holding @4/@0xC)
  defeats CSE and forces the target's extra 0xFC re-materialization. build_insns 77→78,
  sandbox 20→18. Pure C (real value), not a cheat. Grounds H1 in hypotheses.md.
- s1 [recon] Matched sibling func_80072BC4 (COMPLETED-C) byte-matches via `int fc_const=0xFC;`
  hoisted before the outer if. In 72BC4 no var_v0 store sits between the fc_const stores, so 0xFC
  stays in one reg (no re-mat). In 72CD4 the @0xE var_v0 store SPLITS the fc_const uses → target
  re-materializes 0xFC in $v0 at @0x14. That structural split is why 72CD4 is harder than 72BC4.
- s1 [recon] Best LEGITIMATE (non-cheat) form measured = sandbox 18 (build_insns 78), saved as
  candidate.c. The imported floor=12 depends on the reviewer-FAIL empty-do-while barrier form
  (rejected/plus4_first_dowhile.c) — it is NOT a legitimate pure-C floor.
- s1 [recon] clean sibling-mirror with @0x14=fc_const (shared) → sandbox 20, build_insns 77
  (2 insns short; CSE folds every 0xFC into one $v1). KILLED as the closing form.


- WIP rejected_form: {'form': 'Drop do-while barriers + new_var, write 0xFC/var_v0 literals, reorder THEN+ELSE stores to target order', 'score': 16, 'reason': 'Removing the barriers drops insn count to 77 (target 79) and diverges structurally; clean-C is FARTHER, not closer.'}

- WIP rejected_form: {'form': 'new_var = 0xFC assigned BEFORE the inner if (to occupy $v1 early)', 'score': 14, 'reason': 'GCC re-materializes 0xFC anyway; var_v0 stays in $v1. Worse, not a cheat per se but measured-negative.'}

- WIP rejected_form: {'form': '+4-before-0x14 store order + two do { } while(0) barriers', 'score': 11, 'reason': 'Best measured, but carries empty do-while scheduler barriers (cheat-reviewer test #1 FAIL). Preserved under rejected/plus4_first_dowhile.c.'}

- == imported from memory/wip notes.md ==
# func_80072CD4 — register-rotation plateau (text1b.c GameObj setter)

## TL;DR (2026-06-14)
Honest distance 12 (9 regfix rules stripped). The entire gap is a coupled
**$v0<->$v1 register rotation** plus a 10-store ELSE-block reorder. Target
carries `var_v0` (0x46 / 0x32) in **$v0** and the `0xFC` constant in **$v1**
(preset into the inner-if beqz delay slot, re-materialized for the +0x14
store). My build does the inverse and GCC's global allocator will not flip it
with any clean-C lever I tried. **No clean candidate beats HEAD's 12** — the
forms that reached 11 carry empty `do { } while(0)` scheduler barriers
(reviewer FAIL). Blocked on the board; not a fake WIP.

## Resume steps
1. `sandbox func_80072CD4 --disable all` on HEAD = **12** (9 rules stripped).
2. The 9 rules in regfix.txt (lines ~3274-3304) do: 6 `subst` (register
   $2<->$3 swaps + store-offset rewrites), 1 `delete`, 1 lost-codegen
   `insert_after`, 1 ELSE `reorder`. They are ALL the register/order swap.
3. `.greg` of the baseline standalone: pseudo 74 (`var_v0`) -> reg 3 ($v1);
   the 0xFC values -> reg 2 ($v0). Confirmed the inversion is allocator-driven.

## Live hypothesis (best next lever)
- **Lever A on the OTHER pseudo:** the inner byte-store constants
  (0xC3/0x1E/0xC8) all cycle through $v0 right where var_v0 is born. Try
  forcing those constants through a shared local so they vacate $v0 and
  var_v0 can claim it. (Not yet tried — out of budget this pass.)
- Instrumented cc1 ALLOC/PRIO dump ([[register-alloc-deep-dive]]) to read the
  allocno priority tiebreak that pins var_v0 to $v1.
- If clean levers stay negative this is the register-alloc-pure-c
  "confirmed limits" class → canonical-asm review (user decision).

## Ruled out (do not re-derive)
- Drop do-while + literals + reorder THEN/ELSE → 16 (insn count drifts to 77).
- `new_var = 0xFC` before the inner-if → 14 (GCC re-materializes anyway).
- `var_v0` as `s32` (Lever B wide type) → 12 (no change).
- +4-before-0x14 store order WITH do-while barriers → 11 but reviewer-FAIL
  (empty `do{}while(0)` = cheat); saved under rejected/plus4_first_dowhile.c.

## Pointers
- Rule: `.claude/rules/register-alloc-pure-c.md` (Step-0 + Levers A/B/C; the
  "confirmed limits" section is the relevant precedent class).
- The simpler matched family (func_8006133C / func_80061454 / func_800614E0,
  same session) is the SAME $v0/$v1 tie but single-store and was closable with
  a walking-pointer; this one is coupled across a branch merge + ELSE reorder.


- [s1] canonical: verdict C, total 79, distance 12. sandbox --disable all = 12 (9 rules dropped, build_insns 79).

- [s1] Imported floor=12 depends on the reviewer-FAIL empty-do-while barrier form (rejected/plus4_first_dowhile.c); it is NOT a legitimate pure-C floor.

- [s1] Best legitimate (non-cheat) form measured this session = sandbox 18, build_insns 78 (saved as candidate.c).

- [s1] Target register map: $v1=0xFC preset in INNER beqz delay slot, used @4/@0xC; $v0=var_v0 (0x32 THEN/0x46 ELSE) stored @0xE; @0x14=0xFC re-materialized fresh in $v0. 79 insns.

- [s1] Matched sibling func_80072BC4 byte-matches via `int fc_const=0xFC;` hoisted before the outer if; it has NO var_v0 store between fc_const stores, so no re-mat is needed there — that structural split is why 72CD4 is harder.

- [s1] distinct-literal @0x14 confirmed the re-mat mechanism (77->78 insns).
