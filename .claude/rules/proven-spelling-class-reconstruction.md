---
name: proven-spelling-class-reconstruction
paths: [".claude/rules/proven-spelling-class-reconstruction.md"]
description: "Narrow same-bytes respelling exception (user policy 2026-06-10): only with mechanism-level proof the original used a different spelling class, plain-C form, most natural variant, last lever."
metadata:
  type: rule
---

# Proven-spelling-class reconstruction — the narrow same-bytes respelling exception

**User policy 2026-06-10.** A same-bytes respelling — a C form that emits byte-identical instructions to the
current form and differs only in its effect on GCC's internal analysis — is sanctioned ONLY when ALL hold:

1. **Mechanical proof of the original spelling class.** The target bytes are only producible from a spelling
   class OTHER than the current one, proven at the compiler-mechanism level (RTL dumps + GCC source reading,
   e.g. the MEM_IN_STRUCT_P `/s` flag controlling a sched.c `true_dependence` escape clause). "This spelling
   scores better" is NOT proof; the proof is "the original source provably did not write the current form."
2. **The committed form is plain, natural C** — something a 1998 programmer would write (a pointer local, a
   cast read, a differently-typed access). No dead code, no unused declarations, no asm.
3. **The most human-plausible representative is chosen**, annotated with a comment citing this rule (so
   nobody "simplifies" it back).
4. **The respelling is exhaustively the last lever.** All conventional levers tried and measured negative; the
   cheat-reviewer adjudicated the candidate forms.

It covers CHOOSING AMONG equally natural spellings of a line the function genuinely needs; it never covers
ADDING semantically empty constructs. Not sanctioned: respellings without the mechanism proof; forms that
aren't natural C (e.g. re-typing a global as `s16[2]` with a never-used element); unvetted permuter finds.

## Example — InitHiraRmd_80041AC8

Target stores `sh a1,D_800A9A20` BEFORE reloading `lh v0,8(a0)`; our `arg0[4]` reload was hoisted above the
store by sched1 because pointer indexing sets `MEM_IN_STRUCT_P=1`, firing `true_dependence`'s in-struct escape
(no dependence edge). Target's order requires `/s=0` on the reload, so the original did not write `arg0[4]`.
Committed: `id_ptr = &arg0[4]; ... *id_ptr`. (A `*(T *)((u8 *)p + off)` cast access is likewise not MEM_IN_STRUCT_P.)

## Related

[[no-new-park-categories]] · [[do-while-zero-exception]] · [[legitimate-volatile-interrupt-touched]] ·
[[switch-break-shared-return-sched-hoist]] (check for a structural fix first)
