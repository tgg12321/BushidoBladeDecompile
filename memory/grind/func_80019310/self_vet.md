# SELF-VET — func_80019310

CONSTRUCTS: (1) four `__asm__ volatile` GTE islands reproducing PsyQ SDK macro bodies — gte_SetRotMatrix, gte_SetTransMatrix, gte_ldv0 (lwc2 form) + 2 load-delay nops, cop2 MVMVA `.word 0x4A480012`, gte_stlvnl — each in the `move $12, %0` / hardcoded `$12-$15` macro-body spelling with `"$12".."$15"` clobbers; (2) a `"memory"` clobber on every island's clobber list (ADDED by us on islands 1-3, and the macro's own published list on island 4 only — see the clobber-provenance block below; island 1's is byte-load-bearing: it forces the 0x8001934C reload of the MATRIX pointer; islands 1-3 do read memory through $12 and island 4 writes out[] in fact); (3) a new prototype `void func_80019310(s16 *arg0, s32 *arg1);` at src/code6cac.c:314 (the caller predates the definition); (4) the ordinary C: `s32 out[3]` local read back after gte_stlvnl, `for (i = 0; i < ((s16 *)arg1)[2]; i++)` loop over `(s32 *)(arg1[0] + i * 8)`, six `dst[k]` stores with `dst = (s32 *)((u8 *)dst + 0x40)`, and the struct copy `*(MATRIX *)(arg1 + 5) = **(MATRIX **)(arg0 + 2)`. Zero register pins, zero free-standing GPR asm, zero empty-asm barriers, zero volatile, zero dead locals/stores, zero `/* FAKE */` constructs, zero regfix/asmfix rules (rules_dropped 0).

## T1 semantic purpose:
(1) The islands ARE the function's GTE work — the 14 cop2 insns have no C form (GCC 2.7.2's MIPS backend emits no cop2 mnemonics). (2) The "memory" clobber tells GCC the islands touch memory: island 3 reads the SVECTOR through $12, island 4 writes out[0..2] which the C reads back (without it GCC has no reason to order/keep those reads honestly); on island 1 its observable effect is the second read of the MATRIX pointer before gte_SetTransMatrix — a real emitted load the target contains. (3) The prototype only types the existing call. (4) Every C statement is program logic present in the bytes.

## T2 human-programmer:
A PsyQ programmer writes exactly `gte_SetRotMatrix(m); gte_SetTransMatrix(m); for (...) { gte_ldv0(v); gte_rtv0(); gte_stlvnl(out); dst->... = out.. << 7; ... } desc->mat = *m;` — the islands are the SDK macros spelled out (the repo's accepted func_800203B4 form), and the uniform clobber list across all four islands is the shape the committed same-file precedent func_8002D320 (src/code6cac_b.c:935) already ships: a truthful `"$12", "memory"` on a cop2 read island, applied per island rather than tuned to the one island where it changes bytes. The clobber list is NOT claimed to be SDK text on islands 1-3 — the in-source comments say plainly that it is added, and cite the precedent. Reading the pointer again after a macro that declares it clobbers memory is the compiler's business, not something the reader would question.

## T3 GCC-internals justification:
The candidate is explained by program logic + the SDK macro text. GCC internals (cse.c invalidate_memory on a `(clobber (mem:BLK (scratch)))`) are cited in evidence.md H2 only to explain WHY the target's reload exists, i.e. as evidence the original macros carried a memory clobber — not as a lever chosen to steer a pass. Clobber provenance is now measured, not assumed: the Sony `inline_c.h` IS on disk at `tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_c.h` (the path the 2026-09-02 13:41 func_800325E0 ruling cited) and I read it this session. Only `gte_stlvnl` publishes "memory" (inline_c.h:1116); `gte_SetRotMatrix` (:310) and `gte_SetTransMatrix` (:369) publish `"$12","$13","$14"` only, and `gte_ldv0` (:16-20) publishes no clobber list. The "memory" on islands 1-3 is therefore ADDED, attributed in-source to func_8002D320 (src/code6cac_b.c:935) and accepted-in-precedent by func_800300B4's Judge PASS (docs/grind/decisions.md:20650) — not to SDK text. If the reviewer/Judge holds that the clobber list is outside the condition-3 template, the correct disposition is a ruling-request on that exact question, not a respelling (volatile-param / cast reload spellings are the banned forms of the same reload and were NOT tried).

## T4 permuter/search provenance:
No permuter, no search. Hand-written from the asm in six measured variants (v1..v6, tmp/grind/func_80019310/s1/); the only choices made were the local's size (frame 24) and the clobber list.

## T5 family check:
Not a coercion family. The islands fall under the COP2 addressing-preamble cluster canonical-asm path (owner grant 2026-09-01, widened anchor) — func_80019310 is named in that grant as a confirmed handwritten-tagged carrier; bucket is COMPLETED-INLINE-ASM-CANONICAL, never COMPLETED-C. The "memory" clobber matches the func_800300B4 Judge-PASSed island form (truthful clobber, same-file precedent code6cac_b.c:1708) and func_8002D320's islands (code6cac_b.c:935/942). It is NOT an `__asm__ volatile("" ::: "memory")` empty barrier (there is no empty asm anywhere), and no forbidden-catalog row (pins, hardcoded-$N injection outside a granted island, volatile coercion, dead locals, alias renames) is present.

## T6 naming-announces-intent:
Locals are `out`, `i`, `dst`; params `arg0`/`arg1` (splat defaults). No pad/dummy/spill/tail names.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: COP2 addressing-preamble cluster — canonical GTE macro islands (widened anchor grant 2026-09-01; condition 3 = the SDK macro body)
  SCOPE: "membership closes the TAIL ISLAND only, when the function's pure-C body independently reaches sandbox 0 under the normal grind gates."
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:154`
  PRECEDENT: `docs/grind/decisions.md:18119`
  PRECEDENT: `inline_asm_canonical.txt:367`
  PRECEDENT: `tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_c.h:297`
  PRECEDENT: `tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_c.h:1111`
  FAMILY: truthful "memory" clobber on a granted GTE island (Judge PASS precedent, func_800300B4)
  SCOPE: "island 4 adds a truthful \"memory\" clobber (the asm writes mac[], which the following C reads) with same-file precedent at src/code6cac_b.c:1708."
  PRECEDENT: `docs/grind/decisions.md:20650`
  PRECEDENT: `src/code6cac_b.c:935`

ANNOTATION-CONFORMANCE: n/a — no /* FAKE */ construct is present in the diff (no coercion family is claimed; the islands are canonical GTE asm under the 2026-09-01 widened-anchor grant, and the C is ordinary program logic). The annotation obligation that DOES apply here is condition 3 of the cop2 cluster rule as clarified by owner Ruling A 2026-09-02 — every island comment must cite its Sony macro NAME **and** its inline_c.h LINE RANGE — and this session's comments-only fix-up satisfies it exactly:
  - `/* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310. ... The "memory" clobber is ADDED, not SDK text -- precedent src/code6cac_b.c:935 (func_8002D320). */`
  - `/* PsyQ libgte inline macro gte_SetTransMatrix(r0) --- inline_c.h:360-369. ... The "memory" clobber is ADDED, not SDK text -- precedent src/code6cac_b.c:935 (func_8002D320). */`
  - `/* PsyQ libgte inline macro gte_ldv0(r0) --- inline_c.h:16-20. ... the macro publishes NO clobber list at all, so both the "$12" and the "memory" clobbers here are ADDED, not SDK text -- precedent src/code6cac_b.c:935 (func_8002D320) ... */`
  - `/* Sony libgte macro gte_mvmva(sf,mx,v,cv,lm) --- inline_c.h:816-817, whose body is gte_mvmva_core(r0) at inline_c.h:809-814 ... Our instance is gte_mvmva(1,0,0,0,0) ... spelled as a bare `.word 0x4A480012` ... */`
  - `/* PsyQ libgte inline macro gte_stlvnl(r0) --- inline_c.h:1111-1117. Stores MAC1/MAC2/MAC3. This is the ONE island whose "memory" clobber IS the macro's own published clobber list (inline_c.h:1116); "$12" is added with the preamble. */`
The function header comment carries the same five name+line-range cites in a table, plus a dedicated CLOBBER PROVENANCE paragraph attributing the three added "memory" clobbers to src/code6cac_b.c:935 rather than to SDK text, which is the exact defect the 2026-09-06 05:55 Judge final call named (docs/grind/decisions.md:24662).

SESSION SCOPE (annotation-fix modality, 2026-09-06): the diff against the judge-failed body (memory/grind/func_80019310/rejected/judge-fail-0906-0554.c) changes COMMENT LINES ONLY — verified mechanically with `diff` filtered to non-comment lines (empty result). No construct added, removed, renamed or reordered. Re-measured after the re-cite: `sandbox func_80019310 --disable all` = score 0, 81/81, rules_dropped 0.

RE-VET (structural-modality session, 2026-09-06, post-registry-row): the body under review is UNCHANGED
from the one this vet was written against — memory/grind/func_80019310/candidate.c was not edited this
session (no construct added, removed, renamed, reordered; no comment change), so it is still the
Judge-cleared body df4741401a310eb2 (clearance: docs/grind/decisions.md 2026-09-06 06:05 ruling PASS).
Every answer above (T1-T6, both family claims, the annotation-conformance list) applies verbatim. The
only thing that changed is OUTSIDE the diff: the operator added the owner-cluster registry row
(tools/grinder/owner_cluster_grants.txt:30, commit 2cef233c), which supplies the grant door that the
2026-09-06 merge refusal named as the sole remaining blocker. Re-measured this session on HEAD 0bb257ca:
`sandbox func_80019310 --disable all` = score 0, 81/81, rules_dropped 0, cheat_asm_stripped 22
(tmp/grind/func_80019310/s2b/sandbox_final_stripped.json). Zero /* FAKE */ constructs remain the case.
