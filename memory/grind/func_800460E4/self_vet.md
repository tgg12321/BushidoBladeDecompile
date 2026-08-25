# SELF-VET — func_800460E4

CONSTRUCTS: (1) integer-add scaled-index address spelling `(s32 *)((s3 << 2) + (s32)s0)` at 4 sites, (2) dropped p/p2 param-alias locals (direct `((s32 *)arg1)[...]` / `(u8 *)arg1 + ...` reads), (3) combine-foldable chain-extender detour on s1's default init (`s1 = (s32 *)((s32)s4 - (s32)s0); s1 = (s32 *)((s32)s1 + (s32)s0);`), FAKE-annotated, (4) fresh pointer intermediates in case 3 (`s32 *pm2 = ptr - 2; s32 *pm1 = ptr - 1;` with `raw_m2 = *pm2; raw_m1 = *pm1;`), FAKE-annotated

REMOVED CONSTRUCTS: the case-3 `*(volatile s32 *)&ptr[-1]` cast (BANNED for this function, layer-1 FAIL 2026-08-25 03:53) is DELETED from the body — the candidate contains no volatile anywhere. The banned arg1/s1 whole-function merge (layer-1 FAIL 03:37) also remains absent: `s32 *s1;` is the real carrier and arg1 is never written after the fp_ptr block.

## T1 semantic purpose
(1) Computes the same element address as the previous spelling — real address arithmetic, value consumed at every site. PASS.
(2) Reads the caller-provided buffer directly through the parameter instead of through single-use alias locals — real loads, all consumed. PASS.
(3) The stored value is LIVE (the default carrier value s4, consumed by `func_80045600` on non-case paths); the DETOUR itself (subtract-then-rebase instead of direct copy) has no purpose beyond its folded equivalent `s1 = s4;` — exactly the shape the sanctioned chain-extender clause covers, hence the mandatory /* FAKE */ annotation. Declared, not hidden.
(4) pm2/pm1 hold the real addresses of the two header words case 3 reads; each is once-written, once-read, and the values are consumed (the loads feed s6/s4, both passed to callees). The SPELLING choice (pointer locals instead of `ptr[-2]`/`ptr[-1]` indexing) is codegen-motivated, which is why it is FAKE-annotated under the named-intermediate family rather than passed off as neutral style.

## T2 human-programmer
(1)(2) Yes — ordinary C (index arithmetic, direct param deref).
(3) No — a human would write `s1 = s4;`. FAKE-family last resort; the annotation announces it.
(4) Plausibly yes (`s32 *pm1 = ptr - 1;` is idiomatic C for "the word before the table"), but because this session CHOSE it for its codegen effect it is disclosed and annotated as FAKE rather than claimed as neutral.

## T3 GCC-internals justification
(1)(2) None needed — program logic; they also happen to match target operand order (measured [s1]).
(3) Yes — mechanism: flow.c records +2 reg_n_refs on s1's pseudo before combine folds the detour to `s1 = s4` with zero emitted bytes; global.c allocno_compare then seats the carrier first, giving target's disposition. A named GCC-pass mechanism is REQUIRED by the claimed family; not a first-reach mechanism claim.
(4) Yes — mechanism: the plain-var derefs emit non-MEM_IN_STRUCT_P MEMs, so sched.c's dependence analysis raises REG_DEP_ANTI edges from both loads to the `D_8009947A` store (dump-proven: dumps/text1a_c2.sched insn 320 carries `REG_DEP_ANTI 312`/`REG_DEP_ANTI 315`), forcing target's load/store order by REAL dependence; the downstream seats then differ from case 34's tail so jump2 cannot cross-jump-merge case 3's 3-insn ALIGN4 suffix. Named-mechanism disclosure is a prerequisite of the claimed family.

## T4 permuter/search provenance
No construct is auto-search output. (4) was derived from first-principles diagnosis (disassembly + .sched dump) after a measured statement-order sweep (P1-P4, all flat) and a sched_solver perturbation kill (target order unreachable at depth<=3 over the spellable atoms and depth 2 over ALL atoms) — banked in rejected/case3-statement-order-sweep.c and evidence.md [s4].

## T5 family check
(1)(2) Ordinary C — no family needed.
(3) Matches the combine-foldable chain-extender clause of the sanctioned dead-store family (owner ruling 2026-07-01 same-day scope extension) exactly: live store, algebraically-equivalent detour, combine folds to direct form, zero emitted bytes (248/248, score 0), surviving effect is the reg_n_refs count. NOT the banned merge: s1 is a real local, arg1 untouched.
(4) Matches the frozen-list named-intermediate entry as re-scoped by the 2026-08-17 owner clarification, all six prongs: (i) each intermediate once-written/once-read; (ii) real value — the addresses materialize in target's own bytes as the `-8`/`-4` load offsets (combine folds the decrements into the addressing); (iii) byte-neutral — build_insns == target_insns == 248, score 0; (iv) fresh locals, not borrows of existing variables (staged-value-reused-variable's bounds are not implicated); (v) destinations not live-pre-initialized (fresh declarations); (vi) dump-proven named mechanism + documented lever-exhaustion + FAKE annotation + this vet. NOT the banned volatile respelled: no volatile qualifier, no coercion cast, no second handle to any object — the construct works through real scheduler dependences on ordinary loads, and the emitted load instructions are byte-identical to target's.
(4, negative check) It is not dead-store (values consumed), not pointer-alias-to-global (pm2/pm1 point into the s0 buffer, not at a global symbol), not variable-reuse (fresh declarations).

## T6 naming-announces-intent
No pad/dummy/spill/unused names. pm2/pm1 are descriptive ("pointer to minus-2/minus-1 word"); raw_m1/raw_m2/off*/ptr descriptive; s0/s1/... follow this TU's committed register-style naming.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-store-fake-exception (combine-foldable chain-extender clause)
  SCOPE: "a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes — its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:32
  FAMILY: named-intermediate declaration order (frozen SOTN list entry, 2026-08-17 clarification)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written, once-read — multi-write carriers are NOT this entry (the `y1` FAIL, decisions.md:1833, stands);"
  PRECEDENT: .claude/rules/no-new-park-categories.md:193
  PRECEDENT: docs/reference/sotn-construct-index.md:113

ANNOTATION-CONFORMANCE: two FAKE constructs, each annotated ON the statement in src/text1a_c2.c. (3): `/* FAKE: live default init of s1 routed through a delta-rebase detour that combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare priority above the s2 pointer so allocation order matches target, lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */`. (4): `/* FAKE: fresh once-written/once-read pointer intermediates for the two header reads; the plain-var derefs emit non-struct MEMs so sched.c's alias check keeps the D_8009947A store after both loads (target order), mechanism: sched.c true/anti-dependence on a fixed-symbol store vs non-MEM_IN_STRUCT_P varying load, lever-exhaustion: evidence.md [s4] (statement-order sweep P1-P4 measured flat; perturb.py goal unreachable depth<=3) */`. Both carry what + mechanism (named GCC pass) + lever-exhaustion pointer.
