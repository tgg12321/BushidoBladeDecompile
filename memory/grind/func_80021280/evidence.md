# Evidence bank — func_80021280

- WIP rejected_form: shadow-redeclare a1 inside inner block (s32 a1 = 0; shadowing outer) -> new pseudo, no priority boost, floor back to 19

- WIP rejected_form: hoist t1/t4/t3/t2/mode/t0 to function-scope decls-without-initializer + plain assignments (to legalize a1=0 first) -> floor 19 (loses the nrefs-priority merge benefit)

- WIP rejected_form: single nested sub-block { a1=0; { t1..t0 decls; loop } } -> floor 19

- WIP rejected_form: fully inline mode/t0/constants directly in loop body (no named locals at all) -> floor 41, build_insns 75 (GCC does NOT auto-hoist the global reads through the aliasing a2 pointer stores; recomputes per-iteration)

- WIP rejected_form: convert loop2 to `for (a1=0; a1<3; a1++)` with continue instead of goto/label -> floor 34

- WIP rejected_form: reorder t1/t4/t3/t2/mode/t0 declaration permutations (t1 last, mode/t0 first, t2/t3/t4 swapped, etc.) -> floor stays 2 or gets worse (3-6), never better

- WIP rejected_form: redundant reload for t1 (`*(u16*)(a2+0x48)` instead of `= val`) -> floor 23

- WIP rejected_form: widen t1 to s32 -> floor 4

- WIP rejected_form: comma-operator stage `u16 t1 = (a1 = 0, val);` to force a1=0 first -> NOT TRIED (rejected as unnatural/cheat-shaped before testing; avoid)

- == imported from memory/wip notes.md ==
# func_80021280 — WIP TL;DR

**Floor: sandbox distance 19 (HEAD) -> 2 (this candidate).** Not yet closed to 0.

## What closed 19 -> 2
The function has TWO loop counters: the outer `a1` (used across the whole
function body, incl. loop1) and, originally, a fresh `s32 i = 0;` for a second
nested loop (loop2). Replacing `i` with a reuse of the existing `a1` variable
(same identifier, reassigned `a1 = 0;` right before loop2, used throughout
loop2's body in place of `i`) fixed a GLOBAL register-allocation priority
mismatch: originally the pointer variable `a2` and the counter `a1` had their
physical registers SWAPPED relative to target (target wants counter in the
lower-numbered reg, pointer in the higher). Reusing `a1` boosts its combined
reference count (nrefs) enough to flip the allocator's low-register preference
in `a1`'s favor, matching target. This is the SOTN-sanctioned variable-reuse
family — cheat-reviewer PASSed it (see meta.json).

## What's left: a 2-instruction (really ~4-slot) scheduling reorder
Deep-dived via `cc1 -da` RTL dumps on a standalone repro (see meta.json
`root_cause` for the full mechanism). Short version: GCC 2.7.2's `loop.c`
relocates the loop-2 induction variable's zero-init (`a1 = 0;`) to a fixed
point relative to two other relocated instructions (loop-invariant global
reads `mode = D_800A38DC;` and `t0 = D_800A384C;`), via two DIFFERENT
relocation mechanisms that fire at different times during the loop scan.
Result: final order is `t1, t4, t3, t2, a1, mode, t0` — target wants
`a1, t1, t4, t3, t2, mode, t0` (a1 needs to be FIRST, not fifth).

Since `t1/t4/t3/t2/mode/t0` are all declared-with-initializer LOCALS in the
same C block as the `a1 = 0;` reassignment, C89 requires all declarations to
precede all statements in that block — so `a1 = 0;` (a plain reassignment,
not a declaration) can never be moved ahead of them in the source text
without either illegal C or an unnatural coercion (rejected, see meta.json).

**~10 variant reorderings/type changes were tried (see meta.json
`rejected_forms`) — none beat floor 2.** The next agent should NOT re-try
those. Promising untried avenues are listed in meta.json `next_avenues`
(directed permuter seeded from this floor; reading `tools/gcc-2.7.2/loop.c`
directly for the biv/movable relocation call sites; restructuring the very
first loop-body statement that triggers the biv scan).

## Resume instructions
1. Apply `candidate.c` to `src/code6cac.c` (replaces `func_80021280`, currently
   at line ~2414).
2. Confirm floor: `& tools/wteng.ps1 <target> sandbox func_80021280 --disable all`
   should read `"score": 2`.
3. Continue from `next_avenues` in meta.json. Do NOT re-derive `rejected_forms`.
4. On close to 0: `retire func_80021280`, verify SHA1, cheat-reviewer PASS
   (already have one for the current candidate.c form — get a FRESH one if the
   final closing form differs), then delete this WIP dir.

