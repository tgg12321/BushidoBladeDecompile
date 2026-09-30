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
