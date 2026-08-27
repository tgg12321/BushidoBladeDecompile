# SELF-VET — func_800485EC

CONSTRUCTS: F6 cancellation pair `tim++; tim--;` (the single FAKE construct); K&R definition + unprototyped `extern void func_800485EC();` (ordinary C, ABI-evidenced); GetClut/GetTPage promoted-arg prototypes (ordinary C, mirror src/gpu.c definitions); named intermediate `bnum` (ordinary once-written/once-read live load); pointer walker `p` with natural defs `p = tim;` / `p = tim + 2;` (ordinary C).

## T1 semantic purpose:
- `tim++; tim--;` — NONE (net-zero, deleted by flow.c before emission). This is exactly the F6 family's definition: "semantically-null fabricated statement pairs ... inserted solely for codegen" — the family sanctions the absence of semantic purpose for this exact shape, with prerequisites (annotation + exhaustion) met. Answered by family membership, the same posture as a sanctioned dead store.
- All other constructs: full semantic purpose — every statement computes or stores a value the function's spec (TIM header parse) requires; `p = tim + 2` is the natural pixel-block pointer (the spelling the layer-1 FAIL itself described as "the 'natural' `p = tim + 2;` spelling").

## T2 human-programmer:
- The body minus the pair is exactly what a human writes for a TIM parser (magic check, flag word, optional clut block walk, pixel block walk); the clut arm and tail mirror each other's walker idiom.
- The pair: a human would not write it from spec — which is why it carries the `!FAKE` annotation the F6 family mandates. Same answer the frozen list gives for dead stores / constant holders / do-while(0).

## T3 GCC-internals justification:
- The pair's justification IS GCC-internals (cse.c fold_rtx PLUS-association / exp_equiv_p reg_tick invalidation; flow.c deletion) — stated openly in the annotation, as the FAKE convention requires. For a sanctioned family this test is answered by the family's own sanction (F6 sanctions statements "inserted solely for codegen"); the test flags UNSANCTIONED constructs justified by internals, not annotated members of the frozen list.
- No other construct in the diff has an internals-based justification: the K&R linkage is ABI-proven from target bytes (callee lhu at +0x38/+0x3C vs caller sll/sra+sw), the prototypes mirror the real gpu.c definitions, `bnum`'s naming is ordinary source structure.

## T4 permuter/search provenance:
- No permuter/auto-search was used in s2. The closer was derived from reading the frozen cse.c source (find_best_addr / fold_rtx) plus the s1 ledger's dump evidence, then confirmed by dumps this session. Not a detector-evasion find.

## T5 family check:
- `tim++; tim--;` matches the sanctioned F6 family EXACTLY: "an adjacent same-variable increment/decrement pair (`i++; i--;`)" — same variable (tim), adjacent, exact ++/-- spelling, net zero. The entry's exclusion ("the `+= 2 / -= 1` respelling ... remains banned") is a NET-NONZERO disguised increment — not this construct. The stride-2 variant (`+= 2; -= 2;`) also measured 0 but was NOT used, to stay inside "Sanctions ONLY the exact cancellation pair".
- BANNED-construct check (this function's ban: `tim += 2; p = tim;`): not present in any spelling. The banned construct's signature — a NET advance of the dead parameter whose advanced value is consumed by a copy-def of p (live-arithmetic laundering, no family) — is absent: here p's def is the independent natural `p = tim + 2;`, tim's net change is zero, and the fabricated statements are a distinct, sanctioned, annotated construct. The layer-1 FAIL's stated ground was "not covered by any currently sanctioned family"; this construct is covered.
- No other forbidden family present: zero asm, zero pins, zero volatile, zero regfix/asmfix rules (sandbox reports rules_dropped 0), no dead stores, no unused locals, no alias renames (grep-verified over the function body).

## T6 naming-announces-intent:
- No coercion-announcing names. Locals: `tim` (parameter, TIM data pointer), `spr` (sprite header out-param), `p` (block walker), `flag`, `bnum` (TIM block byte-count field's conventional name), `x/y/cx/cy` (spec rect args). No pad/dummy/unused/spill/tail names; nothing declared-but-unused.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Semantically-null fabricated statement pairs (cancellation-pair / redundant-condition class), F6
  SCOPE: "an adjacent same-variable increment/decrement pair (`i++; i--;`) or a fabricated redundant condition / empty-if inserted solely for codegen."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:370`

ANNOTATION-CONFORMANCE: the construct site in src/text1b.c carries the inline block comment beginning `/* !FAKE: cancellation pair (sanctioned family: semantically-null fabricated statement pair, .claude/rules/no-new-park-categories.md:371-382, owner ruling 2026-08-18, F6 ESTABLISHED — exact `i++; i--;` shape). ... */` — it states WHAT (net-zero adjacent same-variable inc/dec of tim, byte-free, flow-deleted), the NAMED MECHANISM (cse.c fold_rtx PLUS-association cse.c:5589-5666 via find_best_addr cse.c:2663; exp_equiv_p reg_tick invalidation; flow.c dead-store deletion), and LEVER-EXHAUSTION (memory/grind/func_800485EC/hypotheses.md s1-s2: natural fresh-def 7, tim-walker 39, live-tim routing 22, def-in-arms 3, full-tail dup 16, plus the cse.c closed-form proof that every join-local p==tim+K chain folds).
