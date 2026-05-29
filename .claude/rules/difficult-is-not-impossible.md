---
name: difficult-is-not-impossible
description: A hard register-allocation/scheduling diff is NEVER a "fork wall" or "impossible" — the fork provably matches the original, so matching C exists. Difficult is not an excuse to give up.
paths: ["src/*.c", "regfix.txt", "asmfix.txt"]
---

# "Difficult" ≠ "impossible." A stubborn diff is unfinished work, not a wall.

## The rule

When a function plateaus on a few register-allocation / instruction-scheduling
diffs, do **NOT** conclude "fork-vs-cc1psx divergence," "irreducible," "compiler
wall," or "proven impossible." That conclusion is **almost always wrong**, and
reaching for it is the failure mode this rule exists to stop.

**Why it's wrong (settled, empirical):** `docs/diagnostics/compiler_parity.txt`
proves our decompals-gcc-2.7.2 fork reproduces the **original game byte-for-byte**
(full-build SHA1 == oracle), and that cc1psx is never closer. Therefore the
original compiler's output for *every* function — including the one you're stuck
on — **is reproducible by our fork from the right C source.** A non-match is a
**C-source problem** (typing, dataflow, statement order, register-steering
structure), never the compiler/fork/optimizer. The matching C *exists*; your job
is to find it, not to certify it impossible.

**Don't believe it? Run BOTH compilers and SEE (cc1psx is available).** The
original PsyQ compiler `cc1psx` is installed (`tools/cc1psx_wrapper.sh`, a drop-in
cc1 via dosemu2) precisely so you can settle this empirically instead of asserting
divergence. Preprocess the function, compile it with each, diff the asm:
```bash
# foo.i = preprocessed C (cpp ... src/<file>.c > foo.i, or reuse a sandbox .i)
tools/gcc-2.7.2/build/cc1 <cc1-flags> foo.i -o /tmp/ours.s          # our fork
tools/cc1psx_wrapper.sh   <cc1-flags> < foo.i > /tmp/psx.s          # PsyQ original
diff /tmp/ours.s /tmp/psx.s
```
You will find cc1psx does **not** beat our fork (it matches, or is worse — the
recorded sweep: 0/282 functions where cc1psx wins, 18 where it's worse). So
switching compilers can never turn your non-match into a match. Confirm it for
your function, then get back to finding the C. (No guard blocks this any more —
running cc1psx is allowed; it's a self-disproof tool, not a path forward when
stuck.)

**Why:** in the func_8003DBE4 session (2026-05-28) the orchestrator twice
declared a hard floor "proven impossible / fork wall" at honest distance 5 — once
from manual analysis, once after a 60k-iteration permuter run plateaued. Both
claims were wrong. The user pushed back ("was it proven, or are you struggling?"),
the RTL playbook + a permuter-found lever were applied, and the distance dropped
35 → 2 with pure-C structure (and kept going). "Difficult" had been used as an
excuse to stop.

**How to apply:** before you write "wall / irreducible / impossible / fork
divergence / can't be closed in pure C" about a register/scheduling diff, you owe
the full diagnosis below. If you haven't done all of it, you are *struggling*, not
*blocked* — say that honestly and keep going. Escalate for genuine **policy**
decisions (a global rodata reorder, user canonical-asm sign-off), never because a
pure-C diff is merely hard.

## The diagnosis you owe before any pessimistic claim ([[register-alloc-pure-c]])

1. **RTL `.greg` dump.** `cc1 <build-flags> -da base.i` → read `;; Register
   dispositions:` and the conflict lists. Find the value's pseudo, which hard reg
   it got, and **what it conflicts with**. (func_8003DBE4: `base_val` = pseudo 85,
   forced to `v1` because it *conflicts with hard reg 2 = v0* — the scheduler had
   materialized the constant in the jal delay slot, before the `bnez` consumed the
   call-return still living in v0. A pure scheduling-induced conflict, not a wall.)
2. **Apply the levers** ([[register-alloc-pure-c]] A–D): block-local var split,
   narrow integer type, loop-local precompute, and **dead-store / redundant
   computation to steer allocation**. The func_8003DBE4 fix was a *redundant dead*
   `step = base_val - arg0;` duplicated into one branch arm — it changed RA so the
   constant landed in `v0`, exactly matching target. A dead store that the
   optimizer keeps for its RA side-effect is legitimate pure C, not a cheat.
3. **Permuter — but with a CLEAN single-function target.** A whole-file `target.o`
   makes the function sit at a non-zero offset, so every internal branch/jump
   address mismatches and adds ~340k of constant score noise that *drowns the real
   signal* and makes the search useless. Build `target.o` from
   `asm/funcs/<func>.s` + `tools/decomp-permuter/prelude.inc` (drop the `.set
   gp=64` line for r3000) so the function is at offset 0 like `base.o`. Then the
   base score is the real weighted diff (e.g. 35 = 7 reg-diffs × 5) and the search
   actually works. Run with `--stop-on-zero`.
4. **m2c the target** for the *original structure*, but don't transcribe it
   literally — m2c shows the shape the original compiler produced, which our fork
   may reach from *different* C. Use it to understand dataflow/provenance, then
   find the our-fork-equivalent.

## Metric gotchas that look like walls but aren't

- The engine **sandbox masked score** and the **permuter register-diff score** are
  different metrics; a sandbox "5" ≠ permuter "5". Don't cross-compare them or
  conclude a plateau from one when the other still has gradient.
- A permuter plateau at N over tens of thousands of iterations is **evidence**,
  not proof — it ran *randomization only* from *one* seed. Re-seed from a closer
  base (every manual lever that lowers the diff gives the permuter a better start),
  use directed `PERM_*` macros, or run longer before drawing conclusions.

## Do NOT ask the user for a specific technique (it's giving-up in disguise)

When stuck on a matching technique, **never punt to the user with "do you recall a
pure-C way to do X?" / "do you know the technique for Y?"** The user has explicitly
ruled this out (2026-05-28): *"you have access to all the same information as I do.
If we don't have a technique documented, I will not know it."* Asking them for a
specific codegen/RA/scheduling technique is a softer form of giving up — the agent
owns deriving it.

**What this forbids:** requesting a named technique, a code snippet, or "how was
this pattern solved before" as a way to offload the hard part. If it's not in
`.claude/rules/` or `memory/`, the user doesn't have it either — so derive it from
the compiler source (`tools/gcc-2.7.2/*.c`), the RTL dumps, m2c, matched siblings,
and the permuter. That IS the job.

**What is still fine:** surfacing a genuine **policy/scope** decision (global rodata
reorder, canonical-asm sign-off, budget cap), reporting honest progress, or asking
which of several *concrete already-worked-out* options to take. The line: never ask
the user to supply the missing technical insight; do ask them to make a judgment
call only you two together can make. When you'd be tempted to ask "how do I…",
instead write down what you've ruled out and try the next un-tried derivation.

## Related
- [[register-alloc-pure-c]] — the lever playbook this rule mandates you actually run
- [[compiler-flags-canonical]] — flags/compiler are settled; the lever is C structure
- [[sandbox-zero-retire-fails]] — another "looks stuck, is actually a known fix" case
- [[difficult-not-impossible]] — the feedback-memory companion to this rule
