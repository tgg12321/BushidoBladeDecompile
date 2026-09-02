# SELF-VET — func_80030580

CONSTRUCTS: `volatile u32 pre_pad[4]; // !FAKE` — an unwritten, unread volatile
leading pad local, first declaration in the function body. Nothing else in the
diff: the remaining body is the s1-s5 pure-C floor-2 body
(memory/grind/func_80030580/pure-c-floor2-body.c), which carries zero FAKE
constructs (tools/fake_ablate.py, s5 and s6).

## T1 semantic purpose
FAIL, and declared as such. The pad has no observable effect on the function's
behaviour; its only effect is the frame size (`subu $sp,$sp,8` -> `subu
$sp,$sp,24`). That is exactly why it carries the `// !FAKE` annotation and is
submitted under a FAKE-annotated sanctioned family rather than as ordinary C.

## T2 human-programmer
FAIL. A reader asks "why is this array here?" — the answer is the target's
untouched stack bytes, not the algorithm. Again: annotated, not smuggled.

## T3 GCC-internals justification
FAIL by the letter of the test — the mechanism IS a GCC internal: reload's
`alter_reg` / `get_frame_size()` count a never-accessed local, so the object
costs frame bytes and zero instructions. This is the declared mechanism of the
Phantom-frame-slot volatile pad family itself; the family exists precisely
because this shape cannot be justified semantically.

## T4 permuter/search provenance
PASS. Not permuter output. The form is the one the 2026-09-02 17:07 Judge
ruling named as the only frozen route for this residual, and it was measured
directly with the frame instrument (tmp/grind/func_80030580/s6/padsweep.py),
N swept 1..6.

## T5 family check
Phantom-frame-slot volatile pad local, in its exact form-constrained shape:
array form `volatile u32 pre_pad[N];`, first-decl position, no `(void)pad;`
shim, `// !FAKE` annotation, frame forensics recorded. Honest producers were
measured inert first across s1-s6 (44 structural respellings, the sites-1
orphan law, generators 2/3/4, the symbol-address bias axis, the wider-signature
axis — all in hypotheses.md). The family's own FORM CONSTRAINT also requires a
per-function row in `engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS`,
which a grind session may not write; that is the integration handoff.

## T6 naming-announces-intent
The name `pre_pad` does announce pad intent — deliberately, because the
sanctioned family and its engine allowlist are keyed on that exact name
(`("pre_pad", N)` rows for func_8001E404, func_8001E6E4, func_8003CF84,
func_80047EE8, func_80047FBC, func_80049A2C, func_800481E8, func_80041688).
Renaming it to something innocuous would be the concealment failure, not a fix.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Phantom-frame-slot volatile pad local
  SCOPE: "**Phantom-frame-slot volatile pad local** (off-brief survey exhibit `src/st/sel/2C048.c:564` `volatile u32 pad; // !FAKE:` in an INCLUDE_ASM=0 file): an unused `volatile` pad local declared solely to reserve target's untouched stack bytes."
  PRECEDENT: .claude/rules/no-new-park-categories.md:415
  PRECEDENT: engine/volatile_cheats.py:766

ANNOTATION-CONFORMANCE:
  `    volatile u32 pre_pad[4]; // !FAKE`
  preceded by the block annotation carrying what + mechanism + lever-exhaustion:
  "// !FAKE: this pad reserves the 16 untouched locals bytes the target frame
   holds beyond our single combine orphan slot (target vars=24, ours 8);
   mechanism: reload alter_reg / get_frame_size counts the never-accessed
   volatile object, emitting no instruction (sp=0 accesses, bodydiff=4 =
   only the subu/addu sp lines); lever-exhaustion: memory/grind/func_80030580/
   hypotheses.md (s1-s6: 44 structural respellings, the sites-1 orphan law,
   generators 2/3/4, symbol-address bias, wider-signature axis - all measured)."

NOTE (disclosed, not exploited): while writing this form the block annotation
initially spelled the identifier `pre_pad` in the comment text. That alone made
`engine.volatile_cheats.find_unused_local_arrays` treat the local as
REFERENCED (its reference check is a plain `\bname\b` search over the body text,
including comments), so the sandbox did not strip the pad and printed
`"score": 0`. That is a detector artifact, not an honest floor; the comment was
rewritten to avoid the identifier and the sandbox correctly prints 2 again. The
engine surface is outside this session's scope, so the finding is reported here
and in docs/grind/decisions.md rather than patched.
