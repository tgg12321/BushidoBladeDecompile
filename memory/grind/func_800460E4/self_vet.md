# SELF-VET — func_800460E4

CONSTRUCTS: (1) integer-add scaled-index address spelling `(s32 *)((s3 << 2) + (s32)s0)` at 4 sites, (2) dropped p/p2 param-alias locals (direct `((s32 *)arg1)[...]` / `(u8 *)arg1 + ...` reads), (3) combine-foldable chain-extender detour on s1's default init (`s1 = (s32 *)((s32)s4 - (s32)s0); s1 = (s32 *)((s32)s1 + (s32)s0);`), FAKE-annotated

## T1 semantic purpose
(1) Computes the same element address as the previous spelling — real address arithmetic, value consumed at every site. PASS.
(2) Reads the caller-provided buffer directly through the parameter instead of through single-use alias locals — real loads, all consumed. PASS (removes indirection; byte-identical at those sites).
(3) The stored value is LIVE (the default carrier value s4, consumed by `func_80045600` on non-case paths); the DETOUR itself (subtract-then-rebase instead of direct copy) has no semantic purpose beyond its folded equivalent `s1 = s4;` — which is exactly the shape the sanctioned chain-extender clause covers, and why it carries the mandatory /* FAKE */ annotation. Declared, not hidden.

## T2 human-programmer
(1)(2) Yes — ordinary C a maintainer would write (index arithmetic, direct param deref).
(3) No — a human would write `s1 = s4;`. This is the FAKE-family last resort; the annotation announces it per the SOTN convention the rule encodes.

## T3 GCC-internals justification
(1)(2) None needed — justified by program logic; they also happen to match target operand order, measured in [s1].
(3) Yes — mechanism: flow.c records +2 reg_n_refs on s1's pseudo (80: 11→13, dump text1a_c2.flow:22) before combine folds the detour to `s1 = s4` with zero emitted bytes; global.c allocno_compare then seats 80 first ($s1), giving target's disposition 73→$17, 78→$18, 79→$19, 80→$17 (text1a_c2.greg:29-31). A GCC-pass mechanism is REQUIRED by the claimed sanctioned family (`dead-store-fake-exception` names reg_n_refs/flow.c in its own scope text); this is not a first-reach unsanctioned mechanism claim.

## T4 permuter/search provenance
No construct is auto-search output. (3) was derived from the ra_solver inverse verdict banked in [s1] (REACHABLE only via refs_up +2 on pseudo 73 or 80) plus the rule catalog; measured deliberately this session.

## T5 family check
(1)(2) Ordinary C — no family needed.
(3) Matches the combine-foldable chain-extender clause of the sanctioned dead-store family (owner ruling 2026-07-01 same-day scope extension) exactly: live store, algebraically-equivalent detour, combine folds to direct form, zero emitted bytes (248/248 insns, score 0), surviving effect is the reg_n_refs count. It is NOT the banned construct for this function: `s32 *s1;` is restored as the real carrier, arg1 is never written, no assignment/use site is repointed onto the parameter.

## T6 naming-announces-intent
No pad/dummy/spill/unused names. s0/s1/s2... follow this TU's committed register-style naming; raw_m1/raw_m2/off*/ptr are descriptive. The detour introduces no new names.

PRE-EXISTING CONSTRUCT NOTE (outside this diff): case 3 contains `s32 raw_m1 = *(volatile s32 *)&ptr[-1];` — committed HEAD code inherited from the rule-era body, unchanged by this session's diff, present in the body layer-1 reviewed on 2026-08-25 03:37 (not cited in that FAIL). Measured this session: removing the qualifier loses 3 target insns (245/248, score 9), so it is load-bearing; its provenance/classification is surfaced here for the reviewer rather than silently retained-or-respelled. If the reviewer rules it must go, the residual is that 3-insn fold, not this session's constructs.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-store-fake-exception (combine-foldable chain-extender clause)
  SCOPE: "a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes — its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:32

ANNOTATION-CONFORMANCE: the diff emits one FAKE construct, annotated ON the statement in src/text1a_c2.c: `/* FAKE: live default init of s1 routed through a delta-rebase detour that combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare priority above the s2 pointer so allocation order matches target, lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */` — carries what + mechanism (named GCC pass) + lever-exhaustion pointer (ledger: [s1] ra_solver inverse foreclosures + honest ref audit; [s3] duplicated-statement structural kill; dead stores measured inert for global RA per the duplicated-statement rule's motion_SetMotion record).
