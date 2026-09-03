# SELF-VET — func_8003A728

CONSTRUCTS: (1) `s32 zero;` + `zero = 0;` in block 1, read once as `D_800A3730 != zero`
(constant-holder local, /* FAKE */-annotated); (2) `s32 c0lo` holding `(u16)D_800A36C0`
(ordinary named intermediate, SImode-typed local); (3) multi-set `s32 t` staging for the three
`D_800A36C2 / D_800A36D2 & 0xF` sites (ordinary scratch variable, assigned 3 times, each read
once immediately after); (4) the u16 low-half loads `buf8 = (u16)D_800A3698;` /
`buf8 = (u16)D_800A36D0;` written into the existing `buf8` local (variable reuse); (5)
`hi16 = hi16 | packed;` in place, `s32 flag` for the D_800A3916 read, and the statement position
of `hi16 = D_800A37C4 << 16;` (ordinary C, no family claim); (6)
`((FuncBufType)func_8003A450)(&D_800A3698)` (the file's existing typedef for this call shape).

## T1 semantic purpose
(1) `zero`: NO independent semantic purpose — `D_800A3730 != zero` is behaviourally identical to
`D_800A3730 != 0`. This construct is a match lever and is declared as such: it carries the
mandatory `/* FAKE: ... */` annotation and is claimed under the constant-holder family below.
(2)-(6) all have semantic purpose: `c0lo`, `t`, `flag` and the `buf8` low-half values are real
values that are computed and then consumed (`c0lo` twice, `t` once per assignment, `flag` in the
branch, the `buf8` low half in the two `*(s32 *)(a0 + 8)` stores). `hi16 |= packed` and the
`FuncBufType` cast are the plain expression of what the function does; removing any of them
changes what the function computes or how it calls `func_8003A450`.

## T2 human-programmer
(2)-(6): yes. The matched sibling `func_8003A5A0` in this same TU is written in exactly this
step-wise scratch-variable style (`v0 = a1 ^ (a1 >> 16); v0 = v0 ^ (a0 >> 16); v0 = v0 & 0xFFFF;`),
and re-using one `buf8` local for "the 32-bit word we read from the caller buffer" and then for
"the 16-bit half we are about to write back into it" is ordinary step-wise C, not a device.
(1) `zero`: NO — a reader would ask why the literal 0 was given a name. That is precisely why it
is annotated FAKE and claimed under a sanctioned family rather than presented as ordinary C.

## T3 GCC-internals justification
(1) `zero` is justified by a GCC-internals mechanism, stated openly in its annotation and read
out of this session dumps rather than guessed: the `(set (reg/v:SI 80) (const_int 0))` insn
(insn 24 in tmp/grind/func_8003A728/dumps/code6cac_c_mid.flow) is still present in sched1's
block-1 ready lists (`;; insn[  24]: priority = 1` in the .sched dump), where it takes the slot
immediately before the branch and so displaces the `D_800A369C` store out of it — which is the
memory-unit load-after-store blockage (sched.c:2685, mips.md:153-161) that kept the `D_800A3916`
flag lbu out of that slot for three sessions. `update_equiv_regs`
(tools/gcc-2.7.2/local-alloc.c:947, constant case at local-alloc.c:1031) then recognises the
REG_EQUAL `(const_int 0)`, substitutes the constant at the single use and deletes the insn — reg
80 is absent from the .lreg register table and no instruction is emitted, which is why the
function is 200 insns like the target. This is the cheat SIGNAL the checklist names, and it is
the reason this construct is submitted under the FAKE-annotated constant-holder exception with
the prerequisites met, not as ordinary C.
(2)-(6) are justified by what the function computes; their codegen effects are documented in the
ledger but they are not constructs a reader would call gratuitous.

## T4 permuter/search provenance
The `zero` construct was PROPOSED by a decomp-permuter campaign
(tmp/perm_8003A728_s4, label s4-w3-candidate-chassis, 201 iterations, find at 10.4 s) as
`int new_var2 = 0;` plus a second, inert `int new_var = 8;`. It was NOT accepted as found:
the find was hand-minimised (the `new_var = 8` holder measured byte-neutral at 3 and was
DROPPED — tmp/grind/func_8003A728/s4/p3_nv8.c), re-spelled by hand, and then five natural
alternatives that add a block-1 quantity carrying a REAL value were written and measured
(q1 29, q2 5, q3 14, q4 3, q5 6 — all worse than 0). The construct survives because it is the
only measured member of a sanctioned family that reaches 0, not because a detector missed a
spelling. (2)-(6) predate this session and were derived by hand from dumps in s1-s3.

## T5 family check
(1) matches the sanctioned constant-holder / dead-scalar family of
.claude/rules/named-local-fake-exception.md by shape exactly: a scalar local initialised to a
constant and read once in place of the literal. It is not an array, not frame coercion, not
address-coerced, not a pin, not asm, not a barrier, not a volatile coercion, not a dead store
(the value is read), and not an asm scheduling barrier. (4) is the one other construct that could
be argued into a family: writing a second value into the existing `buf8` local. It is a REAL
value that is immediately consumed, in the idiom the matched sibling in this TU uses, so it is
claimed as ordinary step-wise C; the conservative alternative family
(staged-value-reused-variable) is cited below with its scope so a reviewer can rule on it
directly rather than having to reconstruct it. (2), (3), (5), (6) match no forbidden family: they
are named intermediates and scratch variables whose values are consumed.

## T6 naming-announces-intent
No construct is named `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`
or `_frame_pad`. `zero` names the VALUE it holds and is the spelling SOTN master ships for this
exact family (`s32 zero = 0;`). `buf8`, `packed`, `hi16`, `flag`, `vsync`, `c0lo`, `t` name what
they hold. No construct has only discards, address-of or a bare declaration as its uses: `zero`
is read in the comparison; every other local is both written and read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: constant-holder / dead scalar local (FAKE-annotated exception)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): constant-holder locals (a local existing only to carry a constant in a register across calls) and dead/unused SCALAR locals that bias RA are allowed as LAST-RESORT levers with `/* FAKE: ... */` annotation + documented lever-exhaustion. SOTN ships `s16 three = 3;`, `s32 zero = 0; // needed for PSP`, `fake = 8;`, `new_var` in 9 files. Arrays/frame coercion stay forbidden."
  PRECEDENT: docs/reference/sotn-construct-index.md:379
  PRECEDENT: docs/reference/sotn-construct-index.md:1432
  PRECEDENT: .claude/rules/named-local-fake-exception.md:5

  FAMILY: staged-value-reused-variable (claimed ONLY as the conservative reading of the `buf8`
  low-half reuse; the primary claim is that it is ordinary step-wise C, as the matched sibling
  func_8003A5A0 in this TU writes the same idiom)
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:3

ANNOTATION-CONFORMANCE:
  /* FAKE: constant-holder local; mechanism: the (set (reg) (const_int 0)) survives into
   * sched1's block-1 ready lists and displaces the D_800A369C store from the slot before the
   * branch, then local-alloc.c update_equiv_regs deletes it (no insn emitted);
   * lever-exhaustion: memory/grind/func_8003A728/hypotheses.md s1-s4 */
  — carries WHAT (constant-holder local), MECHANISM (sched1 block-1 ready-list placement plus
  local-alloc.c update_equiv_regs deletion), and LEVER-EXHAUSTION (the s1-s4 hypotheses ledger:
  s2's five hi16-placement measurements, s3's five-arrangement dependence closure, s4's five
  natural block-1 stagings q1-q5 and the three construct ablations).
