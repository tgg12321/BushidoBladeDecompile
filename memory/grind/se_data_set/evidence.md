# Evidence bank — se_data_set

- Audit diagnosis (regressions.md): Two codegen-coercion constructs with no semantic purpose: (1) '(s8)(u16)D_800A36A4' forces lhu vs lh for no semantic gain (sb discards the sign-extension difference); (2) 's8 *p = (s8*)&D_8010277C' caches the address in a callee-save register across a call where a direct cast would suffice — inconsistency between condition (direct cast) and body (pointer) is the tell. Both fail tests 1+2+3 of the 6-test checklist. Function needs a clean redo: remove the (u16) intermediate cast and replace *p with (s8)D_8010277C throughout, then find the pure-C structure that naturally produces lhu and $s0 address-caching.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Target facts (asm/funcs/se_data_set.s): the CONDITION reads D_8010277C direct-split (lui $v0 %hi / lb %lo($v0), lines ~16-17); the BODY materializes &D_8010277C into callee-save $s0 (lui+addiu, lines ~45-46) and does `lb 0($s0)` twice — once before the func_8005BA8C call (line ~49) and once after (line ~62). The D_800A390E store reads D_800A36A4 with lhu (line ~61) while the function's other three reads of the same s16 global are lh (lines ~10, ~38, ~48).

- [s1] [fable-blitz 2026-07-07] Declarations: D_8010277C is `extern u8` (include/code6cac.h:401) so every (s8) VALUE cast is semantic (target loads lb, sign-extended index); D_800A36A4 is s16 (code6cac.h:95); D_800A390E is s8 (code6cac.h:222). The flagged `s8 *p = (s8 *)&D_8010277C` is a signed re-view through an lvalue — same sign semantics as the casts, different codegen route.

- [s1] [fable-blitz 2026-07-07] lhu mechanism (named pattern: mips.md movhi vs extendhisi2): plain HImode loads emit lhu; lh only comes from a fused sign_extend (extendhisi2). A store that only TRUNCATES the value (sb to the s8 global) never needs SImode sign-extension, so a plain `D_800A390E = D_800A36A4;` (or via a u16 local) plausibly loads HImode -> lhu naturally. The committed `(s8)(u16)` double cast is m2c's standard rendering of lhu+sb — likely verbatim transcription, not a needed lever. Three mask-free spellings to sweep: plain assignment, `u16 tmp = D_800A36A4; D_800A390E = tmp;`, single `(u16)` cast.

- [s1] [fable-blitz 2026-07-07] $s0 caching mechanism (cse.c, block-local): CALL_INSNs do not end basic blocks, so both body reads of D_8010277C sit in ONE basic block; their RTL is the identical sign_extend(mem:QI(sym)) form, letting cse unify the split HIGH/LO_SUM address into a full-address pseudo that lives across the call -> callee-save $s0. Direct spelling `*(&D_8008E5A8 + (s8)D_8010277C)` at BOTH body sites (arg of func_8005BA8C + the D_800A30FC refresh) may reproduce the $s0 caching with no pointer local at all.

- [s1] [fable-blitz 2026-07-07] Supporting contrast: D_800A36A4 is also read twice in the same body block but is NOT address-cached in target — its two reads use DIFFERENT extensions (lh at ~48, lhu at ~61), i.e. distinct RTL expressions, so no full-address CSE forms. This is consistent with (and evidence for) the identical-reads CSE story for D_8010277C.

- [s1] [fable-blitz 2026-07-07] Policy timing: this regression was flagged 2026-06-22, BEFORE the 2026-07-01 pointer-alias-fake-exception sanction (SOTN `tilemap = &g_Tilemap` family). If the direct-spelling probe fails to produce $s0, the existing pointer alias is now a last-resort SANCTIONABLE construct needing `/* FAKE */` annotation + documented lever-exhaustion + fresh layer-2 review — plan BOTH paths (clean respelling first, annotation+exhaustion as fallback).

- [s1] [fable-blitz 2026-07-07] No memory/wip/se_data_set/ checkpoint exists; nothing to convert-wip. Current committed body: src/code6cac.c:1492-1523.

- [s3] Flagged construct #1 is CLOSED CLEAN: `D_800A390E = D_800A36A4;` (zero casts) builds an .o whose se_data_set region is register-exact and reloc-exact identical to the HEAD build (diff of objdump -dr, no differences). HEAD is byte-verified on main, so this is a true match, not a masked 0.

- [s3] The &-free subspace floor is 6, measured across 6 forms: direct cast 6, subscript spelling (&X)[i] 6, value-staging locals 9, table-pointer 16, store re-association 17, hoisted-arg block-local temps 24 (95 insns vs target 93).

- [s3] The 6-insn gap is EXACTLY the address-materialization shape and nothing else: HEAD emits `lui/addiu $s0` (= la $s0, D_8010277C) once + `lb 0($s0)` twice; the &-free form re-lui's at each read. Instruction count is 93 in both — the 2-insn la is precisely paid back by the 2 saved luis. Perturbing store order costs 11 more (17), so the gap is not scheduling.

- [s3] CORRECTED CODEGEN RULE (new, measured both directions): GCC 2.7.2/MIPS keeps an address pseudo for a pointer local dereferenced as a PLAIN SCALAR (*p = MEM(reg): 1-insn address, folding the symbol back in would cost 2) but FOLDS AWAY a pointer used as an INDEXED BASE (t[i] = MEM(plus(reg,reg)): MIPS addresses symbol+index natively, so the pointer buys nothing). Proof: `u8 *t = &D_8008E5A8;` produces NO la whatsoever (score 16). The `&` is NOT sufficient — the use shape decides.

- [s3] That rule explains the target bytes end to end: D_8010277C is read as a plain scalar -> cached in callee-save $s0 across func_8005BA8C; D_8008E5A8 is an indexed table base -> re-lui'd at all four of its reads. The asymmetry in the shipped 1998 bytes is the FINGERPRINT of the codegen rule, not of a cheat.

- [s3] The 2026-06-22 audit's core premise ('inconsistency between condition and body is the tell') is empirically false for this function: alias-both = 6, alias-neither = 6, pointer-in-condition-too = 8 (92 insns, one FEWER than target), function-scope decl = 6. The target ITSELF is asymmetric — it caches D_8010277C but not the adjacent D_8010277D, and only inside the block. No consistent spelling can match, so consistency cannot be the completion bar.

- [s3] MATCHED-CORPUS CHECK (Judge-required, now DONE, POSITIVE): func_8003B2C8 (src/code6cac_c_ab.c:431) and func_8003B328 (:444) are COMPLETED-C (absent from engine/queue.json) and both use `u8 *p = &D_8010277C;` — the identical construct on the identical global. func_800656EC (src/text1b.c:14780) is COMPLETED-C and its asm shows the exact lui/addiu-$s0 + loads-through-0($s0)-across-two-jal shape. 16 COMPLETED-C functions show that shape in total.

- [s3] Candidate cleanup: the pointer is now `u8 *p = &D_8010277C;` (type-correct for the u8 global) with the (s8) sign cast at the use sites, spelled exactly as the condition spells it — which answers the 'signed re-view through an lvalue' half of the audit's objection independently of the address-cache question.

- [s3] src/ was left CLEAN at HEAD (the byte-correct form stays on main per the regression policy — a clean replacement has not been routed yet). No engine/rules/pipeline files touched; no commits.
