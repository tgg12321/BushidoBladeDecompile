# func_8001C8DC -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 025f88d91 landing FAILed the 2026-09-29 retro-audit: cross-symbol arithmetic: `p = &D_800A37D2; p[t != 0]++` reaches the separately declared D_800A37D3 (refused 2026-07-20) and is an unannotated pointer alias to a global. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_8001C8DC);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Also restored `INCLUDE_RODATA("asm/rodata", jtbl_800100C4);` (the landing had replaced it with the compiler-emitted table plus the `const u32 D_800100E0[1]` tail word, removed here). The landing's file-scope externs stay: func_80040510/func_80041BF4 are used by later code; `extern u8 *D_800A3894;` is now unused in code6cac.c.
