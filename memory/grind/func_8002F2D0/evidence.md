# func_8002F2D0 -- evidence

## 2026-09-30 -- REOPENED (retro-audit FAIL, Q41, a67c58486)

The b8caedb00 landing FAILed the 2026-09-29 retro-audit: grant row (789ce34d7, 'owner-instructed 2026-09-22') has no recorded owner approval -- the owner answered Q41 'No -- reopen it'; its GTE islands are identical to the three reopened under Q38 (func_8002F770/D780/EBDC), which do not qualify under the 2026-09-26 inline_o.h class grant. Per owner Q41 the body went back to `INCLUDE_ASM("asm/funcs", func_8002F2D0);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-30.c`. Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt; tools/canonical_asm_regions.json entry deleted (its 5 hashes also appear in other live functions' own lists, which are untouched).
