# func_8006CFBC -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 5c79ba8f1 landing FAILed the 2026-09-29 retro-audit: union word view `Counts_8006CFBC { s32 word; s16 half[2]; }` landed 2026-09-20, before Q33 (2026-09-29); also the 3-write carrier `value` with no FAKE annotation or admitting ruling (score 8 without it). Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_8006CFBC);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Removed with the body: the private Env_8006CFBC / Counts_8006CFBC typedefs (no other user). Kept: the extern block between them and the body (D_800A3524, D_800A34FC, g_gpu_ot_ptr, func_8007352C, func_8006E480, SetDrawMode).
