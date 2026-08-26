# SELF-VET — func_8002304C

Diff under review: `src/code6cac.c` — the single line
`INCLUDE_ASM("asm/funcs", func_8002304C);` replaced by the pure-C body of
`func_8002304C` (plus its header comment). No other file is touched. Zero
regfix rules, zero asmfix rules, zero register-asm pins, zero inline `__asm__`,
zero `volatile`, zero `/* FAKE */` constructs, zero alias declarations, zero
`(void)` discards, zero dead locals, zero dead stores, zero unused arrays,
zero `do { } while (0)` wraps.

Measurements backing the submission:
- s2 annotation/citation fix-up session, 2026-08-26: candidate re-applied to
  `src/code6cac.c` and re-measured � `sandbox func_8002304C --disable all` ==
  **0**, target_insns 216, build_insns 216, rules_dropped 0, scorable true.
  The only edits this session are comment/citation text (the wrong precedent
  line numbers named by the 2026-08-26 16:11 layer-1 FAIL); no C code changed.
- `sandbox func_8002304C --disable all` == **0**, 216/216 insns, rules_dropped 0.
- `verify-oracle` == **build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa**,
  build_matches true, equal to the locked original SHA1.

CONSTRUCTS: (1) `s32 m = mode & 0xFFFF;` masked state-id local alongside the raw
`s32 mode` load; (2) `s32 lim = 0x1F8002B8;`-style named scratch-address locals
(`scratch`, `lim`, `scratch_c`, `scratch_d`) with two of them declared then
assigned by a later statement; (3) inline ternary as the 5th argument of the
second `func_8002EBDC` call; (4) two block-scoped `s16 vel` temporaries and one
block-scoped `s16 vel_y`; (5) `goto loop` back-edge and `goto done` /
`done: ;` forward exits.

## T1 semantic purpose: <answer, per construct>
(1) `m` is a REAL, load-bearing value: it is the operand of three separate
equality tests (`!= 8`, `!= 0x22`, `!= 0xA`) and it materializes in the target
bytes as `$v1` (`andi $v1,$a0,0xffff` at 0x232F4, then `beq $v1,$v0` at
0x232F8 / 0x23300 / 0x23314). It is not a discard, not write-only, not a
constant holder. The one sub-part that is semantically redundant *in isolation*
is the `& 0xFFFF`: because `mode` was widened from a `u16` load its value is
already <= 0xFFFF, so masking cannot change the compared values. I am not
hiding that. My claim is that the mask is **in the original source**, and the
evidence is in this repo: `func_80023E40` (`src/code6cac.c:2541-2562`) — a
COMPLETED-C, byte-matched, rule-free function 250 lines below in the SAME
translation unit — reads the SAME struct field at the SAME offset with the SAME
two-line idiom (`src/code6cac.c:2545-2546`):
    a0 = *(u16 *)(arg0 + 0x6A);
    v1 = a0 & 0xFFFF;
and then runs the SAME comparison cascade (`v1 == 8`, `v1 == 0x22`,
`(u32)(a0 - 0x17) < 2`, `v1 == 0xA`; E40 carries one extra arm, `== 0x28`).
The two functions are copy-paste siblings in the original program. Writing the
sibling's idiom here is source-fidelity reconstruction, not a construct added
to move bytes.
(2) Every one of `scratch`, `lim`, `scratch_c`, `scratch_d` is read at least
once by real code (`scratch` throughout; `lim` at both `func_8005344C` call
sites; `scratch_c` at both `func_8002EBDC` sites; `scratch_d` at the second
`func_8002EBDC`). None is dead; none is written-never-read; none has its
address taken. Splitting the four scratchpad addresses into named locals is how
the original expresses four distinct scratch regions; the target proves it by
keeping all four in callee-saves across the loop.
(3) The ternary IS the argument's value — `0x80` or `0x100` selected by the
0x15 test — and both constants appear in the target bytes
(`li $2,0x100` in the bne delay slot, `li $2,0x80` on the fallthrough).
(4)/(5) Ordinary C: the `vel`/`vel_y` temps hold the `lh` results that are
divided and added; the gotos are the loop back-edge (`count < 4`) and the
early-exit paths that target implements as branches to `.L80023330` /
function end.
No construct in the diff is byte-neutral-if-removed. Deleting the `& 0xFFFF`
changes the emitted opcode (andi -> move); deleting any other construct changes
the program's meaning.

## T2 human-programmer: <answer>
Yes for all five, and for (1) this is not a judgement call — a human programmer
DID write it, in this codebase, for this exact field: `src/code6cac.c:2546`.
A reader asking "why mask a value that is already 16-bit?" gets the ordinary
answer a maintenance programmer gives: the routine treats `obj+0x6A` two ways —
as a raw number for the range test `(u32)(mode - 0x17) < 2` and as a normalized
state id for the equality tests — and the author defensively normalized the
second one. That is a readable, self-consistent piece of program logic, and it
is the same logic the sibling function ships. (2) Naming four fixed scratchpad
addresses is what any reader would want over four repeated magic numbers.
(3) A ternary for a two-way constant argument is the most natural spelling
available. (4)/(5) Unremarkable.

## T3 GCC-internals justification: <answer>
No construct in this diff is justified by a GCC internal. To be explicit about
the distinction, because it is the test I most need to answer honestly:
- The **reason the construct is written this way** is source fidelity — the
  in-TU byte-matched sibling at `src/code6cac.c:2546` spells it this way.
- The **explanation of why the bytes differ** between this spelling and the
  narrow-typed spellings s1 measured is a compiler fact, and I did read it in
  a dump rather than guess it: in
  `tmp/grind/func_8002304C/dumps/code6cac.combine`, combine holds
  `reg74(SI) = zero_extend:SI(mem:HI(obj+106))` and
  `reg75(SI) = and:SI(reg74, 65535)`, and cannot substitute the first into the
  second because `reg74` is still live afterwards (it feeds
  `plus:SI(reg74, -23)`), so the `andsi3` insn survives to the assembler.
That is a post-hoc explanation of a measurement, not the construct's raison
d'être, and the construct does not depend on any allocator/scheduler/LUID/
`reg_n_refs`/`INSN_PRIORITY` mechanism. If the mask were removed the program
would still be correct — it would simply no longer be the original's code.
Constructs (2)-(5): s1 discovered their effect on prologue emission order and
on the `step` argument's seat by dump-reading, but each is plain program
structure (named constants, argument expression, block scope, control flow) and
none is a lever whose only function is to perturb a pass.

## T4 permuter/search provenance: <answer>
No permuter, no automated search, no randomized mutation was used in this
session or in s1 for this construct. The winning spelling was found by reading
the `.combine` dump for this translation unit, noticing a surviving
`(and:SI (reg/v:SI 74) (const_int 65535))` in a neighbouring function, and
identifying that function as `func_80023E40` — then copying the original
author's own C idiom from it. Provenance is in-repo source evidence, not
search output, and the construct does not pass detectors "because the detectors
miss this spelling" — it is already shipped, unflagged, in matched main-branch
code.

## T5 family check: <answer>
The construct is not in the forbidden-family catalog and does not match one by
analogy. It is not a register pin, not hardcoded-`$N` asm, not a regfix insert,
not a scheduling barrier, not INLINE_MOVE_ALIASING, not a volatile coercion of
any spelling, not a frame-coercion array, not a dead-param-assign, not a
dead-conditional-store, not an empty-body dead read, not an `if (1)` wrap, not
a dead goto label pad, not a DImode chain, not a `s32 one = 1;` opaque
variable, not an `asm("sym")` rename, not a `bb2.ld` reorder.
The nearest-neighbour I must address head on is
`.claude/rules/no-new-park-categories.md:341`, which lists "redundant width
casts (F2 — evidence insufficient; those findings close by ordinary cleanup)"
among the things the 2026-07-01 research does NOT support relaxing. Three
reasons that refusal does not cover this diff:
  (a) The refusal is explicitly an **evidence** refusal ("evidence
      insufficient"), i.e. no SOTN-master precedent was found for using a
      redundant width cast as a coercion lever. This submission does not rest
      on a community precedent at all — it rests on BB2's own already-matched,
      rule-free, oracle-verified source for the SAME struct field in the SAME
      file, which is strictly stronger evidence about what the ORIGINAL BB2 C
      contained than any SOTN function could be.
  (b) It is not a **cast**. There is no width cast anywhere in the construct:
      both `mode` and `m` are `s32`, and the operation is an ordinary bitwise
      AND against a literal — the same operation the sibling performs.
  (c) The F2 refusal's own remedy sentence — "those findings close by ordinary
      cleanup" — is exactly what happened here. The residual closed by writing
      the block the way the rest of the file already writes it; it did not
      close by adding an artificial coercion.
Constructs (2)-(5) are plain C and require no family: declaration order and
statement order are not constructs, they are the ordering of real declarations
and real statements; the ternary, the block-scoped temps and the gotos are
ordinary language features with real values and real control flow. In
particular this diff makes NO named-intermediate claim, NO dead-store claim,
NO constant-holder claim, NO pointer-alias claim, NO duplicated-statement
claim, NO dead-array claim, NO do-while(0) claim and NO volatile claim — none
of those shapes is present to claim.

## T6 naming-announces-intent: <answer>
No name in the diff announces coercion intent. There is no `pad`, `_pad`,
`dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack` or `_frame_pad`.
The names are `obj`, `pos1`, `pos2`, `arg3`, `scratch`, `count`, `lim`,
`scratch_d`, `scratch_c`, `vel`, `vel_y`, `mode`, `m` — every one describes the
value it holds, and every one is read by real code. `scratch`/`scratch_c`/
`scratch_d` name the PS1 scratchpad regions they point at (0x1F8001B0 /
0x1F8001C0 / 0x1F8001D0), which is descriptive, not intent-announcing; they are
the function's working buffers and are dereferenced dozens of times.

SANCTIONED-FAMILY-CLAIMS: none.
  No construct in this diff is a coercion construct, so no family exception is
  being invoked and no `/* FAKE */` annotation is owed. Every variable holds a
  value the program consumes, every statement has an effect, and every one of
  the 216 emitted instructions comes from compiling this C. The one construct
  whose necessity a reviewer will reasonably question — the `& 0xFFFF` — is
  defended on source-fidelity grounds with an in-repo byte-matched citation
  rather than on a family grant:
    IN-TREE PRECEDENT (original-author idiom, same TU, same struct field,
    COMPLETED-C with zero rules and zero cheat-asm):
      src/code6cac.c:2546
      commit 6d255e79
  If the reviewer's position is that a semantically-redundant mask requires a
  sanctioned-family grant even when an already-matched sibling in the same
  translation unit ships it verbatim for the same field, then the honest
  disposition is a ruling rather than an acceptance, and I would rather that
  ruling be made explicitly than have this pass by inattention. My own reading
  is that this is not a coercion family at all — it is the recovered original
  source — and that treating it as one would imply `func_80023E40`, which is
  already COMPLETED-C on main and inside the oracle, is itself non-compliant.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
