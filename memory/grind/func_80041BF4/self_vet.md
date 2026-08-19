# SELF-VET — func_80041BF4

Diff scope: `src/text1a_post.c`, function `func_80041BF4` only. Sandbox
`--disable all` == 0 at 135/135 instructions with these edits in place.

CONSTRUCTS: (1) `int one; one = 1;` constant-holder for the trailing `== 1`
test; (2) single-level `do { xoff = 0x80; yoff = 0; } while (0);` wrap on the
else-arm offset definitions; (3) `s16 rect[8]` oversized locals object whose
live prefix rect[0..3] is the LoadImage RECT; (4) `s32 x; ... test: x = *(u16 *)
tbl; if ((s16) x >= 0) goto again;` — the loop-terminator read hoisted into the
loop condition (ORDINARY C, no family claimed); (5) `goto test/again/oloop`
mixed loop-and-exit spelling (ORDINARY C, inherited from earlier sessions).

## T1 semantic purpose
(1) `one` — NO independent semantic purpose; it is a pure codegen holder.
    Declared as such, FAKE-annotated, family-claimed below.
(2) the do-while(0) wrap — NO independent semantic purpose; the body executes
    exactly once with or without it. Declared as such, FAKE-annotated,
    family-claimed below.
(3) `rect[8]` vs `rect[4]` — the tail elements are never written or read, so
    the four extra elements have no observable effect on output VALUES; their
    effect is on the FRAME SIZE, which is part of the target's bytes. Declared
    as such, FAKE-annotated, family-claimed below.
(4) `x` — YES, real semantic purpose. `x` carries the halfword that becomes
    rect[0]'s x coordinate; it is written once per test and read once per body
    iteration. The loop terminator genuinely IS the sign bit of that same
    halfword (target's own `lh`/`lhu` pair at 0($s0) proves the original read
    it both ways), so testing `(s16) x >= 0` is a faithful statement of the
    program's terminator condition, not a codegen device. Removing `x` and
    re-reading in the body produces identical program behaviour but 136
    instructions — the difference is byte-level, yet the construct itself is
    the simpler, more direct spelling of the loop: read the entry once, test
    it, use it.
(5) the gotos — real control flow; this is a zero-trip loop's natural
    bottom-test spelling and it is what target's `j .L80041DA0` entry encodes.

## T2 human-programmer
(1) NO — a human would write `== 1`. Cheat-smell acknowledged; sanctioned
    family claimed with the mandated annotation.
(2) NO on its own — but this is the canonical PsyQ-era macro-body idiom and
    the rule cited below sanctions it explicitly at construct level, with the
    annotation carrying the honesty. Acknowledged as a match device.
(3) PARTLY — a human writing a PsyQ blit routine would plausibly declare a
    scratch buffer larger than the RECT it fills. But the specific size is
    chosen from the frame equation, so it is treated as a match device and
    annotated as one.
(4) YES. `x = *(u16 *) tbl; if ((s16) x >= 0)` reads like ordinary loop code:
    fetch the next table entry, stop on a negative sentinel, otherwise use it.
    A reader would not ask "why is this here?".
(5) YES for a decompilation of a bottom-tested zero-trip loop.

## T3 GCC-internals justification
(1)(2)(3) YES — each names a GCC pass in its FAKE annotation, which is exactly
    what the three cited rules REQUIRE (named-local prereq 2: "The GCC-pass
    interaction is named"; do-while(0) prereq 1: name the observed effect;
    oversized-locals prereq 1: the frame-math derivation). These are annotated
    match devices inside sanctioned families, not un-annotated first reaches.
(4) NO — the program-logic explanation stands on its own (read once, test the
    sentinel, use the value). The combine.c mechanism is documented in the
    ledger as the EXPLANATION of why this spelling is byte-exact, but the
    construct is justified by the logic, not by the pass. No FAKE annotation
    is claimed for it and none is required.
(5) NO — ordinary control flow.

## T4 permuter/search provenance
NONE of the five came from the permuter. Permuter modality was declared SPENT
for this function at s5 (3 chassis, 60k+ iterations, zero byte-moving finds)
and no campaign was run this session. Construct (4) — the one that closed the
function — was derived by READING tools/gcc-2.7.2/cse.c
(`cse_end_of_basic_block`, lines 8008-8190) and the cc1 `-da` dumps, then
reasoning about which pass could manufacture a second memory reference after
cse could no longer unify the two. Constructs (1)(2)(3) are inherited from
s4/s9/s10 and were each re-measured against ordinary-C substitutes in the new
basin this session (numbers in hypotheses.md [s11]); none of them "passes
detectors only because the detectors miss this spelling" — all three are
detector-visible by design and are declared here.

## T5 family check
(1) constant-holder — `.claude/rules/named-local-fake-exception.md`. Not a
    register pin, not asm, not volatile, not an array, not an address-escape.
(2) do-while(0) — `.claude/rules/do-while-zero-exception.md`. Single level (no
    nesting, so that rule's prerequisite 3 does not apply). Not a scheduling
    barrier, no asm.
(3) oversized locals — `.claude/rules/dead-vars-local-array.md` OVERSIZED-
    LOCALS carve-out. NOT the forbidden unwritten-array form: rect is
    referenced, written, and read back (rect[0]/rect[1] are passed to
    func_80048A7C) and its ADDRESS is passed to LoadImage, so this is the
    "extend the LIVE object" branch (prereq 2) and not a dead pad. No
    `(void)&local`, no volatile qualifier, no `pad`-style name.
(4) does not match any forbidden family: it is not a dead store (x is read),
    not a dead local (x is read), not a duplicated read into arms (there is now
    exactly ONE read where the previous form had two), not a volatile coercion,
    not a redundant width cast (the `(s16)` cast is the semantic sign test, and
    it REMOVES an instruction rather than adding one). It is adjacent to the
    sanctioned named-intermediate shape but does not need that family's
    protection because it is a plain, once-written / once-read, semantically
    consumed value.
(5) mixed exit forms — `.claude/rules/cross-jump-store-tail-merge.md`;
    ordinary C, no FAKE.

## T6 naming-announces-intent
`one` is the exact identifier the 2026-06-02 sanction and
`.claude/rules/named-local-fake-exception.md:18` name verbatim ("opaque
`s32 one = 1;`"), and it is READ (`func_8003E2A0() == one`), not discarded.
`rect` names what the object IS (a LoadImage RECT); it is not `pad`, `_pad`,
`dummy`, `spill`, `slack` or `tail`, and it is written, read, and has its
address taken for a real callee. `x`, `xoff`, `yoff`, `tbl`, `idx`, `outer`,
`off` all name real values. There is no identifier in the diff whose only uses
are discards, unused declarations, or bare address-of.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: constant-holder / dead scalar local (FAKE-annotated exception)
  SCOPE: "**Constant-holder local:** `s32 k = 1; f(); g_a = k; ...; g_b = k;` — a local initialized to a constant, kept live across intervening calls so the constant sits in a callee-save register instead of being re-materialized. (The `DispSamnailWindow` shape.)"
  PRECEDENT: .claude/rules/named-local-fake-exception.md:24

  FAMILY: do-while(0) match device
  SCOPE: "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation.**"
  PRECEDENT: .claude/rules/do-while-zero-exception.md:23

  FAMILY: oversized-locals carve-out (written-prefix buffer with unwritten tail)
  SCOPE: "**OVERSIZED-LOCALS CARVE-OUT (owner ruling 2026-07-13):** a stack-locals object with an **unwritten tail** (a written-prefix buffer, e.g. func_80037540's `s32 sp[8]` with only `sp[0..5]` stored and the buffer passed live to a callee) — or, as the fallback when no live object exists to extend, a dead pad local — is sanctioned as a last-resort matching construct **when the target bytes PROVE the original declared it**."
  PRECEDENT: .claude/rules/dead-vars-local-array.md:39

Carve-out prerequisite discharge for the oversized-locals claim:
  1. FRAME-MATH PROOF FROM TARGET BYTES ALONE. Target frame = 0x58 = 88.
     Callee-saves stored in the prologue: $s0-$s7, $fp, $ra = 10 x 4 = 40
     bytes (asm/funcs/func_80041BF4.s prologue). Outgoing-args area = 24 bytes
     (the 6-argument func_80048A7C call writes $sp+0x10 and $sp+0x14; six
     4-byte slots = 24). Locals region = 88 - 40 - 24 = 24 bytes, based at
     $sp+0x18. The ONLY stores into that region anywhere in the function are
     the four `sh` at 0x18 / 0x1A / 0x1C / 0x1E = 8 bytes. The fully-written
     form (`s16 rect[4]`) MEASURES frame 80, not 88 — so no fully-written
     locals set produces the target frame. The oracle-checked frame attests
     the slack existed in the original.
  2. PREFER EXTENDING A LIVE OBJECT — done. rect is live (its address is the
     first argument to LoadImage and rect[0]/rect[1] are read back for
     func_80048A7C). No separate dead pad local is used.
  3. RANGE ANNOTATION — present on the declaration, with the frame derivation
     and the carve-out named. The range is MEASURED, not asserted: rect[5]=0,
     rect[6]=0, rect[7]=0, rect[8]=0, rect[4]=22, rect[9]=22.
  4. DOCUMENTED LEVER-EXHAUSTION — memory/grind/func_80041BF4/hypotheses.md
     [s10] (frame sweep: separate pad locals, function-scope off, s16/s32/u16
     coord temps, named width temp — none reaches vars=24 with rect at sp+24)
     and [s11] (re-measured in the new basin; rect[4] and rect[9] banked as
     rejected forms).
  5. LAYER-1 + JUDGE — this session returns candidate-ready for exactly that.

ANNOTATION-CONFORMANCE:
  /* FAKE: opaque constant-holder for the trailing `== 1` test, mechanism:
     local-alloc.c block_alloc / find_free_reg - the bare literal is
     rematerialized by reload into $v1, while a live pseudo carrying it is
     allocated $t0 exactly as target does, lever-exhaustion:
     memory/grind/func_80041BF4/hypotheses.md [s4] and [s10] (ten ordinary-C
     spellings of the test measured inert) plus [s11] (six more re-measured in
     the NEW basin: subtract-compare-zero 12, double-negated 12, switch/case 12,
     named call result in a fresh local 12, in reused `x` 12, in reused `idx`
     20, in reused `outer` 9 - none reaches 0).  Family:
     .claude/rules/named-local-fake-exception.md (owner ruling 2026-07-01). */

  /* FAKE: oversized locals object - rect[0..3] is the live LoadImage RECT and
     rect[4..7] is the unwritten tail, mechanism: mips.c compute_frame_size /
     get_frame_size - ALIGN8(vars) + ALIGN8(args) + gp_regs.  Frame-math proof
     from the TARGET BYTES ALONE: ... the fully-written form (rect[4]) measures
     frame 80, so no fully-written locals set can produce target's 88.  n.b.!
     ALIGN8 ... rect[5], rect[6], rect[7] and rect[8] are all byte-identical
     here ... Family: .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS
     carve-out (owner ruling 2026-07-13); prong 2 is satisfied by extending the
     LIVE object ... Lever-exhaustion: memory/grind/func_80041BF4/
     hypotheses.md [s10] frame sweep + [s11] re-measured in the new basin. */

  /* FAKE: single-level do-while(0) wrap on the else-arm offset defs; observed
     effect - it lifts xoff's and yoff's weighted reference counts (flow.c
     weights REG_N_REFS by loop_depth) so global.c's allocno order matches
     target's and the two offsets land in $s5/$s4; without the wrap this form
     scores 18 instead of 0.  Natural geometry was tried first: arm swap (20),
     defs hoisted above the if (20), ternary (33), block-local and nested-block
     declaration scopes (18), duplicated real statement into both arms (18),
     plain assignment (18) - all measured this session.  Family:
     .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06,
     sanctioned for ANY codegen effect incl. register allocation). */

  Each carries what + mechanism (named GCC pass) + lever-exhaustion pointer.
  Constructs (4) and (5) carry no FAKE annotation and claim no family — they
  are ordinary C, per T1-T6 above.
