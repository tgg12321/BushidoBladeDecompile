# func_8002F770 -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C + Q38, 803d0fea1)

The d052c5ee7 landing FAILed the 2026-09-29 retro-audit: owner_cluster_grants row operator-added (974ce052c) with no recorded owner instruction (the file's own :73-77 correction says so), and the islands do not qualify under the 2026-09-26 inline_o.h class grant (audit-q38); secondary: unannotated det/sum multi-role reuse. Per owner Q37 class C + Q38 the body went back to `INCLUDE_ASM("asm/funcs", func_8002F770);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt, tools/canonical_asm_regions.json.

## 2026-09-30 — laneB: verbatim inline_o.h islands, both do-while(0) wraps retired -> 0 (candidate.c)
Islands respelled statement-for-statement from inline_o.h (probes-0930/convert.py): gte_Lzc(sum,
&sp_tmp) = ldlzc + nop x2 + stlzc; gte_SetRotMatrix(mat) (:272-284); gte_ldlv0(vec) (:95-103) +
gte_rtv0() (:426-430, post-DMPSX word 0x4A486012); gte_stlvnl(vec). All "$12"-"$15","memory".
Measured with probes-0930/score_nostrip.py (strip skipped: SetRotMatrix/ldlv0 not yet PINNED):
| variant | score |
|---|---|
| islands verbatim, wraps kept | 0/298 |
| either wrap or both removed | 0/298 |
| **both wraps removed + `m00` intermediate inlined = candidate.c** | **0/298** |
| + `det` split (fresh `dist`; rejected/inline-o-h-fresh-dist-35.c) | 35 (297 insns) |
| + `sum` not reused for the table byte (rejected/inline-o-h-table-byte-inline-6.c) | 6 |
Remaining construct needing paperwork (the retro-audit's secondary objection): `det` and `sum`
multi-value reuse — Ruling 11 package or Q51 citation owed. Plus PINNED entries + DMPSX grant.

### 2026-09-30 — inline_c.h gte_rtv0 alternative checked (orchestrator route 1)
inline_c.h 4.3 :499-502 writes gte_rtv0 as ONE statement (`"nop;" "nop;" ".word 0x0000013f"`,
no clobber list); inline_o.h :426-430 writes three statements each clobbering
"$12"-"$15","memory". Swapping only the rtv0 run for the inline_c.h form (post-DMPSX word under
the 2026-09-24 Extension) still scores 0 (candidate_alt_rtv0_inline_c_h.c, score_nostrip /
sandbox). Not proposed: it mixes two Sony headers in one function, and the two headers define
the same macro names, so one original TU could not have included both. The DMPSX-word row
question is filed for the owner: docs/grind/borderline.md 2026-09-30 (commit 10626c5b6).
