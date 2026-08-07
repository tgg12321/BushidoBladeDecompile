# SELF-VET — func_80048AD0

CONSTRUCTS: (1) counter-reuse of `sound` as the record-loop counter (FAKE-annotated, carried over unchanged from the s1 layer-2-confirmed (B) form); (2) direct array-read call argument `snd_LoadBgm((&D_80099BCC)[idx])` while `sound` caches the same element for the 0xFF sentinel check (NEW this session — the s2 andi-free spelling); no other construct differs from a plain transcription of the function.

## T1 semantic purpose
- Construct 1 (counter reuse): the counter is live, stored into every record (`sh $a0` per iteration), compared, incremented — fully observable output. The *choice* of reusing `sound` rather than a fresh local is the codegen-control element; it is exactly the SOTN-sanctioned variable-reuse family and is FAKE-annotated with mechanism + exhaustion.
- Construct 2 (direct arg read): the expression IS the call argument — load-bearing dataflow, not decoration. It is not byte-inert relative to the "simpler" spelling (`snd_LoadBgm(sound)`): that spelling emits an extra `andi $a0,$a0,0xff` (measured sandbox 1); this one emits target's exact bytes (sandbox 0). The emitted code contains exactly ONE `lbu` (CSE folds the two source reads), so no dead or duplicated instruction exists in the output.

## T2 human-programmer
- Construct 1: a human would more naturally use a fresh `i`; that is why it carries the FAKE annotation under the sanctioned family rather than claiming naturalness.
- Construct 2: `snd_LoadBgm(sound_table[idx])` is an utterly ordinary spelling — pass the table element to the loader; the cached `sound` local exists for the 0xFF sentinel check. Cache-for-check plus pass-the-expression is common human C. A reader would not ask "why is this here?" — the argument has to come from somewhere, and the table element is the natural source. Also note the type honesty: the table is u8, the callee's declared parameter is u8; passing the u8-typed element (rather than the s32 cache) is the type-accurate spelling.

## T3 GCC-internals justification
- Construct 1: yes, the FAKE annotation names the mechanism (set_preference / expand_preferences / prune_preferences) — as REQUIRED for a FAKE construct inside a sanctioned family; that is the annotation's mandatory content, not an unsanctioned lever.
- Construct 2: the derivation used CSE/preference reasoning to FIND the spelling, honestly disclosed. The program-logic explanation exists independently: the argument passed is the sound id read from the table, and the emitted load is the real load that feeds the call. No instruction exists in the output whose only explanation is a GCC pass.

## T4 permuter/search provenance
Neither construct is auto-search output. Both are hand-derived: construct 1 inherited from the s1/WIP RTL analysis (layer-2 confirmed by two reviewers on its merits per evidence.md), construct 2 derived this session from the global.c expand_preferences source reading plus the target's u8 parameter semantics.

## T5 family check
- Construct 1: sanctioned family "Variable reuse for codegen control" (see SANCTIONED-FAMILY-CLAIMS).
- Construct 2: primary position — ordinary live C (argument expression with real dataflow), needing no family cover; it is not a member of any forbidden family (no dead store, no unused local, no volatile/alias coercion, no asm, no declaration edit). Secondary cover: it is a duplicate READ of a global with sanctioned-precedent shape (SOTN duplicate-read family, see claims). Explicitly NOT a respelling of refused edit (A): (A) was a width/pointer DECLARATION change to the snd_LoadBgm extern; this form touches no declaration — it is the reviewer's own named path (ii): "Reach the andi-free codegen WITHOUT any width/pointer-changing declaration edit" (evidence.md), which the reviewer called UNEXPLORED, not exhausted.

## T6 naming-announces-intent
Locals are `temp_v0, sound, idx, base, delta, p, q` — descriptive of role, no `pad/dummy/unused/spill/slack` names, no address-of-only or discard-only uses. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control (construct 1)
  SCOPE: "Variable reuse for codegen control ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with \"FAKE but makes register allocation work\" comments."
  PRECEDENT: .claude/rules/inline-asm-policy.md §"SOTN-accepted techniques (resolved 2026-06-02 borderline-rule research)" first bullet (SOTN idxSub/randy); in-project: this exact construct on this exact function was layer-2 CONFIRMED by two reviewers — memory/grind/func_80048AD0/evidence.md §"Layer-2: (B) CLEARS on its merits" (2026-08-07).

  FAMILY: Duplicate-read into branch arms (construct 2, secondary cover only)
  SCOPE: "Duplicate-read into branch arms ([[split-read-defeats-hoist]] #1+#2): pin offset computations inside their branch via duplication. SOTN ships `color_fake = *palette;` repeated rebinds (`src/dra/42398.c`)."
  PRECEDENT: SOTN src/dra/42398.c repeated `color_fake = *palette;` rebinds as cited in .claude/rules/inline-asm-policy.md; the construct here is the same duplicate-read intent (re-read a memory location instead of routing the cached local) though not arm-shaped — if layer-1 judges the arm-shape scope sentence too narrow to cover a straight-line duplicate read, the correct disposition is a ruling on construct 2, and the author flags this explicitly rather than claiming the family by analogy.

ANNOTATION-CONFORMANCE: one FAKE construct; annotation present in src/text1b.c at the loop head:
  /* FAKE: the record counter reuses `sound` rather than a fresh local. ... */
  carrying WHAT (counter reuses `sound` rather than a fresh local), MECHANISM (global.c set_preference / expand_preferences / prune_preferences preference routing, named passes), and LEVER-EXHAUSTION (~60 variants over 8 sweeps + 3 fresh s1 kills, pointer to memory/grind/func_80048AD0/evidence.md). Construct 2 carries no FAKE annotation because it is live argument dataflow, not a dead/no-semantic-purpose construct; its derivation and measurements are documented in evidence.md s2.
