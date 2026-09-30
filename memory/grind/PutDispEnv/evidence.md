# PutDispEnv — evidence (manual session 2026-09-23)

COMPLETED-C in de62daf26 (layer-2 cheat-reviewer PASS; oracle SHA1 match).

- Reference: Xeeynamo/psyz decomp/src/libgpu/sys.c `PutDispEnv` (local copy
  tmp/psyz-ref). Adopted near-verbatim; first measurement scored 28.
- The whole 28 came from binding `&g_gpu_disp_env` to a local pointer: GCC
  CSE'd the base into $s4 (extra save/restore, `lhu N(s4)` instead of
  per-access `%hi/%lo`). Addressing the global directly through a
  `(*(_dispenv *)&g_gpu_disp_env)` view (the reference's `info.disp` is a
  static global) scored 0.
- Residual sandbox hunks are masked reloc addends only (target relocs name
  split D_8009BEE8/…, ours g_gpu_disp_env+off; same address).
- `candidate.c` is the landed body, kept for provenance.

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The de62daf26 landing FAILed the 2026-09-29 retro-audit: macro-hidden `(volatile _dispenv_rect *)` cast on the non-IRQ RAM global g_gpu_disp_env (DISP_RECT_EQ), plus the `(*(_dispenv *)&g_gpu_disp_env)` per-use pun; 'Sony semantics' came from psyz, a decompilation. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", PutDispEnv);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Removed with the body: the PutDispEnv-private _dispenv_rect/_dispenv typedefs and the DISP_RECT_EQ/CLAMP/info macros (+ their #undefs); no other code uses them. Kept: the four extern/prototype lines the landing added (D_8009BE77, D_80015FF8, GetVideoMode, get_dx), now unused before get_dx's definition.
