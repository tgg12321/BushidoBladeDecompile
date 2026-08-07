# initDrawMode (src/gpu.c) — WIP

## TL;DR

Honest pure-C floor = 5 (the natural `cmd | val` form, pins removed). The
session-1 "floor 1" claim has been **REJECTED by cheat-reviewer (session 2,
2026-06-08)** — the `val | cmd` operand-reorder was enumeration-derived per
`.claude/rules/or-tree-shape-shift.md` (FAIL tests 3, 4, 5). HEAD still
carries the cheat-asm `register asm("v1")` + `register asm("v0")` pins on
cmd/val ([[inline-asm-policy]]). To reach COMPLETED-C the function needs a
legitimate pure-C lever that keeps $a3 alive past `val = a3 & 0x9FF` so
combine cannot fold val's andi destination into $a3 — without touching any
forbidden cheat-by-spelling family.

## Resume instructions

1. Apply `memory/wip/initDrawMode/candidate.c` to `src/gpu.c::initDrawMode`
   (replaces the 2-pin HEAD body with the natural pin-free `cmd | val` form).
2. `& tools/eng.ps1 sandbox initDrawMode --disable all` should report
   score 5 (target_insns=11, build_insns=11). This is the documented
   pure-C floor — same as HEAD's pin-stripped sandbox.
3. **Do NOT** flip the operand order to `val | cmd` to chase the lower score
   — that's the rejected form (see `rejected/val-or-cmd-operand-reorder.c`).
4. The diff (sandbox-stripped objdump vs `build/src/gpu.o`):
   - build emits `andi a3, a3, 0x9FF; or v0, v0, a3` (val folded into $a3)
   - target wants `andi v0, a3, 0x9FF; or v0, v1, v0` (val in $v0, source order cmd|val)

## The wall (refined per session-2 reviewer FAIL)

Target asm (build/src/gpu.o, canonical):
```
li      v0, 1
sb      v0, 3(a0)
beqz    a2, .L1
 lui    v1, 0xE100      ; cmd in v1
ori     v1, v1, 0x200
.L1:
beqz    a1, .L2
 andi   v0, a3, 0x9FF   ; val in v0, NEW dest (NOT a3 — the key)
ori     v0, v0, 0x400
.L2:
or      v0, v1, v0      ; cmd | val source order, val rt slot
jr      ra
 sw     v0, 4(a0)
```

The NATURAL `cmd | val` source order (candidate.c) emits:
```
andi    a3, a3, 0x9FF   ; val folded into a3 by combine
ori     a3, a3, 0x400
or      v0, v0, a3      ; or v0, v0, a3 — 5-byte diff cluster vs target
```

Combine's RTL substitution sees: `val = a3 & 0x9FF` produces a new pseudo,
a3 has no further uses after this insn, val's pseudo has one use (the final
OR). Combine substitutes `(set new_p (and a3 0x9FF))` -> `(set a3 (and a3
0x9FF))` (allocating new_p to a3's hardreg). The OR then reads val from $a3.

The `val | cmd` source order avoids the fold (val lands in $v0) but encodes
the OR's rs/rt slots as `or v0, v0, v1` instead of target's `or v0, v1, v0`.
That choice was enumeration-derived and cc1-internals-justified — the
cheat-reviewer FAILed it per [[or-tree-shape-shift]].

## What didn't work — session 2

See `meta.json::rejected_forms` (the new session-2 entries):
- `u16 a3` narrow type (Lever B): score 5, no progress
- `val |= cmd; *p = val;` compound assign: score 6, regress
- `cmd |= val; *p = cmd;` compound assign: score 5, same
- hoist `val = a3 & 0x9FF` before `a0[3] = 1`: score 6 (+1 insn), regress

None of these legitimate structural variants keep $a3 alive past val's andi
without emitting bytes.

## Next-step hypotheses

See `meta.json::next_hypotheses`. The key technical question now: is there
ANY pure-C construct that keeps $a3 alive past val's andi without:
- enumerating operand orderings ([[or-tree-shape-shift]]);
- a dead self-assign / dead conditional store of a3 (Lever D forbidden);
- a `(void)a3` / `(void)&a3` address-coercion ([[dead-vars-local-array]]);
- a volatile cast on a3 ([[inline-asm-policy]] expanded catalog);
- a chain-extender that bumps refs but emits bytes
  ([[register-alloc-pure-c]] §6 forbidden);
- a `__asm__` barrier ([[inline-asm-policy]]).

If the answer is "no" — i.e., target's bytes genuinely require an operand-
order choice that cannot be independently justified on program-logic grounds
— the function may need permanent park as a "commutative-OR operand-order
wall" surfaced to the user as a policy question (per the reviewer's
next_action), not as a new cheat-park category.

## Related rules

- [[or-tree-shape-shift]] — the rule the session-1 candidate violated; SESSION-2 PRIMARY
- [[register-alloc-pure-c]] — the parent RA-via-C-structure playbook.
- [[inline-asm-policy]] — HEAD pins are cheat-asm; function cannot be COMPLETED-C with them.
- [[difficult-is-not-impossible]] — but stuck != unfinished work; the matching
  C may not exist within sanctioned pure-C constructs for this function.
- [[no-new-park-categories]] — keep parked + WIP; do NOT propose a new
  "commutative-OR wall" auto-park category. If permanent park is needed,
  surface as a policy question to the user.
- [[review-discipline-before-commit]] — invoke cheat-reviewer BEFORE saving
  any candidate as candidate.c going forward (session-1 skipped this; the
  FAIL caught the cheat in session 2).
