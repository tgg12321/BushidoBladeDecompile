# SELF-VET — func_800401CC

Session s4 (permuter modality, 2026-08-11). Sandbox 0/78 verified this session
with the diff at rest in src/text1a_pre.c. Constructs are described by role per
the 2026-08-11 06:25 judge ruling (docs/grind/decisions.md), which classified
the low-mask staging as a DISTINCT SANCTIONED INSTANCE and directed that the
vet describe it by role rather than by the banned invented-holder's name.

CONSTRUCTS: inline global reads (staging-local removal), store-last statement
order for the packet-cursor global, two staged-mask assignments through the
pre-existing dead-after-call texture coordinate locals u and v (both widened
s16 -> s32; the low 24-bit OT pointer-field mask staged through u, the high
8-bit OT-code mask staged through v), each FAKE-annotated at its set.

## T1 semantic purpose
- Inline global reads and store-last ordering: ordinary spellings of the
  function's real behavior (read parity, select buffer, link packet, advance
  cursor); every statement has observable effect.
- The two staged-mask assignments: the staged values are REAL and LIVE — each
  mask is read by both packet-link statements immediately below (the AND/OR
  OT-insert). Zero dead code; removing either set changes the emitted bytes
  (the masks would have no source). The BORROWING of the coordinate locals
  (rather than fresh names) is the part with no semantic purpose beyond
  codegen, which is exactly what the FAKE annotations declare under the
  sanctioned family.

## T2 human-programmer
- The packet-link logic itself is what any programmer writes for a PS1 OT
  insert. A reader would ask "why reuse the texture-coordinate locals for the
  masks?" — that question is answered in source by the two FAKE annotations;
  the construct is claimed under its sanctioned family, not passed off as
  natural style.

## T3 GCC-internals justification
- Yes for the two staged sets: the mechanism is global.c allocno call-arg copy
  preference plus set-position emission order (stated in the annotations).
  A GCC-internals mechanism is a required element of a FAKE-family claim, not
  a dodge here: the family (staged-value-reused-variable) is owner-sanctioned
  precisely for this kind of mechanism, prerequisites verified below. The
  non-FAKE constructs (inline reads, store order) need no internals story to
  justify — they are plain C the function would carry anyway.

## T4 permuter/search provenance
- The closing form was derived from instrumented-cc1 measurements (QTYDBG) in
  sessions s2/s2-permuter, not adopted from a permuter find. The one permuter
  zero ever found (an invented fresh holder local) was REJECTED per policy and
  is banked in rejected/permuter-newvar-holder.c; this form does not contain
  it. Campaign evidence (24,288-iteration zero-find run) serves as
  lever-exhaustion documentation, not as the source of the construct.

## T5 family check
- The banned construct for this function is a NEWLY-INVENTED dead-scalar
  holder local for the low mask (any fresh scalar, any name/type). This diff
  contains NO newly-invented local: both staged sets borrow variables the
  function already uses for a real job (loaded from the record table,
  +0x80-adjusted, passed to SetDrawMove), dead after the call. The 2026-08-11
  judge ruling holds these are different constructs, not two spellings of one
  — the family's own bound #2 draws that exact line. No other forbidden
  family matches: no pins, no inline asm, no volatile, no dead stores, no
  unused declarations, no alias renames.

## T6 naming-announces-intent
- No coercion-named identifiers. Locals are buf/tbl/u/v/pkt/ot — u and v are
  the texture coordinates' natural names, pre-existing in the function's data
  flow; no pad/dummy/spill/unused-style names.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable (two instances: the low-mask set
    through u, the high-mask set through v)
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:46
  (Function-specific classification of the low-mask instance: judge ruling
  commit d8c4b01f, PASS — "DISTINCT SANCTIONED INSTANCE — form admissible";
  the high-mask instance was ruled "properly annotated and evidenced" by the
  prior layer-1 review, banked in commit e6c1114e.)

ANNOTATION-CONFORMANCE: two FAKE constructs, both annotated verbatim at their
sets in src/text1a_pre.c:

    /* FAKE: OT pointer-field mask staged through the dead u-coord local (its
       value was consumed by the draw call above; the mask is read by both
       packet-link statements below), mechanism: global.c allocno call-arg
       copy preference ($a2 from the pre-call (s16)u arg copy) places the
       multi-set pseudo at $a2 with no local-alloc mask qty left to contest
       it, and the explicit set's LUID position restores the target li+ori
       emission order, lever-exhaustion: memory/grind/func_800401CC/
       hypotheses.md K1-K4 + P1/P2 + s2-permuter A-probes (all natural
       spellings measured broken) */

    /* FAKE: OT-code mask staged through the dead v-coord local (same
       liveness argument as u above), mechanism: global.c allocno call-arg
       copy preference ($a3 from the pre-call (s16)v arg copy) keeps the
       multi-set pseudo at $a3, removing 0xFF000000 from the local-alloc
       qty pool, lever-exhaustion: same ledger sections as u */

Each carries what + mechanism (named GCC pass) + lever-exhaustion (ledger
sections hypotheses.md K1-K9/H8, verified against the ledger not this
session's claims — the exhaustion was measured in prior sessions and the
judge ruling confirmed it "demonstrably spent").
