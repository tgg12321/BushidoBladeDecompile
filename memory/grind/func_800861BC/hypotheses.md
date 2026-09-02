# Hypothesis ledger — func_800861BC

## s1 (recon, 2026-09-02) — chassis: asm-until-matched HEAD 2829a7b0, no FAKE constructs
- H1 CONFIRMED — psyz `_SsVmDoAllocate` shape over a `struct struct_svm` at
  D_801027F0 reproduces the top block's `$v1=&voiceOffset` / `$t1=$v1-2`
  addressing (explow.c memory_address force_reg + cse.c use_related_value +
  loop.c hoist). sandbox 26 = 13 real + 13 addend noise.
- H2 CONFIRMED — the residual is the post-join block's cse anchor (first
  materialized `_svm_cur` address). Target anchors voiceOffset, so dirty|=8
  must be emitted inside the arms and re-merged by jump2 cross-jump.
  v4 (literal duplication) and v8 (`static inline` helper) both: sandbox 18,
  real 0, 132/132. Oracle SHA1 == 62efab4f… with v4 in src/main.c.
- H3 KILLED (instance) — psyz's `voice = _svm_cur.voice;` unused local: dead
  read deleted, sandbox 26, anchor unchanged.
- H4 KILLED (instance) — local `voice` consumed by dirty|=8 only: the local's
  load is still the block's first `_svm_cur` address → anchor voice, 26.
- H5 KILLED (instance) — adsr1 statement before dirty|=8: anchor flips to
  voiceOffset but the dirty|=8 sequence is emitted after the adsr1 stores
  (sched.c will not hoist `sb` past may-alias stores), 43.
- H6 KILLED (instance) — dirty|=8 before the if/else: emitted before the
  branch; target has it after the join label; no pass sinks it, 37.
- H7 KILLED (instance) — retired-chassis body (separate splat scalars +
  `pc=&D_8010280C` pointer alias + byte puns): 37 on this chassis; the pointer
  pun is the form the 2026-08-17 ruling names as debt (prong d).
- H8 CONFIRMED — complete merge (nine per-word symbols → struct fields at every
  main.c consumer) is byte-neutral: verify-oracle ok with merged_main_s1.diff.

## Frontier (for the integrating session / operator)
1. Land the aggregate merge header-canonically (include/sound.h or new
   include/libsnd.h) + candidate.c body; verify-oracle; layer-2 review. The
   sandbox will read 18 (addend artifact) — cite e788983a precedent.
2. Optional cleanup after the merge: measure whether func_80087CAC's FAKE
   pointer alias to D_80102806 (`_svm_cur.seq_sep_no`) retires under natural
   field access (same explow.c force_reg mechanism).
3. If the reviewer rejects the `static inline` helper form, candidate_v4_dup_arms.c
   (FAKE-annotated duplicated-statement family) produces identical bytes.

## s1 re-run (recon, 2026-09-02, second dispatch) — chassis HEAD ea3f0a59, no FAKE constructs
- H9 CONFIRMED — header-canonical form (struct_svm + ProgAtr in include/sound.h,
  helper body reading `.reserved2`/`.reserved3` of BB2's existing ProgAtr,
  nine per-word externs deleted, all consumers rewritten): sandbox 18 at
  132/132, filt real=0 noise=18, verify-oracle --rebuild SHA1 == oracle.
  Banked as integration_patch.diff. Identical instruction stream to v4/v8.
- H10 CONFIRMED (driver-source reading, not a codegen probe) — the candidate
  gate's sandbox-0 requirement is unreachable for this function by
  construction: 18 named-symbol LO16 addends (engine/score.py:61 compares
  them; grind.ps1:578-583 requires 0) and the struct spelling is the only
  one that reproduces `addiu $t1,$v1,-2` (H1/H7). Disposition = integration
  handoff, operator landing (func_80038170 precedent cbcfda04 / 2fb471ac).

## Frontier (unchanged in substance; for the Judge / operator)
1. Land integration_patch.diff (include/sound.h + src/main.c); build ->
   SHA1 == oracle; layer-2 cheat-reviewer; queue done. The sandbox reads 18
   until the reference object is regenerated from C, then 0.
2. After landing: measure whether func_80087CAC's FAKE pointer alias to
   `_svm_cur.seq_sep_no` retires under natural field access (explow.c
   force_reg on a struct-offset address).
3. If layer-2 rejects the `static inline` helper, candidate_v4_dup_arms.c
   (FAKE-annotated duplicated-statement family) is byte-identical.

## [s1] The header-canonical form (struct struct_svm + ProgAtr in include/sound.h, static-inline helper body, nine per-word externs deleted, all main.c consumers rewritten) compiles to the target instruction stream and links to the oracle SHA1.
- mechanism: explow.c memory_address force_reg materializes struct-offset addresses into pseudos; cse.c use_related_value anchors later _svm_cur accesses on the block's first materialized field; jump2 cross-jump re-merges the helper's two inlined copies into the post-join tail, leaving voiceOffset as the tail anchor.
- probe: python tmp/grind/func_800861BC/s1/integrate.py; sandbox func_800861BC --disable all; pairdiff + filt.py; verify-oracle --rebuild --allow-dirty
- result: sandbox 18 at 132/132 (rules_dropped 0); filt real=0 noise=18 (every hunk = identical insn, LO16 immediate D_801027F0+N vs per-word symbol +0); verify-oracle ok:true build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle; nm build/src/main.o: T func_800861BC.
- verdict: CONFIRMED

## [s1] The driver candidate gate's sandbox-0 requirement is unreachable for this function on this chassis: the struct spelling is the only one measured to reproduce the target's addiu $t1,$v1,-2 (separate scalars measured 37, H7), and it necessarily addresses all nine fields as D_801027F0+N with N != 0, which engine/score.py:61 compares against the INCLUDE_ASM reference's per-word symbols at addend 0.
- mechanism: engine/sandbox.py:72 reference = build/src/main.o (INCLUDE_ASM'd asm naming D_8010280A/0C/... at addend 0); engine/score.py:61 masks section-relative addends only; tools/grinder/grind.ps1:578-583 requires score 0. Fix B (oracle-SHA1 acceptance) declined in f45b33cd on the ground that no function needed it.
- probe: driver/engine source reading + the 18-hunk pairdiff classification (s1/v9_header_form_pairdiff.txt)
- result: 18 of 18 residual hunks are addend-only (real 0); the gate reads 18 for every struct spelling (H1 26, v4 18, v8 18, v9 header form 18) and 37 for the pointer-free scalar chassis. Disposition: integration handoff / operator landing (func_80038170 precedent).
- verdict: CONFIRMED
