# SELF-VET — func_80032314

CONSTRUCTS: single-level do-while(0) wrap (FAKE-annotated), canonical GTE LZCS/LZCR
inline-asm island (mtc2/swc2 mnemonic block), goto-form loop (loop:/next: labels),
sub-word pointer reads (*(u16*)(ent+0x6A), *(u8*)(a3+1), *a3), sibling tail
arithmetic (v0_m mask / shift-chain log2 spelling), register-convention local names
(t0/t1/a3/ent/a0/v0/v1 per the file's established splat/m2c naming style).

## T1 semantic purpose
- do-while(0) wrap: NONE beyond what the unwrapped body produces — it is purely a
  match device. That is exactly the case its sanctioning rule covers: the owner's
  2026-07-06 FINAL ruling sanctions this ONE no-semantic-purpose wrapper as a pure-C
  match device; it asserts nothing false ("this body executes once" is true) and every
  byte still comes from the pristine compiler consuming legal C. Declared openly via
  the mandatory FAKE annotation.
- GTE island: real computation (leading-zero count feeding the log2 table index);
  cop2 ops have no C form; ASM-PARTIAL per the canonical gate (2/109 insns).
- All other constructs: ordinary program logic (loop over 4 records, state filtering,
  squared distance, table lookup, threshold store). Every statement's value is
  consumed; there are no dead stores, no unused locals, no pads.

## T2 human-programmer
- do-while(0): a human would not write it from the spec — which is why it carries the
  FAKE annotation, per the sanctioned family's own convention (SOTN ships 18+ such
  wraps in master with the same posture). Within the family, T2 is answered by the
  annotation requirement, and it is the canonical C macro-body idiom of the PsyQ era
  (a wrap is indistinguishable from an original macro expansion).
- Everything else: yes — the goto-form loop and byte-pointer reads mirror the matched
  same-file siblings (func_800274BC etc.); the tail arithmetic is copied from the
  user-authorized matched sibling's spelling.

## T3 GCC-internals justification
- do-while(0): the mechanism IS GCC-internal (flow.c loop-note ref weighting ->
  global.c allocno priority, ALLOCDBG-measured), which is permitted for a construct
  sitting INSIDE a sanctioned family with its prerequisites met — the rule itself
  says the annotation should name the observed effect. The FAKE annotation does.
- No other construct in the diff is justified by GCC internals; each is the plain
  spelling of the function's behavior (or, for the GTE island, the authorized
  canonical form).

## T4 permuter/search provenance
- The wrap WAS found by the s4 permuter campaign (basin B, output-0-1). It is
  submitted not because detectors miss it but because it lands inside a sanctioned
  family with every prerequisite met: family sanction verified against the rule file,
  mechanism measured independently (ALLOCDBG), honest sandbox re-verified (0 at
  109/109 with 0 rules, re-measured this session), FAKE annotation present. Vetting
  per permuter-directives §Vetting and no-new-park-categories §Auto-search tools was
  performed (this document is that vet). Basin A's 32,797-iteration null is banked as
  the natural-geometry exhaustion measurement.

## T5 family check
- do-while(0) wrap: IS the sanctioned family (block below). Single-level, so the
  nested-wrap extra documentation duty does not apply. It is not a syntactic
  equivalent dodge (no for(;;)/while(1)/if(1) respelling) — it is the literal
  sanctioned construct.
- GTE island: canonical-asm category (inline-asm-policy "canonical"), NOT cheat-asm:
  mnemonic cop2 block with %-placeholder input binding, no hardcoded-$N GP-only
  moves outside the authorized preamble shape, identical to the user-authorized
  sibling block (src/code6cac_b.c:292) and covered by the 2026-08-17 cluster ruling
  membership (block below).
- No other construct matches any forbidden family: no pins, no barriers, no volatile,
  no alias renames, no dead stores, no unused arrays/scalars, no detour arithmetic
  (the s3 F4 chain-extender lines are NOT in this submission).

## T6 naming-announces-intent
- No pad/dummy/unused/spill/buf/tail/slack names anywhere. Locals named t0/t1/a3/
  v0/v1/ent follow the file's established register-convention naming used across its
  matched siblings (e.g. func_800274BC, func_800324D0); every one is written AND read
  on live paths. sp_tmp is the GTE island's result slot — written by the authorized
  swc2, read into clz — the same name/shape as the authorized sibling.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap (do-while-zero-exception)
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:23

  FAMILY: canonical GTE cop2 island (cop2-addressing-preamble-cluster membership)
  SCOPE: "A member inherits the disposition only when all of these hold. This is a check, not a lever — it does not lower anyone's distance:"
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:104

ANNOTATION-CONFORMANCE: the following inline annotation is present at the construct
site in src/code6cac_b.c (what + mechanism + lever-exhaustion) — re-verified in the
s5 annotation-fix session (2026-08-20) with the candidate re-applied to src/ and
sandbox --disable all re-measured at 0 (109/109 insns, 0 rules); the only ledger
change this session is the cop2-addressing-preamble-cluster PRECEDENT line
correction :86 -> :104 per the Judge's citation fix-up notice:
  /* FAKE: single-level do-while(0) wrap (body executes once), mechanism:
   * the wrap's NOTE_INSN_LOOP notes make flow.c weight in-wrap reg_n_refs
   * by loop_depth, re-ranking global.c allocno priorities (walker 4390 <
   * ent 4761 < mult-temp 8000) into the target $a1/$a2/$a3 seating —
   * ALLOCDBG trace tmp/grind/func_80032314/s4/allocdbg.txt.
   * lever-exhaustion: memory/grind/func_80032314/hypotheses.md (s2
   * arithmetic closure of the pure-C rotation + s3 premise-hole
   * measurements + s4 permuter nulls on the natural-geometry chassis). */
The GTE island carries its canonical-authorization comment (sibling-identical block,
user-authorized 2026-06-10) rather than a FAKE line — it is canonical asm, not a FAKE
construct.
