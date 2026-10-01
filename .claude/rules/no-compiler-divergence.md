---
name: no-compiler-divergence
paths: ["src/*.c", "tools/gcc-2.7.2/**", "tools/maspsx/**"]
description: "HARD RULE: the compiler is FROZEN (a compiler patch is a cheat, Q17) and difficult is not impossible: every unmatched function closes in pure C source structure. Never claim a 'compiler wall'."
metadata:
  type: rule
---

# The compiler is frozen; difficult is not impossible

The matching C exists for every function: our cc1 reproduces the original game
byte-for-byte and cc1psx is never closer (recorded sweep: 0/282 functions where
cc1psx wins). A non-match is a C-source problem (types, dataflow, statement
order, variable identity), never the compiler. The compiler's behaviour is the
shape the C must fit.

## Forbidden

1. **Patching the build compiler.** Any change to `tools/gcc-2.7.2/build/cc1`
   (pinned decompals/mips-gcc-2.7.2 @43d1cdb6) that changes its output is a
   cheat; a function whose match depends on one is reverted to `INCLUDE_ASM`
   (owner ruling Q17, docs/grind/decisions.md 2026-09-26 OWNER RULING — a
   compiler patch is a cheat). The one kept host-build fix is
   `tools/cc1-reorg-negate-rtx-decl.patch` (output-neutral crash fix); any new
   host-build fix needs its own owner ruling. Read GCC source to UNDERSTAND the
   shape the C must have, never to change what GCC does.
2. **Patching `tools/maspsx/` beyond bug-fix scope.** Bug fixes must be
   byte-neutral for every object (build with stock and fixed maspsx, compare,
   `verify-oracle --rebuild`, `engine test`). The live per-function gates are in
   [[maspsx-gate-lists]]; new GLOBAL behaviour needs owner sign-off (examples
   on record: the global COMMON model Q62, the `.sdata` model Q68 in
   [[per-file-gp-model]], the `($REG)` empty-offset parser fix).
3. **Switching the build to cc1psx**, or per-function cc1psx opt-in. cc1psx is
   calibration / self-disproof only.
4. **Forking** any toolchain component (cc1, ld, as, maspsx), even for one
   function.
5. **"The toolchain is the variable"** as a reason to escalate, park, rotate
   or stop.

Compile flags are not patches: canonical `CC_FLAGS` (incl. `-mel`,
`-msoft-float`) configure the unmodified compiler and are frozen too
([[compiler-flags-canonical]]). The instrumented diagnostic cc1
(`tools/gcc-2.7.2/cc1`, `BB2_*_DEBUG` hooks) and cc1psx are diagnostic only;
no function lands on their output.

## Before any pessimistic claim

Do not write "wall / irreducible / impossible / fork divergence / can't be
closed in pure C" until you have done all of:

1. **Self-disproof with cc1psx.** Preprocess, compile with both, diff:
   ```bash
   tools/gcc-2.7.2/build/cc1 <cc1-flags> foo.i -o /tmp/ours.s
   tools/cc1psx_wrapper.sh   <cc1-flags> < foo.i > /tmp/psx.s
   ```
   cc1psx does not beat our fork, so switching compilers cannot close the gap.
2. **RTL dumps** (`-da`; `.greg` register dispositions + conflicts, `.lreg`,
   `.sched`): find which decision differs and what it depends on.
3. **The lever playbook** ([[register-alloc-pure-c]] and the
   codegen-technique-index), structural respellings, m2c of the target for the
   original's shape, matched siblings' dumps.
4. **Permuter with a clean single-function target** (`target.o` built from
   `asm/funcs/<func>.s` at offset 0), re-seeded from your best base, directed
   `PERM_*` macros ([[permuter-directives]]). A plateau is evidence, not proof.

The sandbox masked score and the permuter score are different metrics; do not
cross-compare them.

## Keep going

- Never ask the owner for a technique ("do you recall how to do X?"). If it is
  not in `.claude/rules/` or `memory/`, the owner does not have it either;
  derive it from the GCC source, dumps, m2c, siblings and the permuter.
- Never stop while your own notes name concrete un-run levers: execute each
  one, or bank its measured negative, first. A genuine endpoint reads "tried A,
  B, C (measurements); no further lever derivable from the evidence I have".
- Valid non-C dispositions are only: canonical-asm authorization for a
  construct with no C form ([[canonical-asm-authorization-recipe]]), or a
  landed owner ruling. A stuck item changes MODALITY, never target
  ([[rotation-not-foreclosure]]).
- Build tools when they help (`tmp/` sweeps, probes); read
  `tools/gcc-2.7.2/{combine,jump,sched,reorg,global,local-alloc,flow,cse,loop}.c`.

Related: [[compiler-flags-canonical]] · [[register-alloc-pure-c]] ·
[[no-new-park-categories]] · [[rotation-not-foreclosure]]
