# func_8002F770 -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C + Q38, 803d0fea1)

The d052c5ee7 landing FAILed the 2026-09-29 retro-audit: owner_cluster_grants row operator-added (974ce052c) with no recorded owner instruction (the file's own :73-77 correction says so), and the islands do not qualify under the 2026-09-26 inline_o.h class grant (audit-q38); secondary: unannotated det/sum multi-role reuse. Per owner Q37 class C + Q38 the body went back to `INCLUDE_ASM("asm/funcs", func_8002F770);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt, tools/canonical_asm_regions.json.
