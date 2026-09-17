# SELF-VET — func_80073200

CONSTRUCTS: (1) `s.sp43 = 0x14;` duplicated into BOTH arms of the `D_800A35C4+8 & 4` if/else (else-arm copy is the FAKE-annotated one; the if-arm copy is not annotated because it is the arm's ordinary/first write of that field); (2) `cond` — a fresh once-written, once-read local naming the `D_800A3580` read ahead of the pointer-bump statement.

## T1 semantic purpose
(1) Yes at the C-semantics level (each arm now independently sets its own `sp43` value — same runtime behaviour as the prior single join-point write, but this is a legitimate duplicated-statement spelling, not a no-op). At the CODEGEN level the duplication's purpose is to remove the `sb` store from schedule pass 1's basic-block-4 ready list so the `(s32)&s` argument-set insn is not displaced — a real, measured effect (score 2 -> 0, confirmed this session on the current chassis).
(2) `cond` has a real effect: it holds a value that is genuinely read once at `if (cond < 2)`. Removing it changes codegen (measured: 0 -> 7, `tmp/grind/func_80073200/s9/a_no_cond.c`), so it is not a no-op relative to the compiled bytes, though relative to *program semantics* it is a pure rename of `D_800A3580`'s read.

## T2 human-programmer test
(1) A human writing this from the spec alone would likely write the single join-point store once. This construct is only present because it closes a measured scheduling residual — it fits the SOTN `duplicated-statement-into-arms` sanctioned family exactly (a real statement written into 2+ arms), which the family's own rule text says is legitimate specifically because ordinary human C over that family's precedent files does exactly this (7-arm/11-arm identical duplicated stores in doppleganger.c). Marked FAKE, as required.
(2) A human might or might not name the read `cond` before the branch; it reads as an ordinary staged predicate variable, no smell.

## T3 GCC-internals justification test
(1) Yes — the FAKE comment cites the mechanism (`schedule_select`'s `potential_hazard` swap, sched.c:2717) BUT the construct's own C-level behavior (each arm sets its own byte value) is a completely ordinary and independently defensible spelling; the GCC-internals note is present because this is a LAST-RESORT sanctioned construct under `duplicated-statement-into-arms`, which mandates naming the mechanism.
(2) Yes, same treatment: `cond` is annotated with its LUID/scheduling mechanism per `staged-value-reused-variable`-adjacent reasoning (though `cond` is a FRESH named intermediate, so `named-intermediate` family, not `staged-value-reused-variable`, is the more precise fit — see FAMILY block below).

## T4 permuter/search provenance
Both constructs were *validated* across multiple permuter/enumeration sessions (s4-s8) that swept alternative spellings and found them all dead — but neither construct originated from "permuter found this, ship it blind." Both are named, mechanism-cited, and hand-derived from the `.sched` dump reading this session.

## T5 family check
(1) Matches `duplicated-statement-into-arms` (`.claude/rules/duplicated-statement-into-arms.md`) exactly: a REAL statement (`s.sp43 = 0x14;`) written into 2+ control-flow arms, with the effect being a scheduling/priority change, byte-neutral because jump2's `find_cross_jump` re-merges the two identical tails.
(2) Matches the amended named-intermediate / `new_var_temp` family (`.claude/rules/no-new-park-categories.md` § SOTN-accepted, "Named-intermediate declaration order", as relaxed by [[ordinary-c-judge-decidable]] Ruling 1 to "once-written" — here `cond` is also once-read, satisfying the stricter original prong too).

## T6 naming-announces-intent test
`cond` and `s.sp43`/arm-store carry no `pad`/`dummy`/`spill`/`unused`/`slack` naming. `cond` reads as an ordinary boolean/comparison staging variable — no coercion-intent name. The previously-flagged `u8 var_v0;` (layer-1 FAIL target) has been DELETED from this body — it is no longer present; the CONSTRUCTS list above no longer includes it, and grep confirms zero remaining references to `var_v0` in the source.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, **including** when: - GCC's jump2 cross-jump re-merges the copies so the final bytes are identical to the shared-label form, and - the duplication's surviving effect is the extra `reg_n_refs` count flow.c records (allocno-priority lift for global RA)"
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:13

  FAMILY: named-intermediate (new_var_temp, ordinary-c-judge-decidable Ruling 1 relaxation)
  SCOPE: "the named-intermediate entry's 2026-08-17 prong (1) is relaxed from "once-written, once-read" to **"once-written"** — a fresh local holding a real, consumed value may be read any number of times."
  PRECEDENT: .claude/rules/ordinary-c-judge-decidable.md:65

ANNOTATION-CONFORMANCE:
  /* FAKE: `s.sp43 = 0x14;` is written in BOTH arms instead of once
   * at the join.  Byte-neutral - jump2's find_cross_jump re-merges
   * the two identical arm tails, so nothing extra materializes
   * (build_insns 203 == target_insns 203).  mechanism: the FIRST
   * scheduling pass, schedule_select's `potential_hazard` ready-list
   * swap (tools/gcc-2.7.2/sched.c:2717).  Keeping the two `sb` stores
   * out of that pass's basic block 4 removes the only ready insns
   * that could displace the `(s32)&s` argument set at its T-50 step.
   * lever-exhaustion: memory/grind/func_80073200/hypotheses.md s4-s8
   * (two permuter campaigns, the 1957-ordering spelling_enum sweep,
   * all 23 orderings of this call group, the addr-local naming). */
  (src/text1b.c, else-arm of the D_800A35C4+8&4 branch)

  /* FAKE: `cond` names the D_800A3580 read as a fresh once-written,
   * once-read intermediate declared ahead of the pointer bump, so the read
   * is emitted before the branch rather than after it.  mechanism: LUID
   * order into the first scheduling pass - the named read becomes the
   * delay-slot-fillable insn the target puts between `lw v0,0x18(s0)` and
   * `beqz` (asm/funcs/func_80073200.s:151-157).  lever-exhaustion:
   * memory/grind/func_80073200/hypotheses.md s5/s6 (the complementary
   * in-block form and both declaration-order sweeps measured dead); ablated
   * again s9 - removing it regresses 0 -> 7
   * (tmp/grind/func_80073200/s9/a_no_cond.c). */
  (src/text1b.c, before the D_800A3580 branch)

BANNED-CONSTRUCT CHECK: the brief's banned-construct entry (`u8 var_v0;` — declared, zero other occurrences) is DELETED from this body. Grepped `var_v0` in src/text1b.c: zero hits. The remaining two constructs above are unrelated, separately-sanctioned families, each already carried forward from the s9 candidate (unchanged this session other than the var_v0 deletion).
