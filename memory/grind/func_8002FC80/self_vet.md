# SELF-VET — func_8002FC80

CONSTRUCTS: gte.h include addition; six VECTOR-component scratchpad stores at
folded constant addresses (0x1F800360/0x1F800370 vx/vy/vz); shared named locals
v1/v2; pointer local p = (s32 *)0x1F800380 (island operand + p[0] call arg);
three volatile cop2 asm islands (PsyQ libgte macro bodies: gte_SetRotMatrix,
gte_ldlvl, gte_stlvnl) + `.word 0x4B70000C` (GTE OP); ratan2 tail with
+0x800 adjustment.

## T1 semantic purpose
- VECTOR stores: real stores of real computed values (a1-a0, a2-a0 diff
  vectors) consumed by the GTE islands; removing or respelling them changes the
  emitted bytes (34 vs 0), so they are not byte-inert. The typed spelling names
  what the memory IS: three VECTOR slots at 0x360/0x370/0x380 (0x10 stride ==
  sizeof(VECTOR)) fed to gte_SetRotMatrix/gte_ldlvl/gte_stlvnl. PASS.
- v1/v2 locals: written and read every block, real values. PASS.
- p: real pointer used twice (asm operand, p[0] arg to ratan2). PASS.
- cop2 islands + .word: the function's core computation (GTE cross product);
  no C analog exists for cop2 transfers/ops. PASS.
- Tail: the function's return-value logic. PASS.
- No construct in the diff is behavior-inert.

## T2 human-programmer
A PsyQ programmer writing "load two vectors into scratchpad, run GTE OP, take
the angle" writes typed VECTOR accesses and the SDK gte macros — this is the
idiomatic shape (the completed sibling func_8002FDB0 in src/code6cac_b.c is
the in-repo exhibit of the same macro bodies). Nothing in the body prompts
"why is this here?": every statement is a step of the stated computation. PASS.

## T3 GCC-internals justification
Honest disclosure: the choice of the VECTOR-typed store spelling over the
plain `*(s32 *)CONST` spelling was DISCOVERED via scheduler behavior (the plain
form's stores sink; see evidence.md s1 #3, sched.c fixed-vs-struct exemption).
However the construct's justification does not REST on GCC internals: the
program-logic explanation (this memory is a VECTOR, typed access is the
idiomatic PsyQ spelling) stands alone, the construct is ordinary semantic C
with zero dead code, and the object-model-first posture is exactly what
`.claude/rules/split-scalars-hide-aggregate.md` prescribes over qualifier
hacks ("Reach for the object model first"). No lever-named construct, no
byte-inert coercion. PASS.

## T4 permuter/search provenance
No permuter or auto-search was used. All three spellings were hand-derived
from the target asm and the completed sibling, each with its own sandbox
measurement. PASS.

## T5 family check
- No register pins, no placeholder-move aliasing blocks, no standalone
  scheduling barriers, no volatile coercion (volatile-on-scratchpad is
  Judge-BANNED per evidence.md s1 #4 and was NOT used), no dead
  stores/locals/arrays, no alias renames, no regfix/asmfix, no hardcoded-$N
  GPR asm OUTSIDE the sanctioned cop2 macro-body islands.
- The islands' hardcoded $12-$15 + leading `move $12, %0` are the PsyQ SDK
  macro bodies, the exact island spelling of the accepted cluster exemplar
  func_8002FDB0 (inline_asm_canonical.txt:268); in-island GPR insns are limited
  to the cop2 addressing preamble (macro move + lw feeds + GTE load-delay
  nops), per the cluster's mechanical condition 3. PASS.

## T6 naming-announces-intent
Names: v1, v2, p, ret. No pad/dummy/spill/slack/buf names, no discard-only
uses. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: cop2-addressing-preamble-cluster (canonical inline-asm island
    disposition, 2026-08-17 owner cluster ruling — not a SOTN coercion family;
    declared here so the Judge applies the mechanical per-function check)
  SCOPE: "A member inherits the disposition **only** when all of these hold. This is a check, not a lever — it does not lower anyone's distance:"
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:83
  PRECEDENT: docs/grind/decisions.md:5610
  PRECEDENT: inline_asm_canonical.txt:268
  Mechanical conditions verified this session (2026-08-26 s2): (1) sandbox
  --disable all == 0 (74/74); (2) zero pins / zero `move %0,%1` aliasing
  blocks / zero standalone barriers; (3) in-island GPR insns limited to the
  cop2 addressing preamble. Condition (4) — fresh layer-2 cheat-reviewer +
  verify-oracle --rebuild — is the driver/Judge step that follows this outcome.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. No claimed family mandates a
/* FAKE */ annotation: the cluster disposition requires the island provenance
comments (present: each island is annotated with its PsyQ libgte macro name),
not FAKE annotations; every C construct in the diff is ordinary live semantic
code outside all FAKE-mandating families.
