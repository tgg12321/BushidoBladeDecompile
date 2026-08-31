# SELF-VET — func_80078654

CONSTRUCTS: (1) 8-deep `do { ... } while (0);` wrap around the loop-top block
`s.a = var_s0[0]; s.b = s.a + 0xC; s.h = -D_800A3608;`; (2) constant-holder
local `s32 zero;` initialised to 0 and passed to func_8006E480 across the
SetDrawMode/AddPrim call sequence. Nothing else in the diff: the remainder is
the ordinary re-derived body (struct-typed frame `S78654 s`, goto-based walk
loop, `s16 sv` clamp) that has been the ledger's best form since session 1 and
is byte-accounted against asm/funcs/func_80078654.s by the session-8 hand
derivation. No asm, no pins, no volatile, no rules, no aliases.

## T1 semantic purpose
(1) The wrap has NO semantic purpose — `do { X } while (0)` executes X exactly
once, which is what plain `X` does. It is a pure match device and is declared
as such with an inline FAKE annotation. (2) `zero` DOES carry a real consumed
value (it is the second argument of func_8006E480), but the value is a literal
constant, so the local's reason to exist is also codegen; it is likewise
FAKE-annotated. Neither construct asserts a false program fact: "this block
executes once" and "this variable holds 0" are both true.

## T2 human-programmer
(1) No — a human writing this from a specification would not nest eight
do-while(0) levels; it is declared a match device, not disguised as logic.
It is, however, exactly the shape a period macro expansion leaves behind, and
it is the construct the owner sanctioned for this purpose (ruling 2026-07-06).
(2) Plausibly yes — `s32 zero = 0;` is shipped verbatim in SOTN master
(src/dra/7879C.c:2067) — but I do not rest on that: it is annotated.

## T3 GCC-internals justification
Yes, and stated openly rather than hidden: the wrap works through flow.c:2081,
where each register mention is counted as `loop_depth` references and
`loop_depth` is 1 + the number of enclosing NOTE_INSN_LOOP_BEG notes
(flow.c:434 initialises depth = 1; flow.c:440-443 counts the notes). Because
notes are not instructions, this raises the walk pointer's reg_n_refs from 5 to
13 at zero byte cost, and global.c's allocno priority then seats it in $s0
(measured: pri 4285 vs the parameter's 3979). Under the do-while(0) rule this
is not a disqualifier: the owner's 2026-07-06 ruling explicitly abolished the
mechanism-scoping and sanctions the construct "for ANY codegen effect,
including register allocation", requiring only that the effect be annotated.
The annotation names the effect first and the mechanism second.

## T4 permuter/search provenance
Not permuter output. Sessions 4 and 5 ran 129,034 permuter iterations on this
function and found nothing; this form came from reading the compiler source
(flow.c / local-alloc.c) during the session's disposition review, deriving the
required reference count arithmetically (nrefs >= 13 at live length 91 to
outrank pri 3979), and then measuring the wrap depth and the wrap SITE with the
inherited ALLOCDBG rig. Every intermediate depth and site is banked with its
measurement (tmp/grind/func_80078654/s11/w*.c).

## T5 family check
(1) The wrap is the sanctioned do-while(0) match-device family. Note the frozen
one-line summary in .claude/rules/no-new-park-categories.md still carries the
retired "LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY" scoping; the rule
file itself records that the owner superseded that on 2026-07-06 and the
family now covers any codegen effect including RA. Nested use is explicitly
contemplated by the rule (prerequisite 3) and carries a documentation duty,
discharged in the inline annotation and below. (2) The constant-holder is the
sanctioned constant-holder / dead-scalar family, scalar-only, no array or frame
coercion. Neither construct is a register pin, hardcoded-$N asm, scheduling
barrier, volatile coercion, dead-param assign, alias rename, or any other
member of the forbidden catalog.

## T6 naming-announces-intent
No `pad` / `dummy` / `unused` / `spill` / `_buf` / `tail` / `slack` names in the
diff. `zero` names its value, not a coercion intent, and it is read. `var_s0`
is the inherited m2c-era name for the table-walk pointer and is read four times.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) match device (nested)
  SCOPE: "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation.**"
  PRECEDENT: .claude/rules/do-while-zero-exception.md:29
  FAMILY: do-while(0) match device — SOTN master instances
  SCOPE: "do { ... } while (0) wraps"
  PRECEDENT: docs/reference/sotn-construct-index.md:599
  FAMILY: constant-holder local
  SCOPE: "**Constant-holder local:** `s32 k = 1; f(); g_a = k; ...; g_b = k;` — a local initialized to a constant, kept live across intervening calls so the constant sits in a callee-save register instead of being re-materialized."
  PRECEDENT: .claude/rules/named-local-fake-exception.md:31

NESTED-WRAP JUSTIFICATION (prerequisite 3, .claude/rules/do-while-zero-exception.md:70):
  A single level is measured INSUFFICIENT. The walk pseudo has 5 emitted
  mentions and a live length of 91; the parameter has 13 mentions / 98 live and
  allocno priority 3979. The walk pointer needs nrefs >= 13 to outrank it
  (13 -> 4285 > 3979; 12 -> 3956 < 3979). Wrapping the loop-top block at depth
  d gives nrefs = 4 + (1 + d), so depth 1 yields 6, depth 4 yields 9, and depth
  8 is the first value that reaches 13. Measured, not assumed: every depth was
  run through the ALLOCDBG rig (tmp/grind/func_80078654/s5/s11_w*/alloc.log).
  The site is likewise forced: a loop-note pair is a scheduling boundary that
  costs one neighbour-supplied delay-slot fill, so every wrap site that is not
  self-filling builds at 117 insns instead of 116 (var_s0++ site: 117; loop-top
  read alone: 117; init site: 117; loop-test site: 117). Extending the region
  to include `s.h = -D_800A3608;` supplies the fill from inside the region and
  yields 116 == 116 with zero differing instructions.

ANNOTATION-CONFORMANCE:
  /* FAKE: 8-deep do-while(0) wrap around the walk-pointer read block.
     Effect: seats the table-walk pointer var_s0 in $s0 and the parameter
     arg0 in $s1 (loop-note reference weighting -> allocno priority).
     Mechanism: flow.c:2081 counts each register mention as loop_depth
     references, so the wrap multiplies var_s0's reg_n_refs without
     emitting an instruction; global.c's allocno priority then ranks
     var_s0 (13 refs / 91 live, pri 4285) above arg0 (13 / 98, pri 3979).
     Single level measured INSUFFICIENT (prerequisite 3 of
     .claude/rules/do-while-zero-exception.md): depth 1 yields 6 of the 13
     references the priority inversion requires; depth 8 is the minimum
     that reaches 13 at the only wrap site that costs no delay slot.
     Lever-exhaustion: memory/grind/func_80078654/hypotheses.md s1-s11
     (11 sessions, 8 modalities, 129k permuter iterations, 14 banked
     rejected forms). */
  /* FAKE: constant-holder local, kept live across the SetDrawMode /
     func_8006E480 / AddPrim call sequence so the 0 argument comes out of a
     register instead of being re-materialized at each use.  Mechanism:
     local-alloc/global-alloc seat the constant in a call-saved quantity;
     replacing it with the literal 0 was measured this session at 113 insns
     vs the target's 116 (tmp/grind/func_80078654/s5/s11_w25/, chassis
     tmp/grind/func_80078654/s11/w25_nozero.c), so the holder is
     load-bearing.  Lever-exhaustion: memory/grind/func_80078654/hypotheses.md
     s1-s11. */
  Both are inline at their construct sites in src/text1b_b.c (not file-header
  prose), and each carries what + mechanism + lever-exhaustion.
