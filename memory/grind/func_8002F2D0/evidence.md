# func_8002F2D0 -- evidence

## 2026-09-30 -- REOPENED (retro-audit FAIL, Q41, a67c58486)

The b8caedb00 landing FAILed the 2026-09-29 retro-audit: grant row (789ce34d7, 'owner-instructed 2026-09-22') has no recorded owner approval -- the owner answered Q41 'No -- reopen it'; its GTE islands are identical to the three reopened under Q38 (func_8002F770/D780/EBDC), which do not qualify under the 2026-09-26 inline_o.h class grant. Per owner Q41 the body went back to `INCLUDE_ASM("asm/funcs", func_8002F2D0);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-30.c`. Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt; tools/canonical_asm_regions.json entry deleted (its 5 hashes also appear in other live functions' own lists, which are untouched).

## 2026-09-30 — laneB: verbatim inline_o.h islands, both do-while(0) wraps retired -> 0 (candidate.c)
Islands respelled statement-for-statement from inline_o.h (probes-0930/convert.py): gte_Lzc(sum,
&sp_tmp) = ldlzc + nop x2 + stlzc; gte_SetRotMatrix(mat) (:272-284); gte_ldlv0(vec) (:95-103) +
gte_rtv0() (:426-430, post-DMPSX word 0x4A486012); gte_stlvnl(vec). All "$12"-"$15","memory".
Measured with probes-0930/score_nostrip.py (strip skipped: SetRotMatrix/ldlv0 not yet PINNED):
| variant | score |
|---|---|
| islands verbatim, both wraps kept (rejected/inline-o-h-with-both-wraps-8.c) | 8 (s3/s4 seat swap) |
| wrap around `i2 = c2 / det` removed | 0/270 |
| wrap around SetRotMatrix removed only | 8 |
| **both wraps removed = candidate.c** | **0/270** |
| + `det` split (fresh `dist` for the sqrt result; rejected/inline-o-h-fresh-dist-35.c) | 35 (269 insns) |
| + `sum` not reused for the table byte (rejected/inline-o-h-table-byte-inline-6.c) | 6 |
Remaining construct needing paperwork: `det` (determinant, then the sqrt result) and `sum`
(sum of squares, then the table byte) are multi-value locals — Ruling 11 package (dumps,
honest generic names, annotation) or a Q51 citation owed before landing. Plus the PINNED
entries and the per-function DMPSX-word grant, as func_8002EBDC.

### 2026-09-30 — inline_c.h gte_rtv0 alternative checked (orchestrator route 1)
inline_c.h 4.3 :499-502 writes gte_rtv0 as ONE statement (`"nop;" "nop;" ".word 0x0000013f"`,
no clobber list); inline_o.h :426-430 writes three statements each clobbering
"$12"-"$15","memory". Swapping only the rtv0 run for the inline_c.h form (post-DMPSX word under
the 2026-09-24 Extension) still scores 0 (candidate_alt_rtv0_inline_c_h.c, score_nostrip /
sandbox). Not proposed: it mixes two Sony headers in one function, and the two headers define
the same macro names, so one original TU could not have included both. The DMPSX-word row
question is filed for the owner: docs/grind/borderline.md 2026-09-30 (commit 10626c5b6).

## 2026-09-30 — laneB: Ruling 11 package for det/sum started (r11/)
Mechanisms named from dumps (r11/proof.md, r11/dumps_table.txt): work/det = local-alloc.c:472 admission (split -> block-0 pseudo local-allocated to $v0; reuse -> global, $t2 as target); temp/sum = global.c set_preference (:1484, :1671-1760) giving temp's allocno the $a0 preference only when it is the `<< 16` source (split -> $s0). Renamed/annotated body: candidate_r11.c (0). Per-value spellings: 41 / 41 / 41; singles 6 and 35. Permuter: 37,248 iterations, best 30 (re-creates the reuse). Status: banked, not submitted.

## 2026-10-01 — laneA: landing prep on main — Ruling 11 package completed on the landing body
candidate.c is now the landing body (0/270): laneB's renamed `work`/`temp` + annotations (was
candidate_r11.c, removed), plus three byte-neutral changes: the normalized cofactors in fresh `i0`/`i1`
like `i2` (was in-place `c0 = c0 / det; c1 = c1 / det;`, which made c0/c1 hold two values each);
sqrt-table reads spelled `(&g_sqrt_table_u8)[i]`; island comments as func_8002EBDC (gte_Lzc unit name,
gte_rtv0 placeholder + post-DMPSX word). r11/proof.md rewritten on this chassis: the per-value family
re-derived (r11/variants_landing/: 41 x5, work alone 35, temp alone 6) and re-dumped
(r11/dumps_table_landing.txt: same two decisions, sum = pseudo 91 here); first-chassis extras banked
(variants/pv_cdiv.c, pv_detblock.c, pv_detblock_cdiv.c: 41 each). Fresh-seed permuter on the landing
per-value body (12,583 iterations, best 30): every low find re-creates a variable holding the
determinant plus a second value. `d0` (the m00 cofactor-expansion term, computed right after c0, where
the target multiplies it) inlined into the determinant = 53 (rejected/d0-inlined-into-determinant-53.c);
kept as the ordinary-C named term the previous landing carried.

## 2026-10-01 — LANDED (laneA): COMPLETED-INLINE-ASM-CANONICAL
Layer-2 cheat-reviewer rev-2f2d0, round 1, fresh PASS on body_hash ed550499206d26fb (= candidate.c @ 689cd5281;
layer2.jsonl). Key findings: 0/270 + oracle, 31/31 region hashes, work/temp meet Ruling 11 (A)-(H) with dumps
matching the target seats (41/6 re-measured), i0/i1 and d0 ordinary C (d0 = the term the target computes at
0x8002F38C), parameter casts truthful views, islands + Q61 word 0x4A486012 decoded, retro-audit objection resolved.
Commits: auth 3f97afb08, Match d606763ed, queue c800ccfe7; check_completion_integrity OK.
